mod can;
mod perdos;
pub use can::CanCodeGenerator;
pub use perdos::PerDosCodeGenerator;

use std::collections::HashSet;

use per::{devices::Device, value::Value};

trait CodeGenerator {
    fn is_used_by(&self, d: &Device) -> bool;

    fn to_header(&self, d: &Device) -> String;
    fn get_header_includes(&self, d: &Device) -> String;

    fn to_cpp(&self, d: &Device, values: &HashSet<Value>) -> String;
    fn get_cpp_includes(&self, d: &Device) -> String;

    fn to_update(&self, d: &Device, indent: &str) -> String;
}
