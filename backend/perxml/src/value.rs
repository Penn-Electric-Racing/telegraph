use serde::{Deserialize, Serialize};

#[derive(Debug, Deserialize, Serialize, PartialEq)]
#[serde(deny_unknown_fields, rename_all = "PascalCase")]
pub struct Value {
    #[serde(rename = "Type")]
    ty: String,
    access_string: String,
    name: String,
    description: String,
    uppercase_identifier: Option<String>,
    #[serde(alias = "Units")]
    unit: Option<String>,
    can_frequency: Option<String>,
    default: Option<String>,
    enum_values: Option<String>,
    min: Option<String>,
    max: Option<String>,
    per_dos_frequency: String,
    read_only: bool,
    array_dimensions: Option<u32>,
}
