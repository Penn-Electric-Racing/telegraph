use serde::{Deserialize, Serialize};

#[derive(Debug, Deserialize, Serialize, PartialEq)]
#[serde(deny_unknown_fields, rename_all = "PascalCase")]
pub struct Value {
    #[serde(rename = "Type")]
    pub ty: String,
    pub access_string: String,
    pub name: String,
    pub description: String,
    pub uppercase_identifier: Option<String>,
    #[serde(alias = "Units")]
    pub unit: Option<String>,
    pub can_frequency: Option<String>,
    pub default: Option<String>,
    pub enum_values: Option<String>,
    pub min: Option<String>,
    pub max: Option<String>,
    pub per_dos_frequency: String,
    pub read_only: bool,
    pub array_dimensions: Option<u32>,
}
