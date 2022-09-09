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

impl AssignedCanIds {
    pub fn read_from<R: std::io::Read>(reader: R) -> eyre::Result<Self> {
        let buf_reader = std::io::BufReader::new(reader);
        let mut devices_deserializer = quick_xml::de::Deserializer::from_reader(buf_reader);
        let out: AssignedCanIds = serde_path_to_error::deserialize(&mut devices_deserializer)?;

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

    const ASSIGNED_CAN_IDS: &'static str = include_str!("../test/AssignedCanIds.xml");

    #[test]
    fn deserialize_rev5() -> Result<()> {
        let assigned_can_ids = AssignedCanIds::read_from(ASSIGNED_CAN_IDS.as_bytes())?;
        println!("{:?}", assigned_can_ids);

        Ok(())
    }
}
