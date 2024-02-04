use crate::addresses::xyzpub::{convert_version, Version};
use crate::errors::BitcoinError;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use keystore::algorithms::secp256k1::derive_public_key;
use third_party::bitcoin::absolute::LockTime;
use third_party::{bitcoin, hex};

use crate::errors::Result;
use core::str::FromStr;
use third_party::bitcoin::consensus::Encodable;
use third_party::bitcoin::sighash::{
    EcdsaSighashType, LegacySighash, SegwitV0Sighash, SighashCache,
};
use third_party::bitcoin::{Amount, Script};
use third_party::bitcoin::{PubkeyHash, ScriptBuf, Transaction};
use third_party::bitcoin_hashes::{sha256, sha256d, Hash};

use crate::addresses::cashaddr::{Base58Codec, CashAddrCodec};
use crate::addresses::get_address;
use crate::network::Network;
use crate::transactions::legacy::constants::{SIGHASH_ALL, SIGHASH_FORKID};
use crate::transactions::legacy::input::{InputConverter, TxIn};
use crate::transactions::legacy::output::{OutputConverter, TxOut};
use crate::transactions::script_type::ScriptType;
use crate::{collect, derivation_address_path};
use app_utils::keystone;
use third_party::bitcoin::ecdsa::Signature as EcdsaSignature;
use third_party::either::{Either, Left, Right};
use third_party::hex::ToHex;
use third_party::secp256k1::ecdsa::Signature;
use third_party::ur_registry::pb::protoc;
use third_party::ur_registry::pb::protoc::SignTransaction;

#[derive(Clone, Debug)]
pub struct TxData {
    pub(crate) inputs: Vec<TxIn>,
    pub(crate) outputs: Vec<TxOut>,
    pub(crate) script_type: String,
    pub(crate) network: String,
    pub(crate) extended_pubkey: String,
    pub(crate) transaction: Transaction,
    pub(crate) xfp: String,
}

#[derive(Debug)]
struct CommonCache {
    prevouts: sha256::Hash,
    sequences: sha256::Hash,
    outputs: sha256::Hash,
}

#[derive(Debug)]
struct SegwitCache {
    prevouts: sha256d::Hash,
    sequences: sha256d::Hash,
    outputs: sha256d::Hash,
}

impl TxData {
    pub fn from_payload(
        payload: protoc::Payload,
        context: &keystone::ParseContext,
    ) -> Result<Self> {
        let sign_tx_content: Result<SignTransaction> = match payload.content {
            Some(protoc::payload::Content::SignTx(sign_tx_content)) => Ok(sign_tx_content),
            _ => {
                return Err(BitcoinError::InvalidRawTxCryptoBytes(format!(
                    "invalid payload content {:?}",
                    payload.content
                )));
            }
        };
        let content = sign_tx_content?;
        let tx = &content
            .transaction
            .ok_or(BitcoinError::InvalidRawTxCryptoBytes(
                "empty transaction field for payload content".to_string(),
            ))?;
        let inputs: Vec<TxIn> = tx.to_tx_in()?;
        let outputs: Vec<TxOut> = tx.to_tx_out()?;
        let script_type = ScriptType::from_coin_code(content.coin_code.to_string())?;
        let transaction_mapped_input: Result<Vec<bitcoin::blockdata::transaction::TxIn>> =
            collect!(inputs);
        let transaction_mapped_output: Result<Vec<bitcoin::blockdata::transaction::TxOut>> =
            collect!(outputs);
        let extended_pubkey =
            convert_version(context.extended_public_key.to_string(), &Version::Xpub)?;
        return Ok(Self {
            inputs,
            outputs,
            script_type: script_type.to_string(),
            network: content.coin_code,
            extended_pubkey,
            xfp: payload.xfp,
            transaction: Transaction {
                version: third_party::bitcoin::transaction::Version(2),
                lock_time: LockTime::from_consensus(0),
                input: transaction_mapped_input?,
                output: transaction_mapped_output?,
            },
        });
    }

    pub fn check_inputs(&self, context: &keystone::ParseContext) -> Result<()> {
        if self.inputs.len() == 0 {
            return Err(BitcoinError::NoInputs);
        }
        if self.xfp.to_uppercase() != hex::encode(context.master_fingerprint).to_uppercase() {
            return Err(BitcoinError::InvalidParseContext(format!(
                "invalid xfp, expected {}, got {}",
                hex::encode(context.master_fingerprint),
                self.xfp
            )));
        }
        let has_my_input = self
            .inputs
            .iter()
            .enumerate()
            .map(|(_, inp)| self.check_my_input(inp, context))
            .fold(Ok(false), |acc, cur| match (acc, cur) {
                (Ok(b1), Ok(b2)) => Ok(b1 | b2),
                (a, b) => a.and(b),
            })?;
        if !has_my_input {
            return Err(BitcoinError::NoMyInputs);
        }
        Ok(())
    }

    pub fn check_my_input(&self, input: &TxIn, context: &keystone::ParseContext) -> Result<bool> {
        let hd_path = input.hd_path.to_string();
        let address_path_str = derivation_address_path!(hd_path)?;
        let pubkey =
            derive_public_key(&context.extended_public_key.to_string(), &address_path_str)?;
        if pubkey.to_string() == input.pubkey {
            return Ok(true);
        }
        Ok(false)
    }

    pub fn check_outputs(&self, _context: &keystone::ParseContext) -> Result<()> {
        if self.outputs.len() == 0 {
            return Err(BitcoinError::NoOutputs);
        }
        self.outputs
            .iter()
            .enumerate()
            .map(|(_, output)| self.judge_then_check_my_output(output))
            .fold(Ok(()), |acc, cur| acc.and(cur))
    }

    pub fn judge_then_check_my_output(&self, output: &TxOut) -> Result<()> {
        if output.is_change {
            let mut raw_addr = output.address.to_string();
            if self.network == Network::BitcoinCash.get_unit() && raw_addr.starts_with("1") {
                // convert bitcoin cash legacy address to cash address
                let decoded = Base58Codec::decode(raw_addr.as_str())?;
                raw_addr = CashAddrCodec::encode(decoded)?;
            }
            let change_address_path = output.change_address_path.to_string();
            let output_addr = get_address(change_address_path, &self.extended_pubkey)?;
            if output_addr != raw_addr {
                return Err(BitcoinError::InvalidOutput);
            }
        }
        Ok(())
    }

    fn common_cache(&mut self) -> Result<CommonCache> {
        let mut enc_prevouts = sha256::Hash::engine();
        let mut enc_sequences = sha256::Hash::engine();
        for txin in self.transaction.input.iter() {
            txin.previous_output.consensus_encode(&mut enc_prevouts)?;
            txin.sequence.consensus_encode(&mut enc_sequences)?;
        }
        Ok(CommonCache {
            prevouts: sha256::Hash::from_engine(enc_prevouts),
            sequences: sha256::Hash::from_engine(enc_sequences),
            outputs: {
                let mut enc = sha256::Hash::engine();
                for txout in self.transaction.output.iter() {
                    txout.consensus_encode(&mut &mut enc)?;
                }
                sha256::Hash::from_engine(enc)
            },
        })
    }

    fn segwit_encode_none_standard_sig_hash_type(
        &mut self,
        input_index: usize,
        script: &Script,
        sig_hash_type: u32,
    ) -> Result<SegwitV0Sighash> {
        let raw_input = &self.inputs[input_index.clone()].clone();
        let mut enc = SegwitV0Sighash::engine();
        self.transaction.version.consensus_encode(&mut enc)?;
        let common_cache = self.common_cache()?;
        let segwit_cache = SegwitCache {
            prevouts: common_cache.prevouts.hash_again(),
            sequences: common_cache.sequences.hash_again(),
            outputs: common_cache.outputs.hash_again(),
        };
        segwit_cache.prevouts.consensus_encode(&mut enc)?;
        segwit_cache.sequences.consensus_encode(&mut enc)?;
        {
            let txin = self
                .transaction
                .input
                .get(input_index)
                .ok_or(BitcoinError::InvalidInput)?;
            txin.previous_output.consensus_encode(&mut enc)?;
            script.consensus_encode(&mut enc)?;
            raw_input.value.consensus_encode(&mut enc)?;
            txin.sequence.consensus_encode(&mut enc)?;
        }
        segwit_cache.outputs.consensus_encode(&mut enc)?;
        self.transaction.lock_time.consensus_encode(&mut enc)?;
        sig_hash_type.consensus_encode(&mut enc)?;
        Ok(SegwitV0Sighash::from_engine(enc))
    }

    #[inline]
    pub fn signature_hash(
        &mut self,
        input_index: usize,
    ) -> Result<Either<LegacySighash, SegwitV0Sighash>> {
        let raw_input = &self.inputs[input_index.clone()].clone();
        let mut sig_hasher = SighashCache::new(&self.transaction);
        let script_type = ScriptType::from_str(&self.script_type)?;
        let pubkey_slice =
            hex::decode(&raw_input.pubkey).map_err(|_e| BitcoinError::InvalidInput)?;
        match script_type {
            ScriptType::P2PKH => {
                let script = ScriptBuf::new_p2pkh(&PubkeyHash::hash(&pubkey_slice));
                if let Ok(Network::BitcoinCash) = self.network.parse::<Network>() {
                    let sig_hash_type = self.sig_hash_type() as u32;
                    self.segwit_encode_none_standard_sig_hash_type(
                        input_index,
                        &script,
                        sig_hash_type,
                    )
                    .map(|v| Right(v))
                } else {
                    sig_hasher
                        .legacy_signature_hash(
                            input_index.clone(),
                            &script,
                            EcdsaSighashType::All.to_u32(),
                        )
                        .map_err(|_e| {
                            BitcoinError::SignLegacyTxError(format!(
                                "invalid sig hash for {:?}",
                                script_type
                            ))
                        })
                        .map(|v| Left(v))
                }
            }
            ScriptType::P2WPKH | ScriptType::P2SHP2WPKH => {
                let script = ScriptBuf::new_p2pkh(&PubkeyHash::hash(&pubkey_slice));
                sig_hasher
                    .p2wsh_signature_hash(
                        input_index,
                        &script,
                        Amount::from_sat(raw_input.value),
                        EcdsaSighashType::All,
                    )
                    .map(|v| Right(v))
                    .map_err(|_e| {
                        BitcoinError::SignLegacyTxError(format!(
                            "invalid sig hash for {:?}",
                            script_type
                        ))
                    })
            }
            _ => Err(BitcoinError::SignLegacyTxError(format!(
                "invalid script type sig hash {:?}",
                script_type
            ))),
        }
    }

    #[inline]
    fn sig_hash_type(&self) -> u8 {
        match self.network.parse::<Network>() {
            Ok(Network::BitcoinCash) => SIGHASH_FORKID | SIGHASH_ALL,
            _ => SIGHASH_ALL,
        }
    }
    #[inline]
    pub fn add_signature(&mut self, input_index: usize, signature: &[u8; 64]) -> Result<&mut Self> {
        let signature = Signature::from_compact(signature).map_err(|_| {
            BitcoinError::SignFailure(format!(
                "invalid signature {}",
                signature.encode_hex::<String>()
            ))
        })?;
        let raw_input = &self.inputs[input_index].clone();
        let script_type = ScriptType::from_str(&self.script_type)?;
        let signature_type = self.sig_hash_type();
        let pubkey_slice =
            hex::decode(&raw_input.pubkey).map_err(|_e| BitcoinError::InvalidInput)?;
        let input = &mut self.transaction.input[input_index.clone()];
        if script_type == ScriptType::P2PKH {
            input.script_sig = raw_input.script_sig(signature, signature_type, &script_type)?;
        } else if script_type == ScriptType::P2WPKH {
            let sig = EcdsaSignature {
                sig: signature,
                hash_ty: EcdsaSighashType::All,
            };
            input.witness.push_ecdsa_signature(&sig);
            input.witness.push(pubkey_slice);
        } else if script_type == ScriptType::P2SHP2WPKH {
            let sig = EcdsaSignature {
                sig: signature.clone(),
                hash_ty: EcdsaSighashType::All,
            };
            input.witness.push_ecdsa_signature(&sig);
            input.witness.push(pubkey_slice);
            input.script_sig = raw_input.script_sig(signature, signature_type, &script_type)?;
        } else {
            return Err(BitcoinError::SignFailure(format!(
                "invalid script type {:?}",
                script_type
            )));
        }
        return Ok(self);
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::test::{prepare_parse_context, prepare_payload};
    use crate::transactions::legacy::tx_data::TxOut;
    use alloc::string::ToString;

    #[test]
    // tube
    fn test_p2pkh_script_pubkey() {
        let output = TxOut {
            value: 199996600,
            address: "1NVWpSCxyzpPgSeGRs4zqFciZ7N1UEQtEc".to_string(),
            is_change: false,
            change_address_path: "".to_string(),
        };
        let mapped_output: bitcoin::TxOut = output.try_into().unwrap();

        assert_eq!(
            mapped_output.script_pubkey.encode_hex::<String>(),
            "76a914ebbf2dd7547576c780e7e449936e26e4b63efdfb88ac"
        );
    }

    #[test]
    // tube
    fn test_p2sh_p2pkh_pubkey() {
        let output = TxOut {
            value: 199996600,
            address: "36kTQjs54H29LRhL9WiBSFbNtUyNMNCpEv".to_string(),
            is_change: false,
            change_address_path: "".to_string(),
        };
        let mapped_output: bitcoin::TxOut = output.try_into().unwrap();
        assert_eq!(
            mapped_output.script_pubkey.encode_hex::<String>(),
            "a914377f26ccc4f581811f38a0cdaeb26654b162347987"
        );
    }

    #[test]
    // tube
    fn test_p2wpkh_script_pubkey() {
        let output = TxOut {
            value: 199996600,
            address: "bc1qj9pgg94s75tggzm8u4443stl9yutssuxl0kpu0".to_string(),
            is_change: false,
            change_address_path: "".to_string(),
        };
        let mapped_output: bitcoin::TxOut = output.try_into().unwrap();
        assert_eq!(
            mapped_output.script_pubkey.encode_hex::<String>(),
            "001491428416b0f516840b67e56b58c17f2938b84386"
        );
    }

    #[test]
    fn test_p2pkh_signature_hash() {
        let hex = "1f8b08000000000000035d90cf6a935110c5632a18be4d633686ac62109442c8ccbdf3dd99bbd3c6d08d420537aee4fe99b1584b9a5a89b8f4415cb8f115dcf4010a7d83be806fe046c14fc58d3067318bc3f99d33e88f769f9d2dd755a78767ebf37559bf995ced0cfaa30103af568fc37276b1d334fbcf972f9fac0e1e2d5f8cee792cb91648f3e20acc095b99c79cfc5ccc2b39c31a939ff4a6d7971fbf7c870703f7b33fb8ba3bfc7c73f6e946f310414128040ce640a56a27884256384b0e2e7224c59c8422e5564939438ad55a32b39a6dd89b1c34fbe035ab1a86a2482187448c1ca16d63086c2495a97a0b46ea29b9e22b082791ecbb9824d9098dbfde99ed3e5d10dd5fc0df5bd01fc0ae4a85d6392c0a05d8aa7122c939f36fc45c09b85b8624f9a089c9aab49ab2380da5ade8e41f60a6d0fd41318ac51858adad013b5b04344dde77b50aa690108d0b1278f444d179d36e19d3f18fdbff03c29e6ff672c1cdf1db0da5f7f1f5e96673f2ee646b47af5a3a3edf1eb9181c7f3016e7797b3afc766bdc9bf5a61787bf001ef38242e5010000";
        let pubkey_str = "xpub6Ch68rD9nNm8AReQe9VwV6Mi67okew4oiYoNpKAESTvmjoDn5dBUQ9A4oqdxc9VpveH1cxWeH237HLFSmC37gRVwxnL4KvFYxCQqfprRinW";
        let payload = prepare_payload(hex);
        let context = prepare_parse_context(pubkey_str);
        let tx_data = &mut TxData::from_payload(payload, &context).unwrap();
        let p2pkh_signature_hash = tx_data.signature_hash(0).unwrap();
        assert_eq!(
            "0818c48d32946e54b6f03e5d4597e78ac04c06395751d115db44699cb31991f8",
            p2pkh_signature_hash.encode_hex::<String>()
        );
    }

    #[test]
    fn test_p2pkh_signature_hash_bch() {
        let hex = "1f8b08000000000000030d8d3b4e025114400336131a81ca580131c14c3299f7eefb77061a4b7507f77d2e062423cc88ba0a57a05bb07709d616962cc0d858183b2739d5494e4ed61d1e5e6ee7554ca38b6dd554a1ba397ee9b6363322a8880c274fdddec16c7e3e3c09c149ed131436c95048f0a94001bc501a95401b48713d7afddebffdb1d3eceab393bd0ffa1ff9e4b9d33bd3367a9242a1f4481645f049f3e0c8a055ca5be1109c16c210779c11a04c29044ac854d2013d3fdaff8e273306de27137d88643d80e64a30b4da4432dc32cf0d3ae6a5053292c008eddb138b828cd24942026ef2c1ba94725a72a9a6256b2959ce7af9e6966a5804ba5f08803a2d564b79dd967772d72c1f77f543b3d1eb7ae518aafed7cff81f4c55a87f34010000";
        let pubkey_str = "xpub6ByHsPNSQXTWZ7PLESMY2FufyYWtLXagSUpMQq7Un96SiThZH2iJB1X7pwviH1WtKVeDP6K8d6xxFzzoaFzF3s8BKCZx8oEDdDkNnp4owAZ";
        let payload = prepare_payload(hex);
        let context = prepare_parse_context(pubkey_str);
        let tx_data = &mut TxData::from_payload(payload, &context).unwrap();
        let p2pkh_signature_hash = tx_data.signature_hash(0).unwrap();
        assert_eq!(
            "5049afbbf8b274aae6d7f65cea0d413d1d8c151606a5cead356bb556502f27aa",
            p2pkh_signature_hash.encode_hex::<String>()
        );
    }

    #[test]
    fn test_p2wpkh_signature_hash() {
        let hex = "1f8b0800000000000003658d3b4b42611c87f5d07070e9e4244e22412188effdb29517caa130921ae5ffbee73d8a17f498e6e553b4340611f8151a823e41d027696b288820ad31f80dcff27b1edf4b6f9f8d2bc3d0e51ae3e1646887fdec97e77b695f2259ab554525ffe6a576cacd4aebf4b059bfa8b5ce6b4797f5667a5771ee8cc3bc4811124566382d2ab05004ee2270cc1029c36c22f7f8f9bdfa40fb3e5979fe6b2678d8cadf275307c2027296600c587050a112064ba4003bc3a5c004bbc882309a6b4d210a29a20c21b329291552428264f6385546c49ad051a5b1a2c2ad554e6bc04681d09668ceb4e31b00bb4ea9b5433106045b4c8120c443a121f37e97ce072725c5f64ae877b8444981a50ac6e2b877153398ebee288e07d3c12ceab439eb4d661da20591cb482a42e56c14dcbc2433897ca250fd7bcdf56241898d05a1f162acd4a2ddebb6afbbedfe14f59642f0100d74a483dba72093fcd7a6b9e7c60f67e05bf594010000";
        let pubkey_str = "zpub6rQ4BxDEb1xJE2RJTyaoZT2NEr7FkGCgeEwkbgpBfPUTd6KmtBstbP9G81dPnJZJVAmbg2ZmfSc55FkrcHcKPvmNkLGmXAFqERtiUCn25LH";
        let payload = prepare_payload(hex);
        let context = prepare_parse_context(pubkey_str);
        let tx_data = &mut TxData::from_payload(payload, &context).unwrap();
        let signature_hash = tx_data.signature_hash(0).unwrap();
        assert_eq!(
            "95efa1aff5e340a71c7f4e1afd0282853b1b50e5801691add913a1bd20cb1926",
            signature_hash.encode_hex::<String>()
        );
    }

    #[test]
    fn test_p2sh_p2wpkh_signature_hash() {
        let hex = "1f8b08000000000000ff5d8fb16bd4411085f14ef0388b8454e1aa700811e1b8999dd9d99d4e730645f4b818112c7776670b0939120e0b2bff103bc156b0f46fb1b0b4107b3b7fc44e78cd3c988fef4d46077b67d7ab6df3a3cdf576b7addb8bd9d7f1d04e12a4d3d3c7b29a7f1a4fc727af5607f76a5516f3b0c8ce75c1c17c5128e0224a895472ed11e5e8cbef1fdffec0fd49f8359a84fd0fa3f9c75bd38792624a59aa6517ee1c6a278fc325ca033273909e81aaa0b93b2150d4ac21aaa92258c9b327d31308484da44a4ed2b281b55c232a23472c1a093473cfc59b70c39c30abd6864906aea2694be1f0f3edf9de8b25ebf112fe6509377a4c0edd9a0ecfad99d4ea54802163007524714b665124040b05a1296182ead952005029377a641255057b8f894ba786053db14b37d19806d9c6a6587818eea5162c503ba7a883716c03f1f0e79dfff5f0c1f1744eebf5f9d5a3f7f4bcbc5dbf133f3fdb5d064aabdda63c6dcfda9bcdeb97fbdfeffe05f955ecdcc8010000";
        let pubkey_str = "ypub6WuuYTJLGMKvLmzxPSU28HTcUFeeaMHrycuJr4Q6JZe7suJWuZLEYgzHYUF9MJ4Y17q4AcVKgJaLa2UwvWPRartJVqcuhMCK5TFGCfZXhFD";
        let payload = prepare_payload(hex);
        let context = prepare_parse_context(pubkey_str);
        let tx_data = &mut TxData::from_payload(payload, &context).unwrap();
        let signature_hash = tx_data.signature_hash(0).unwrap();
        assert_eq!(
            "1fd732e7c2464592fb12c5248e6f1488daabea524e5886389fd539cf3d349e2c",
            signature_hash.encode_hex::<String>(),
        );
    }
}
