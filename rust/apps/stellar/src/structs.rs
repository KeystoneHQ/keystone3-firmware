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
        let public_hash = Network::Public.hash();
        let testnet_hash = Network::Testnet.hash();
        let futurenet_hash = Network::Futurenet.hash();
        let sandbox_hash = Network::Sandbox.hash();
        let standalone_hash = Network::Standalone.hash();

        match hex::encode(hash) {
            public_hash => Network::Public,
            testnet_hash => Network::Testnet,
            futurenet_hash => Network::Futurenet,
            sandbox_hash => Network::Sandbox,
            standalone_hash => Network::Standalone,
            _ => Network::Public,
        }
    }
}