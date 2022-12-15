use serde::{Deserialize, Serialize};

#[derive(Debug, Deserialize, Serialize, PartialEq)]
#[serde(deny_unknown_fields, rename_all = "PascalCase")]
pub struct Value {
    pub regex: String,
    pub fault_state: Option<bool>,
    pub start_visible: Option<bool>,
}

#[derive(Debug, Deserialize, Serialize, PartialEq)]
#[serde(deny_unknown_fields, rename_all = "PascalCase")]
pub struct Group {
    pub name: String,
    pub orientation: Option<String>,
    pub hideable: Option<bool>,
    pub start_visible: Option<bool>,
    #[serde(rename = "Value")]
    pub values: Vec<Value>,
}

#[derive(Debug, Deserialize, Serialize, PartialEq)]
#[serde(deny_unknown_fields, rename_all = "PascalCase")]
pub struct Device {
    pub name: String,
    pub regex: Option<String>,
    pub orientation: Option<String>,
    pub start_visible: Option<bool>,
    pub hideable: Option<bool>,
    pub identifier: Option<String>,
    #[serde(rename = "Group")]
    pub groups: Vec<Group>,
}

#[derive(Debug, Deserialize, Serialize, PartialEq)]
#[serde(deny_unknown_fields, rename_all = "PascalCase", rename = "Devices")]
pub struct Layouts {
    #[serde(rename = "Device", default)]
    pub devices: Vec<Device>,
}

impl Layouts {
    pub fn read_from<R: std::io::Read>(reader: R) -> eyre::Result<Self> {
        let buf_reader = std::io::BufReader::new(reader);
        let mut devices_deserializer = quick_xml::de::Deserializer::from_reader(buf_reader);
        let out = serde_path_to_error::deserialize(&mut devices_deserializer)?;

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

    const DISPLAY: &'static str = include_str!("../test/rev5 Display.xml");

    #[test]
    fn deserialize_rev5() -> Result<()> {
        let display = Layouts::read_from(DISPLAY.as_bytes())?;
        println!("{:?}", display);

        Ok(())
    }
}
