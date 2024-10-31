use crate::key::{PublicKey, PrivateKey};
use crate::utils::{decrypt_data_with_pvk, PUBKEY_LEH, OUTPUT_EXPORT_MAGIC};
use alloc::vec;
use alloc::vec::Vec;
use alloc::string::String;
use chacha20::{ChaCha20Legacy, ChaCha20LegacyCore, ChaChaCore};
use chacha20::cipher::{KeyIvInit, StreamCipher, StreamCipherSeek, generic_array::GenericArray};
use cuprate_cryptonight::cryptonight_hash_v0;
use varinteger::*;
use hex;

struct ExportedTransferDetail {
    pubkey: PublicKey,
    internal_output_index: u64,
    global_output_index: u64,
    tx_pubkey: PublicKey,
    flags: u8,
    amount: u64,
    additional_tx_keys: Vec<PublicKey>,
    major: u32,
    minor: u32,
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

struct ExportedTransferDetails {
    offset: u64,
    size: u64,
    details: Vec<ExportedTransferDetail>,
}

impl ExportedTransferDetails {
    pub fn from_bytes(bytes: &[u8]) -> Self {
        let mut offset = 0;
        let mut value = 0u64;
        let (_, offset) = read_varinteger(&bytes, offset);
        let (_, offset) = read_varinteger(&bytes, offset);
        // offset
        let (value_offset, offset) = read_varinteger(&bytes, offset);
        // size
        let (value_size, mut offset) = read_varinteger(&bytes, offset);

        // for size
        let mut details = Vec::new();
        let mut global_offset = offset;
        for _ in 0..value_size {
            offset = global_offset;
            // version ignore
            let (_, offset) = read_varinteger(&bytes, offset);
            let pubkey = PublicKey::from_bytes(&bytes[offset..offset + PUBKEY_LEH]).unwrap();
            let (internal_output_index, offset) = read_varinteger(&bytes, offset + PUBKEY_LEH);
            let (global_output_index, offset) = read_varinteger(&bytes, offset);
            let tx_pubkey = PublicKey::from_bytes(&bytes[offset..offset + PUBKEY_LEH]).unwrap();
            let flags = bytes[offset + PUBKEY_LEH];
            let (amount, offset) = read_varinteger(&bytes, offset + PUBKEY_LEH + 1);
            // TODO: additional_tx_keys
            let (_, offset) = read_varinteger(&bytes, offset);
            let (major, offset) = read_varinteger(&bytes, offset);
            let (minor, offset) = read_varinteger(&bytes, offset);
            
            details.push(ExportedTransferDetail {
                pubkey,
                internal_output_index,
                global_output_index,
                tx_pubkey,
                flags,
                amount,
                additional_tx_keys: Vec::new(),
                major: major as u32,
                minor: minor as u32,
            });

            global_offset = offset;
        }

        ExportedTransferDetails {
            offset: value_offset,
            size: value_size,
            details,
        }
    }
}

fn read_varinteger(data: &[u8], mut offset: usize) -> (u64, usize) {
    let mut value = 0u64;
    let begin = varinteger::decode_with_offset(data, offset, &mut value) + offset;
    (value, begin)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_decrypt_data_with_pvk() {
        let pvk = hex::decode("bb4346a861b208744ff939ff1faacbbe0c5298a4996f4de05e0d9c04c769d501").unwrap();
        let data = hex::decode("4d6f6e65726f206f7574707574206578706f727404eb5fb0d1fc8358931053f6e24d93ec0766aad43a54453593287d0d3dcfdef9371f411a0e179a9c1b0da94a3fe3d51cccf3573c01b6f8d6ee215caf3238976d8e9af5347e44b0d575fa622accdd4b4d5d272e13d77ff897752f52d7617be986efb4d2b1f841bae6c1d041d6ff9df46262b1251a988d5b0fbe5012d2af7b9ff318381bfd8cbe06af6e0750c16ff7a61d31d36526d83d7b6b614b2fd602941f2e94de01d0e3fc5a84414cdeabd943e5d8f0226ab7bea5e47c97253bf2f062e92a6bf27b6099a47cb8bca47e5ad544049611d77bfeb5c16b5b7849ce5d46bb928ce2e9a2b6679653a769f53c7c17d3e91df35ae7b62a4cffcea2d25df1c2e21a58b1746aae00a273317ec3873c53d8ae71d89d70637a6bd1da974e548b48a0f96d119f0f7d04ff034bb7fed3dbe9081d3e3a3212d330328c0edbacad85bab43780f9b5dfd81f359b0827146ebc421e60dba0badab1941bc31a0086aac99d59f55f07d58c02a48a3e1f70222bae1a612dacd09d0b176345a115e6ae6523ecbc346d8a8078111da7f9932f31d6e35500f5195cfdfe6b6eb2b223d171430a1cb7e11a51ac41d06f3a81546378b1ff342a18fb1f01cfd10df9c1ac86531456f240e5500d9c7ba4c47ba8d4455ea2b7e460ee207c064b76019f6bb4efe5a3e27a126b0c8be6a2e6f3d7ede9580ff49598501aafa36187896e245d64461f9f1c24323b1271af9e0a7a9108422de5ecfdaccdcb2b4520a6d75b2511be6f17a272d21e05ead99818e697559714af0a220494004e393eeefdfe029cff0db22c3adadf6f00edbf6bf4fcbcfc1e225451be3c1c700fe796fce6480b02d0cb1f9fbcf6c05895df2eeb8192980df50a0523922c1247fef83a5f631cf64132125477e1a3b13bcbaa691da1e9b45288eb6c7669e7a7857f87ed45f74725b72b4604fda6b44d3999e1d6fab0786f9b14f00a6518ca3fbc5f865d9fc8acd6e5773208").unwrap();
    
        let (public_spend_key, public_view_key, decrypted_data) = decrypt_data_with_pvk(pvk.try_into().unwrap(), data, OUTPUT_EXPORT_MAGIC);

        assert_eq!(hex::encode(public_spend_key.as_bytes()), "fc6339d8849cae36319535ee50950ffc586aca1678529bd79161e158fc1ba298");
        assert_eq!(hex::encode(public_view_key.as_bytes()), "1981d791ec8683dd818a5d7ef99d5fe1ada7fc71f7518d230af1daf12b6debe1");
        assert_eq!(hex::encode(decrypted_data), "03000707013e8c52245d21b22cbcb90f95270a7937d4974d726209f0a41fdefc7f9df01fde01c8b486383e45d72b841a8b76094dbaa26f9800aac4eaced3bc06122a3380bcf6c666d2281480a0b787e905000000012d58a6378c07f230148c11979cc6e3bec2719f0ec92de21f7fae02029ab025e000f385873857dc102abc6d35c878db7be629646658ae1a418afb27a943f8a2591be4f450e9148094ebdc03000001014ef323a52d2e048594ad73acbe5fb7e588b1859ec9aa02b2670f487660b2700901f485873857dc102abc6d35c878db7be629646658ae1a418afb27a943f8a2591be4f450e914c0b5809ce50500000001cb8ab3c1b4dd10404a4a3c9275a7e2e1e9bf2e4edf1c84f61952bb97965573a300d0c78a38bdd50fdc0367b3141fdc055dec3af5e3ac920dd55816823dfe02f70c3d1816431480c2d72f00000301dd8c2a791056760d903bf06e7930585201e0bd20bcba1e720b85ad0e4d628e4801d1c78a38bdd50fdc0367b3141fdc055dec3af5e3ac920dd55816823dfe02f70c3d18164314a0eec19e03000000019b65ada69049d73e4b049ebd50393410cdc05dad5314690d2b4a36628c4e257600a4909d385d43421399107bd34350b8938f9ff69da18e8f083e6522adf6aa270b3f370ed41480e8eda1ba01000100016311ba60a0a8c636806e232db3e1ad7f79e26df3d24258e264e4351e47f4374d01a5909d385d43421399107bd34350b8938f9ff69da18e8f083e6522adf6aa270b3f370ed414c0c2b383ae0400000063c57cc457a1485fc5f8e6dfc8b70430f41946a7d0cd51e84ef5ac819ff2b2c4bcec6f1e6dd57e7e791d8cca2091169bba53496d72375331f8d56cd33f5e0ca4");
    }

    #[test]
    fn test_parse_data() {
        let pvk = hex::decode("bb4346a861b208744ff939ff1faacbbe0c5298a4996f4de05e0d9c04c769d501").unwrap();
        let data = hex::decode("4d6f6e65726f206f7574707574206578706f7274046fb4e6d12d99de6987a506170094cf5fd0b4fcce824c455f93002fdee027b3e3a6d59d21068c10599081cf0a47030f7564f4b9e6a909877088db58016c8bc3efee9ea180310ab47be52b0b25d509da5d0f78af8f24e891cd824b6b5772735e8fa9df2238c76859b488118fc0f6d90d3cb0f25a2aaabf34f0f5d2a0f323866328de4e2add019edf4e956377f2dddb27c4e97c5fef9de2726f570ea9ecaa069092962da4dcaf6a4280b31c8fae34eaba7bd2eb1b253d45a1f0e2f86d0748b0b58048bc056052351044064b9408216a201acaa0213bdf505bcf90297f2f7ce5265fbfca32640bf18468d6ba7ab3e52e873f4f7e0d52afc50289f83bc3765ab6ba38e4bdbf450edb841fcc8e8affde6994d916d12c89bdbf0903ed05126397a38c2d7237a012e5c488d7d6915780ba064126d4e4de02510ea4a7a0dc24f9b6d6efdf777c556028178cf6ef7198d795e8b372e434aa822aa6def7ae4a71682b51a64355c057a8c4fc7b2ff45d1eb6b8e1cbf737ea7fa00a5be12056482505012bc08b65026f4823c148a761d1f538079d3ce207a4aa549d6d0d219799a530bc4f716fc405f7bad1b67ecf7d24603aabd0da4748badfa558fa246f53afce0d35ee9c2c5716d6425e631d516c0e1d738c724aa1c621fb89f444e04cf183699bbdab229f480fb5767c608ee903dba49cedae4d595eb5d185dcd3c994db53360db88dfefac2622533733feae0ce4b1547af740193966cc0d34cb1f965317bb0d2b595036baa6b9c84cc2ec280e7a8fe07e16e339db7f5b8ba5da1dc29330a65e24ddb6787b0dff66876402081a98d97b719a9260a65b87f5b274b5eedf29fdec3c8bc06a442530a2b8c799876458da2b455ef41b5d2a58e6da234067f90da173050b5116854cf70dc40d69721d8ad11357ce658c4e4586b82252ede443546dc40428485e705ee3e54f8c5a0a79687883761b512c2b76c99a87e3677031ce364220059c59c20d8df798810ed5e106f4be4862784c58c7e01873996094297db30c38e5297edc89bc091e8eec40f36db2159edc626b5ec240836f851d932b8088884df2e84b9553219a4f2121c1ac368cf237a2e5092b332bcc994d300edd0041534238c90e3c1e065def203770b295b34e0c70e8ac01c7c40ab61f07016d9be1a1c7f4624bf6e9e90b24bb68236797c0d37574a7a7318a1fdd0d9b4496f7603623e2d6a9538277c141de8c252b28690d6129966cb54d716eb004b52834ec83b94676a78466055b397a37c738bc9948160a3aa278efc60857265cd130140c6d4da73902631287ddf369d11086d0d5f6fd5cb657544b4148ab9f3d823eab69848903e2f6bb1f2be60ec9c5febd08549cea16aaeacfe14ebf93b95c7628620275f56450a6e762a4789666c0b8d400f9103035545811f1da52abf8cbaa0b5d1062d7ae3e29893135e0fd0dadae0669a2070a8926224599253570ee3d436d5d1bc604064a53fa226d5af6ae6d66d05b7bc00b87ff552d8ccd7004d4ffe800745a75d4ba2be59b3d1515a7351169e04").unwrap();
    
        let (_, _, decrypted_data) = decrypt_data_with_pvk(pvk.try_into().unwrap(), data, OUTPUT_EXPORT_MAGIC);

        assert_eq!(hex::encode(decrypted_data.clone()), "03000c0c013e8c52245d21b22cbcb90f95270a7937d4974d726209f0a41fdefc7f9df01fde01c8b486383e45d72b841a8b76094dbaa26f9800aac4eaced3bc06122a3380bcf6c666d2281480a0b787e905000000012d58a6378c07f230148c11979cc6e3bec2719f0ec92de21f7fae02029ab025e000f385873857dc102abc6d35c878db7be629646658ae1a418afb27a943f8a2591be4f450e9148094ebdc03000001014ef323a52d2e048594ad73acbe5fb7e588b1859ec9aa02b2670f487660b2700901f485873857dc102abc6d35c878db7be629646658ae1a418afb27a943f8a2591be4f450e914c0b5809ce50500000001cb8ab3c1b4dd10404a4a3c9275a7e2e1e9bf2e4edf1c84f61952bb97965573a300d0c78a38bdd50fdc0367b3141fdc055dec3af5e3ac920dd55816823dfe02f70c3d1816431480c2d72f00000301dd8c2a791056760d903bf06e7930585201e0bd20bcba1e720b85ad0e4d628e4801d1c78a38bdd50fdc0367b3141fdc055dec3af5e3ac920dd55816823dfe02f70c3d18164314a0eec19e03000000019b65ada69049d73e4b049ebd50393410cdc05dad5314690d2b4a36628c4e257600a4909d385d43421399107bd34350b8938f9ff69da18e8f083e6522adf6aa270b3f370ed41480e8eda1ba01000100016311ba60a0a8c636806e232db3e1ad7f79e26df3d24258e264e4351e47f4374d01a5909d385d43421399107bd34350b8938f9ff69da18e8f083e6522adf6aa270b3f370ed414c0c2b383ae0400000001c6c6ff890ff7eb21b94c6339544b6710a62db5e4db1bf24d21ba7f58f9fedb4a00d2eba5381089fe4eba8d0dc478f56eedfbf8c33c5be2fa11eb4990b30eb09653e86674d114a097e4f4ad04000101012a00c725d3974b30064be120546d12c7e7e62cb65111c1370050d6b989fa2a660084dfa6385559db18ebb605f831c5d2f697bd5a968b151276a1b7d13ce37fb7a833a30a1f1480d48982940100010001376805372f4021fe7659da328086775fca38388da87ec996d799a406f2590d810185dfa6385559db18ebb605f831c5d2f697bd5a968b151276a1b7d13ce37fb7a833a30a1f1480988be4990300000201d5871df1287b277790ef31ac5f23dea30c7af13af54c821b68b7574b7521065100c3e0a6383c646ecd346bce68460c246734a5330a3c4d3eb180442760831d16fd64e7f5071480c8afa0250000020100e82e301544c3c48fe4d204691609f65f4dae5d4faa0054a00bd444bfb6330a01c4e0a6383c646ecd346bce68460c246734a5330a3c4d3eb180442760831d16fd64e7f5071480afe6f29401000100883015bb10f1af3b7791eb95b785a96aed6af91f8dbf8d89ac049b97ac9db91fd89e598d5da327a18b61c0fb98abe7d185075104c0de4c9dc61ab298942a801b");

        let outputs = ExportedTransferDetails::from_bytes(&decrypted_data);

        assert_eq!(outputs.size, 12);
        assert_eq!(outputs.offset, 12);
        assert_eq!(outputs.details[0].internal_output_index, 1);
        assert_eq!(outputs.details[0].global_output_index, 117545544);
        assert_eq!(hex::encode(outputs.details[0].tx_pubkey.as_bytes()), "3e45d72b841a8b76094dbaa26f9800aac4eaced3bc06122a3380bcf6c666d228");
        assert_eq!(outputs.details[0].flags, 20);
        assert_eq!(outputs.details[0].amount, 200000000000);
        assert_eq!(outputs.details[0].additional_tx_keys.len(), 0);
        assert_eq!(outputs.details[0].major, 0);
        assert_eq!(outputs.details[0].minor, 0);

        assert_eq!(hex::encode(outputs.details[7].tx_pubkey.as_bytes()), "1089fe4eba8d0dc478f56eedfbf8c33c5be2fa11eb4990b30eb09653e86674d1");
        assert_eq!(outputs.details[7].amount, 149763460000);
        assert_eq!(outputs.details[7].major, 1);
        assert_eq!(outputs.details[7].minor, 1);
        assert_eq!(outputs.details[7].is_key_image_known(), false);
        assert_eq!(outputs.details[7].is_key_image_request(), true);

        assert_eq!(hex::encode(outputs.details[11].tx_pubkey.as_bytes()), "3c646ecd346bce68460c246734a5330a3c4d3eb180442760831d16fd64e7f507");
        assert_eq!(outputs.details[11].amount, 39969200000);
        assert_eq!(outputs.details[11].global_output_index, 118075460);
        assert_eq!(outputs.details[11].major, 1);
        assert_eq!(outputs.details[11].minor, 0);

        assert_eq!(
            hex::encode(outputs.details[0].pubkey.as_bytes()),
            "3e8c52245d21b22cbcb90f95270a7937d4974d726209f0a41fdefc7f9df01fde"
        );
        assert_eq!(
            hex::encode(outputs.details[1].pubkey.as_bytes()),
            "2d58a6378c07f230148c11979cc6e3bec2719f0ec92de21f7fae02029ab025e0"
        );
        assert_eq!(
            hex::encode(outputs.details[2].pubkey.as_bytes()),
            "4ef323a52d2e048594ad73acbe5fb7e588b1859ec9aa02b2670f487660b27009"
        );
        assert_eq!(
            hex::encode(outputs.details[3].pubkey.as_bytes()),
            "cb8ab3c1b4dd10404a4a3c9275a7e2e1e9bf2e4edf1c84f61952bb97965573a3"
        );
        assert_eq!(
            hex::encode(outputs.details[4].pubkey.as_bytes()),
            "dd8c2a791056760d903bf06e7930585201e0bd20bcba1e720b85ad0e4d628e48"
        );
        assert_eq!(
            hex::encode(outputs.details[5].pubkey.as_bytes()),
            "9b65ada69049d73e4b049ebd50393410cdc05dad5314690d2b4a36628c4e2576"
        );
        assert_eq!(
            hex::encode(outputs.details[6].pubkey.as_bytes()),
            "6311ba60a0a8c636806e232db3e1ad7f79e26df3d24258e264e4351e47f4374d"
        );
        assert_eq!(
            hex::encode(outputs.details[7].pubkey.as_bytes()),
            "c6c6ff890ff7eb21b94c6339544b6710a62db5e4db1bf24d21ba7f58f9fedb4a"
        );
        assert_eq!(
            hex::encode(outputs.details[8].pubkey.as_bytes()),
            "2a00c725d3974b30064be120546d12c7e7e62cb65111c1370050d6b989fa2a66"
        );
        assert_eq!(
            hex::encode(outputs.details[9].pubkey.as_bytes()),
            "376805372f4021fe7659da328086775fca38388da87ec996d799a406f2590d81"
        );
        assert_eq!(
            hex::encode(outputs.details[10].pubkey.as_bytes()),
            "d5871df1287b277790ef31ac5f23dea30c7af13af54c821b68b7574b75210651"
        );
        assert_eq!(
            hex::encode(outputs.details[11].pubkey.as_bytes()),
            "00e82e301544c3c48fe4d204691609f65f4dae5d4faa0054a00bd444bfb6330a"
        );
    }
}
