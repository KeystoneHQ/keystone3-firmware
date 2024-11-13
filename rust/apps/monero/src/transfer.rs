use alloc::vec;
use alloc::vec::Vec;
use alloc::string::String;
use bitcoin::amount;
use crate::outputs::ExportedTransferDetails;
use crate::utils::read_varinteger;

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum RangeProofType {
    Null = 0,
    Full = 1,
    Simple = 2,
    Bulletproof = 3,
    Bulletproof2 = 4,
    CLSAG = 5,
    BulletproofPlus = 6,
}

#[derive(Debug, Clone, Copy)]
struct RCTConfig {
    version: u32,
    range_proof_type: RangeProofType,
    bp_version: u32,
}

#[derive(Debug, Clone, Copy)]
struct CtKey {
    dest: [u8; 32],
    mask: [u8; 32],
}

#[derive(Debug, Clone, Copy)]
struct OutputEntry {
    index: u32,
    key: CtKey,
}

#[derive(Debug, Clone, Copy)]
struct AccountPublicAddress {
    spend_public_key: [u8; 32],
    view_public_key: [u8; 32],
}

#[derive(Debug, Clone)]
struct TxDestinationEntry {
    original: String,
    amount: u64,
    addr: AccountPublicAddress,
    is_subaddress: bool,
    is_integrated: bool,
}

#[derive(Debug, Clone, Copy)]
struct Multisig_kLRki {
    k: [u8; 32],
    L: [u8; 32],
    R: [u8; 32],
    ki: [u8; 32],
}

#[derive(Debug, Clone)]
struct TxSourceEntry {
    outputs: Vec<OutputEntry>,
    pub real_output: u64,
    real_out_tx_key: [u8; 32],
    real_out_additional_tx_keys: Vec<[u8; 32]>,
    real_output_in_tx_index: u64,
    amount: u64,
    rct: bool,
    mask: [u8; 32],
    multisig_kLRki: Multisig_kLRki,
}

#[derive(Debug, Clone)]
struct TxConstructionData {
    pub sources: Vec<TxSourceEntry>,
    change_dts: TxDestinationEntry,
    splitted_dsts: Vec<TxDestinationEntry>,
    selected_transfers: Vec<usize>,
    extra: Vec<u8>,
    unlock_time: u64,
    use_rct: u8,
    rct_config: RCTConfig,
    dests: Vec<TxDestinationEntry>,
    subaddr_account: u32,
    subaddr_indices: Vec<u32>,
}

#[derive(Debug, Clone)]
pub struct  UnsignedTx {
    pub txes: Vec<TxConstructionData>,
    transfers: ExportedTransferDetails,
}

fn read_next_u8(bytes: &[u8], offset: &mut usize) -> u8 {
    let value = u8::from_le_bytes(bytes[*offset..*offset + 1].try_into().unwrap());
    *offset += 1;
    value
}

fn read_next_u32(bytes: &[u8], offset: &mut usize) -> u32 {
    let value = u32::from_le_bytes(bytes[*offset..*offset + 4].try_into().unwrap());
    *offset += 4;
    value
}

fn read_next_u64(bytes: &[u8], offset: &mut usize) -> u64 {
    let value = u64::from_le_bytes(bytes[*offset..*offset + 8].try_into().unwrap());
    *offset += 8;
    value
}

fn read_next_bool(bytes: &[u8], offset: &mut usize) -> bool {
    read_next_u8(bytes, offset) != 0
}

fn read_next_vec_u8(bytes: &[u8], offset: &mut usize, len: usize) -> Vec<u8> {
    let value = bytes[*offset..*offset + len].to_vec();
    *offset += len;
    value
}

fn read_next_tx_destination_entry(bytes: &[u8], offset: &mut usize) -> TxDestinationEntry {
    let original_len = read_next_u8(bytes, offset) as usize;
    let original = String::from_utf8(bytes[*offset..*offset + original_len].to_vec()).unwrap();
    *offset += original_len;
    let amount = read_varinteger(bytes, offset);
    let mut spend_public_key = [0u8; 32];
    spend_public_key.copy_from_slice(&bytes[*offset..*offset + 32]);
    *offset += 32;
    let mut view_public_key = [0u8; 32];
    view_public_key.copy_from_slice(&bytes[*offset..*offset + 32]);
    *offset += 32;
    let is_subaddress = read_next_bool(bytes, offset);
    let is_integrated = read_next_bool(bytes, offset);
    TxDestinationEntry {
        original,
        amount,
        addr: AccountPublicAddress { spend_public_key, view_public_key },
        is_subaddress,
        is_integrated,
    }
}

fn read_next_u8_32(bytes: &[u8], offset: &mut usize) -> [u8; 32] {
    let mut data = [0u8; 32];
    data.copy_from_slice(&bytes[*offset..*offset + 32]);
    *offset += 32;

    data
}

impl UnsignedTx {
    pub fn deserialize(bytes: &[u8]) -> UnsignedTx {
        let mut offset = 0;
        read_varinteger(bytes, &mut offset); // version: should be 0x02
        let txes_len = read_varinteger(bytes, &mut offset);
        let mut txes = vec![];
        for _ in 0..txes_len {
            let sources_len = read_varinteger(bytes, &mut offset);
            let mut sources = vec![];
            for _ in 0..sources_len {
                let outputs_len = read_varinteger(bytes, &mut offset);
                let mut outputs = vec![];
                for _ in 0..outputs_len {
                    read_varinteger(bytes, &mut offset); // should be 0x02
                    let index = read_next_u32(bytes, &mut offset);
                    let dest = read_next_u8_32(bytes, &mut offset);
                    let mask = read_next_u8_32(bytes, &mut offset);
                    outputs.push(OutputEntry { index, key: CtKey { dest, mask } });
                }
                let real_output = read_next_u64(bytes, &mut offset);
                let real_out_tx_key = read_next_u8_32(bytes, &mut offset);
                let real_out_additional_tx_keys_len = read_varinteger(bytes, &mut offset);
                let mut real_out_additional_tx_keys = vec![];
                for _ in 0..real_out_additional_tx_keys_len {
                    let key = read_next_u8_32(bytes, &mut offset);
                    real_out_additional_tx_keys.push(key);
                }
                let real_output_in_tx_index = read_next_u64(bytes, &mut offset);
                let amount = read_next_u64(bytes, &mut offset);
                let rct = read_next_bool(bytes, &mut offset);
                let mask = read_next_u8_32(bytes, &mut offset);
                let multisig_kLRki = Multisig_kLRki {
                    k: read_next_u8_32(bytes, &mut offset),
                    L: read_next_u8_32(bytes, &mut offset),
                    R: read_next_u8_32(bytes, &mut offset),
                    ki: read_next_u8_32(bytes, &mut offset),
                };
                sources.push(TxSourceEntry {
                    outputs,
                    real_output,
                    real_out_tx_key,
                    real_out_additional_tx_keys,
                    real_output_in_tx_index,
                    amount,
                    rct,
                    mask,
                    multisig_kLRki,
                });
            }
            let change_dts = read_next_tx_destination_entry(bytes, &mut offset);
            let splitted_dsts_len = read_varinteger(bytes, &mut offset);
            let mut splitted_dsts = vec![];
            for _ in 0..splitted_dsts_len {
                splitted_dsts.push(read_next_tx_destination_entry(bytes, &mut offset));
            }
            let selected_transfers_len = read_varinteger(bytes, &mut offset);
            let mut selected_transfers = vec![];
            for _ in 0..selected_transfers_len {
                selected_transfers.push(read_varinteger(bytes, &mut offset) as usize);
            }
            let extra_len = read_varinteger(bytes, &mut offset);
            let extra = read_next_vec_u8(bytes, &mut offset, extra_len as usize);
            let unlock_time = read_next_u64(bytes, &mut offset);
            let use_rct = read_next_u8(bytes, &mut offset);
            // RCTConfig
            let version = read_varinteger(bytes, &mut offset);
            let range_proof_type = read_varinteger(bytes, &mut offset);
            let range_proof_type = match range_proof_type {
                0 => RangeProofType::Null,
                1 => RangeProofType::Full,
                2 => RangeProofType::Simple,
                3 => RangeProofType::Bulletproof,
                4 => RangeProofType::Bulletproof2,
                5 => RangeProofType::CLSAG,
                6 => RangeProofType::BulletproofPlus,
                _ => panic!("Invalid range_proof_type"),
            };
            let bp_version = read_varinteger(bytes, &mut offset);
            let rct_config = RCTConfig {
                version: version as u32,
                range_proof_type,
                bp_version: bp_version as u32,
            };
            let dests_len = read_varinteger(bytes, &mut offset);
            let mut dests = vec![];
            for _ in 0..dests_len {
                dests.push(read_next_tx_destination_entry(bytes, &mut offset));
            }
            let subaddr_account = read_next_u32(bytes, &mut offset);
            let subaddr_indices_len = read_varinteger(bytes, &mut offset);
            let mut subaddr_indices = vec![];
            for _ in 0..subaddr_indices_len {
                subaddr_indices.push(read_next_u32(bytes, &mut offset));
            }
            txes.push(TxConstructionData {
                sources,
                change_dts,
                splitted_dsts,
                selected_transfers,
                extra,
                unlock_time,
                use_rct,
                rct_config,
                dests,
                subaddr_account,
                subaddr_indices,
            });
        }
        let transfers = ExportedTransferDetails::from_bytes(&bytes[offset..]);

        UnsignedTx {
            txes,
            transfers,
        }
    }

    pub fn sign_tx(&self) {

    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::key::PrivateKey;
    use crate::utils::PUBKEY_LEH;
    use crate::utils::*;
    use rand_core::{RngCore, SeedableRng};
    use alloc::vec;
    use core::ops::Deref;
    use curve25519_dalek::edwards::EdwardsPoint;
    use curve25519_dalek::scalar::Scalar;
    use zeroize::Zeroizing;

    use monero_clsag_mirror::{Clsag, ClsagContext};

    #[test]
    fn test_clsag_signature() {
        const RING_LEN: u64 = 11;
        const AMOUNT: u64 = 1337;

        let rng_seed = [1; 32];
        let mut rng = rand_chacha::ChaCha20Rng::from_seed(rng_seed);

        for real in 0..RING_LEN {
            let msg = [1; PUBKEY_LEH];

            let mut secrets = (Zeroizing::new(Scalar::ZERO), Scalar::ZERO);
            let mut ring = vec![];
            for i in 0..RING_LEN {
                let dest = Zeroizing::new(generate_random_scalar(&mut rng));
                let mask = generate_random_scalar(&mut rng);
                let amount;
                if i == real {
                    secrets = (dest.clone(), mask);
                    amount = AMOUNT;
                } else {
                    amount = rng.next_u64();
                }
                let point = EdwardsPoint::mul_base(dest.deref());
                ring.push([
                    point,
                    monero_primitives_mirror::Commitment::new(mask, amount).calculate(),
                ]);
            }

            let sum_outputs = generate_random_scalar(&mut rng);

            let (clsag, pseudo_out) = Clsag::sign(
                &mut rng,
                vec![(
                    secrets.0.clone(),
                    ClsagContext::new(
                        monero_primitives_mirror::Decoys::new(
                            (1..=RING_LEN).collect(),
                            u8::try_from(real).unwrap(),
                            ring.clone(),
                        )
                        .unwrap(),
                        monero_primitives_mirror::Commitment::new(secrets.1, AMOUNT),
                    )
                    .unwrap(),
                )],
                sum_outputs,
                msg,
            )
            .unwrap()
            .swap_remove(0);

            let image = monero_generators_mirror::hash_to_point(
                (EdwardsPoint::mul_base(secrets.0.deref())).compress().0,
            ) * secrets.0.deref();

            assert_eq!(clsag.verify(&ring, &image, &pseudo_out, &msg), Ok(()));
        }
    }

    #[test]
    fn test_decrypt_data_with_pvk_tx() {
        let sec_s_key = PrivateKey::from_bytes(
            &hex::decode("6ae3c3f834b39aa102158b3a54a6e9557f0ff71e196e7b08b89a11be5093ad03")
                .unwrap(),
        );
        let sec_v_key = PrivateKey::from_bytes(
            &hex::decode("bb4346a861b208744ff939ff1faacbbe0c5298a4996f4de05e0d9c04c769d501")
                .unwrap(),
        );
        let keypair = crate::key::KeyPair::new(sec_v_key.clone(), sec_s_key.clone());

        let ur_bytes = hex::decode("4d6f6e65726f20756e7369676e65642074782073657405a7e1735bc5193e2f48805bc5ab08fb85fd6a12dd204abcac0a6e6e88e96ca4874f353662c4c4b15171ccc1825218707786775e656f6ec518deddb097df40d238fdd5faf200155bb24d84e2fe90d4c5128f335897e84d7c337c8bcf0a216315ca8ed87952961ac8c12afdee44bafb7007e391f48986031cf4cfb76d04e428a6c7626aa0e965eac3cd70164cef472b5d0c4abe143c3ae87a95a37e6e73b70077da188a84dcf926640f3ba19e3ca3ad024720884cf765f0b899a86dfa84cdfee121be873661791d6b1bef021fcc92afcb3291e48603fb73229a7a01e9a31c98362d4074a58b93df9523b3d3eb73fb57227c9e4cc60b368fe2fad79257651cd2577e3f747e0dfea360343c11bc6591acad2f3d7fddab1ee3200740ecf895c9805c7b0b6867c150a960828e6e3596e053b11f56ad3d4a313cf50d50fcdc49ee655814e2d81fbc07ac27870404f0e9a63f0580535cce2483b0ae0e90e25e555d8f20cd1f7023df8b050b4662f03044bfc4ecbc8d673aac85113ba96387f568a258df2c0263f4c4c1ff4b78ee320d6dbef1448a0b9d9533b97a0404993d2fc7360fb6c18ec639e4bef7db9a2e7d9c393d337577f249dc7f9fa48857aefb46622afa625a39a26e7ac4577ed2d069b62e7a4f81d184f1b36b86203f02a0541a2afa790b78a9f4dcb6467627383511ac21171b6447e9f6c066b8d05767bc7f00e1605810651aee4b183027a996984a37e70f77e6202d9c815051361bc65d49ac7dfa3867c3e002979b7e50a8b829a10dafd7a0025fa82f755d93ecf2d862b37dab1094d7dae29312e825981e98212b56624e949f9740206ead754c7deffc46499d625ad8a48845446cec6882ded7d214d9150062166b8c3e9ce7f8b9c3c8e716a4163dd27c2dcc57b623f1ba5eec7fc87867229729531564852b3e4e8cd6810b749fe0899a87098a3f19ba901369d411b04ca1afd20ab2eae7f65d09c7b006b8a645e7b7230f7666005c55a378648edffa38982b245de538b5e74573f918d74900f6e5935cd0ec34c4049a40dff9e9049b7e33fd6505ea5a40ea3430dc1d346c933d5ab2d0073e1e39ac9f8ef21f34b2780b3eef39a506dedda8de1fdebc55a3c7cd353838d12d8fa39c4c7203738758f5ab24f3679cc864eec38a2f2d4b0e1bf897a92c953b09b5c79a639ad0d2dd7d387e4d84d0d4a9df6153591418f041b47a43af7caf99dbbec315fd96ff576500b63e53eb015da30bc3e7888cdf62d9d30c915db93f9939576effbec8eaf56e20acff0f9940e1e7938eb7349a19acf2e1e62a592c519f7f5950570915972329fd86a9c6fccec6fe6c91a2df6623e2c2b47033375aea25cc558c240ab54fbb8b698f30ed8b4e2d17716499120ab09282c9ee98d91c340bdd11422749ef1442601dc9479f29151688bc57bc7ecdf6398330e4fe483693e488873948c5f6c5cf3f60a3205884992f09c53db145628a75870580ffc033baf0112f1265f09736c721ef4538ecc997c89f990b25a54bfc5c3fcc2ce0787adc39743808b7e67ff12315e3954094bd40bbf7742933d51011d6a3745e885c21843a4c1dddbc6d345e0573e356b783a46c77ed43aa3ffe7c5a80385a02ff157d0608b424c3d9b65f4007bd149a84334b2987e9933fd202a468b92ec62b74ec9f64e19b7a2d1edcce09d8b9570c25ee1bf2e0d236ce5165376ff0a04d18f262d1d7ac48adcbc1795f8e8ab6ec2da0eabfb0d9235e2f3122bff90f94d78ef00c25949f6f44e2cce4e2ad159f50b3a9cae6691301229458c488d29f9cfe30e98f441067b083754f1187e5fbdfa3216b8468bf068c8b01d0cab4a5a1d22560fd79582df4f21182bca76931a343e8279738f6e5ca6bbf5e7e6df9ab79d1c942e2ccf5236bb263ecba0440a06ae0862b6b297eadf39c1d8b9ba2a5fb4c6abe868e2a997bb6b2c586c1216d6d530a4eec0467bffc9e803e19a03f2d07819ecb2d70a10bcbe1398367c869263348b37dec000009661b0c2c4f42e28c7e6f135d9fd8a1d2a4c2179b70a4adb7e97995e958e6ecd2bfcaffd64cc0fadb72aa461e660da51b879b31593f62ddb0d59437062f2eb469eb4074afa4d15e8f3dfbe128febe7297553d7964037777c1bdf5a19d1082ac8bcac8a931eedcbdc78d6817f0dce6a31e91928c6218426dc71850c15448aa6822f996c347d3cabe2abc75f4fb5b965bc4a475703fb2dfe3fce13ca334978ec1391c0283e56c7af4ab533e2fafb4c7ffd0cc7cab0d66acf0cadfbef904484210481fd6c26d9c29ba1e7625fe9c8e8141e6e526158479b4d9f96b280312900f953a9d0f9fb2bf0e333927379c25d003d57d56fe7547d5ba031eedfcd19268653982a074eb22e31868dbd7c2087111ec93c18cad41ca44fd0c5c6e3443058a1150aa1b80ccb91c27520835ae6f50f0e0e89c6918ce1f4075c2459b2b68ec7146a235bb33543759847403f1cc8ce600e205453ae5a0fe9a49fcf121ae1496be8e4bfd55c47358314d468cc2146a65d8a97b55e9146d18d67cffefa49bef7d81896822932c1fa5e83167b3c3224f9f2d1a35728f616f08b289a0c3c435ba13b1d7adfa7f6f9a1b087749a1235eb4691398474656937fcfe001e667be01b27165a1347349d6006628f0bafa412858d64484b93063ae779541d729404bfe7388064148cd3a02765c1ce4a1ec2628eabb45f5350c6154b1701e65e1ba57ae131a1f94cd33c42da619486db9719054a43300667b9cd420a89aeb3a20c345b1a010fc26fd30bd4aadd80c79ff91777d71bb83e7d3d1efaf56273aaefce2a19623669e99b181e88f57c4a80aa97a6944ab0016d631cfe1a087c3e3aad11a1a362801831ba8642699bfc8cbdaea2a9695097755e7cb9fcb3d8f93a342dc1b146f0b12d25c6e94116a62438e44f2e57628f1045742689b7a387a7ea1fb8e987a104d03684a2be99ad99fca70956e97c61ad40e95e837ab915c8f750798a9aa0a4bedea35db8aed1f297c17c95cc194abc270e0e9e27a324bed200c5be3acf52392ea1d7727c3e762f1d31d47f7cd955b57f6e11d360492ccd73c851da2f737d0b95ebfe64763e463bf0028d4ea6231b3829cff2bd522fb6598bcd82eb7d8ad96be1ef712a17d98bb656d8b1f50ea043b58b838be07e56ac094a18d0ce51c1276f355f58cf80b34c25cd87be264c7bea1466b6997692a9cafa0c24e1454293b5eb834323ecbe97d02359c00c2ca4b1eab55cbd3deb247010bf8ef5bb948f98cb17ec64272eff6e23485e8d691667df7cfb99f92f34f13398d99134eb9f1129e04a93fa1d284a7a85e9f4891336239cab1500406239bd66a94f9f5633bcee8cd9622a8ab86aff3c8aea98001aee24644193b8f471b8900dc2ec5af35539dab005203e7d295ad330749a65444c3538bf23d9dbf5c208b0500e61415ad969a7525528b8d73f081e1b91da14978b08817ac0c8b38da330518d74da7cf6decac4dc01c4f0d60dc8bb5b27f5a63d4dfbe4b5e08e9e2a7c4071eed49dd24ad91157ce1dd4c9d1b2f1dbe05850d3b57825e5e3647c0b1dcf357217f61c7cb4b0cfaf6203528d675896df1561924e004176095331576e4b2eda762f2412a300a4d5cf7b33623a08ccea75fc36f902a41f196df4db8a7bea107270db355f623326909bb10853b28ca3816e4555020dafdb9fbff2f517094e90552b488aa0862825a5d643f028cb3a30406f89902da9f624403fc503b5c8b415a133824de332f62d3450f75204c70a0ef32d01e449463674537934e54655742930f45e470f221f767bb8a2be685dc3f728db256316bfc55c275993b7d47029bab88aa2766b1d1ec70c62091d10d2b8cc5d605f8aedce37767499364df2484f23529b9eef3959e45f739e8bee477384be2daabd2a4996e208790aa1548dacb9731e7e1f8352b7736f4d133b766b37c5b06da13c7f455ff951f3eeb615a9a2b54e7a7d7763020f733aad2326269a2db6ce8bdcbe1171b4a3ce4b90271526ef2834f344ee3138c3f4f7719eb8c736c80eb213c546dfe3f341495f0f0d359f742f4057bfb48b7e0aed493544170c957ab971377461b26b3931c8d671e4d785159752cc1360dd3610db939a649d04a1659fdccb045c6ad66a29f1bf91e9e81b49650727e92c70afc00b31c6dab4d27b003b30c6fc2fb87204544cb78ed71bbcd6469d77f34b6bdcc3286b18303eb6838025947607a60c069818b6547e128489297cc83bf42f245e1bed61f6dd72b5bed1912e8b63a347a4fc2ca699d6ac7fd2f357220e7a1c89e2cd20591d7563dfc97a87ff7e09cee96f11449d0ddef8428e092dfa4eae67ac799b80d").unwrap();

        let data = decrypt_data_with_pvk(keypair.view.to_bytes(), ur_bytes, UNSIGNED_TX_PREFIX);

        assert_eq!(
            hex::encode(data.data),
            "0201021002ffccf51df0330f7e83af1eaf259628ec4194c92193b98ed440bd216ccce88113b12cfaf0d6860f4543adc321858ef40e7061edd18f6ae5a23e94d5e271ae71be8cc66c0a02b9c99e2d0e6d5af044fade5bcd842385e7adb0bab398c52fc91f539edcbfd9e025b1b7df2762c6b2bdbca1fbd8543c9d05078653948c812fb5007826a90d1f87b30f067502eacf8b30d8568103df3cdb37612a6010ee91b20b6a3f8f36592391aed19ad119d000bfad02f447bd7387c55a1ad3e6d7cf58995e7c9f1e205076e40d6245f1efdd613d2c02a5bbdf3569d3c51b5a17c60c02bcc12f9f962234c9a0650167cdc26b5697b5f4d66a5b8b136e8d5ec3f98e39d0c4010d04a2f84450881b2f32dabbb00cbe637e2e4bb3370294bb92366b55ecbe5aa922bf96a683d7a09956bd61da4119496539d4ccdd674ba81efbfb0ff6e6ec497ddb66aebefd901385ec320217eb3cd356ae7b0602993d90b58db4029cf39d36c95fd6b9fff86f7cbe4cebdebebec2a818553643dba70d9e62ef0208c9ee2d7adcd1acfcccecc37970d0f2cd3b3aec91b069d835b71764b76e34ce67b73237e202d4c6b237a557cad374873c0dd628a21f0d3af1881cc047a08c6cac00a27fac2ab50d21a04490a2e2fa0c471c5cb23fde1c3a4f4d1932f313a18078ae40f83e7abd2418bc02ecf98b388cdd6e3ad6fc3c8d4c820d59d9903cda595e2d892d7fdf8ac35f3a2e1661211d6e0e93fe0c670ffc2fed532e009de3c5a007a92bb6962b861b547090e076e23002b6879d38fa0bc15d7e2bcb76eb0049ea70a0596ab4bcbfac92486c4a3ef89932fcefdd474c6ae33edc039f8955612c43f371b91c152a1efef60ec4bcac1b5184364851ec0285dfa638376805372f4021fe7659da328086775fca38388da87ec996d799a406f2590d81c44eeda4ba85d9df31f5f2cfe715f290f242c121d5cef43ba9097595a9e0e0ef029ceeae383f69fa48f53131861fc908fbfe7f7efa0a1330176f8dfd1f23b4873a275364c88ed516c907da82991aefb3021f03d3629dae319ac346ca8a746ec0fd8b1421ee02e4e9b038f6f386b06fb3a495b90a8cb0f60b1a7cc3ca7f20efde206479137bcdd9a02e966716435bf262c4014d0c637657930e9024babd2927d4ef53997e06e6183224c402ac9db338abc7e23792b1243bacdc1822382237de7da008877b418a843f3502b2a2e82c16fd944c2d215c0fdf9178a42f218cc0d1641f412473807d772bc8228bddfa809802d1fdb43804959f101b13395e93dca7fd08bc1425555fbe84a6c6800d31047222249e7614461b908f8ab46411c6c804cbb729a52662c49021d8799f535b053906931e9fbd02e0e9c938947959c15ca0f38330c93039f6ead12429ac6ff663f92edfa5a02c6747d0f84d4b754db712aba1aa5e21f53b6aca078391bf97e52c82a9e08fce99b7e58658e502929bca3880b72e3786ef8e8216efa7e16f35c099dbcdbbb6b8614876b80e91eab0b454f7567c75519bf3192e49875ceffe19e7d2e55cd0323c8a1ce9df9ceade6364501709000000000000005559db18ebb605f831c5d2f697bd5a968b151276a1b7d13ce37fb7a833a30a1f00010000000000000000cc829c19000000017f362a8e06f8a25c576f2b16663bf7b15ad1cf55dc78d4fd5a4e8a78c364c20600000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001002c7aed136f4c61e84683f8a94f51a1bc2d6cc8eac77a171129ea5f2b99c8192e96397944bf5b4001e77e1aedbc192eab65ef1a50806b8a14a1203cb0355e2646a6e5dbc2d02be83cd375876a8641a7618d049bfafcf61f53e4f084a58e0fab2fd7dc3e076ac43ebf5c3b59a3dd40ac2f568d192ff03e55a2e7b21d245d6772f7e2d24681ae71e06b7780294ce8c38c6533970ef53a0e9886281c096b3633c447af39aeb68d0b9db07432e5eabe3f27fc0530c76843b356a55e632446c465ed91d0585d8cbef8e3314498dd0252e3e028cfd9d38749973d1e5e80d6d646f6153544e4b3b74985f92bd762390651d9ef9eb666eecc83ccc1d9673b8948bcc35e5e0760a8fd1252e24eae323fcd4f18855d665657c02f19d9e38c7b78775c4f65b407e3e4cf73256c180dbf537638b79f13ecd0bed47d965758276a31423c1c29c03b00248f636dd53ab92684e031bd126cced0c9f16e840e4ef02c3e0a638d5871df1287b277790ef31ac5f23dea30c7af13af54c821b68b7574b752106519f49206b529458d9a70c8022a89cb5a8be00e1d46ba082c23d7c40282cb8643d02cfe3af38ff7e26f1b89888e84f2f24550224adabb944dab4540d04f63d8b4e02c2d52ab866c3fbe1370385d56025447fea6f7087ce829f6be3111f7ab2e8d0daa9aca11f0299f3c538a2bcdd10ae858c2347169be8a642e9bfe555a4597e2b197f9c2374a1131a1e05a92b9999abe98a0c3f94e6366b60d2c76c87c953e19086246fb54980b22fc36302bf83c73814c89cda97bc7fa146fbf5e834ad86c3d4f61cab67b7e4e36b514fd55bc4c295ad00f91eb12c74a880342df9420f2cd46e12d9e998c48188f8985cadc865859502c5f9c7389365c7346fa1118640510400bad84625a12feccd63d4e536b30453082d88210fd65f5d46f859a5251933b330c2b6320431a6c659a9f1a6cbaf9e8916dab4b04d02c5cac838415bbb009f575ab135b91d9f0ac8525e8a36a3978571abe0d8257b9573b9f09b3c265409a362d0312fc74849fc48eeb989bbc2eae888a231195328904994cad002e6ebc9383b7301f177f4505bc2b87b225bd35d06721d09e29498e9480b3a74d0bd3cc2a443cea022b3f7a3157e83330a8391a71fb70c12de50331be015140f7fb214ad69029a88ca388e2c086af978a7974f25b02f26cd8bd21c851ee958986a40555f9ac9893376529c9304537d4b5411c0f4272682c5be95d89c41432f15f90592ac52f2676f913902a199ca38d825b0b0248bf78aa8c4d9228bbf5dbec15a3e0546d4d0bd5886607d3ed7e8a896cdb04822e670b3fa8ab37f6884464bdd59ef155b95090c14ee70a382e2cfee02cf9aca38fd9da1ee2eb030504882ec33e9ab8df98b9d9894432bd8c607de2ad318c85521af84ddaa64a79d8da6db3cf8f279e0971fe750a44d46ec68b39eb80065a179e302f98bcb380c448b1ae2746a1d9bac4e9f92876236730b72cb0f03206b63585760e8251bfd8beefbdfd3827614977cc88aded407ae47e9d058675efe0d7ba2c900e3cce7ef05000000000000003c646ecd346bce68460c246734a5330a3c4d3eb180442760831d16fd64e7f50700000000000000000000e40b5402000000013ef41a7f29672164ff35528f7b5f8ae625f0e24183fada1126246b83474efc04000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000009cf654deae54f4adb87a7f53181c143c2f54361aa0ca452140cbe9b064b5e2bf1b9570f11005d07992d3a8eb0748c6aef4bc4257f9602503d6664787d549b6df00000200009cf654deae54f4adb87a7f53181c143c2f54361aa0ca452140cbe9b064b5e2bf1b9570f11005d07992d3a8eb0748c6aef4bc4257f9602503d6664787d549b6df000000a09f9cefbe03d8c9116508740f1c9a6c408b327fdaba26366a4eee6e96c7d7a449cb4ec4598232610a6138729610587c4d889e0b4c193bcc7a778021ca96985f491d7357d67701000204052c01cd4075fb7461cf620ac282d214a024b01c20ea650a57473976c4471cb3e0662802090100000000000000000000000000000000030003040100a09f9cefbe03d8c9116508740f1c9a6c408b327fdaba26366a4eee6e96c7d7a449cb4ec4598232610a6138729610587c4d889e0b4c193bcc7a778021ca96985f491d7357d677010000000000010203070700"
        );
    }

    #[test]
    fn test_unsigned_from_bytes() {
        let data = hex::decode("0201021002ffccf51df0330f7e83af1eaf259628ec4194c92193b98ed440bd216ccce88113b12cfaf0d6860f4543adc321858ef40e7061edd18f6ae5a23e94d5e271ae71be8cc66c0a02b9c99e2d0e6d5af044fade5bcd842385e7adb0bab398c52fc91f539edcbfd9e025b1b7df2762c6b2bdbca1fbd8543c9d05078653948c812fb5007826a90d1f87b30f067502eacf8b30d8568103df3cdb37612a6010ee91b20b6a3f8f36592391aed19ad119d000bfad02f447bd7387c55a1ad3e6d7cf58995e7c9f1e205076e40d6245f1efdd613d2c02a5bbdf3569d3c51b5a17c60c02bcc12f9f962234c9a0650167cdc26b5697b5f4d66a5b8b136e8d5ec3f98e39d0c4010d04a2f84450881b2f32dabbb00cbe637e2e4bb3370294bb92366b55ecbe5aa922bf96a683d7a09956bd61da4119496539d4ccdd674ba81efbfb0ff6e6ec497ddb66aebefd901385ec320217eb3cd356ae7b0602993d90b58db4029cf39d36c95fd6b9fff86f7cbe4cebdebebec2a818553643dba70d9e62ef0208c9ee2d7adcd1acfcccecc37970d0f2cd3b3aec91b069d835b71764b76e34ce67b73237e202d4c6b237a557cad374873c0dd628a21f0d3af1881cc047a08c6cac00a27fac2ab50d21a04490a2e2fa0c471c5cb23fde1c3a4f4d1932f313a18078ae40f83e7abd2418bc02ecf98b388cdd6e3ad6fc3c8d4c820d59d9903cda595e2d892d7fdf8ac35f3a2e1661211d6e0e93fe0c670ffc2fed532e009de3c5a007a92bb6962b861b547090e076e23002b6879d38fa0bc15d7e2bcb76eb0049ea70a0596ab4bcbfac92486c4a3ef89932fcefdd474c6ae33edc039f8955612c43f371b91c152a1efef60ec4bcac1b5184364851ec0285dfa638376805372f4021fe7659da328086775fca38388da87ec996d799a406f2590d81c44eeda4ba85d9df31f5f2cfe715f290f242c121d5cef43ba9097595a9e0e0ef029ceeae383f69fa48f53131861fc908fbfe7f7efa0a1330176f8dfd1f23b4873a275364c88ed516c907da82991aefb3021f03d3629dae319ac346ca8a746ec0fd8b1421ee02e4e9b038f6f386b06fb3a495b90a8cb0f60b1a7cc3ca7f20efde206479137bcdd9a02e966716435bf262c4014d0c637657930e9024babd2927d4ef53997e06e6183224c402ac9db338abc7e23792b1243bacdc1822382237de7da008877b418a843f3502b2a2e82c16fd944c2d215c0fdf9178a42f218cc0d1641f412473807d772bc8228bddfa809802d1fdb43804959f101b13395e93dca7fd08bc1425555fbe84a6c6800d31047222249e7614461b908f8ab46411c6c804cbb729a52662c49021d8799f535b053906931e9fbd02e0e9c938947959c15ca0f38330c93039f6ead12429ac6ff663f92edfa5a02c6747d0f84d4b754db712aba1aa5e21f53b6aca078391bf97e52c82a9e08fce99b7e58658e502929bca3880b72e3786ef8e8216efa7e16f35c099dbcdbbb6b8614876b80e91eab0b454f7567c75519bf3192e49875ceffe19e7d2e55cd0323c8a1ce9df9ceade6364501709000000000000005559db18ebb605f831c5d2f697bd5a968b151276a1b7d13ce37fb7a833a30a1f00010000000000000000cc829c19000000017f362a8e06f8a25c576f2b16663bf7b15ad1cf55dc78d4fd5a4e8a78c364c20600000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001002c7aed136f4c61e84683f8a94f51a1bc2d6cc8eac77a171129ea5f2b99c8192e96397944bf5b4001e77e1aedbc192eab65ef1a50806b8a14a1203cb0355e2646a6e5dbc2d02be83cd375876a8641a7618d049bfafcf61f53e4f084a58e0fab2fd7dc3e076ac43ebf5c3b59a3dd40ac2f568d192ff03e55a2e7b21d245d6772f7e2d24681ae71e06b7780294ce8c38c6533970ef53a0e9886281c096b3633c447af39aeb68d0b9db07432e5eabe3f27fc0530c76843b356a55e632446c465ed91d0585d8cbef8e3314498dd0252e3e028cfd9d38749973d1e5e80d6d646f6153544e4b3b74985f92bd762390651d9ef9eb666eecc83ccc1d9673b8948bcc35e5e0760a8fd1252e24eae323fcd4f18855d665657c02f19d9e38c7b78775c4f65b407e3e4cf73256c180dbf537638b79f13ecd0bed47d965758276a31423c1c29c03b00248f636dd53ab92684e031bd126cced0c9f16e840e4ef02c3e0a638d5871df1287b277790ef31ac5f23dea30c7af13af54c821b68b7574b752106519f49206b529458d9a70c8022a89cb5a8be00e1d46ba082c23d7c40282cb8643d02cfe3af38ff7e26f1b89888e84f2f24550224adabb944dab4540d04f63d8b4e02c2d52ab866c3fbe1370385d56025447fea6f7087ce829f6be3111f7ab2e8d0daa9aca11f0299f3c538a2bcdd10ae858c2347169be8a642e9bfe555a4597e2b197f9c2374a1131a1e05a92b9999abe98a0c3f94e6366b60d2c76c87c953e19086246fb54980b22fc36302bf83c73814c89cda97bc7fa146fbf5e834ad86c3d4f61cab67b7e4e36b514fd55bc4c295ad00f91eb12c74a880342df9420f2cd46e12d9e998c48188f8985cadc865859502c5f9c7389365c7346fa1118640510400bad84625a12feccd63d4e536b30453082d88210fd65f5d46f859a5251933b330c2b6320431a6c659a9f1a6cbaf9e8916dab4b04d02c5cac838415bbb009f575ab135b91d9f0ac8525e8a36a3978571abe0d8257b9573b9f09b3c265409a362d0312fc74849fc48eeb989bbc2eae888a231195328904994cad002e6ebc9383b7301f177f4505bc2b87b225bd35d06721d09e29498e9480b3a74d0bd3cc2a443cea022b3f7a3157e83330a8391a71fb70c12de50331be015140f7fb214ad69029a88ca388e2c086af978a7974f25b02f26cd8bd21c851ee958986a40555f9ac9893376529c9304537d4b5411c0f4272682c5be95d89c41432f15f90592ac52f2676f913902a199ca38d825b0b0248bf78aa8c4d9228bbf5dbec15a3e0546d4d0bd5886607d3ed7e8a896cdb04822e670b3fa8ab37f6884464bdd59ef155b95090c14ee70a382e2cfee02cf9aca38fd9da1ee2eb030504882ec33e9ab8df98b9d9894432bd8c607de2ad318c85521af84ddaa64a79d8da6db3cf8f279e0971fe750a44d46ec68b39eb80065a179e302f98bcb380c448b1ae2746a1d9bac4e9f92876236730b72cb0f03206b63585760e8251bfd8beefbdfd3827614977cc88aded407ae47e9d058675efe0d7ba2c900e3cce7ef05000000000000003c646ecd346bce68460c246734a5330a3c4d3eb180442760831d16fd64e7f50700000000000000000000e40b5402000000013ef41a7f29672164ff35528f7b5f8ae625f0e24183fada1126246b83474efc04000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000009cf654deae54f4adb87a7f53181c143c2f54361aa0ca452140cbe9b064b5e2bf1b9570f11005d07992d3a8eb0748c6aef4bc4257f9602503d6664787d549b6df00000200009cf654deae54f4adb87a7f53181c143c2f54361aa0ca452140cbe9b064b5e2bf1b9570f11005d07992d3a8eb0748c6aef4bc4257f9602503d6664787d549b6df000000a09f9cefbe03d8c9116508740f1c9a6c408b327fdaba26366a4eee6e96c7d7a449cb4ec4598232610a6138729610587c4d889e0b4c193bcc7a778021ca96985f491d7357d67701000204052c01cd4075fb7461cf620ac282d214a024b01c20ea650a57473976c4471cb3e0662802090100000000000000000000000000000000030003040100a09f9cefbe03d8c9116508740f1c9a6c408b327fdaba26366a4eee6e96c7d7a449cb4ec4598232610a6138729610587c4d889e0b4c193bcc7a778021ca96985f491d7357d677010000000000010203070700").unwrap();
        let unsigned = UnsignedTx::deserialize(&data);
        
        assert_eq!(unsigned.transfers.size, 0);

        assert_eq!(unsigned.txes[0].sources.len(), 2);
        assert_eq!(
            hex::encode(unsigned.txes[0].sources[0].outputs[7].key.dest),
            "8cdd6e3ad6fc3c8d4c820d59d9903cda595e2d892d7fdf8ac35f3a2e1661211d"
        );
        assert_eq!(
            hex::encode(unsigned.txes[0].sources[0].multisig_kLRki.ki),
            "0000000000000000000000000000000000000000000000000000000000000000"
        );
        assert_eq!(
            unsigned.txes[0].rct_config.range_proof_type,
            RangeProofType::Bulletproof,
        );
    }

    #[test]
    fn test_read_next() {
        let data = hex::decode("02011111111111111100112233").unwrap();

        let mut offset = 0;
        let first = read_next_u8(&data, &mut offset);
        let second = read_next_u32(&data,&mut offset);
        let third = read_next_u64(&data, &mut offset);

        assert_eq!(
            (first, second, third),
            (2, 286331137, 3684526137126490385)
        )
    }

    #[test]
    fn test_parse_signed_tx() {
        let sec_s_key = PrivateKey::from_bytes(
            &hex::decode("6ae3c3f834b39aa102158b3a54a6e9557f0ff71e196e7b08b89a11be5093ad03")
                .unwrap(),
        );
        let sec_v_key = PrivateKey::from_bytes(
            &hex::decode("bb4346a861b208744ff939ff1faacbbe0c5298a4996f4de05e0d9c04c769d501")
                .unwrap(),
        );
        let keypair = crate::key::KeyPair::new(sec_v_key.clone(), sec_s_key.clone());

        let raw_ur_data = hex::decode("4d6f6e65726f207369676e65642074782073657405d8e6accb90ce0743932e3ffc2bfb6a10d5112794bd3d96cae89bd04125914c4dd36784902eeef6d63c42bd211337a263656d56420e2f10c97d0deaa9fca47ff30c65b169af4dd719e0bfcb8ae5d7e53d66c48c93985fe1e83bdbcd0b563704e92ec47beadd3434c1a1be231d069c9cd933448789dadfc2e96570c05289fffab3d9edb6e8eb927e5f66e69921a5acafc59c337ddea3b4351eb9ab46ed1c4dea96c71d8b899b624eb58e0f251b9fa73035ab9d02a668232b6cc1c046c1ae44476706d32191dac73704c1990d5542f8002d960c2f8a02f5de97b13cf55d3339998a6e0a6d97789d4186ab8a007970ca33c9730c436adcdeb7bfccff1eaea9b4d83f92838d59b2cc059d2c5f537413691ac32477428601698c158ea35f694f0a3f093eca67855b32cf3e71e0c956797de75e893828e071bb7aa88fe9915b4ab13f6078d8e886cebe8502b47eee2e47a3063a8ccc3346615de25091dec1cb10b7619033e8fbc3a41eaa5c119f6013d731e7128fad8f7e0c82045be5992f8a105c401a5ee2270ed1d9ad8750380b976791c8c0707f3ea6d5e83ffa3f0936e1ecbc996ed0fd457bb524486414f4aec85ea247bff1cc9e81b51d3ac7d9e680e075956caefec6c64d754fb059a82bf7631fc729cbeaf1febc342059bea77e78157ccccb00f168926f2b558f89ba2771bbb543b4fbcbd81d0add435bf02228dadc6d82d88054e6f4487fb042361c67340d719887b6e766257c8b1f9420bd8b0d17b02d7897b30e31cedeeacbfe34bd650a433b9b5cbddfafd6c377eb61fa4b6e29255a31be411a9bd1f1667982c8384c5f7ff1f8a4672000976631c7e23e317800f36cd71f96e70b497e732ab57f2ae476fe296667fe614c1281bead7ede65ef2ccc5c53c42c2e135d1e23552761c189838cd05bfdca13b92c0e696c8ebf571cc4b1563811399e854bf44f6dea17434eba2186ee1ac87adfcd2de5fed3da6c873995f1af47b54227681e75d6ee140f26078bc5e342a7e3e0146d32c4e28acf2b1310e31f0b8c3c2ea7ab9429bd9034a80793e0f97e00069f8f7fb6a390aee60428fea06d017e3f22d202f0fd81bca4dc910339aad30223b81777c6e5eb99478072b0acf3a489cefa9c3c1ab79513d75ec7409f1d7626ee2f23457bfa40da84df4ef3aaaf5d12581ca698cff90a45115fb77bea48622b8d7fc38ac8a1322e47ca8f1b7727c1f1325a84ba044d51ad8af2e3f8a0f5f2ee2cfb8f7788c8e967d53d85f7d6615c111d2ad768b485647142bbb799933965f50722a9573139cc01ad08db8f9f02d4edda659327ee6d76a2b2ef34b62d56bc1d72f0b28ddd2beec0c3600a0ac5ddb88facf3f2aad70f3709a4076c43bdd5d740da8c63600f2e7d6f5e6842e4f3bb5254a97247743bf480332fadf0bbcbda348faa865b1bfc996d38db6a332449874800294ad065186882957098cef5c12216e35116c3ce79af74bd689a6602b2969a3a20c781b5aae62e4da59a9f89c301b7170c9d6ed5f133c66e73129c08b4456cb725d3a64151c529d99d99a235c76cedc0b98d139ccbc495a08f346abba4ba79fe36f3f5c88c1a71854bcb96a02c9be67f65a05e89138262582328fe47055d075ed7344462ba8d25f30101cde11854eb82e6f4f892d5b43c2e25a50f992e4330383fd774bd319de80b61900ea92cf02f8548f5d1ecadcfb28c542d35cdce3a405cd13fcadc21762045a1a47fa427cfba6b779689c49d6d4828d34fe2721ba95a08ba84add647214f873995a7a677b6ff51398e8e393bd1d52905b71c08ca79ad564b6045d862c9f72005afb5c39dd82827558bb2a58d3ae0588869c529b6f8c6be65cc3b630c958c6cbdb6e066aa5b27c501398e9533d98f13bccc10dbbc74a76bb8b42da45a0aad249c6eb42ddf90598ddfd7072a9cfbe0a253da454b942c4d0f7ef78cdcdb4f131ccaf00198ec02b7fbb332e5d0a2dbec44b84110e8dfff612ead04f45167387001d6d321a10514d4b4816f1e0b1d4d2d5cda757707562ec6da1caa88a18caaf77a991d5a48e8f5b1ea054ee847ec966f2fc816de3dcb227740dafcd0f8c3127110d8afcddc26b2a0997e152ba8ea21e2445be7ee31227c162edaeb8a344aeaaedcadc5959fcf91c7a6120fca8a7ece1eb1c9fffa7e08d59f7cd8e977aa26a1ce0b7712063711948e457246663c3dc669eeac73c64486a861e0302cad208f98c87728e1e87f3c0686d50bc9d3429a30689706a94062293e40fe81c7b39ed6ebbbb97a296e7d6d17f97bf9f7f039e7828ec53f051c83b2313f792905b9a4155c9dde2f489f358ffdaf9a2d78888ed159db9fc27b51015de050dd56be546eb01284a7fe66d9811c1a5627f7272ca408cc4a1e5b1a9e305c991c57628331a5a79c26fe352fe160ac1d7a5a2aff2a596429dccb6231468e66b8e98f4859f9d0c6eaaea3b55c547e47699867d13688f812926774c69a5785633304bf1c06de05719a83114e986a37d0628dc63c10af088daa3b0d9f3e0e714ed83ab36d89d2c952dea957e5b9d90389361150a4501f206b579a714a97c196bf20d0ad534afd1d63cef28899a03ea8665bf7f543a2abc4e132a4ea0138cab8a28b2e1fc6bc6445fa6206863d7e445e1b4f6b378e9d07ba3b56a973c71614ad3c69e6ea12490abc16e699cc52598ea375848b5463b6afe44f74fac688bc8fc19a7fd7c4dc465e84b84ffd74f65a35488882b4f2eb122f6da5654761362a09c66de97b4554967c1c24365014ec370d15a45ee964002f0f8abfc7632b1f1f12dbde11289e0f5563660b4f23360bed646487a69b0bcfc26c60ca7059718468ae24c3c5b4890d2045973da6e56140e7d05202321eccba7808c8dd1d7fccd33b1a7ef4e26618faa22c96eb728b4c1809e2d59a0eea62b6309920dea6038a3e56736804e4de1b25a4e80de4d6a1ee0a51b070766b009845c6650b6cba95a98d0352788dc3b4d8dbb64b408f1fd6a67137441bd2a118a8d849fea4b53f26d9265e2fc1ad442b64d92a3dc3e129bfe2f554f92a1cf000404fabbea1c4c78de225b0d050eb99a5426a630a8b5f8a96107aa7c85613dc1206255bf02d31c32e004765a88a673170e093e63f73ed3f2343e21fe80617161656b19ac4a7ee2c915fdb0b653f7970806af74b74b0c9f536eb7947fbd86f611812660f3cb18d6751b91ed91a6cbcaf28ac7ed23961166df6c673363a537b1fdf299466d71c118409c539a95b52a083e877ef6f9d7f3e0d129968bdf6b6c004b5bd177a276cb67d085dc8bcc9e3785a73f933f3831cc733a90400cf89c13fcf7344fd06d30849175776ab789ea406f3905575cd21a03d676dce0ccd3b9176296522d69b92a8e49a7e8ce6a5ee893954ef66e29d1746d9cc6f0ea46d93b7bded4fb64d8a63fca7a6f7709495ea5021dc83a6d4c9d5490e0df888a34bc40ee8bdcb3c85afba6308570578d9d2029af923fe77e693b518670219f50ca85f398b0b10e32e02afd57b3a6931aab6f33a113c6fc141c6caa60436ca916099c94cc415b516a177b5ee0bf629ede8b9387370d02e423836606ad3d5cbd4f547afa3e0cb2a8afbc2036a58632d5d7d16a47e05a60fbe674e0694dab5d27521a4b02f5d77375c603dfd63ccdd1fdc0a847171b1ce532f0d4180a74862c5d22efb7ba85fda2fab694f7d74015a32f6797bd3feb4c4117a8f46042f340f1712c8afb97c1b331e1c1df84ccbbbc2e6f94d204b834321666b1a414b77e9733a28457a1023e5549c9f79d01e12e20d76b291e951acc54eb8fe0945da1a1866fd3c21461cd4ddf4c75898b4017d59c315c6de1a398436fe5d3ccda6035377f87e615f9243f7285c089af3ba6cfbd113d84b7df7fdf7609f54e2d6a98802a88854c42eaed5f369e6bf73647af9c9f40885303e8cc95bddf65a8974829a42b5c3dd9f92e29466429a745aac736cb4ce6b3fbf14c9efc797d1989b73cf0a9dc629dea100828065b075b25ec45e008c44399e9e36a6ef8e024dd99d52ade5afdbd8dfec1fd1b0484f67b58300930ee46a4b3b3143d82252db544619b39ae289fa9718a2ffdad27f688ddaf95782986ac3a35de9c5c428834895d75b7f70fffd79907e93b4d698f583aa605e3bede4865292d72552344785d970469fafd3d178fb2486d9a8277abc77833e1fe89b3435e891fa5c14b2396cd89845edcdf0991ca58cbec4c20aa86352246d76cb6904f684a2144fe176f067de29d28c7f01d412dc2499e2329d8eec895846f3cfc2c34458a0d2c3f6cdd923db615c54c1874e6a7b76aae24eacaf1d3485798a89e83444b47e88a24d886c36320c639f8ccc85d8e795aa8018e8459c5d7b9cb11302ff8f09790760e93ad17e7885b6df8a7b2083d6f2fc58c3c75b19edd68e0c052a2f72a4e61fa54076973bc4f67861cbd2579fe9a466a2e26796b073abd2bddcf8927895e0e1c525e9a6b4ac8c2ca1fbb87847531a9c4d43835a21ceb169a52c12228afb84e623773b3bae056cfa55b964a9793254f4c5b73125961547eaae1a457a5b761ed90bacf050a2f0fb82262114832caef50ed2ae9086212e43b2600f969464fb4aed5372f6205410dfd578a6f10f71149c2d449587d7aea64c4ae62ddcdae967f13a588304ddcfe733ca9d1cb6f7412abd9834b30b7feb96de64cd0716ead3953d5f786c9aa57565c98b63e4c402d215e3ab7cd99d200e7597bef913cf226dfcb1646039e86bf4df79af479fd768ca016bf6d6166bcfa4bcff9de0eae3720b245680607e7f0d49971d37eb2788819e07e6e111e3f871f1dd35247d94788a227b98bfb20af58a6fa450006de4d9c87f2a5a7c8dce9c65bc3ae2a4097a7e84e3422cdb7eb566816bc559f51638b9a057bd0e6314c09212b8a0f24397b0ecb61cd20a313d58bbd05f729dbe9a625094fc684acaa756754cb10d68448d316dcf832dbc7a03308da1c5ee04e594a9b248cc03e1653d2576a8749ea22738abdaf3b5292b7f3c1b12257879f8fc2b84b3ebb1030fea42a2f0c5b19fdbc11fbf93dd82865a0219d6828f74f9432db0da022bffd4e2b4a0c6db750f3550e4341bf309dfa75bfdc35c177adbc7bc5be9a46b2d8307c78eba4c141b88db67cd216547381411420f90b1d615e63bbf86a530a7ce775becc874926294ce9e978bb10a9f1c7d74a4c53e63f03c57a4c124c292b40baf1eea8dac1d045e6fdff21c057b6b2c6d6fc553929d89d472a8bbb717f8e76a472ff55529cd0f323ea7f87eda7b75338eeca89008b74627f637f56fe4335c7f4e909adc7db670a8d63cd250f92a5c64f014a9855f0902d9083ba4bf2120418caadd698c021d3bd7bbe542c3f618137dcda70676a8b70acd6815d8e14214793b7db1995e4971ed73669d46ae206a18a45afce3558533cc1082989e696d450d1f67b901696bb286c884772ecb7c3e1fa1df0f484a34551d6402a0bc6557061a8a1dd611fdbf217377e966a0e6dfa0339ef776087256bfbdfc03f57f6fce0efd2e486cc28ee3c4f04056e36637d37130f14558824d9fd74f6d60a85f8ec40215c89f3c78f5924e30e0735fc89990dc359f50796936ebcd20623492d6a2614791db1e24138fb14d85195e1caec1754ab1757c5304119df052c9c8ebd2451514108e74d7c52de4d4727590fb72478b805fb22cb9845eeb36925db3a712498ad26238a719d70693714ec52f1fb65f8f33be9f25fed321470dd3c27779eb7b7f97e528f76155fb6f7f6239b6c1be3d063bb91ca18968101796bff0d715a9ae0159b646827d8896bea170919f9c505672157713a29f815e6714fd684385b54fc3304bef22184f525202751cdbe0f1fd75bc240090ab93784c932acd2c94126509de30c7e8795be434c5a5cb634d0ce8296b02465ed2d3771e18768f6c668f494e4e0ca9da6700892faef0895166c07a1ed0117a0fc320ec592d6a57107897f8158e042786d9b6bd50022bd46c91f8635c57011e036391025797c0d90e3f533957fc412f9589f7722a62221ebbe1811afd83b335b847574c0ec771e572035f0339eec52821d17d60d7769c6617b247a2bbb0c2770d90cf7dd64b6c635009c78000268b02171f7769a1c44b1e58a2462ac1b080e4922c711139066cff8a4ddcb19a58487937f4a05259328c39bf98032db93d96286cad94e21596678b16430827c4f4ca690a7de76329a636e7c53007e58e3625d4a7bd1302c519db7d41376f96af9d98823831c8663a0d5fe6760c77db0a973891b6a45724e24fb2323801cfbdb96dff8fa9a9dfb67b423edb4af1b1c3305f9be9fed27929b32cf7742e6fd3554b8561b8b7f6cccfea02752db9490738ed360d90b50503eee1792b37f871ff4322d0f490d303398e90bf1c092ee13afba1a6a8467561c5361337453ab849e18585523d8ab0c0b9da290d593f31b1dbc56631ae54bc52c164fcec94e31e30824eb5804d0b99a1b8f126ad570eeb29cc481f39de378654086ad7c999b282ad13401e07054880a5e027c4bdb3411522913627ba82a524cdd639e4790f1ec1a38c0a7bbd1a0c7ae1bd36c79dcc4976b3d9cf40fa4220d5cff0a130120d70993f9ddbef5ce65112ecf01d0e66fd17868964e8ffe04bb9e0f3e0362807d4c6bcc1928fb27ee44369bd47fb325aaae4fdb5902d46fd3a589d91331aca31174ef0f0637bf9ddb7719345237fb864b32f32649127c79e8907f4c8c373fb5b65ebce5bfca0ef39d91682c1bdb29e70d18bce4f85a803793f489274f6ca812027eab18f6a439cd3fc927894839480fb8f25ef2b32a35583a01f6aaec15bb0f4e70a451855ce00395e8c93a6b100e00ec23a638ac1ce9c2a31f72e8cc4fcb79c3397f3a4cabf54e2d5cc79cb56b0920fdd778a03e40e3f5302382d4511ec9fc179d8f6dfa21618701e008def456799aec88f5cec018da826907f44e8ffed5be0f6e7594f2c8cc760f1d5f7d64d843bf3df8f7dcb20c34227f12ad7d692a57718f85084ff13a9c6bc8bed95fa04e2da52de11d73f0308eed225b3a9d723f74791bb7cc201a03c1931a9c35efb658d0f4983e04da4f06289f4f2b606872e305d4b5328a0fe988d062dde787d0c5df907c60f9d6e79d1eb27f5669560b476c38749f252acc016b103829cd552d00e802b1b9859210cedc2ac96008d2984721a8be6d780b1ce001c3d14985fad75c9b732c12920fd92682923051d5bf3d28253356a0c7dedebc9b8746519cc66e8f8f09e2bfee7d78bce41156518594358f2c347a4a97fd26a74d199c788fe088f0f600f79607ff7b1bfc6bc230f72601ba3a6725ce09c78553446963cb948db17550e23e5d402cfb143ea1ab353f4700b48805a07944eb51706762a5c86c1658dffd2a5d185e72935aca205fbf23ca1c7eecd33eafbdbcb39479b23deabe4add47796480f78ea87024eec2ca2187dbe9b3d6174bae8d0aa37c2b4028c0620e1c7fb19ebf71422202ad09e7d8256375b8a91904a5f811b2425174e3c12324fe161a14402a8369770d63f1654d91bca983d215f424d97f7d302d7eb115be7962143fe0b52293ec892b6064d257310aa792e226a0092c62d0b22d15a0f32ddff7f5f4235f02786d4059c4538938659977a0fd9d8993d5233958ed672e11c23459b31fd889c8258c9f8ca761287ea886ce1d17e1d4170552aa75e3a28048fd23c4ffa245d0d62449a26b0bf36de358f9ca561007816785fd0e31c3fe3a156b1d4c65b53d161610cbf77beb982e3134a98482e30daa07b6e9ca63c29b5892a409b203e20275d98f23fb8573f5941011ef4474b5816db7f041c4563c4debe33be772b0dcdc5bb928b25d5317a7ff5b64707d6a523a10ed16bb827ef3f4ff6c951433dff4fa56686db44adc0676e66746f67f7de442a138c206ddb1176bf63d844fe03b7d67de50cdfdfa35a90e9448825e82ec465b9010371edf8063bc4d7a600b4501b20a698df5cf0e441dc3da492a38535343db6d1da025c3bf54796be9dd068425c27ca2368f5375cb7dd6e285d9df86909200c4c172a4db828d5ec79079acbeb20f99fbb363a099d649634ae9bc8805e332a9fd9cfc18bf2d4f7592b07").unwrap();
        
        let parsed_ur_data = decrypt_data_with_pvk(keypair.view.to_bytes(), raw_ur_data, SIGNED_TX_PREFIX);

        assert_eq!(
            hex::encode(parsed_ur_data.data),
            "000101020002020010ffccf51dbafca80fb186ed02bbebd305efff3288b80bb8d3940198b359ca8d11cfd709978f08c8fb01c8b302a5e0018fec14b23179b156c82006bba69721bc33f4319174dbb3434108f53dbb4f4482cd269e906c020010c7aed136f7d47bd6ca3ff8ae11e520d2c2088c8309ca8f16a6900186768051a1a101b41c8711ae01aa7178f5808a3a8b3f5ed7ad742ac8a2e90e9dfb39cada70a5ee03d84c9ad96dac5b020003747bf791553ba6957c8de603d1f339921c8b69dd920efaf580300c86933616a3fe00032dfefe16e01b65622d2ec3fcd1abef1bdea09094af19cb15038bb64f7c62c76c982c011a717bf7983763abee4f76432e4f2c434a75b7fcd6d8c630beea98f789a2331f02090173c7741df3f965a106e0c09e154b35527c942cf2ad1c742bf02cea93238d5816ab144a4b247636c77614f1469e5520d0111f76a0079071701d4af31f8c34b44c0e204d49e1ff08db07ce575ac74539bb1908210a05603c5abff7f39ca9016981f8aa789c23fce9220e573dc1e7047f07f339073345f5860bd4c3d7abdb05fdabc5957b574efebcf27452c6f206fcf65b40a7c76cbcb79dae19b4a2d86149cfbe091045d439a335317315f390dfd134310d83458b2348bacaae493d7d165d4e3ec7762878760e385f0226593936fd8e0ce47489497353debb5179e22b0c0d4c3bc22191375992e6875ee5db2ae285ab7ab62a67bc5fa17ff4ab786c1142052da064e9c0b77225c6b23623ffe54f32ebb5064ce00abf3974f1ca6e5dd18c0e07c8765e32052b5a1a54144a97413ee9a70be67c3a83bd79fd73b9c8a873dbc38681a24c03c2ec5c29e1caa0a72ee913f35debca805c5f63a3ef6af53ecaab44e63e0914baeec5152043102e1a8d001dd1c3581e0a923bed5ea8d73bedc15ec67135080947b708d5ddfc9196150493b11388891e8906d6f6b6dc48b6c5d382b2e0ff515b3d8230ee11bce8544139eaab86d79fb1fc5bc67462870d203e723bb8676d3042d284a82f68c5fda766347d78f5d9a07828b55d110b2ef8ac1030aac4490dcf02dc594579c010262ae45c1d6a3cd66a682ffa35ce206692b391633858750710a9b6e2c8a99453245fda4df7586434e5cd589de1a712b0417c8c25b9bbc3b4592c6246e5deebf730cb5b441f0cf8bc4b4430cad3e35f23405030d44f9c973162824abc24f905de5c57cdaca71d45085f573a78e13c69c17cfecb1fc03fcc7a4cbc92659b4784be1b09056902f9a9f921c9bbf168fe90aef95d5fd083d71bf979eb9537ff93c86679d50eb7a3b5cbeec24ff42d140be9f2d2fc557dcc7b8c7cd20fc886aaf961f7f7a1f580db5dbb55ca5ebd4dd87a35427fe0676af8e63bfa357d69b318fd04cb464f14aad4e5e9d1045fe95ecb6a4488905901b1ce792e2a329fd6fd0e37ce8775c013ea5d9806bc6a3a71b53d7109006cda70e6a68501029e07070cbc53031ba10adf552b8324284e6dda34409c9c571145006f6b34140aef44f57bea7ea6af73bce6cd4cec36e9bc2179c0f34ef92af8b9dd8cbdca090cbf555e0600ac3af852d25c31fefd80ec3924aab5392053c8dc256bb61054a00fb4e44b1e92937ef774a71be686769c3439323120f464bcf9b5bfa67d2f6f600a3a781cfbbbf5afa476dccbc8b073026425dd9b96eb53557f67ea46ac02031d0727813b986a6ab7e3bea515adb3bdffe56472b1ed3abb072d4edafe0ab9ca310f0ecc264e495dd14a6d53f8767919fa6f653d8be41f34949aeee97a5321896e037536f5d2b7681cbf0d3a55a2a522d6b74d2e03ab5fa91d7210deb5083d83c1046235e315768fe73dabc95987634f116f7d4e54665fe177eb15d226dba664db0855369764fd31d67ace43bb129624eb70a996ccee69ec3cf981c7174d3bf257043543ab665ce2af07137f6deabba3a53b8553a1839569c10b6be5a4c057bb990207ae856a5f63a9a656ea93be484f58f0b85da1267d856467092e6a537df94805e50f5f229a17d65fe40e882168fbf9131b1fba00050f1d2da77b3047e10c1c008bd002b7fd8e4ecaaa47ad5c17f76881d38e8b90cfd9d38a1764afefc3552001b1d4769ba25ee0ae574eab8beb31e75075f7023bfe6f3ef2e6538eae449a4e008ed0528819f20d4d7a865dba4dac4d9ebccad4d7935f73beaace9131721c2308b53922548017c2e012c55358487fe4e2374baa065b94b25e40b33ee828cd3d70f66e2f63b09e9e377582d082bb37b47173b9e32594e72cc63321bc1086b516055a6b96a387a35aaaaf8c7f2665a9e86667deb8293967a27b41c0a6104470660a02572958c841890c8425f640b22b5ca6bfe6dba2cf786b1b99ac3183a465a202510e5e0e223204ff308c1b213cc8068518df9fd590535b7146d53225fa4d2407f449b51e92a7fc44a39efb4f231c1e837899fd1853145746ae57ffc6a92dcd081899347c4f4fd3de45a2476f5d03a39036cf9478cb961ef1d1e889dd9b74a40b9b8ec99b9a924cec0332633f995b46c488a64df61df9b9be494d281d6f58be06a0c1f3623b25439943152f4083074505a8cf81a8d092524d993e70875ac9010be8f13b155277bd6afef4dec367f91bff86b8f5432d769e8b5439ca788296d906b67d0cc7ad6525c207ebf3bb7844c173b32da605e510f04f7087c8a1224da90abbb34a37f7d2b9bd906d46ed25a7471c3f8255da86ac3b743c2849e184146000309dd725ff45a2c70eb21a34664b5d96ad5cbe927ac51d68d4d524c70086cc092683014505a6c30457f7dc49f5775307fec3da8eb3c37c879dbb185c5bc36f03031199c9df65db6edf301baac90d7ed087cff087c0e331aaab5eaab114789e06d86e76768091ea1d118666b179b965d8e9739bef04a2119fa1761cd1baf1cf031a6a50e9226973e34ab2626b97c53e6c28441cf6e9a35c32da93d103196fc404f44e5cea0a10164eb96cb085d714ed701e3cbebf5ecfd937104b4196cf18da007f1c1c371e288b02371ac14b396d51269d2370c43d6fbe45a172d97323b6d8a59e9ddd3fcda156543893ff3136ef4c466af7d79525d1130a12bac2c7c88333115d7d3baa42db4026fc3b0df50cb1c256a55a1b4afc1d40c9c61ae0a0f5f6d2db000000000000000060a0a702000000000000009cf654deae54f4adb87a7f53181c143c2f54361aa0ca452140cbe9b064b5e2bf1b9570f11005d07992d3a8eb0748c6aef4bc4257f9602503d6664787d549b6df000002040586013c373962313536633832303036626261363937323162633333663433313931373464626233343334313038663533646262346634343832636432363965393036633e203c373866353830386133613862336635656437616437343261633861326539306539646662333963616461373061356565303364383463396164393664616335623e200100000000000000000000000000000000000000000000000000000000000000000100a09f9cefbe03d8c9116508740f1c9a6c408b327fdaba26366a4eee6e96c7d7a449cb4ec4598232610a6138729610587c4d889e0b4c193bcc7a778021ca96985f491d7357d6770100021002ffccf51df0330f7e83af1eaf259628ec4194c92193b98ed440bd216ccce88113b12cfaf0d6860f4543adc321858ef40e7061edd18f6ae5a23e94d5e271ae71be8cc66c0a02b9c99e2d0e6d5af044fade5bcd842385e7adb0bab398c52fc91f539edcbfd9e025b1b7df2762c6b2bdbca1fbd8543c9d05078653948c812fb5007826a90d1f87b30f067502eacf8b30d8568103df3cdb37612a6010ee91b20b6a3f8f36592391aed19ad119d000bfad02f447bd7387c55a1ad3e6d7cf58995e7c9f1e205076e40d6245f1efdd613d2c02a5bbdf3569d3c51b5a17c60c02bcc12f9f962234c9a0650167cdc26b5697b5f4d66a5b8b136e8d5ec3f98e39d0c4010d04a2f84450881b2f32dabbb00cbe637e2e4bb3370294bb92366b55ecbe5aa922bf96a683d7a09956bd61da4119496539d4ccdd674ba81efbfb0ff6e6ec497ddb66aebefd901385ec320217eb3cd356ae7b0602993d90b58db4029cf39d36c95fd6b9fff86f7cbe4cebdebebec2a818553643dba70d9e62ef0208c9ee2d7adcd1acfcccecc37970d0f2cd3b3aec91b069d835b71764b76e34ce67b73237e202d4c6b237a557cad374873c0dd628a21f0d3af1881cc047a08c6cac00a27fac2ab50d21a04490a2e2fa0c471c5cb23fde1c3a4f4d1932f313a18078ae40f83e7abd2418bc02ecf98b388cdd6e3ad6fc3c8d4c820d59d9903cda595e2d892d7fdf8ac35f3a2e1661211d6e0e93fe0c670ffc2fed532e009de3c5a007a92bb6962b861b547090e076e23002b6879d38fa0bc15d7e2bcb76eb0049ea70a0596ab4bcbfac92486c4a3ef89932fcefdd474c6ae33edc039f8955612c43f371b91c152a1efef60ec4bcac1b5184364851ec0285dfa638376805372f4021fe7659da328086775fca38388da87ec996d799a406f2590d81c44eeda4ba85d9df31f5f2cfe715f290f242c121d5cef43ba9097595a9e0e0ef029ceeae383f69fa48f53131861fc908fbfe7f7efa0a1330176f8dfd1f23b4873a275364c88ed516c907da82991aefb3021f03d3629dae319ac346ca8a746ec0fd8b1421ee02e4e9b038f6f386b06fb3a495b90a8cb0f60b1a7cc3ca7f20efde206479137bcdd9a02e966716435bf262c4014d0c637657930e9024babd2927d4ef53997e06e6183224c402ac9db338abc7e23792b1243bacdc1822382237de7da008877b418a843f3502b2a2e82c16fd944c2d215c0fdf9178a42f218cc0d1641f412473807d772bc8228bddfa809802d1fdb43804959f101b13395e93dca7fd08bc1425555fbe84a6c6800d31047222249e7614461b908f8ab46411c6c804cbb729a52662c49021d8799f535b053906931e9fbd02e0e9c938947959c15ca0f38330c93039f6ead12429ac6ff663f92edfa5a02c6747d0f84d4b754db712aba1aa5e21f53b6aca078391bf97e52c82a9e08fce99b7e58658e502929bca3880b72e3786ef8e8216efa7e16f35c099dbcdbbb6b8614876b80e91eab0b454f7567c75519bf3192e49875ceffe19e7d2e55cd0323c8a1ce9df9ceade6364501709000000000000005559db18ebb605f831c5d2f697bd5a968b151276a1b7d13ce37fb7a833a30a1f00010000000000000000cc829c19000000017f362a8e06f8a25c576f2b16663bf7b15ad1cf55dc78d4fd5a4e8a78c364c20600000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001002c7aed136f4c61e84683f8a94f51a1bc2d6cc8eac77a171129ea5f2b99c8192e96397944bf5b4001e77e1aedbc192eab65ef1a50806b8a14a1203cb0355e2646a6e5dbc2d02be83cd375876a8641a7618d049bfafcf61f53e4f084a58e0fab2fd7dc3e076ac43ebf5c3b59a3dd40ac2f568d192ff03e55a2e7b21d245d6772f7e2d24681ae71e06b7780294ce8c38c6533970ef53a0e9886281c096b3633c447af39aeb68d0b9db07432e5eabe3f27fc0530c76843b356a55e632446c465ed91d0585d8cbef8e3314498dd0252e3e028cfd9d38749973d1e5e80d6d646f6153544e4b3b74985f92bd762390651d9ef9eb666eecc83ccc1d9673b8948bcc35e5e0760a8fd1252e24eae323fcd4f18855d665657c02f19d9e38c7b78775c4f65b407e3e4cf73256c180dbf537638b79f13ecd0bed47d965758276a31423c1c29c03b00248f636dd53ab92684e031bd126cced0c9f16e840e4ef02c3e0a638d5871df1287b277790ef31ac5f23dea30c7af13af54c821b68b7574b752106519f49206b529458d9a70c8022a89cb5a8be00e1d46ba082c23d7c40282cb8643d02cfe3af38ff7e26f1b89888e84f2f24550224adabb944dab4540d04f63d8b4e02c2d52ab866c3fbe1370385d56025447fea6f7087ce829f6be3111f7ab2e8d0daa9aca11f0299f3c538a2bcdd10ae858c2347169be8a642e9bfe555a4597e2b197f9c2374a1131a1e05a92b9999abe98a0c3f94e6366b60d2c76c87c953e19086246fb54980b22fc36302bf83c73814c89cda97bc7fa146fbf5e834ad86c3d4f61cab67b7e4e36b514fd55bc4c295ad00f91eb12c74a880342df9420f2cd46e12d9e998c48188f8985cadc865859502c5f9c7389365c7346fa1118640510400bad84625a12feccd63d4e536b30453082d88210fd65f5d46f859a5251933b330c2b6320431a6c659a9f1a6cbaf9e8916dab4b04d02c5cac838415bbb009f575ab135b91d9f0ac8525e8a36a3978571abe0d8257b9573b9f09b3c265409a362d0312fc74849fc48eeb989bbc2eae888a231195328904994cad002e6ebc9383b7301f177f4505bc2b87b225bd35d06721d09e29498e9480b3a74d0bd3cc2a443cea022b3f7a3157e83330a8391a71fb70c12de50331be015140f7fb214ad69029a88ca388e2c086af978a7974f25b02f26cd8bd21c851ee958986a40555f9ac9893376529c9304537d4b5411c0f4272682c5be95d89c41432f15f90592ac52f2676f913902a199ca38d825b0b0248bf78aa8c4d9228bbf5dbec15a3e0546d4d0bd5886607d3ed7e8a896cdb04822e670b3fa8ab37f6884464bdd59ef155b95090c14ee70a382e2cfee02cf9aca38fd9da1ee2eb030504882ec33e9ab8df98b9d9894432bd8c607de2ad318c85521af84ddaa64a79d8da6db3cf8f279e0971fe750a44d46ec68b39eb80065a179e302f98bcb380c448b1ae2746a1d9bac4e9f92876236730b72cb0f03206b63585760e8251bfd8beefbdfd3827614977cc88aded407ae47e9d058675efe0d7ba2c900e3cce7ef05000000000000003c646ecd346bce68460c246734a5330a3c4d3eb180442760831d16fd64e7f50700000000000000000000e40b5402000000013ef41a7f29672164ff35528f7b5f8ae625f0e24183fada1126246b83474efc04000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000009cf654deae54f4adb87a7f53181c143c2f54361aa0ca452140cbe9b064b5e2bf1b9570f11005d07992d3a8eb0748c6aef4bc4257f9602503d6664787d549b6df00000200009cf654deae54f4adb87a7f53181c143c2f54361aa0ca452140cbe9b064b5e2bf1b9570f11005d07992d3a8eb0748c6aef4bc4257f9602503d6664787d549b6df000000a09f9cefbe03d8c9116508740f1c9a6c408b327fdaba26366a4eee6e96c7d7a449cb4ec4598232610a6138729610587c4d889e0b4c193bcc7a778021ca96985f491d7357d67701000204052c01cd4075fb7461cf620ac282d214a024b01c20ea650a57473976c4471cb3e0662802090100000000000000000000000000000000030003040100a09f9cefbe03d8c9116508740f1c9a6c408b327fdaba26366a4eee6e96c7d7a449cb4ec4598232610a6138729610587c4d889e0b4c193bcc7a778021ca96985f491d7357d67701000000000001020000000000000000000000000000000000000000000000000000000000000000000001022dfefe16e01b65622d2ec3fcd1abef1bdea09094af19cb15038bb64f7c62c76cda8522dd1543bf62b07a5d9927d67ce72fb356553bbb02107011f3472d8bb1f7"
        );
    }
}
