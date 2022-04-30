use async_graphql::{SimpleObject, InputObject, Enum};
use bytes::Bytes;

#[derive(Enum, Copy, Clone, Eq, PartialEq)]
pub enum Type {
    Bool, ByteArray,
    Str, F32, F64,
    U8, U16, U32, U64,
    I8, I16, I32, I64,
}

pub enum RawValue {
    I8(i8), I16(i16), I32(i32), I64(i64),
    U8(u8), U16(u16), U32(u32), U64(u64),
    F32(f32), F64(f64), Str(String), Bool(bool), ByteArray(Bytes)
}

#[derive(SimpleObject, InputObject)]
#[graphql(input_name= "ValueInput")]
pub struct Value {
    boolean : bool,
    string : Option<String>,
    byte_array : Option<Bytes>,
    float32 : Option<f32>,
    float64 : Option<f64>,
    int8 : Option<i8>,
    int16 : Option<i16>,
    int32 : Option<i32>,
    int64 : Option<i64>,
    uint8 : Option<u8>,
    uint16 : Option<u16>,
    uint32 : Option<u32>,
    uint64 : Option<u64>,
}

#[derive(SimpleObject, InputObject)]
#[graphql(input_name= "DatapointInput")]
pub struct Datapoint {
    value: Value
}