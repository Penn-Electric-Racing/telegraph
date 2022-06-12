use async_graphql::{SimpleObject, InputObject, Enum};
use bytes::Bytes;
use byteorder::{ReadBytesExt, LittleEndian};

#[derive(Enum, Copy, Clone, Eq, PartialEq)]
pub enum DataType {
    Bool, ByteArray,
    Str, F32, F64,
    U8, U16, U32, U64,
    I8, I16, I32, I64,
}

impl DataType {
    pub fn parse_perxml(s: &str) -> Self {
        use DataType::*;
        match s {
            "bool" => Bool,
            "int8" => I8,
            "uint8" => U8,
            "int16" => I16,
            "uint16" => U16,
            "int32" => I32,
            "uint32" => U32,
            "int64" => I64,
            "uint64" => U64,
            "float" => F32,
            "dobule" => F64,
            _ => panic!("Unrecognized type")
        }
    }

    pub fn parse(&self, data: &mut &[u8], bit_off: &mut usize) -> RawValue {
        use DataType::*;
        match self {
            Str | ByteArray => panic!(),
            Bool => {
                let b = ((data[0] >> *bit_off) & 0x01) != 0;
                *bit_off = *bit_off + 1;
                if *bit_off >= 8 {
                    *data = &data[1..];
                    *bit_off = 0;
                }
                RawValue::Bool(b)
            },
            _ => {
                if *bit_off != 0 { panic!("misaligned data!")}
                match self {
                Str | ByteArray | Bool => panic!(),
                U8 => { RawValue::U8(data.read_u8().unwrap()) },
                I8 => { RawValue::I8(data.read_i8().unwrap()) },
                U16 => { RawValue::U16(data.read_u16::<LittleEndian>().unwrap()) },
                I16 => { RawValue::I16(data.read_i16::<LittleEndian>().unwrap()) },
                U32 => { RawValue::U32(data.read_u32::<LittleEndian>().unwrap()) },
                I32 => { RawValue::I32(data.read_i32::<LittleEndian>().unwrap()) },
                U64 => { RawValue::U64(data.read_u64::<LittleEndian>().unwrap()) },
                I64 => { RawValue::I64(data.read_i64::<LittleEndian>().unwrap()) },
                F32 => { RawValue::F32(data.read_f32::<LittleEndian>().unwrap()) },
                F64 => { RawValue::F64(data.read_f64::<LittleEndian>().unwrap()) },
                }
            }
        }
    }
}

#[derive(SimpleObject, InputObject, Clone)]
pub struct Type {
    pub format: DataType,
    pub units: Option<String>,
    pub enum_values: Option<Vec<String>>
}

impl Type {
    pub fn new(format: DataType, units: Option<String>, enum_values: Option<Vec<String>>) -> Self {
        Self { format, units, enum_values }
    }
}

pub enum RawValue {
    I8(i8), I16(i16), I32(i32), I64(i64),
    U8(u8), U16(u16), U32(u32), U64(u64),
    F32(f32), F64(f64), Str(String), Bool(bool), ByteArray(Bytes)
}

impl Into<Value> for RawValue {
    fn into(self) -> Value {
        let mut v = Value::default();
        use RawValue::*;
        match self {
            Str(s) => v.string = Some(s),
            ByteArray(b) => v.byte_array = Some(b),
            Bool(b) => v.boolean = Some(b),
            I8(i) => v.int8 = Some(i),
            U8(u) => v.uint8 = Some(u),
            I16(i) => v.int16 = Some(i),
            U16(u) => v.uint16 = Some(u),
            I32(i) => v.int32 = Some(i),
            U32(u) => v.uint32 = Some(u),
            I64(i) => v.int64 = Some(i),
            U64(u) => v.uint64 = Some(u),
            F32(f) => v.float32 = Some(f),
            F64(f) => v.float64 = Some(f)
        }
        v
    }
}

#[derive(SimpleObject, InputObject, Clone, Default)]
#[graphql(input_name= "ValueInput")]
pub struct Value {
    boolean : Option<bool>,
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

#[derive(SimpleObject, InputObject, Clone)]
#[graphql(input_name= "DatapointInput")]
pub struct Datapoint {
    pub value: Value
}