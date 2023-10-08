use alloc::vec::Vec;
use alloc::string::String;
use core::fmt::Write;

pub fn frame_parser(frame: Vec<u8>) -> Vec<String> {
    frame.into_iter().map(|byte| {
        let mut s = String::new();
        write!(&mut s, "{:02x}", byte).expect("Unable to write");
        s
    }).collect()
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_frame_parser() {
        let input = vec![0x01, 0x02, 0x03, 0x0A, 0x0B, 0x0C, 0xFF];
        let output = frame_parser(input);
        let expected_output = vec!["01", "02", "03", "0a", "0b", "0c", "ff"]
            .into_iter()
            .map(String::from)
            .collect::<Vec<String>>();
        assert_eq!(output, expected_output);
    }
}