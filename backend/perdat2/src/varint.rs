use bytes::BytesMut;

pub fn read_from(bytes: &[u8], start_idx: usize) -> Option<(u32, usize)> {
    let mut value: u32 = 0;
    let mut shift = 0;
    let mut len = 0;

    loop {
        let b = bytes.get(start_idx + len)?;
        len += 1;

        value |= ((b & 0x7F) as u32) << shift;

        if ((b >> 7) & 1) == 0 {
            break;
        }
        shift += 7;
    }

    Some((value, len))
}

pub fn to_bytes(mut value: u32) -> Vec<u8> {
    let mut out = Vec::new();

    loop {
        let b = (value & 0x7F) as u8;
        value >>= 7;

        // Set the MSB if there are more bytes to come
        out.push(b | if value == 0 { 0 } else { 1 << 7 });

        if value == 0 {
            break;
        }
    }
    out
}
