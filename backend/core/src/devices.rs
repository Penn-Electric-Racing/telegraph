use std::hash::{Hash, Hasher};

use serde::{de, ser, Deserialize, Deserializer, Serialize, Serializer};
use std::collections::HashMap;

use super::value::Type;

#[derive(Debug, Deserialize, Serialize, PartialEq, Eq, PartialOrd, Ord, Hash)]
#[serde(deny_unknown_fields)]
pub enum VariableNode {
    #[serde(rename_all = "PascalCase")]
    Enum {
        name: String,
        identifier: String,
        uppercase_identifier: Option<String>,
        can_frequency: Option<String>,
        per_dos_frequency: Option<String>,
        #[serde(rename = "Type")]
        ty: Type,
        default: String,
        enum_values: String,
        description: String,
        min: Option<u32>,
        max: Option<u32>,
    },
    #[serde(rename_all = "PascalCase")]
    Variable {
        name: String,
        identifier: String,
        #[serde(rename = "Type")]
        ty: Type,
        description: String,
        can_frequency: Option<String>,
        per_dos_frequency: Option<String>,
        #[serde(alias = "Unit")]
        units: Option<String>,
        #[serde(alias = "DefaultValue")]
        default: Option<String>,
        array_dimensions: Option<u32>,
        min: Option<String>,
        max: Option<String>,
        read_only: Option<bool>,
    },
    #[serde(rename_all = "PascalCase")]
    Struct {
        name: String,
        identifier: String,
        uppercase_identifier: Option<String>,
        description: Option<String>,
        can_frequency: Option<String>,
        per_dos_frequency: Option<String>,
        read_only: Option<bool>,
        array_dimensions: Option<u32>,
        #[serde(default, rename = "$value")]
        variable_tree: Vec<VariableNode>,
    },
    #[serde(rename_all = "PascalCase")]
    Repeat {
        name: String,
        identifier: String,
        uppercase_identifier: Option<String>,
        description: Option<String>,
        can_frequency: Option<String>,
        per_dos_frequency: Option<String>,
        prefixes: Option<String>,
        #[serde(default, rename = "$value")]
        variable_tree: Vec<VariableNode>,
    },
}

#[derive(Debug, Deserialize, Serialize, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum Platform {
    STM32,
    External,
}

#[derive(Debug, Deserialize, Serialize, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum UpdateType {
    ManualUpdate,
    ManualFunctions,
}

#[derive(Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct PinBaud {
    pub baud: u32,
    pub rx_pin: String,
    pub tx_pin: String,
}

#[derive(Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct UartConfig(Vec<PinBaud>);

impl UartConfig {
    pub fn is_empty(&self) -> bool {
        self.0.len() == 0
    }

    pub fn num_devices(&self) -> usize {
        self.0.len()
    }

    pub fn configs(&self) -> &Vec<PinBaud> {
        &self.0
    }
}

impl<'de> Deserialize<'de> for UartConfig {
    fn deserialize<D>(deserializer: D) -> Result<Self, D::Error>
    where
        D: Deserializer<'de>,
    {
        let config_str = String::deserialize(deserializer)?;

        let configs = config_str.split(',').map(|pin_and_baud: &str| {
            let parts = pin_and_baud.trim().split('|').collect::<Vec<_>>();
            if parts.len() != 3 {
                return Err(serde::de::Error::custom("Invalid pin and baud specification. Expected format: 115200|PA9|PB2, 115200|PC4|PD3"))
            }
            let baud = u32::from_str_radix(parts[0], 10).map_err(|e| de::Error::custom(e))?;
            Ok(PinBaud {
                baud,
                rx_pin: parts[1].to_owned(),
                tx_pin: parts[2].to_owned(),
            })
        }).collect::<Result<Vec<_>, _>>()?;
        Ok(UartConfig(configs))
    }
}

impl Serialize for UartConfig {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: Serializer,
    {
        let config_str = (&self.0)
            .into_iter()
            .map(|pb| format!("{}|{}|{}", pb.baud, pb.rx_pin, pb.tx_pin))
            .collect::<Vec<_>>()
            .join(", ");

        config_str.serialize(serializer)
    }
}

#[derive(Debug, PartialEq, Eq)]
pub struct CanConfig(HashMap<String, PinBaud>);

impl CanConfig {
    pub fn is_empty(&self) -> bool {
        self.0.len() == 0
    }

    pub fn num_devices(&self) -> usize {
        self.0.len()
    }

    pub fn configs(&self) -> &HashMap<String, PinBaud> {
        &self.0
    }
}

impl<'de> Deserialize<'de> for CanConfig {
    fn deserialize<D>(deserializer: D) -> Result<Self, D::Error>
    where
        D: Deserializer<'de>,
    {
        let config_str = String::deserialize(deserializer)?;

        let mut configs = HashMap::new();

        for pin_and_baud in config_str.split(',') {
            let parts = pin_and_baud.trim().split('|').collect::<Vec<_>>();

            if parts.len() != 3 && parts.len() != 4 {
                return Err(serde::de::Error::custom("Invalid pin and baud specification. Expected format: 500000|PA8|PA15|Default, 1000000|PD0|PD1|Moc"));
            }

            let baud = u32::from_str_radix(parts[0], 10).map_err(|e| de::Error::custom(e))?;

            let name = parts.get(3).unwrap_or(&"Default").to_string();
            if configs.get(&name).is_some() {
                return Err(serde::de::Error::custom(
                    "Two CAN busses are configured with the same name",
                ));
            }

            configs.insert(
                name,
                PinBaud {
                    baud,
                    rx_pin: parts[1].to_owned(),
                    tx_pin: parts[2].to_owned(),
                },
            );
        }
        Ok(CanConfig(configs))
    }
}

impl Serialize for CanConfig {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: Serializer,
    {
        let config_str = (&self.0)
            .into_iter()
            .map(|(name, v)| format!("{}|{}|{}|{}", v.baud, v.rx_pin, v.tx_pin, name))
            .collect::<Vec<_>>()
            .join(", ");

        config_str.serialize(serializer)
    }
}

#[derive(Debug, Deserialize, Serialize)]
#[serde(deny_unknown_fields, rename_all = "PascalCase")]
pub struct Device {
    pub name: String,
    pub names: Option<String>,
    pub identifier: String,
    pub uppercase_identifier: Option<String>,
    pub description: Option<String>,
    #[serde(rename = "ID")]
    pub id: u32,
    pub platform: Platform,
    pub include: String,
    pub read_only: String,
    pub update_frequency: f64,
    pub per_dos_frequency: Option<String>,
    pub per_dos_uart: UartConfig,
    pub can_frequency: Option<String>,
    pub can_configuration: CanConfig,
    pub update: UpdateType,
    pub namespace: Option<String>,
    pub using: Option<String>,
    pub can_handlers: Option<bool>,
    pub can_stagger: Option<bool>,
    #[serde(default, rename = "$value")]
    pub variable_tree: Vec<VariableNode>,
}

impl Hash for Device {
    fn hash<H: Hasher>(&self, state: &mut H) {
        self.id.hash(state);
    }
}

impl PartialEq for Device {
    fn eq(&self, other: &Self) -> bool {
        self.id == other.id
    }
}
impl Eq for Device {}

#[derive(Debug, Deserialize, Serialize, PartialEq, Eq, Hash)]
#[serde(deny_unknown_fields)]
pub struct Devices {
    #[serde(rename = "Device", default)]
    pub devices: Vec<Device>,
}

impl Devices {
    pub fn read_from<R: std::io::Read>(reader: R) -> eyre::Result<Self> {
        let buf_reader = std::io::BufReader::new(reader);
        let mut devices_deserializer = quick_xml::de::Deserializer::from_reader(buf_reader);

        // TODO: I want the path to the error to be in terms of the variable
        // access string, not whatever serde_path_to_error uses
        let out: Devices = serde_path_to_error::deserialize(&mut devices_deserializer)?;

        // TODO: there are some validations that need to happen here.

        // List<string> duplicateValues = configuration.Devices.SelectMany(d =>
        //     d.Values().Select(v => v.DirectAccessString).Duplicates()).ToList();
        // if (duplicateValues.Any())
        // {
        //     Console.WriteLine("Multiple variables have the same name in the same struct: \n" +
        //                         string.Join(", ", duplicateValues));
        //     return 1;
        // }

        // List<uint> duplicateDevices = configuration.Devices.SelectMany(d => d.Ids).Duplicates().ToList();
        // if (duplicateDevices.Any())
        // {
        //     Console.WriteLine($"Multiple devices have the id(s): {string.Join(", ", duplicateDevices)}");
        // }

        Ok(out)
    }

    pub fn write_to<W: std::io::Write>(&self, writer: W) -> eyre::Result<()> {
        quick_xml::se::to_writer(writer, self)?;

        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use eyre::Result;

    const REV5_XML: &'static str = include_str!("../test/rev5.xml");

    #[test]
    fn deserialize_rev5() -> Result<()> {
        let devices = Devices::read_from(REV5_XML.as_bytes())?;
        println!("{:?}", devices);

        assert_eq!(devices.devices.len(), 6);

        Ok(())
    }
}
