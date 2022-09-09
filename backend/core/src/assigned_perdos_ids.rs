use crate::{devices::Device, value::Value};
use serde::{de, Deserialize, Deserializer, Serialize, Serializer};

#[derive(Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct DeviceIds(Vec<u32>);

impl DeviceIds {
    pub fn contains(&self, d: &Device) -> bool {
        self.0.contains(&d.id)
    }
}

impl<'de> Deserialize<'de> for DeviceIds {
    fn deserialize<D>(deserializer: D) -> Result<Self, D::Error>
    where
        D: Deserializer<'de>,
    {
        let device_id_str = String::deserialize(deserializer)?;

        let ids = device_id_str
            .split(',')
            .map(|id: &str| {
                let id = id.trim();
                u32::from_str_radix(id, 10).map_err(|e| de::Error::custom(e))
            })
            .collect::<Result<Vec<_>, _>>()?;
        Ok(DeviceIds(ids))
    }
}

impl Serialize for DeviceIds {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: Serializer,
    {
        let config_str = (&self.0)
            .into_iter()
            .map(|id| format!("{}", id))
            .collect::<Vec<_>>()
            .join(", ");

        config_str.serialize(serializer)
    }
}

#[derive(Debug, Deserialize, Serialize, PartialEq)]
#[serde(deny_unknown_fields, rename_all = "PascalCase")]
pub struct PerDosId {
    pub id: u32,
    #[serde(rename = "Type")]
    pub ty: String,
    pub access_string: String,
    pub creation_time: String,
    pub devices: DeviceIds,
    pub value: Value,
}

#[derive(Debug, Deserialize, Serialize, PartialEq)]
#[serde(deny_unknown_fields, rename_all = "PascalCase", rename = "PerDos")]
pub struct AssignedPerDosIds {
    #[serde(rename = "Device", default)]
    pub devices: Vec<Device>,
    #[serde(rename = "PerDosId", default)]
    pub ids: Vec<PerDosId>,
}

impl AssignedPerDosIds {
    pub fn read_from<R: std::io::Read>(reader: R) -> eyre::Result<Self> {
        let buf_reader = std::io::BufReader::new(reader);
        let mut devices_deserializer = quick_xml::de::Deserializer::from_reader(buf_reader);
        let out: AssignedPerDosIds = serde_path_to_error::deserialize(&mut devices_deserializer)?;

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

    const ASSIGNED_PERDOS_IDS: &'static str = include_str!("../test/AssignedPerDosIds.xml");

    #[test]
    fn deserialize_rev5() -> Result<()> {
        let assigned_perdos_ids = AssignedPerDosIds::read_from(ASSIGNED_PERDOS_IDS.as_bytes())?;
        println!("{:?}", assigned_perdos_ids);

        assert_eq!(assigned_perdos_ids.devices.len(), 6);

        Ok(())
    }
}
