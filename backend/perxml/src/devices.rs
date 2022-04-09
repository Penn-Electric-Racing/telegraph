use serde::{Deserialize, Serialize};

#[derive(Debug, Deserialize, Serialize, PartialEq)]
#[serde(deny_unknown_fields)]
pub struct Devices {
    #[serde(rename = "Device", default)]
    pub devices: Vec<Device>,
}

#[derive(Debug, Deserialize, Serialize, PartialEq)]
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
        ty: String,
        default: String,
        enum_values: String,
        description: String,
    },
    #[serde(rename_all = "PascalCase")]
    Variable {
        name: String,
        identifier: String,
        #[serde(rename = "Type")]
        ty: String,
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

#[derive(Debug, Deserialize, Serialize, PartialEq)]
#[serde(deny_unknown_fields, rename_all = "PascalCase")]
pub struct Device {
    pub name: String,
    pub identifier: String,
    pub uppercase_identifier: Option<String>,
    pub description: Option<String>,
    #[serde(rename = "ID")]
    pub id: String,
    pub platform: String,
    pub include: String,
    pub read_only: String,
    pub update_frequency: String,
    pub per_dos_frequency: Option<String>,
    pub per_dos_uart: String,
    pub can_frequency: Option<String>,
    pub can_configuration: String,
    pub update: String,
    pub namespace: String,
    pub using: Option<String>,
    pub can_handlers: Option<bool>,
    pub can_stagger: Option<bool>,
    #[serde(default, rename = "$value")]
    pub variable_tree: Vec<VariableNode>,
}

#[cfg(test)]
mod tests {
    use super::*;
    use eyre::Result;
    use quick_xml::de::Deserializer;
    use serde_path_to_error;

    const REV5_XML: &'static str = include_str!("../test/rev5.xml");

    #[test]
    fn deserialize_rev5() -> Result<()> {
        let mut devices_deserializer = Deserializer::from_reader(REV5_XML.as_bytes());
        // to_string(&devices).unwrap()
        let devices: Devices = serde_path_to_error::deserialize(&mut devices_deserializer)?;
        println!("{:?}", devices);

        assert_eq!(devices.devices.len(), 6);

        Ok(())
    }
}
