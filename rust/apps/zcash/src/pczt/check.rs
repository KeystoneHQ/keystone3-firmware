use super::*;

use orchard::{
    commitment::ExtractedNoteCommitment,
    keys::{FullViewingKey, Scope},
    note::{Memo, Note, Nullifier, Rho},
    note_ext::{calculate_note_commitment, calculate_nullifier},
    Address,
};
use parse::decode_output_enc_ciphertext;
use zcash_vendor::{
    bip32::ChildNumber,
    pczt,
    ripemd::Ripemd160,
    sha2::{Digest, Sha256},
    zcash_primitives::legacy::{keys::AccountPubKey, Script, TransparentAddress},
};

pub fn check_pczt(
    seed_fingerprint: &[u8; 32],
    ufvk: &UnifiedFullViewingKey,
    pczt: &Pczt,
) -> Result<(), ZcashError> {
    let xpub = ufvk.transparent().ok_or(ZcashError::InvalidDataError(
        "transparent xpub is not present".to_string(),
    ))?;
    let orchard = ufvk.orchard().ok_or(ZcashError::InvalidDataError(
        "orchard fvk is not present".to_string(),
    ))?;
    check_orchard(&seed_fingerprint, &orchard, &pczt.orchard())?;
    check_transparent(&seed_fingerprint, &xpub, &pczt.transparent())?;
    Ok(())
}

fn check_transparent(
    seed_fingerprint: &[u8; 32],
    xpub: &AccountPubKey,
    bundle: &pczt::transparent::Bundle,
) -> Result<(), ZcashError> {
    bundle.inputs().iter().try_for_each(|input| {
        check_transparent_input(seed_fingerprint, xpub, input)?;
        Ok(())
    })?;
    bundle.outputs().iter().try_for_each(|output| {
        check_transparent_output(seed_fingerprint, xpub, output)?;
        Ok(())
    })?;
    Ok(())
}

fn check_transparent_input(
    seed_fingerprint: &[u8; 32],
    xpub: &AccountPubKey,
    input: &pczt::transparent::Input,
) -> Result<(), ZcashError> {
    let script = Script(input.script_pubkey().clone());
    match script.address() {
        Some(TransparentAddress::PublicKeyHash(hash)) => {
            let pubkey = input.bip32_derivation().keys().find(|pubkey| {
                return hash[..] == Ripemd160::digest(Sha256::digest(pubkey))[..];
            });
            match pubkey {
                Some(pubkey) => {
                    match input.bip32_derivation().get(pubkey) {
                        Some(bip32_derivation) => {
                            if seed_fingerprint == &bip32_derivation.seed_fingerprint {
                                //verify public key
                                let xpub = xpub.to_inner();
                                let target = bip32_derivation.derivation_path.iter().try_fold(
                                    xpub,
                                    |acc, n| {
                                        acc.derive_child(ChildNumber::from(*n)).map_err(|_| {
                                            ZcashError::InvalidPczt(
                                                "transparent input bip32 derivation path invalid"
                                                    .to_string(),
                                            )
                                        })
                                    },
                                )?;
                                if target.public_key().serialize().to_vec()
                                    != input.script_pubkey().clone()
                                {
                                    return Err(ZcashError::InvalidPczt(
                                        "transparent input script pubkey mismatch".to_string(),
                                    ));
                                }
                                Ok(())
                            } else {
                                //not my input, pass
                                Ok(())
                            }
                        }
                        //not my input, pass
                        None => Ok(()),
                    }
                }
                None => Ok(()),
            }
        }
        _ => Err(ZcashError::InvalidPczt(
            "transparent input script pubkey is not a public key hash".to_string(),
        )),
    }
}

fn check_transparent_output(
    seed_fingerprint: &[u8; 32],
    xpub: &AccountPubKey,
    output: &pczt::transparent::Output,
) -> Result<(), ZcashError> {
    let script = Script(output.script_pubkey().clone());
    match script.address() {
        Some(TransparentAddress::PublicKeyHash(hash)) => {
            let pubkey = output
                .bip32_derivation()
                .keys()
                .find(|pubkey| hash[..] == Ripemd160::digest(Sha256::digest(pubkey))[..]);
            match pubkey {
                Some(pubkey) => {
                    match output.bip32_derivation().get(pubkey) {
                        Some(bip32_derivation) => {
                            if seed_fingerprint == &bip32_derivation.seed_fingerprint {
                                //verify public key
                                let xpub = xpub.to_inner();
                                let target = bip32_derivation.derivation_path.iter().try_fold(
                                    xpub,
                                    |acc, n| {
                                        acc.derive_child(ChildNumber::from(*n)).map_err(|_| {
                                            ZcashError::InvalidPczt(
                                                "transparent output bip32 derivation path invalid"
                                                    .to_string(),
                                            )
                                        })
                                    },
                                )?;
                                if target.public_key().serialize().to_vec()
                                    != output.script_pubkey().clone()
                                {
                                    return Err(ZcashError::InvalidPczt(
                                        "transparent output script pubkey mismatch".to_string(),
                                    ));
                                }
                                Ok(())
                            } else {
                                //not my output, pass
                                Ok(())
                            }
                        }
                        //not my output, pass
                        None => Ok(()),
                    }
                }
                //not my output, pass
                None => Ok(()),
            }
        }
        _ => Err(ZcashError::InvalidPczt(
            "transparent output script pubkey is not a public key hash".to_string(),
        )),
    }
}

fn check_orchard(
    seed_fingerprint: &[u8; 32],
    fvk: &FullViewingKey,
    bundle: &pczt::orchard::Bundle,
) -> Result<(), ZcashError> {
    bundle.actions().iter().try_for_each(|action| {
        check_action(seed_fingerprint, fvk, action)?;
        Ok(())
    })?;
    Ok(())
}

fn check_action(
    seed_fingerprint: &[u8; 32],
    fvk: &FullViewingKey,
    action: &pczt::orchard::Action,
) -> Result<(), ZcashError> {
    check_action_spend(seed_fingerprint, fvk, &action.spend())?;
    check_action_output(fvk, action)
}

// check spend nullifier
fn check_action_spend(
    seed_fingerprint: &[u8; 32],
    fvk: &FullViewingKey,
    spend: &pczt::orchard::Spend,
) -> Result<(), ZcashError> {
    if let Some(zip32_derivation) = spend.zip32_derivation().as_ref() {
        if zip32_derivation.seed_fingerprint == *seed_fingerprint {
            let nullifier = spend.nullifier();
            let rho = spend.rho().ok_or(ZcashError::InvalidPczt(
                "spend.rho is not present".to_string(),
            ))?;
            let rseed = spend.rseed().ok_or(ZcashError::InvalidPczt(
                "spend.rseed is not present".to_string(),
            ))?;
            let value = spend.value().ok_or(ZcashError::InvalidPczt(
                "spend.value is not present".to_string(),
            ))?;
            let nk = fvk.nk();

            let recipient = spend.recipient().ok_or(ZcashError::InvalidPczt(
                "spend.recipient is not present".to_string(),
            ))?;

            let note_commitment = calculate_note_commitment(&recipient, value, &rho, &rseed);

            let derived_nullifier = calculate_nullifier(&nk, &rho, &rseed, note_commitment);

            if nullifier.clone() != derived_nullifier {
                return Err(ZcashError::InvalidPczt(
                    "orchard action nullifier wrong".to_string(),
                ));
            }
        }
    }
    Ok(())
}

//check output cmx
fn check_action_output(
    fvk: &FullViewingKey,
    action: &pczt::orchard::Action,
) -> Result<(), ZcashError> {
    let result = decode_action_output(fvk, action)?;
    if let Some((note, _address, _memo, _)) = result {
        let node_commitment = note.commitment();
        let cmx: ExtractedNoteCommitment = node_commitment.into();
        if cmx.to_bytes() != action.output().cmx().clone() {
            return Err(ZcashError::InvalidPczt(
                "orchard action cmx wrong".to_string(),
            ));
        }
    }
    Ok(())
}

pub fn decode_action_output(
    fvk: &FullViewingKey,
    action: &pczt::orchard::Action,
) -> Result<Option<(Note, Address, Memo, bool)>, ZcashError> {
    let nullifier = Nullifier::from_bytes(&action.spend().nullifier()).unwrap();

    let rho = Rho::from_nf_old(nullifier);

    let epk = action.output().ephemeral_key();

    let cmx = action.output().cmx();

    let cv = action.cv_net();

    let enc_ciphertext = action.output().enc_ciphertext().clone().try_into().unwrap();

    let out_ciphertext = action.output().out_ciphertext().clone().try_into().unwrap();

    let external_ovk = fvk.to_ovk(Scope::External);

    let internal_ovk = fvk.to_ovk(Scope::Internal);

    if let Some((note, address, memo)) = decode_output_enc_ciphertext(
        &external_ovk,
        &rho,
        &epk,
        &cmx,
        &cv,
        &enc_ciphertext,
        &out_ciphertext,
    )? {
        return Ok(Some((note, address, memo, true)));
    }

    if let Some((note, address, memo)) = decode_output_enc_ciphertext(
        &internal_ovk,
        &rho,
        &epk,
        &cmx,
        &cv,
        &enc_ciphertext,
        &out_ciphertext,
    )? {
        return Ok(Some((note, address, memo, false)));
    }

    Ok(None)
}

#[cfg(test)]
mod tests {
    use alloc::vec;
    use keystore::algorithms::zcash::derive_ufvk;
    use pczt::common::HARDENED_MASK;
    use zcash_vendor::{
        pczt::{
            common::{Global, Zip32Derivation},
            orchard::{self, Action},
            sapling, transparent, Pczt, V5_TX_VERSION, V5_VERSION_GROUP_ID,
        },
        zcash_protocol::consensus::MAIN_NETWORK,
    };

    use super::*;

    extern crate std;
    use std::println;

    #[test]
    fn test_decode_output_enc_ciphertext() {
        {
            let fingerprint =
                hex::decode("a833c2361e2d72d8fef1ec19071a6433b5f3c0b8aafb82ce2930b2349ad985c5")
                    .unwrap();

            let ufvk = "uview10zf3gnxd08cne6g7ryh6lln79duzsayg0qxktvyc3l6uutfk0agmyclm5g82h5z0lqv4c2gzp0eu0qc0nxzurxhj4ympwn3gj5c3dc9g7ca4eh3q09fw9kka7qplzq0wnauekf45w9vs4g22khtq57sc8k6j6s70kz0rtqlyat6zsjkcqfrlm9quje8vzszs8y9mjvduf7j2vx329hk2v956g6svnhqswxfp3n760mw233w7ffgsja2szdhy5954hsfldalf28wvav0tctxwkmkgrk43tq2p7sqchzc6";

            let unified_fvk = UnifiedFullViewingKey::decode(&MAIN_NETWORK, ufvk).unwrap();

            let hex_str = "50435a5401000000058ace9cb502d5a09cc70c0100a8ade204850100000101010101010101010101010101010101010101010101010101010101010101010100000000c0843d1976a9149517c77b7fcc08e66122dccb6ee6713eb7712d2b88ac00000100000000000000000000fbc2f4300c01f0b7820d00e3347c8da4ee614674376cbc45359daa54f9b5493e01000000000000000000000000000000000000000000000000000000000000000002a5c716d0c9f3c45e5e1f8796af8573e1a275272ae9529ba5cc0adfe9a6b9f686be0d1a2a68e2aa3d5133e82b2d8779819bf6751960dccd03ffc1b2d3b6a6b61a0083be728d26e15e49f2e480250326e4ae08d1cb3d4e79c92c560bf1abbbdc9b016f8767388722baa4ad5b4531f110ab2391c3909e5a316fb8386ce805f180bf9f20e0f4a8aa15e8305d2ca8f5f67df51346ba7f22ecf5eb0271f73b7c0da9df290190985447edfcd442a94ef0e73b5aafefde0d67aac27fcb8ed1554a45d5ebbac9f1373bcb52e5b25aa33ca40100016479a7c25fcf1e3e08a884a9c3398d44bd044c56b898f0a29409886e67dd8b32017bfaf3f5870ddb8fbf0f57e54a77fd67019c4f21209c36ff36f41ca0c994fc7a01ba448c971edea439845e8b39ede810ef4889272e869bbf807c1e36847e5340381a4a60fd1dc5b743a8619b2720e600802ad412b35ec577f6c29abdc69250201844c1871a94477936bccb27526d89deaa1dfacf5ba262620fb12d675053743b2401f8ded4e70e0e1881f320598f3580e5a4a6509b218bbfedee8ded503c74100bf6e6fa13f3193aee65f5b7e3924e53fafc46df07bf16ad8c87e3da0fa9cc83697bf3ea1425150065844e766354ab32377a291971c4d6eaf6ed131cb3afc6caa0b576764eb615dc1799f39d2c3a941d167808e8491288b4e95dd2f4e4ff26b782d273118a103b2d428cf5a57d97a41bb950b69298b21717981b84ff72b265a8ae57d5dfa58f186eaba40b95d617a221b0cdb684fd581b8c5d804f6c3390b087bda219cd1d510bb6a00392722c74caedf7ed56bd891b5369811717dc9e974f17d122710cce230279d2c53990c92b3fc10a0fd55ae13d0cf111c3bb33a6a8c07370d86dc3ff49100c51bfb37639c59bfae381d8a81eb78af2cbe65ce661f2e7f67cefbde721ab24357b7d9da069c31d0362af88a8442ba8209a4cc8b7690791972c06d08cbf143cca210a13e34910b4beda7cb6cc1ecd11aa4c8d27b3f14776bb74980ab9bfcd150c36b285d90a55e066975341c6824ce23049c25c3f48221a63f410684fdd5d0de05d1d0b55fbdccae380ee100c030fad4f0ad43acb9c0da8a285964be661301f3d6de24038158b059dadb4fd59bb97d2599e199eeda00da69bbc91a40eed6c09d2ee76f9d788833d3a8c4903d64c56b678140ad5e6d0d98aae3c8bb53ae92538559d2dca6a664c5e39ab1f0d34a4d556a9272d27a6cbe14f02c14f6f794aaa32ef8d5b0ff907bc60f8fe535c2fe1b2633bdc428487e9824d29f2324625a14f0ef6e076cc75ff32db5fa75139010b83a08dfee15a1c5bf3a0c2d933d76ef7f13bd25cfdade1929cabbbe32370d55a08183b33e0625def1077ab6e760cadf41013d244080f09ed62e488ec2bf77b83f0512dadf0461045e1a1755a902038b4a40096f26d4dce03e4de7a8055cea57c52683c2e65df29efea3fc8aa1fd47cfdb42f7708932e9339d7ba50e1a66fa96763968fdd101b7faec7d047131587eb7b8c1984829a286a55d3d65ecf0a4b627644303c000c15d2a8dfd2550b2e0457ae2a0f5a234ae1139018264336a0cbdc59cbb49e0ed943f2f2ba9225f85622e626210f4a1d8d58e05c88b311ffa08f6965202aa44c128e8ba18c49cf42e32b246bf934281ede6cd98f563b12904bc757e13f78f28ff92f854134502f984eff00299531501f1114d3c2dfa8190c84a6f9542859c1f359efcb31533d02c3f22938f0d5207171bd8e193d71753fe60c8faf416c887d51f397984e3ff3a31a1e703a78711ea94ca3d2b73a0c26438bf2027f6cbe8fb2b6f1fcabb54fad174acef97a435424d31f06d9a165b1f7c8bffa5cdf65fc98a383cd9d579d9bbfdeacf5a712e8da16317768d438bf3be8ca99a4abfa74388e6bc4ba38800cd6fe19e4d4bde2535a1bf8c2356e2bc5dd55a2e036a014451280baa5fbc6c298aa2d6d8780e5fc20b02401287b98a611a99f565b88169a71964f7bc96bee05bd72e287456d54b60aff8d07000000bf25e919a52fbf40434a476ef28c8cbcfedcfad7f7bcbe9f95e9a4ba1b07c812b4054cc40d46498657bfe6c98de9f507c07b384efbd45690e3e3d864b6019436c404c0f9bf41d6e7dc5d8c00125da712b8959ceef39a9a9c5c45439e9ea2b75920e174f270e369cf1225a989e0d1ef27c528e499a8864822d8fd1e2df6162906397bfc2201fc08c7258668fec89740f3843bd1c74a82206bef0db19a39dbb637b9d4dea48e8e5c1f7a33e75932ba975d2799b4c82be79483ab6cd83c266eb8a02335acee212166cbd4317b2604f262a346367f866d881580ef1471125659f519135758f48045620f13b6b68e2f873ab80b3f7f233dce91beabebc160443c10426683d5ae24d34f445cb467770666fdb811c8193e531999de5359351fc403630e6a2092476e17e83879392d869ca977ad104fb38d8cbc8c45362ce17b12cc8a9ba63bc169dd34c05d9e5557306c023a678be5fa42c0be13ea5ed524f07f4cda17c58eeac9afd7377c4c0374928eba2dc1ef091492816717a1b74e0355078969a6be4cee182361b31ec0f6f16174410d19cda1771fc63bd57f8dcaf5bbc39ed7fc9a121dd677ed496e78bfc7c98bb31f10a470405788d057cc4d1254f16e46db31b659dbdbea3ff6d7b4e3da1af8b2468c632c8f340358b6c1b743ba9cc1d661945400ee3f102555d7742085013da53fab9648e2dd137fc94ff77ef0c94042bf456406d5a596e4cc1717c28fafa9a4077779e2073a474753b0d2c842d7cb0d53f4761fc297e88278b7c4c845aa008a9165f888848f568d377b4dedfc9d419c705f59ed3b10050ddee2d14b9db79f774142370962b8490eb660a00837cb711c4502b5b49902b1ac94abc5518a76ba1b3aafcfedd5a7fb2bd05f6fdc12d8ce743a46e4744ca6c0a9509ac1f54e68c933c39e7f22aee376d1071eb22d51c61437ed20daa1c7781a984cb80718963b6a9a392e0b3438ff14c38a5f5b3d9f7952340f05699f55c9d3ab8b515a9d5b374d232b7a62d8d49027a4710114b0cac3e14c4e1a4a4fd22c98563b9cbe859c67768a39f67abd5dbe55d3d16ff58e5c45b21d8c573f54b90188823601553cabb7845e9d6b426a83729bdbd96c54b6536244c5c236f80b72b24e8b70fe000000015a1ceea2af1d7bdaf40134651f4fae34d277b3a9e07beca02344429517478310037c7ea3ffe956f357696a97dc5cb5538ea1ecec486d0f76794a1fb56267f7975f777d9ba30e8b3cbd336321e82bd21ac25e19d4071f69d3be28f1e7257f2403f8f9574983f03e2bae13501a6893c7819291f113de5aacb3541bb335cf1e78990144620f2dfa185b193f7c56b434aceb19a84d504a4353ecc1a8a7f308ee455385df6483f82c53d7e738068148d089ac3a9a1a937bcb3dfe479946b6466cc64f1f014bb0aa8256647f62567cc37d2af2d79cac1df23e09723af1122d018220d5807ac90e040c8bd4789c6449390100010be2dc9e7c2cd9bdd92e9e96f9a5414f9a55e73547952b835752732e3cca7f12015ca4c1f945615fe381587640372f3e5339a73bc7d808f2afe11216a4361376de016d08886ecafc5a76561def2786e6b5662bb46b19f3ec81642fb2194d668be0068ad9d8a2a1c4c7e0b9179e0d48f00e99f21a921f637f230ced688c548e2eac17b31d9671e030628dc9dfc4a04ea1becd3673812b79c329640fa63401b7cb130501e6a5efe10b4c957d49edeb381f6e4bbbfd6a91dff33b6128dc74aa5c5e466fd1c25ddeb909531eb078279b3b2366bdf4e60707a793dfe64df454124e9358e11d6fd4c0bd2c5c7c6b9a8adab7315362cc59e84c4fae62bf9ad6697badd32dcfe1f8baa3d11f9ed88f87f620a3563fbe6fd5768a4305ed6f86f33f949c9f1d7cd66679e3d93bb693242a2349094bc5e7be2b5b2c40a35c0d6946d990d9b3bb2269de1241ea1ba53c80de18325e276f5ef78d24e7dbcd88e6b721ea05f457e66b6311bce5512495a4bf1131a31c02e6868de7a26757e7d4c9b4ed8c8c4bdc9c646f8b4c1c161c935d7ff7bc37c6d16b51c2ceabdfa2ae60eed03bb443a97919dac7f8cde84d27c21bf9ef337856e092955b8cac0277f10e7a0ca7443537af53fa3e35fcd2b432aaf28b8eb6953c20ce16be2a505f5fb1864e1fdc09b24a51d17dec28250a1b3b9a6b702ce3a7d773de30ec445707dc07c2560d1a2213b6421833ea6fa7494b3827a96c0aaa58e339250e9bd4f10d01524b4eb0df3fe42dc64849363f8b5c2d0d5295053ff3a8d19a60345b665e1dc32f947c9a40ecd66036480c0bf8fda5053d55260cbadfce3618dae041602621ee0b0d644dd0720f979c56d127639d902b0bbcf8478eb4bb332bc016dfc2514806fa4afbe4122383c7e7c1e96144d4db4d1c682160f27c010514985d93b7b8cf1ae02715f631296cf1bb02b0b5aa403a8e0a902090e24487c3b1571cd767b445fa8afced5045ca6cfec11c145e5619e70c0c31be6ea5294c663811fa9f0cd422339f97734eeeb605569f6f722c411ba6680d0b2183bf0e5781a5168126d2b7185a07c4d6a6c68ae98edafa88c6a34e489526730b42f410b63dc7634bd178853ad9fd9515482c151b172fdd108a0fb814413d60d5e1976fe86172087b7ee05fa5de9cff32d7744a404a8c754033705ed62900bb6d66cec6700022ab0453b3303c761de4d3d7afe2829cb5ee4a90cc5e58b521390facddcf3fb24d07a85a63f09aa74e9b1c07c6210f398b48d9f0d4cfc8d21cdf540a0849010681d2fcc20da39484c10be64d41c7cb781df15471610a5e9d1a1e92d50559506dacce0b1d96157dcf61c0e29a4046e8efdb9e88e38735a28e28fa209e97486bf131b2468fc1308e75d831978d6ae36aebaad0fbf7d584f1e236167982bb36d05f40402661f792484c64fa38b3d530b0a1d07e8daf9f1a2b6f17e9e2dd249b10cbc01eadd4210463bd22e319a11bfeddab5cd2cc041378143536c1a9cec009c2c324f5f2339c657de7e651c1bded048e95ce7c2e2399980a8d0c335c7474bc7a41c1b61f63453e55e9e070483d00b06106892fad10346fd4b328cb8c3afed8d972f7619c1150a984240f84a0d4e54fcbc9dde7a57a95ea960f343427dc08980919703cad11774681ced7122fd6606354db07d941dc2cb384871b01d34d4945507c5f743fac9aec25f6bbbd4ab8a03dc5a11ec6cd30f60a91cd9e2b000000d663b018b0cbe9f79eca42f336c8f0941cd86d1fef09f665a7358f43ddf2d92dfc6424f6d9646edad28b5ff1deeb47825e6b701fafe06d769d46ba6dd8c4e332c404533012b488c0ab0911f12e1395af1940dd819ba81131d9030d07e9a4065be77ef3b1d2cfbb35b415fd37cb68f3468bad14d0e8748128c660c140b1744d40178c7c8f2d54f54fb09fbec2d96e194651bf20cd017517b71527bf21fea24c622811c5c891acdd258f170665e4da2a3f9237b0032ee41a5be5e9bb8dee4cf37cbf2706a168856d2dfdb9fab61a62af17bb19aba9a98a2367abed2a72c8607cccf7fbdc75135b86ca1026a7ed7777e969ce19ba0e4629ee01a707205917d053e855d46d3979b1d02116d10be54cf3a7a9b7b6cc2a44bd1fc0a8317a4c7ae17316afb912baf85c820872a59b1730b5dc6934ed99e4ed1cf4fe6e123421168ff951184c42a6bcc6a5d50aea06a2652c09a8fe4a5523e3b0ea7d7b266b2cc77ed7791f4551296e5fbc6fbce4eff79db197cd1f783a1445c63a2f84f0be91fe8b65fc8aa5f5f5458485953554e7ba2c491adbd0d329674ac47914e2cffa7e9fdc85780cbcfcc46fed98de8ad5fd62615462892f5dd65f77fe1eb66056e5a9a422b092ea456b5017a33f2753085d8301eb08c5c25aaadaa1ca38aadd4bdba0a6cbe9a62b982240b60c61bb510f23fe2348af080da7d1e9e3a444997c8fa9f3f69ea86567308f1035325b0271098c1c1948dba2fd60f8a0b3b354a81961250e9d8186a9ba79e155316617cc86688752ca7096749647291dbb2a5928216548be7b7357bf4917234a0e790b52db2a6c20784e0e3848face544f9e8ab7b09fe87a241b4c0ca4d5310c29822100e59b2ea5c0b67955a2a209d6ccdcb7f2c7346c047c0e41c052ec1a82e92d502956eb894ebf24c0a4201a00aa167e1fa60dae6caae132ae0921f48faa2aff692da678c57ca23ace54ddaba40dc2b15d44c345bf2061c4172d537bc43d22d08e3c0826192e96ce236a82a102213fbe4701d288df138de809fa058e8b7b240650cb1aae966ea3614fc54db414e8294066e8936b02d96b006cbfffc52701a08d060122a93bdb8a34520d98b46489336524f121e7556f127abd068c0f7ec787683d5700000001c1fb0dfdaf7aac7c6c3ba651d8693674cb0eb5cec9845a3cff7581a914cb670303a88f3c01ae2935f1dfd8a24aed7c70df7de3a668eb7a49b1319880dde2bbd9031ae5d82f00011b18fc9f5f982757613ddab6f7b8e4a89d866878aa0047dd22bac33e2c12eb13";
            let pczt_hex = hex::decode(hex_str).unwrap();
            let pczt = Pczt::parse(&pczt_hex).unwrap();

            let fingerprint = fingerprint.try_into().unwrap();

            let result = check_pczt(&fingerprint, &unified_fvk, &pczt);

            assert_eq!(true, result.is_ok());
        }
    }
    //TODO: add test for happy path
}
