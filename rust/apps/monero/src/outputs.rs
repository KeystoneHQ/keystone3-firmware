use crate::key::PublicKey;
use crate::errors::Result;
use crate::utils::{constants::PUBKEY_LEH, io::*};
use alloc::vec::Vec;

#[allow(dead_code)]
#[derive(Debug, Clone)]
pub struct ExportedTransferDetail {
    pub pubkey: [u8; 32],
    pub internal_output_index: u64,
    pub global_output_index: u64,
    pub tx_pubkey: [u8; 32],
    pub flags: u8,
    pub amount: u64,
    pub additional_tx_keys: Vec<[u8; 32]>,
    pub major: u32,
    pub minor: u32,
}

impl ExportedTransferDetail {
    pub fn is_spent(&self) -> bool {
        self.flags & 0b00000001 != 0
    }
    pub fn is_frozen(&self) -> bool {
        self.flags & 0b00000010 != 0
    }
    pub fn is_rct(&self) -> bool {
        self.flags & 0b00000100 != 0
    }
    pub fn is_key_image_known(&self) -> bool {
        self.flags & 0b00001000 != 0
    }
    pub fn is_key_image_request(&self) -> bool {
        self.flags & 0b00010000 != 0
    }
    pub fn is_key_image_partial(&self) -> bool {
        self.flags & 0b00100000 != 0
    }
}

#[derive(Debug, Clone, Default)]
pub struct ExportedTransferDetails {
    pub offset: u64,
    pub size: u64,
    pub details: Vec<ExportedTransferDetail>,
}

impl ExportedTransferDetails {
    pub fn from_bytes(bytes: &[u8]) -> Result<ExportedTransferDetails> {
        let mut offset = 0;
        let has_transfers = read_varinteger(&bytes, &mut offset);
        if has_transfers == 0 {
            return Ok(ExportedTransferDetails {
                offset: 0,
                size: 0,
                details: Vec::new(),
            });
        }
        // offset
        read_varinteger(&bytes, &mut offset);
        // transfers.size()
        let value_offset = read_varinteger(&bytes, &mut offset);
        // details size
        let value_size = read_varinteger(&bytes, &mut offset);
        // for size
        let mut details = Vec::new();
        for _ in 0..value_size {
            // version ignore
            read_varinteger(&bytes, &mut offset);
            let pubkey = PublicKey::from_bytes(&bytes[offset..offset + PUBKEY_LEH]).unwrap();
            offset += PUBKEY_LEH;
            let internal_output_index = read_varinteger(&bytes, &mut offset);
            let global_output_index = read_varinteger(&bytes, &mut offset);
            let tx_pubkey = PublicKey::from_bytes(&bytes[offset..offset + PUBKEY_LEH]).unwrap();
            let flags = bytes[offset + PUBKEY_LEH];
            offset += PUBKEY_LEH + 1;
            let amount= read_varinteger(&bytes, &mut offset);
            // FIXME: additional_tx_keys
            let keys_num = read_varinteger(&bytes, &mut offset);
            let mut additional_tx_keys = Vec::new();
            for _ in 0..keys_num {
                let key = PublicKey::from_bytes(&bytes[offset..offset + PUBKEY_LEH]).unwrap();
                additional_tx_keys.push(key);
                offset += PUBKEY_LEH;
            }
            let major = read_varinteger(&bytes, &mut offset);
            let minor = read_varinteger(&bytes, &mut offset);

            details.push(ExportedTransferDetail {
                pubkey: pubkey.as_bytes(),
                internal_output_index,
                global_output_index,
                tx_pubkey: tx_pubkey.as_bytes(),
                flags,
                amount,
                additional_tx_keys: Vec::new(),
                major: major as u32,
                minor: minor as u32,
            });
        }

        Ok(ExportedTransferDetails {
            offset: value_offset,
            size: value_size,
            details,
        })
    }
}

#[cfg(test)]
mod tests {
    use crate::utils::*;
    use crate::utils::constants::OUTPUT_EXPORT_MAGIC;
    use hex;

    #[test]
    fn test_decrypt_data_with_pvk() {
        let pvk = hex::decode("bb4346a861b208744ff939ff1faacbbe0c5298a4996f4de05e0d9c04c769d501")
            .unwrap();
        let data = hex::decode("4d6f6e65726f206f7574707574206578706f727404eb5fb0d1fc8358931053f6e24d93ec0766aad43a54453593287d0d3dcfdef9371f411a0e179a9c1b0da94a3fe3d51cccf3573c01b6f8d6ee215caf3238976d8e9af5347e44b0d575fa622accdd4b4d5d272e13d77ff897752f52d7617be986efb4d2b1f841bae6c1d041d6ff9df46262b1251a988d5b0fbe5012d2af7b9ff318381bfd8cbe06af6e0750c16ff7a61d31d36526d83d7b6b614b2fd602941f2e94de01d0e3fc5a84414cdeabd943e5d8f0226ab7bea5e47c97253bf2f062e92a6bf27b6099a47cb8bca47e5ad544049611d77bfeb5c16b5b7849ce5d46bb928ce2e9a2b6679653a769f53c7c17d3e91df35ae7b62a4cffcea2d25df1c2e21a58b1746aae00a273317ec3873c53d8ae71d89d70637a6bd1da974e548b48a0f96d119f0f7d04ff034bb7fed3dbe9081d3e3a3212d330328c0edbacad85bab43780f9b5dfd81f359b0827146ebc421e60dba0badab1941bc31a0086aac99d59f55f07d58c02a48a3e1f70222bae1a612dacd09d0b176345a115e6ae6523ecbc346d8a8078111da7f9932f31d6e35500f5195cfdfe6b6eb2b223d171430a1cb7e11a51ac41d06f3a81546378b1ff342a18fb1f01cfd10df9c1ac86531456f240e5500d9c7ba4c47ba8d4455ea2b7e460ee207c064b76019f6bb4efe5a3e27a126b0c8be6a2e6f3d7ede9580ff49598501aafa36187896e245d64461f9f1c24323b1271af9e0a7a9108422de5ecfdaccdcb2b4520a6d75b2511be6f17a272d21e05ead99818e697559714af0a220494004e393eeefdfe029cff0db22c3adadf6f00edbf6bf4fcbcfc1e225451be3c1c700fe796fce6480b02d0cb1f9fbcf6c05895df2eeb8192980df50a0523922c1247fef83a5f631cf64132125477e1a3b13bcbaa691da1e9b45288eb6c7669e7a7857f87ed45f74725b72b4604fda6b44d3999e1d6fab0786f9b14f00a6518ca3fbc5f865d9fc8acd6e5773208").unwrap();

        let res = decrypt_data_with_pvk(pvk.try_into().unwrap(), data, OUTPUT_EXPORT_MAGIC).unwrap();

        assert_eq!(
            hex::encode(res.pk1.unwrap().as_bytes()),
            "fc6339d8849cae36319535ee50950ffc586aca1678529bd79161e158fc1ba298"
        );
        assert_eq!(
            hex::encode(res.pk2.unwrap().as_bytes()),
            "1981d791ec8683dd818a5d7ef99d5fe1ada7fc71f7518d230af1daf12b6debe1"
        );
        assert_eq!(hex::encode(res.data), "03000707013e8c52245d21b22cbcb90f95270a7937d4974d726209f0a41fdefc7f9df01fde01c8b486383e45d72b841a8b76094dbaa26f9800aac4eaced3bc06122a3380bcf6c666d2281480a0b787e905000000012d58a6378c07f230148c11979cc6e3bec2719f0ec92de21f7fae02029ab025e000f385873857dc102abc6d35c878db7be629646658ae1a418afb27a943f8a2591be4f450e9148094ebdc03000001014ef323a52d2e048594ad73acbe5fb7e588b1859ec9aa02b2670f487660b2700901f485873857dc102abc6d35c878db7be629646658ae1a418afb27a943f8a2591be4f450e914c0b5809ce50500000001cb8ab3c1b4dd10404a4a3c9275a7e2e1e9bf2e4edf1c84f61952bb97965573a300d0c78a38bdd50fdc0367b3141fdc055dec3af5e3ac920dd55816823dfe02f70c3d1816431480c2d72f00000301dd8c2a791056760d903bf06e7930585201e0bd20bcba1e720b85ad0e4d628e4801d1c78a38bdd50fdc0367b3141fdc055dec3af5e3ac920dd55816823dfe02f70c3d18164314a0eec19e03000000019b65ada69049d73e4b049ebd50393410cdc05dad5314690d2b4a36628c4e257600a4909d385d43421399107bd34350b8938f9ff69da18e8f083e6522adf6aa270b3f370ed41480e8eda1ba01000100016311ba60a0a8c636806e232db3e1ad7f79e26df3d24258e264e4351e47f4374d01a5909d385d43421399107bd34350b8938f9ff69da18e8f083e6522adf6aa270b3f370ed414c0c2b383ae04000000");
    }
}
