use crate::{devices::Device, value::Value};
use serde::{Deserialize, Serialize};

#[derive(Debug, Deserialize, Serialize, PartialEq)]
#[serde(deny_unknown_fields, rename_all = "PascalCase")]
pub struct PerDosId {
    id: u32,
    #[serde(rename = "Type")]
    ty: String,
    access_string: String,
    creation_time: String,
    devices: String,
    value: Value,
}

#[derive(Debug, Deserialize, Serialize, PartialEq)]
#[serde(deny_unknown_fields, rename_all = "PascalCase", rename = "PerDos")]
pub struct AssignedPerDosIds {
    #[serde(rename = "Device", default)]
    pub devices: Vec<Device>,
    #[serde(rename = "PerDosId", default)]
    pub ids: Vec<PerDosId>,
}

#[cfg(test)]
mod tests {
    use super::*;
    use eyre::Result;
    use quick_xml::de::Deserializer;
    use serde_path_to_error;

    const ASSIGNED_PERDOS_IDS: &'static str = include_str!("../test/AssignedPerDosIds.xml");

    #[test]
    fn deserialize_rev5() -> Result<()> {
        let mut assigned_perdos_ids_deserializer =
            Deserializer::from_reader(ASSIGNED_PERDOS_IDS.as_bytes());
        // to_string(&devices).unwrap()
        let assigned_perdos_ids: AssignedPerDosIds =
            serde_path_to_error::deserialize(&mut assigned_perdos_ids_deserializer)?;
        println!("{:?}", assigned_perdos_ids);

        assert_eq!(assigned_perdos_ids.devices.len(), 6);

        Ok(())
    }
}
