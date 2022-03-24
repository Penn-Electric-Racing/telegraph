use std::cmp::min;

/// Calculate the Fletcher32 checksum of the first `words` 16-bit words of
/// `data` starting at `start_index`.
pub fn fletcher32(data: &[u8], mut start_index: usize, mut words: u32) -> u32 {
    // Based on the C# code from the old CDP
    // Originally based on https://en.wikipedia.org/wiki/Fletcher%27s_checksum

    let mut sum1: u32 = 0xffff;
    let mut sum2: u32 = 0xffff;

    while words > 0 {
        let mut tlen: u32 = min(359, words);
        words -= tlen;

        loop {
            // data is stored little-endian
            let word: u32 = (data[start_index] as u32) | ((data[start_index + 1] as u32) << 8);
            sum1 += word;
            sum2 += sum1;

            start_index += 2;
            tlen -= 1;

            if tlen <= 0 {
                break;
            }
        }

        sum1 = (sum1 & 0xffff) + (sum1 >> 16);
        sum2 = (sum2 & 0xffff) + (sum2 >> 16);
    }

    /* Second reduction step to reduce sums to 16 bits */
    sum1 = (sum1 & 0xffff) + (sum1 >> 16);
    sum2 = (sum2 & 0xffff) + (sum2 >> 16);
    return (sum2 << 16) | sum1;
}

#[cfg(test)]
mod tests {
    #[test]
    fn wikipedia_test_cases() {
        // See https://en.wikipedia.org/wiki/Fletcher%27s_checksum#Test_vectors

        // "abcde" -> 4031760169 (0xF04FC729)
        let str1 = "abcde\0"; // zero-padded
        assert_eq!(super::fletcher32(str1.as_bytes(), 0, 3), 0xF04FC729);

        // "abcdef" -> 1448095018 (0x56502D2A)
        let str2 = "abcdef";
        assert_eq!(super::fletcher32(str2.as_bytes(), 0, 3), 0x56502D2A);

        // "abcdefgh" -> 3957429649 (0xEBE19591)
        let str3 = "abcdefgh";
        assert_eq!(super::fletcher32(str3.as_bytes(), 0, 4), 0xEBE19591);
    }
}
