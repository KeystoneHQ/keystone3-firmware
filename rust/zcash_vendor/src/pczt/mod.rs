//! The Partially Created Zcash Transaction (PCZT) format.
//!
//! General flow for creating a shielded transaction:
//! - Create "unsigned transaction"
//!   - In practice means deciding on the global parts of the transaction
//! - Collect each output
//!   - Proofs can be created at this time
//! - Decide on an anchor
//!   - All spends should use the same anchor for indistinguishability
//!   - In a future transaction version, all spends will be required to do so
//! - Collect each spend
//!   - Proofs can and should be created at this time
//! - Create proofs for each spend and output
//!   - Data necessary for proofs can be stripped out of the format
//! - Collect proofs
//! - Distribute collected data to signers
//!   - Signers must verify the transaction before signing, and reject if not satisfied.
//!   - This is the no-turning-back point regarding spend authorization!
//! - Collect signatures
//! - Create binding signature
//!   - The party that performs this does not need to be trusted, because each signer
//!     has verified the transaction and signed it, so the bindingSig can only be
//!     computed over the same data if a valid transaction is to be created.
//! - Extract final transaction
//!
//! Goal is to split up the parts of creating a transaction across distinct entities.
//! The entity roles roughly match BIP 174: Partially Signed Bitcoin Transaction Format.
//! - Creator
//!   - Creates the base PCZT with no information about spends or outputs.
//! - Constructor
//!   - Adds spends and outputs to the PCZT.
//!   - Before any input or output may be added, the constructor must check the
//!     PSBT_GLOBAL_TX_MODIFIABLE field. Inputs may only be added if the Inputs Modifiable
//!     flag is True. Outputs may only be added if the Outputs Modifiable flag is True.
//!   - A single entity is likely to be both a Creator and Constructor.
//! - IO Finalizer
//!   - Sets the appropriate bits in PSBT_GLOBAL_TX_MODIFIABLE to 0. (TODO fix up)
//!   - Inspects the inputs and outputs throughout the PCZT and picks a transaction
//!     version that is compatible with all of them (or returns an error).
//!   - Updates the various bsk values using the rcv information from spends and outputs.
//!   - This can happen after each spend or output is added if they are added serially.
//!     If spends and outputs are created in parallel, the IO Finalizer must act after
//!     the Combiner.
//! - Updater
//!   - Adds information necessary for subsequent entities to proceed, such as key paths
//!     for signing spends.
//! - Redactor
//!   - Removes information that is unnecessary for subsequent entities to proceed.
//!   - This can be useful e.g. when creating a transaction that has inputs from multiple
//!     independent Signers; each can receive a PCZT with just the information they need
//!     to sign, but (e.g.) not the `alpha` values for other Signers.
//! - Prover
//!   - Needs all private information for a single spend or output.
//!   - In practice, the Updater that adds a given spend or output will either act as
//!     the Prover themselves, or add the necessary data, offload to the Prover, and
//!     then receive back the PCZT with private data stripped and proof added.
//! - Signer
//!   - Needs the spend authorization randomizers to create signatures.
//!   - Needs sufficient information to verify that the proof is over the correct data.
//!     without needing to verify the proof itself.
//!   - A Signer should only need to implement:
//!     - Pedersen commitments using Jubjub arithmetic (for note and value commitments)
//!     - BLAKE2b and BLAKE2s (and the various PRFs / CRHs they are used in)
//!     - Nullifier check (using Jubjub arithmetic)
//!     - KDF plus note decryption (AEAD_CHACHA20_POLY1305)
//!     - SignatureHash algorithm
//!     - Signatures (RedJubjub)
//!     - A source of randomness.
//! - Combiner
//!   - Combines several PCZTs that represent the same transaction into a single PCZT.
//!   - Because we aren't storing the partial transaction in network format, we need to
//!     carefully define equality for PCZTs.
//!     - If we say "pczt.global must be identical" then:
//!       - If we add spends or outputs in series, we should always update bsk when adding
//!         spends or outputs, even if rcv is present.
//!       - If we add spends or outputs in parallel and then combine, we must _never_ update
//!         bsk, and then update it when we prepare for signature creation.
//!       We can't control which happens, ergo we need an IO Finalizer step.
//!     - Once every spend and output has its zkproof field set, PCZT equality MUST include
//!       the SpendDescription and OutputDescription contents being identical.
//!       - In practice enforced by creating a TransactionData / CMutableTransaction from
//!         the PCZT, with spendAuthSigs and bindingSig empty, and then enforcing equality.
//!       - This is equivalent to BIP 147's equality definition (the partial transactions
//!         must be identical).
//! - Spend Finalizer
//!   - Currently unnecessary, but when shielded multisig is implemented, this would be the
//!     entity that combines the separate signatures into a multisignature.
//! - Transaction Extractor
//!   - Creates bindingSig and extracts the final transaction.

use alloc::{collections::btree_map::BTreeMap, vec::Vec, vec};


pub mod common;
pub mod orchard;
pub mod sapling;
pub mod transparent;

pub mod pczt_ext;

use getset::{Getters, MutGetters};
use serde::{Deserialize, Serialize};

const MAGIC_BYTES: &[u8] = b"PCZT";
const PCZT_VERSION_1: u32 = 1;

pub const SAPLING_TX_VERSION: u32 = 4;
pub const V5_TX_VERSION: u32 = 5;
pub const V5_VERSION_GROUP_ID: u32 = 0x26A7270A;


/// A partially-created Zcash transaction.
#[derive(Clone, Debug, Serialize, Deserialize, Getters, MutGetters)]
pub struct Pczt {
    /// Global fields that are relevant to the transaction as a whole.
    pub global: common::Global,

    //
    // Protocol-specific fields.
    //
    // Unlike the `TransactionData` type in `zcash_primitives`, these are not optional.
    // This is because a PCZT does not always contain a semantically-valid transaction,
    // and there may be phases where we need to store protocol-specific metadata before
    // it has been determined whether there are protocol-specific inputs or outputs.
    //
    #[getset(get = "pub", get_mut = "pub")]
    transparent: transparent::Bundle,
    #[getset(get = "pub", get_mut = "pub")]
    sapling: sapling::Bundle,
    #[getset(get = "pub", get_mut = "pub")]
    orchard: orchard::Bundle,
}

impl Pczt {
    /// Parses a PCZT from its encoding.
    pub fn parse(bytes: &[u8]) -> Result<Self, ParseError> {
        if bytes.len() < 8 {
            return Err(ParseError::TooShort);
        }
        if &bytes[..4] != MAGIC_BYTES {
            return Err(ParseError::NotPczt);
        }
        let version = u32::from_le_bytes(bytes[4..8].try_into().unwrap());
        if version != PCZT_VERSION_1 {
            return Err(ParseError::UnknownVersion(version));
        }

        // This is a v1 PCZT.
        postcard::from_bytes(&bytes[8..]).map_err(ParseError::Postcard)
    }

    /// Serializes this PCZT.
    pub fn serialize(&self) -> Vec<u8> {
        let mut bytes = vec![];
        bytes.extend_from_slice(MAGIC_BYTES);
        bytes.extend_from_slice(&PCZT_VERSION_1.to_le_bytes());
        postcard::to_extend(self, bytes).expect("can serialize into memory")
    }
}

/// Errors that can occur while parsing a PCZT.
#[derive(Debug)]
pub enum ParseError {
    /// The bytes do not contain a PCZT.
    NotPczt,
    /// The PCZT encoding was invalid.
    Postcard(postcard::Error),
    /// The bytes are too short to contain a PCZT.
    TooShort,
    /// The PCZT has an unknown version.
    UnknownVersion(u32),
}

/// Merges two values for an optional field together.
///
/// Returns `false` if the values cannot be merged.
fn merge_optional<T: PartialEq>(lhs: &mut Option<T>, rhs: Option<T>) -> bool {
    match (&lhs, rhs) {
        // If the RHS is not present, keep the LHS.
        (_, None) => (),
        // If the LHS is not present, set it to the RHS.
        (None, Some(rhs)) => *lhs = Some(rhs),
        // If both are present and are equal, nothing to do.
        (Some(lhs), Some(rhs)) if lhs == &rhs => (),
        // If both are present and are not equal, fail. Here we differ from BIP 174.
        (Some(_), Some(_)) => return false,
    }

    // Success!
    true
}

/// Merges two maps together.
///
/// Returns `false` if the values cannot be merged.
pub(crate) fn merge_map<K: Ord, V: PartialEq>(
    lhs: &mut BTreeMap<K, V>,
    rhs: BTreeMap<K, V>,
) -> bool {
    for (key, rhs_value) in rhs.into_iter() {
        if let Some(lhs_value) = lhs.get_mut(&key) {
            // If the key is present in both maps, and their values are not equal, fail.
            // Here we differ from BIP 174.
            if lhs_value != &rhs_value {
                return false;
            }
        } else {
            lhs.insert(key, rhs_value);
        }
    }

    // Success!
    true
}

#[cfg(test)]
mod test {
    use super::Pczt;
    extern crate std;
    use std::println;

    #[test]
    fn test_decode() {
        let pczt_hex = "50435a5401000000058ace9cb502d5a09cc70c0100a8ade204850100000101010101010101010101010101010101010101010101010101010101010101010100000000c0843d1976a914f02de957dce208211ea9010197e0c8f69f31fff588ac00000100000000000000000000fbc2f4300c01f0b7820d00e3347c8da4ee614674376cbc45359daa54f9b5493e01000000000000000000000000000000000000000000000000000000000000000002716761507b67baf75227c9c761271b3af86b4c6b7be810b99df64128d0dd64bca4c21460d4182405c38e3778eb1ed4abf403a89d6790e961b323d654213ed812c44822091f8e42c164a74652b0a0d0a5efb1a4806dd651f6a700e8eb96c25c920199e35a6f67d93646b2026fa01af4de2d8cca06acef257bb0d11077e72679ec8080eda665bfb8903d49ad3f06ce03364d493c37c881d6ea1330b92634714c29300116b6703574247b85a120fcbb0b902385b65a38d0e464096e420d60a52117123b5130480e50854f50dfcca001000106758ef54626a4e6d9f36b0b4ecb0500c73c843c55bf6dc48190c86dacebc32f0146b70af68763863d39f61173906207539e037b92cdabd853b536c9b10d9b0d8d01e4b2543e3ebc543acdf2c5c2892ec1665ae526595a4adce259b1e2ad7b7bb708d16f16a4358c2ae7d8ebbed3af5beca59f1c5e1506d97b6250dcc3d87ee9cf2da4ba0d778319462d0da279e315f1d00a4ca5b82b97d5b1ed1c95413071b5021501c5ebda8b04c18c4651b154aca201996285e6fabd01ae701523a715f69a8e04fcaded6d131a0ef59e34beb6eb44a76df8ff70e5ccaca38eab825f2b1ff0c21666cd88c8e8043ede50a6ff29f27c8d67a719c4a7d1bceccc89defad945f2058394d28d94cf05356528d5d69e35902840bec659139bec673be93683d73a7fe4090bbdf0ab1a2ef8e2499f3282e7bdea84fbfbf6bf290fab392070a2334e59a1f19a7cfc2fbc06ee5a9b91a0dff70df0cd2ffab696bd1356502b65666788661867f00f1dc3991582391359d6580e06ca7081db2dfedb81050382e6fe508655219d9b2c3fd1b6157f2debaae5b06140b8e1b8d47598bf45152feb2b57932db7a96fb18d0e3c2d38d81b478696877979490bbe81c1c2e01f4cf9c8764f104722710f4de86c73113d4b9d12d13550cb94c2a101769fd14e4db372cb02f84778c02b4638bc376d4e060539feacc8deeb0bc74260a805dba85ac90201dcfe703b9e203d80342fb6712ae84944b09d0c7f34fe599a873efc1dc45da9d541c04ed0d326f9d6577caba82ab568a0d9779ad3ad62bd0f92a6d1afc6e41f55f3468a8974899eebe842c7dc11e5069d5ac27363e831f46de7c8ab6842374943a579305e855ce310c39978b509e776cc9565351a02198f783d9d7640edabb677b5d0e3ef39399e625d6b0975305244ab0f8a676831fd94c0e4875b1d43d22ad5b4460fb56d79f452e992070d2d200ea580b1d74f5bcfc0a08c20e8ac2e89199485c9fb14dcce73a4ff724bfb20eeec1d0f0e9be8bc39c92ed51ae06c7c7826a88d1b2ed59283a93e585068a80fa36425bd2fec9120fa2927265836ed84f87fa8fd2b0ba71d6b2b74ff4275df232bd6717ed7f09ca4cc38783417d6d1fe2e41914b4c0e8313cc12a6b2c817f123fd2151910b75ba4a9389dfda185d52ad4323e48c8e0f425170f8669a922ae8005f86ea9f64bbb6ff2191bc4b2ee13bb0ab0828494b6868e4fdfceb5d2e63531c614c06ba96d5278b3ec9286376a255bda203986d6a179f9c5a866cdbdf94af3ef7d58c36662b0e7b3e95c4c183ddc1cd72a2ae3a2cf977481116967e6e267f2d40c4e6281853906ae8717ca7751ae4e0dabf764d05a6ee2734c78caae20bdc12eb563dd5182e5b52344ab3f45d879d6f43ea263a9f950715a0e186812613953b48f5c8b9f20b0a161cbd8f1b941232c62ba53ddb4ec592348dbfbccc76c972150416f92c810c47b166bb27728c154745e2d9a9106f364f9f4068eaa8a9f8bd08e4b79e4ab4fd428b3a6149f10c909fd3b6762f593c9c4b4732283f2576349613b27001a1197916740f3fb152d728279218848c69251e3f054378f5919d72b4325e14ada298781ed084f44689b46f9844bf48e78d8c73e2d710f1997069b9542ab6daa903405506cb4328d22d7184458add48a0eb65bcd6893778f44466359d3301bb59587690214e8de77435cda137ba0715b0a54b9d7af0b6e13905cd21004a2c0000002389138b817483d2d5e8bb366b14a2c3fd56b8017cddb9e8c65d335a4c80282b2d9e440da987d14e644dc4bd0827df92a726f428997888278ff641eabade3e9cc404a2118d08131680458989ef65d554ee436b5da1df84c5a440e5d9404d7bbec82dec676714f67912a6f48250dafd4723c7254d25f7de4c88549a53f9f679290cf1867af21300a0c85281292110e7afd3f57ab877bae35b2492d00ff273c03a4f53f22eb30e2defdae2e783daacf9389aa43810e089e8b88a92ee79e713c29026a38b166c54b33af2762c868493712c80530b562ec97a73a9470bd623ce7f5e5bc0348b2b9506ab83b500bd672b77432206b164b5c2221aa728713f8a88db112cc39c42768d120ebd27e03736e86c71d83fb4f7a6521dfd9348425171472a12180d2e1f404bf6a2c4568316f6ee093f2ce9fbf220c02736bea79590af40c0016ca856066cf236a7547cad91d01c41f18c5fa247009ba5c21298742cc03483ce9919b75da26eafbf87ee08e60ff4dae36256f7614e7d7a48cca1fa1365640c62f9ca3257df1f993ba2fd1baef35a58b93f0a47f3aa05be978af710247b30721b7eb445d5f22c51d603916aa6b97533312177f81d2ce6ed64dd06ced24589afadce011ed858a0700f2b8eda0d8f31bf7e5f44bee93acd36966b09328611a8a367b2ce61f50f70a7a64a13eee473b7ce70759e273dcbe1687495a99bc06a3fca40224d64011b074305486a13d4ca1b715de60b30112c48c9e2fb657e28c41d250c5daed931e2c07a6f9b3e8193e359ec18ec826ffc8f46009c0553f05c1e44a4fc645dd3911958062270922a5797807c3d59d7f90f84ee25999ba1b70908ba1b125b65192d048a57d4cc32df070f24df0eb45a3b8f27a46f31e188f332e1d5fa34593cc5a3d1585018419067ca773009dc2c907d185c305bc9e2e355c7fc9cb8cf90aeaeac7ac2a5ea1c76b8f105d4ecfc6f9833c950467d070f128fa54576bef02b0b4e3d818a0b0425920acd0427e5cbfb13883fde1258010d387850618fa8d6c586da735bc6f3ddbfb6d589ecac39470ef559d2c313d6ac17b08f20baa13b65bac42e01888236019cb5f4353bb359e5baf7b8faccb47dec5053115c31140bbba1dd4dcaee636b77000000019000d57ac28266b7bcb259ac324a1cbfe867dcaa04bf7ba0cc025da861a69c0d1108f3bfd91588d24c83443993b3391e8175d15862606d7d3b223f16983e27015196e141e46b861e9580551539d069126b0d5794b7338b4aa8f2ec570c20351b201f5044a05bb9a57bbc66e347605b02410c7a4f713d1c4da62d87ddb011e68d01330834d244835f3cc06ea61647665c1a2c4e51497dd6791c306ea63728f89a14073a2697288fea5aaae387b6d93611559aafc2338050de9663e4cf24a2f8f436016d2b3a238161e4aba76f19a22b182406e4c280594443e132bd9d5f6f73d07a45de690089ebcb1d9e190b0e010001af659bffa492fe867b0c25b2b4d03e2591c909ed74c153c6adbb3357698e900c01d10a1ddb3e7101f99884736ad952ac68c6e09e0cc09b523cfaf4899bc363048e01d6e1f3d0ef42aee466d1b381afb48100fe07ff6b214a89ca58f5d87a311ce50970bf4f27d6b02eb9ca89fe953a9201a8bbcde69f86d8d3a236b9af1509f5b710b5143c43bbaa0fb37eac127686129123de2380c3a5e92e60e32bac75d56c912f01cb94e12bac858edd038515421cdee306b3136645b78f46ce91f302dedc165a71957153379091c83f69640d8694d6afc604894b841a7c6f9f9e2a25fef5f73b1668144a004c229e9dccc0944f2cd8e42e01273a51a9813bee0649f934850421e9ae20860202a74d8765d17e11ba7cc18c68f1a00a2108ca7ad9858612cab9a26ce20c6235f458875f69b3f8b53b0e2d2898e3a431d1dd645c6a54801f109e6b19054ecd2fd078f909516098fc7e72da9e68cc2663a40280091ff64fb8c6bf8e61e69eb80c1019cc45d6744520dd018ba8771f07d67b9cbc57f46de18839709e467a4f062145d6b0e9cf50eab94809c1d7581580cc17e604e4fa4ed058ce428168031b9d26783c642a4eae54d9c922a624ebbcf7eaeab9273a5b0a05cb2e2e28df2fe9df0ed48675d5a03baa0ba2db4105107771dd7df29bb772d9f81b457ce1696586b028a7634ac7192137b8ca933993adc981ec8e4722de974ee98887397113ace21f0c754ded0ccd1253fdc1e67bf5c49cb250db993c3d94fa2bf7aacf6ca9d1392525341a41a6c9785c62c3e8dbb4c1a0eec601b906fd0f78973ac0ed8f74e5651b0dc9c1ab180039a639e02f4b4b94001a5494372ce1fcdf7aab62dfd3f81a2f6629b8f2947d778b5a2bb30139a78757b5b9153e40df0945f3438c3e507f034e6d16708860986f87ba462a9575d95a3f1a259986e022bac402b7c767ca560f65b724c43567b1a407077fffb7d5b5c91c205ff73936e48550941b01e289a47b30a40980d12ebca61d2283ce1c9930c79e7bfe1fde5fa8d25898f9e16c67b7255d7a3529dd1a51f2ca06e7d6af756c0d9b1674ac3d02e2f535d80a4e751ce36fa73911853769a4bd2ff8b72039f84d6c4826615b3fb0b8ccc000aa148b845f490d9f161e2edaa15ea90ee14629ab3dc09497284b74eba8d56cf715ec807807e8730838dc7d778df3969b614ab8b3818f465e1e014db1de7f0be4b507f1c6f46b356a17fa5b2a6734d79c0315a6e22aab588da5c9968a954822986e134ab11c3d972b31fd8cb2e43814d314dd3ecf52c765c9075ac2c72cff336cfc03c022472fa4a432403fda134f221819b7c165d68d6e04d66c0c0597ebe5547ce73ad417afef1d15f66c29accfb877781c1f1d76899e12e4d1d7968687272a72157c6867eb274a39c7db18e299c3bcb50e58b161496f891787cf716f1e579d07ab38fb4fbbed2101a9b129f04a51342cd8d9ffe337175df304dbb52d2435717281d39d7d088cd31c17524fa4b373f19a7dd2f33752638944665ed18b1913fd8f52ea090f01c0f11cd12f16bebbec5229723253073b2468e2afa053d839d62f918f3ec4b828925b25252559f5f1a34e3e226c98e046bb2a3dc8f6ebf85b8a4974edddf06efd1c5904c3ff9f90d845cb83333e9cd7406eac075f77bc510c68ddca0c1f9d038cb1891801e84fa6f77836a17b48d62dfe22ca971c09378ff3f111e49e20cc1e14b34af40d000000fd5d08b9bc2b222a4b7e0662ec3a4c8c98c398840faafb07cf000bb7a680703fb7bc6011c95a6b4427a1c98f9fe6a28294b3d5d8e29ce48dfe369fe984bac505c404b15a3d9f99901f596eeb9114aaf6bab170019ada32e37ac273eed734d98dd4216fec56e46aee4d1494ac81386e00cc8d6f5d468f3d4fdeb8e5f3c865bb8c56b16014b22f803e1170bd8d92109242194a6c972f15ec2985953f58f34bc7f3f9c3588a54d13d9338beff16dd5785a584c855cf54ed5b1929c8757783bb57b9111f77b61745fe4498fe83bbb69d9e40c71de2850ebcedb32828375c45533c12c51a41621a8d010e370f4dffe95f379aaeff4936d9cee44d413ee72b41cc81ea0a4af461a9acb9638d2d0094d583ca73bcad295e75640bb9aa3bd0a332388416398814bfbf46312dd674d75ce2596f51c42ce798877f18f9d013178c3287aaf4eea0e000c9514ee27b0ed7ad0153fae94e3d6def309c6732aab575755892890d14155fb55fbbf5318263322a60cbcc332f13681d4efb45dcd46bffc14770b041e2aa04717ebab9afb1a720b9e59a93ececc9fbc8662a8e56ba79ef440bb7ffd03f4501cbdbf7c1f70f6c0dcdef8257dec739de75198499c8590c4d269ae91bd23d68f6a7fa741c1d63489e20fe6464de65161de955b90f6672b8e286d9ed71e75315ec31722b6b4d77f8b1982f2ca7a041e9bebab1455aeb1a18884288c81bd2c95639ad46ecd56f99760e1be1334166fdab849ae3256897f08d85d0da4ce03e8e5468cb1b0229b867d5367f848970f557bf05df91f417653d3cba8ce15c7d2d8831c9f1e5def6e23d9beffbfc48f7d381478a5cac9acf480b11599707a9bd41030de2f0bfd151f3f2174f8f1cd250e087bfc2cdf0a1bf184a3e97920876e135f0b973af3b1d50aa3c30e4e094d81648cc8b2d8fbad9d18d03cfdef9c067a0aeb731d044d3e1e5f6dbf50664794b9727da55632509a024ac8bdfe6242808ebe48d7ae2cded16e155d2fa39900601a095c420265b8e43b401cc36601959213b6b0cdb96a75c17c3a668a97f0d6a8c5ce164a518ea9ba9a50ea75191fd861b0ff10e62b001a08d0601122115570090052358700c36e84313aba6d826eb2a3125bf0d6589796d46437c00000001b3bc5ad68ff28e6708795d8f788bb679f57df01de32e257c6490f2b6a98d313203a88f3c01ae2935f1dfd8a24aed7c70df7de3a668eb7a49b1319880dde2bbd9031ae5d82f000143bd2f515275f51ec52bb73babd5d238dee5ccc8e7eda01c31934f5f0b34ce3f";
        let pczt = Pczt::parse(&hex::decode(pczt_hex).unwrap());
        println!("{:?}", pczt);
    }
}