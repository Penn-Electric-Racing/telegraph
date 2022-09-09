use std::collections::HashMap;
use std::hash::{Hash, Hasher};

use serde::{de, Deserialize, Deserializer, Serialize, Serializer};

#[derive(Debug, Deserialize, Serialize, PartialEq, Eq, PartialOrd, Ord, Hash, Clone)]
#[serde(rename_all = "lowercase")]
pub enum Type {
    Bool,
    UInt8,
    UInt16,
    UInt32,
    UInt64,
    Int8,
    Int16,
    Int32,
    Int64,
    Float,
    Double,
}

impl Type {
    pub fn size(&self) -> usize {
        match self {
            Type::Bool | Type::UInt8 | Type::Int8 => 1,
            Type::UInt16 | Type::Int16 => 2,
            Type::UInt32 | Type::Int32 | Type::Float => 4,
            Type::UInt64 | Type::Int64 | Type::Double => 8,
        }
    }
}

#[derive(Debug)]
pub struct CanFrequencySettings(HashMap<String, f64>);

impl CanFrequencySettings {
    pub fn max(&self) -> Option<f64> {
        self.0.values().fold(None, |acc: Option<f64>, v| {
            if let Some(f) = acc {
                Some(f64::max(*v, f))
            } else {
                Some(*v)
            }
        })
    }
}

impl Default for CanFrequencySettings {
    fn default() -> Self {
        CanFrequencySettings(HashMap::new())
    }
}

impl<'de> Deserialize<'de> for CanFrequencySettings {
    fn deserialize<D>(deserializer: D) -> Result<Self, D::Error>
    where
        D: Deserializer<'de>,
    {
        let config_str = String::deserialize(deserializer)?;

        if config_str.trim() == "" {
            return Ok(CanFrequencySettings::default());
        }

        // If we don't have a , or a :, we only have one can frequency, and it
        // implicitly for the Default bus
        if !config_str.contains(",") && !config_str.contains(":") {
            let mut configs = HashMap::new();
            configs.insert(
                "Default".to_owned(),
                config_str
                    .trim()
                    .parse::<f64>()
                    .map_err(|e| de::Error::custom(e))?,
            );
            return Ok(CanFrequencySettings(configs));
        }

        let configs = config_str
            .split(',')
            .map(|bus_and_freq: &str| {
                let parts = bus_and_freq.trim().split(':').collect::<Vec<_>>();
                if parts.len() != 2 {
                    return Err(serde::de::Error::custom(
                        "Invalid CAN bus and frequency specification. Expected format: Default:100",
                    ));
                }

                let name = parts[0].to_owned();
                let frequency = parts[1].parse::<f64>().map_err(|e| de::Error::custom(e))?;
                Ok((name, frequency))
            })
            .collect::<Result<HashMap<_, _>, _>>()?;
        Ok(CanFrequencySettings(configs))
    }
}

impl Serialize for CanFrequencySettings {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: Serializer,
    {
        let config_str = (&self.0)
            .into_iter()
            .map(|(bus, freq)| format!("{bus}:{freq}"))
            .collect::<Vec<_>>()
            .join(", ");

        config_str.serialize(serializer)
    }
}

#[derive(Debug, Deserialize, Serialize)]
#[serde(deny_unknown_fields, rename_all = "PascalCase")]
pub struct Value {
    #[serde(rename = "Type")]
    pub ty: Type,
    pub access_string: String,
    pub name: String,
    pub description: String,
    pub uppercase_identifier: Option<String>,
    #[serde(alias = "Units")]
    pub unit: Option<String>,
    #[serde(default = "CanFrequencySettings::default")]
    pub can_frequency: CanFrequencySettings,
    pub default: Option<String>,
    pub enum_values: Option<String>,
    pub min: Option<String>,
    pub max: Option<String>,
    pub per_dos_frequency: f64,
    pub read_only: bool,
    pub array_dimensions: Option<u32>,
}

impl Value {
    pub fn value_name(&self) -> String {
        self.access_string
            .replace(".", "_")
            .replace("[", "_")
            .replace("]", "")
    }
}

impl Hash for Value {
    fn hash<H: Hasher>(&self, state: &mut H) {
        self.access_string.hash(state);
    }
}

impl PartialEq for Value {
    fn eq(&self, other: &Self) -> bool {
        self.access_string == other.access_string
    }
}
impl Eq for Value {}
