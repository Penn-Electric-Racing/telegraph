use crate::util::{FromBytes, ParseError};

#[allow(non_camel_case_types)]
#[derive(Copy, Clone)]
/// Our custom base-128 unsigned integer. We declare perdat_int as a newtype
/// wrapper over u32 because these are treated specially over the wire.
/// Downstream consumers can safely convert this to a u32 when necessary.
pub struct perdat_int(u32);

impl AsRef<u32> for perdat_int {
    fn as_ref(&self) -> &u32 {
        &self.0
    }
}

impl AsMut<u32> for perdat_int {
    fn as_mut(&mut self) -> &mut u32 {
        &mut self.0
    }
}

impl From<perdat_int> for u32 {
    fn from(i: perdat_int) -> u32 {
        i.0
    }
}

impl From<u32> for perdat_int {
    fn from(i: u32) -> Self {
        Self(i)
    }
}

impl FromBytes for perdat_int {
    fn read_from<B: Iterator<Item = u8>>(bytes: &mut B) -> Result<Self, ParseError> {
        let mut value: u32 = 0;
        let mut shift = 0;

        loop {
            let b = bytes.next().ok_or(ParseError::FellOffEnd)?;
            value |= ((b & 0x7F) as u32) << shift;

            if ((b >> 7) & 1) == 0 {
                break;
            }
            shift += 7;
        }

        Ok(value.into())
    }
}

// static void Write7BitEncodedUInt(uint8_t* const data, uint32_t& index, const uint32_t maxIndex, uint32_t value) {
//     while (index < maxIndex)
//     {
//         uint8_t byte = value & 0x7F;
//         value >>= 7;
//         if (value)
//             byte |= 1 << 7;
//         data[index] = byte;
//         index++;
//         if (value == 0)
//             break;
//     }
// }

// static void Read7BitEncodedUInt(const uint8_t* const data, uint32_t& index, const uint32_t maxIndex, uint32_t& value)
// {
//     value = 0;
//     int shift = 0;
//     uint8_t b;
//     while (index < maxIndex)
//     {
//         b = data[index];
//         value |= (uint32_t)(b & 0x7F) << shift;
//         index++;
//         if ((b >> 7 & 1) == 0)
//             break;
//         shift += 7;
//     }
// }
