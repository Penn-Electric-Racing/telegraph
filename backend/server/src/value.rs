use crate::errors::UnpackError;
use crate::wire;

pub enum Value {
    Invalid,
    None,
    Enum(u8),
    Bool(bool),
    Uint8(u8),
    Uint16(u16),
    Uint32(u32),
    Uint64(u64),
    Int8(i8),
    Int16(i16),
    Int32(i32),
    Int64(i64),
    Float(f32),
    Double(f64),
}

impl Value {
    pub fn pack(&self) -> wire::Value {
        wire::Value {
            r#type: Some(match &self {
                Value::Invalid => wire::value::Type::Invalid(wire::Empty {}),
                Value::None => wire::value::Type::None(wire::Empty {}),
                Value::Enum(e) => wire::value::Type::En(*e as i32),
                Value::Bool(b) => wire::value::Type::B(*b),
                Value::Uint8(u) => wire::value::Type::U8(*u as u32),
                Value::Uint16(u) => wire::value::Type::U16(*u as u32),
                Value::Uint32(u) => wire::value::Type::U32(*u),
                Value::Uint64(u) => wire::value::Type::U64(*u),
                Value::Int8(i) => wire::value::Type::I8(*i as i32),
                Value::Int16(i) => wire::value::Type::I16(*i as i32),
                Value::Int32(i) => wire::value::Type::I32(*i),
                Value::Int64(i) => wire::value::Type::I64(*i),
                Value::Float(f) => wire::value::Type::F(*f),
                Value::Double(d) => wire::value::Type::D(*d),
            }),
        }
    }

    pub fn unpack(value: &wire::Value) -> Result<Self, UnpackError> {
        match value.r#type {
            None => Err(UnpackError::NullFieldError),
            Some(wire::value::Type::Invalid(_)) => Ok(Value::Invalid),
            Some(wire::value::Type::None(_)) => Ok(Value::None),
            Some(wire::value::Type::En(e)) => Ok(Value::Enum(e as u8)),
            Some(wire::value::Type::B(b)) => Ok(Value::Bool(b)),
            Some(wire::value::Type::U8(u)) => Ok(Value::Uint8(u as u8)),
            Some(wire::value::Type::U16(u)) => Ok(Value::Uint16(u as u16)),
            Some(wire::value::Type::U32(u)) => Ok(Value::Uint32(u)),
            Some(wire::value::Type::U64(u)) => Ok(Value::Uint64(u)),
            Some(wire::value::Type::I8(i)) => Ok(Value::Int8(i as i8)),
            Some(wire::value::Type::I16(i)) => Ok(Value::Int16(i as i16)),
            Some(wire::value::Type::I32(i)) => Ok(Value::Int32(i)),
            Some(wire::value::Type::I64(i)) => Ok(Value::Int64(i)),
            Some(wire::value::Type::F(f)) => Ok(Value::Float(f)),
            Some(wire::value::Type::D(d)) => Ok(Value::Double(d)),
        }
    }
}

pub fn enum_value(v: u8) -> Value {
    Value::Enum(v)
}

impl From<bool> for Value {
    fn from(v: bool) -> Self {
        Value::Bool(v)
    }
}

impl From<u8> for Value {
    fn from(v: u8) -> Self {
        Value::Uint8(v)
    }
}

impl From<u16> for Value {
    fn from(v: u16) -> Self {
        Value::Uint16(v)
    }
}

impl From<u32> for Value {
    fn from(v: u32) -> Self {
        Value::Uint32(v)
    }
}

impl From<u64> for Value {
    fn from(v: u64) -> Self {
        Value::Uint64(v)
    }
}

impl From<i8> for Value {
    fn from(v: i8) -> Self {
        Value::Int8(v)
    }
}

impl From<i16> for Value {
    fn from(v: i16) -> Self {
        Value::Int16(v)
    }
}

impl From<i32> for Value {
    fn from(v: i32) -> Self {
        Value::Int32(v)
    }
}

impl From<i64> for Value {
    fn from(v: i64) -> Self {
        Value::Int64(v)
    }
}

impl From<f32> for Value {
    fn from(v: f32) -> Self {
        Value::Float(v)
    }
}

impl From<f64> for Value {
    fn from(v: f64) -> Self {
        Value::Double(v)
    }
}
