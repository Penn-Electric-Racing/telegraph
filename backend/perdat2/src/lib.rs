pub mod fletcher32;
mod util;
pub mod values;
pub mod varint;

pub use values::*;
pub use varint::perdat_int;

/// See https://confluence.pennelectricracing.com/display/SOFT/.PERDAT2 for the
/// documentation of the format.
pub struct Packet<'a> {
    pub data_length: u8,
    pub data: &'a [u8],
    pub checksum: u32,
}

// i(id) type(data type) L(ticks between data (0 to use timestamps)) s(name) s(access string) s(description) s(units)
pub struct CAddChannel {
    pub id: perdat_int,
    pub ty: Type,
    /// Set to 0 to use timestamps
    pub ticks_between_data: i64,
    pub name: String,
    pub access_string: String,
    pub description: String,
    pub units: String,
}

// i(property id) s(property) type(data type) s(description) s(units)
pub struct CRegisterProperty {
    pub id: perdat_int,
    pub property: String,
    pub ty: Type,
    pub description: String,
    pub units: String,
}

// i(property id) i(related channel/command id (0 for global)) t(value) s(description) s(units)
pub struct CSetProperty {
    pub id: perdat_int,
    /// Related channel or command ID; set to 0 for global
    pub related_id: perdat_int,

    // TODO: to parse this, I think we need to already know what type the property is.
    pub value: Value,
}

// L(time)
pub struct CSetTime {
    pub timestamp: i64,
}

// L(time)
pub struct CAddTime {
    pub offset: i64,
}

// i(time)
pub struct CAddShorterTime {
    pub offset: perdat_int,
}

pub enum Command {
    End,
    AddChannel(CAddChannel),
    RegisterProperty(CRegisterProperty),
    SetProperty(CSetProperty),
    SetTime(CSetTime),
    AddTime(CAddTime),
    AddShorterTime(CAddShorterTime),
    IncrementTick,
    PushTime,
    PopTime,
    Padding,
    Reserved,
}
