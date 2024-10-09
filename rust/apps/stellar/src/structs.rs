use alloc::string::String;
use third_party::hex;

#[derive(Debug, Clone, Copy)]
pub enum Network {
    Public = 1,
    Testnet,
    Futurenet,
    Sandbox,
    Standalone,
}

impl Network {
    pub fn get(&self) -> &'static str {
        match *self {
            Network::Public => "Public Global Stellar Network ; September 2015",
            Network::Testnet => "Test SDF Network ; September 2015",
            Network::Futurenet => "Test SDF Future Network ; October 2022",
            Network::Sandbox => "Local Sandbox Stellar Network ; September 2022",
            Network::Standalone => "Standalone Network ; February 2017",
        }
    }

    pub fn hash(&self) -> String {
        let hash_bytes = third_party::cryptoxide::hashing::sha256(self.get().as_bytes());
        hex::encode(hash_bytes)
    }

    pub fn from_hash(hash: &[u8]) -> Network {
        match hex::encode(hash).as_str() {
            "7ac33997544e3175d266bd022439b22cdb16508c01163f26e5cb2a3e1045a979" => Network::Public,
            "cee0302d59844d32bdca915c8203dd44b33fbb7edc19051ea37abedf28ecd472" => Network::Testnet,
            "a3a1c6a78286713e29be0e9785670fa838d13917cd8eaeb4a3579ff1debc7fd5" => {
                Network::Futurenet
            }
            "9a8931194023c5a25b7006a217b0ea590c5eaea9f6c61a356989899bbcf478bc" => Network::Sandbox,
            "baefd734b8d3e48472cff83912375fedbc7573701912fe308af730180f97d74a" => {
                Network::Standalone
            }
            _ => Network::Public,
        }
    }
}
