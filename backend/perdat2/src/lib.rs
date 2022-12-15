pub mod fletcher32;
mod util;
// pub mod values;
pub mod varint;

use bytes::{Buf, BytesMut};
use std::io::ErrorKind;
use tokio_util::codec::{Decoder, Encoder};

#[derive(Debug)]
pub struct PerDosMessage {
    pub id: u32,
    pub payload: Vec<u8>,
}

const HEADER_SIZE: usize = 4;
const FOOTER_SIZE: usize = 4;
const MAX_PERDOS_LENGTH: usize = 262;
const PADDING_BYTE: u8 = 0x10;

pub struct PerDos;
impl Decoder for PerDos {
    type Item = Vec<PerDosMessage>;
    type Error = std::io::Error;
    fn decode(&mut self, buf: &mut BytesMut) -> std::io::Result<Option<Self::Item>> {
        let mut ok = false;
        // Skip ahead until we find the start of message frame
        while buf.len() >= HEADER_SIZE {
            if buf.starts_with(&[0xFF, 0xFF, 0xFF]) {
                ok = true;
                break;
            }

            let _ = buf.split_to(1);
        }

        if !ok {
            // Not enough bytes to even read the header
            return Ok(None);
        }

        let len = buf[3] as usize;
        if buf.len() < HEADER_SIZE + len + FOOTER_SIZE {
            return Ok(None);
        }

        let buf = buf.split_to(HEADER_SIZE + len + FOOTER_SIZE);

        let actual_checksum = fletcher32::fletcher32(&buf, HEADER_SIZE, len / 2);
        let expected_checksum = (&buf[HEADER_SIZE + len..]).get_u32_le();
        if expected_checksum != actual_checksum {
            // Checksum did not match, just skip this frame
            return Ok(None);
        }

        let mut idx = 4;
        let mut output = vec![];

        while idx < HEADER_SIZE + len {
            let (code, skip) = match varint::read_from(&buf, idx) {
                Some(o) => o,
                None => return Ok(None),
            };
            idx += skip;

            if code == PADDING_BYTE as u32 {
                continue;
            }

            let (data_len, skip) = match varint::read_from(&buf, idx) {
                Some(o) => o,
                None => return Ok(None),
            };
            idx += skip;

            let data_len = data_len as usize;

            if data_len + idx > HEADER_SIZE + len {
                return Err(std::io::Error::new(
                    ErrorKind::InvalidData,
                    "Data was too long",
                ));
            }
            output.push(PerDosMessage {
                id: code,
                payload: (&buf[idx..idx + data_len]).iter().cloned().collect(),
            });
            idx += data_len;
        }

        Ok(Some(output))
    }
}

impl Encoder<Vec<PerDosMessage>> for PerDos {
    type Error = std::io::Error;

    fn encode(&mut self, items: Vec<PerDosMessage>, dst: &mut BytesMut) -> Result<(), Self::Error> {
        let mut body = BytesMut::new();

        for item in items {
            if item.payload.len() > 255 {
                return Err(std::io::Error::new(
                    std::io::ErrorKind::InvalidData,
                    format!("Item of length {} is too large.", item.payload.len()),
                ));
            }
            let id_bytes = varint::to_bytes(item.id);
            body.extend_from_slice(&id_bytes);
            body.extend_from_slice(&[item.payload.len() as u8]);
            body.extend_from_slice(&item.payload);
        }

        if body.len() % 2 != 0 {
            body.extend_from_slice(&[PADDING_BYTE])
        }

        // TODO: check for max length
        // We have to check at the end because of all of the variable-length nonsense
        // // Don't send a string if it is longer than the other end will
        // // accept.
        let total_len = body.len() + HEADER_SIZE + FOOTER_SIZE;
        if total_len > MAX_PERDOS_LENGTH {
            return Err(std::io::Error::new(
                std::io::ErrorKind::InvalidData,
                format!("Frame of length {} is too large.", total_len),
            ));
        }

        // Reserve space in the buffer.
        dst.reserve(total_len);

        // Write the results to the target buffer
        dst.extend_from_slice(&[0xFF, 0xFF, 0xFF, body.len() as u8]);
        dst.extend(&body);
        dst.extend(u32::to_le_bytes(fletcher32::fletcher32(
            &body,
            0,
            body.len() / 2,
        )));

        Ok(())
    }
}
