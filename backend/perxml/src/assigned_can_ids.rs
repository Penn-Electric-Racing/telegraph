use crate::value::Value;
use serde::{Deserialize, Serialize};

#[derive(Debug, Deserialize, Serialize, PartialEq)]
#[serde(deny_unknown_fields, rename_all = "PascalCase")]
pub struct CanId {
    pub id: u32,
    pub device_id: u32,
    pub can_frequency: String,
    pub creation_time: String,
    #[serde(rename = "Value")]
    pub values: Vec<Value>,
}

#[derive(Debug, Deserialize, Serialize, PartialEq)]
#[serde(deny_unknown_fields, rename_all = "PascalCase", rename = "PerDos")]
pub struct AssignedCanIds {
    #[serde(rename = "CanId", default)]
    pub ids: Vec<CanId>,
}

use quick_xml::de::Deserializer;
use serde_path_to_error;
use eyre::Result;

impl AssignedCanIds {
    pub fn parse(s: String) -> Result<Self> {
        let mut assigned_can_ids_deserializer =
            Deserializer::from_reader(s.as_bytes());
        let assigned_can_ids: AssignedCanIds =
            serde_path_to_error::deserialize(&mut assigned_can_ids_deserializer)?;
        Ok(assigned_can_ids)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use quick_xml::de::Deserializer;
    use serde_path_to_error;

    const ASSIGNED_CAN_IDS: &'static str = include_str!("../test/AssignedCanIds.xml");

    #[test]
    fn deserialize_rev5() -> Result<()> {
        let mut assigned_can_ids_deserializer =
            Deserializer::from_reader(ASSIGNED_CAN_IDS.as_bytes());
        let assigned_can_ids: AssignedCanIds =
            serde_path_to_error::deserialize(&mut assigned_can_ids_deserializer)?;
        println!("{:?}", assigned_can_ids);

        Ok(())
    }
}
