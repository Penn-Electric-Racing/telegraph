use crate::{
    perdat_int,
    util::{FromBytes, ParseError},
};

pub enum Type {
    Bool,
    ByteArray,
    Str,
    F32,
    F64,
    U8,
    U16,
    U32,
    U64,
    I8,
    I16,
    I32,
    I64,
}

const K_BOOL: u8 = 1 << 4;
const K_BYTE_ARRAY: u8 = 1 << 3;
const K_STR: u8 = 1 << 2;
const K_FLOAT: u8 = 3;
const K_SIGNED: u8 = 1;
const K_UNSIGNED: u8 = 0;

impl FromBytes for Type {
    fn read_from<B: Iterator<Item = u8>>(bytes: &mut B) -> Result<Self, ParseError> {
        // type = byte(byte[] | string | floating point | signed) i(byte count)
        let kind = bytes.next().ok_or(ParseError::FellOffEnd)?;
        let size = perdat_int::read_from(bytes)?;

        match (kind, u32::from(size)) {
            (K_BOOL, _) => Ok(Type::Bool),
            (K_BYTE_ARRAY, _) => Ok(Type::ByteArray),
            (K_STR, _) => Ok(Type::Str),
            (K_FLOAT, 4) => Ok(Type::F32),
            (K_FLOAT, 8) => Ok(Type::F64),
            (K_SIGNED, 1) => Ok(Type::I8),
            (K_SIGNED, 2) => Ok(Type::I16),
            (K_SIGNED, 4) => Ok(Type::I32),
            (K_SIGNED, 8) => Ok(Type::I64),
            (K_UNSIGNED, 1) => Ok(Type::U8),
            (K_UNSIGNED, 2) => Ok(Type::U16),
            (K_UNSIGNED, 4) => Ok(Type::U32),
            (K_UNSIGNED, 8) => Ok(Type::U64),
            _ => Err(ParseError::UnrecognizedVariant),
        }
    }
}

// TODO: what are the possible floating point / signed widths
pub enum Value {
    ByteArray(Vec<u8>),
    Str(String),
    F32(f32),
    F64(f64),
    I8(i8),
    I16(i16),
    I32(i32),
    I64(i64),
}
