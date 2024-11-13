mod blake256;
mod cnaes;
mod util;
mod slow_hash;

use slow_hash::cn_slow_hash;

pub fn cryptonight_hash_v0(buf: &[u8]) -> [u8; 32] {
    cn_slow_hash(buf)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn slow_hash_0() {
        fn test(inp: &str, exp: &str) {
            let res = hex::encode(cryptonight_hash_v0(&hex::decode(inp).unwrap()));
            assert_eq!(&res, exp);
        }

        // https://github.com/monero-project/monero/blob/67d190ce7c33602b6a3b804f633ee1ddb7fbb4a1/tests/hash/tests-slow.txt
        test(
            "6465206f6d6e69627573206475626974616e64756d",
            "2f8e3df40bd11f9ac90c743ca8e32bb391da4fb98612aa3b6cdc639ee00b31f5",
        );
        test(
            "6162756e64616e732063617574656c61206e6f6e206e6f636574",
            "722fa8ccd594d40e4a41f3822734304c8d5eff7e1b528408e2229da38ba553c4",
        );
        test(
            "63617665617420656d70746f72",
            "bbec2cacf69866a8e740380fe7b818fc78f8571221742d729d9d02d7f8989b87",
        );
        test(
            "6578206e6968696c6f206e6968696c20666974",
            "b1257de4efc5ce28c6b40ceb1c6c8f812a64634eb3e81c5220bee9b2b76a6f05",
        );
    }
}
