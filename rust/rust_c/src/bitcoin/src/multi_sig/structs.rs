use alloc::boxed::Box;
use alloc::format;
use alloc::string::ToString;

use alloc::vec::Vec;
use app_bitcoin::multi_sig::wallet::{BsmsWallet, MultiSigWalletConfig};
use app_bitcoin::multi_sig::{MultiSigFormat, MultiSigType, MultiSigXPubInfo, Network};
use common_rust_c::ffi::{VecFFI};
use common_rust_c::free::Free;
use common_rust_c::types::{Ptr, PtrBytes, PtrString, PtrT};
use common_rust_c::ur::UREncodeResult;
use common_rust_c::utils::{convert_c_char, recover_c_char};
use common_rust_c::{check_and_free_ptr, free_str_ptr, free_vec, impl_c_ptr, make_free_method};

#[repr(C)]
pub enum NetworkType {
    MainNet,
    TestNet,
}

impl Into<Network> for NetworkType {
    fn into(self) -> Network {
        match self {
            NetworkType::MainNet => Network::MainNet,
            NetworkType::TestNet => Network::TestNet,
        }
    }
}

impl Into<Network> for &NetworkType {
    fn into(self) -> Network {
        match self {
            NetworkType::MainNet => Network::MainNet,
            NetworkType::TestNet => Network::TestNet,
        }
    }
}

#[repr(C)]
pub enum MultiSigFormatType {
    P2sh,
    P2wshP2sh,
    P2wsh,
    P2shTest,
    P2wshP2shTest,
    P2wshTest,
}

impl Into<MultiSigType> for MultiSigFormatType {
    fn into(self) -> MultiSigType {
        match self {
            MultiSigFormatType::P2sh => MultiSigType::P2sh,
            MultiSigFormatType::P2wshP2sh => MultiSigType::P2wshP2sh,
            MultiSigFormatType::P2wsh => MultiSigType::P2wsh,
            MultiSigFormatType::P2shTest => MultiSigType::P2shTest,
            MultiSigFormatType::P2wshP2shTest => MultiSigType::P2wshP2shTest,
            MultiSigFormatType::P2wshTest => MultiSigType::P2wshTest,
        }
    }
}

impl Into<MultiSigType> for &MultiSigFormatType {
    fn into(self) -> MultiSigType {
        match self {
            MultiSigFormatType::P2sh => MultiSigType::P2sh,
            MultiSigFormatType::P2wshP2sh => MultiSigType::P2wshP2sh,
            MultiSigFormatType::P2wsh => MultiSigType::P2wsh,
            MultiSigFormatType::P2shTest => MultiSigType::P2shTest,
            MultiSigFormatType::P2wshP2shTest => MultiSigType::P2wshP2shTest,
            MultiSigFormatType::P2wshTest => MultiSigType::P2wshTest,
        }
    }
}

impl Into<MultiSigFormat> for MultiSigFormatType {
    fn into(self) -> MultiSigFormat {
        match self {
            MultiSigFormatType::P2sh => MultiSigFormat::P2sh,
            MultiSigFormatType::P2wshP2sh => MultiSigFormat::P2wshP2sh,
            MultiSigFormatType::P2wsh => MultiSigFormat::P2wsh,
            MultiSigFormatType::P2shTest => MultiSigFormat::P2sh,
            MultiSigFormatType::P2wshP2shTest => MultiSigFormat::P2wshP2sh,
            MultiSigFormatType::P2wshTest => MultiSigFormat::P2wsh,
        }
    }
}

impl Into<MultiSigFormat> for &MultiSigFormatType {
    fn into(self) -> MultiSigFormat {
        match self {
            MultiSigFormatType::P2sh => MultiSigFormat::P2sh,
            MultiSigFormatType::P2wshP2sh => MultiSigFormat::P2wshP2sh,
            MultiSigFormatType::P2wsh => MultiSigFormat::P2wsh,
            MultiSigFormatType::P2shTest => MultiSigFormat::P2sh,
            MultiSigFormatType::P2wshP2shTest => MultiSigFormat::P2wshP2sh,
            MultiSigFormatType::P2wshTest => MultiSigFormat::P2wsh,
        }
    }
}

#[repr(C)]
pub struct MultiSigXPubInfoItem {
    path: PtrString,
    xfp: PtrString,
    xpub: PtrString,
}

impl From<MultiSigXPubInfo> for MultiSigXPubInfoItem {
    fn from(value: MultiSigXPubInfo) -> Self {
        MultiSigXPubInfoItem {
            path: convert_c_char(value.path.to_string()),
            xfp: convert_c_char(value.xfp.to_string()),
            xpub: convert_c_char(value.xpub.to_string()),
        }
    }
}

impl Into<MultiSigXPubInfo> for &MultiSigXPubInfoItem {
    fn into(self) -> MultiSigXPubInfo {
        MultiSigXPubInfo {
            path: recover_c_char(self.path),
            xfp: recover_c_char(self.xfp),
            xpub: recover_c_char(self.xpub),
        }
    }
}

impl From<BsmsWallet> for MultiSigXPubInfoItem {
    fn from(value: BsmsWallet) -> Self {
        MultiSigXPubInfoItem {
            path: convert_c_char(value.derivation_path.to_string()),
            xfp: convert_c_char(value.xfp.to_string()),
            xpub: convert_c_char(value.extended_pubkey.to_string()),
        }
    }
}

impl Into<BsmsWallet> for &MultiSigXPubInfoItem {
    fn into(self) -> BsmsWallet {
        BsmsWallet {
            bsms_version: "BSMS 1.0".to_string(),
            derivation_path: recover_c_char(self.path),
            xfp: recover_c_char(self.xfp),
            extended_pubkey: recover_c_char(self.xpub),
        }
    }
}

impl_c_ptr!(MultiSigXPubInfoItem);

impl Free for MultiSigXPubInfoItem {
    fn free(&self) {
        free_str_ptr!(self.xfp);
        free_str_ptr!(self.xpub);
        free_str_ptr!(self.path);
    }
}
make_free_method!(MultiSigXPubInfoItem);

#[repr(C)]
pub struct MultiSigXPubItem {
    xfp: PtrString,
    xpub: PtrString,
}

impl From<&app_bitcoin::multi_sig::wallet::MultiSigXPubItem> for MultiSigXPubItem {
    fn from(value: &app_bitcoin::multi_sig::wallet::MultiSigXPubItem) -> Self {
        MultiSigXPubItem {
            xfp: convert_c_char(value.xfp.to_string()),
            xpub: convert_c_char(value.xpub.to_string()),
        }
    }
}

impl Into<app_bitcoin::multi_sig::wallet::MultiSigXPubItem> for &MultiSigXPubItem {
    fn into(self) -> app_bitcoin::multi_sig::wallet::MultiSigXPubItem {
        app_bitcoin::multi_sig::wallet::MultiSigXPubItem {
            xfp: recover_c_char(self.xfp),
            xpub: recover_c_char(self.xpub),
        }
    }
}

impl_c_ptr!(MultiSigXPubItem);

impl Free for MultiSigXPubItem {
    fn free(&self) {
        free_str_ptr!(self.xfp);
        free_str_ptr!(self.xpub);
    }
}

#[repr(C)]
pub struct MultiSigWallet {
    creator: PtrString,
    name: PtrString,
    policy: PtrString,
    threshold: u32,
    total: u32,
    derivations: PtrT<VecFFI<PtrString>>,
    format: PtrString,
    xpub_items: PtrT<VecFFI<MultiSigXPubItem>>,
    verify_code: PtrString,
    config_text: PtrString,
    network: u32,
}

impl From<MultiSigWalletConfig> for MultiSigWallet {
    fn from(value: MultiSigWalletConfig) -> Self {
        MultiSigWallet {
            creator: convert_c_char(value.creator.clone()),
            name: convert_c_char(value.name.clone()),
            policy: convert_c_char(format!("{} of {}", value.threshold, value.total)),
            threshold: value.threshold,
            total: value.total,
            derivations: VecFFI::from(
                value
                    .derivations
                    .clone()
                    .iter()
                    .map(|v| convert_c_char(v.to_string()))
                    .collect::<Vec<_>>(),
            )
            .c_ptr(),
            format: convert_c_char(value.format.clone()),
            xpub_items: VecFFI::from(
                value
                    .xpub_items
                    .clone()
                    .iter()
                    .map(|v| MultiSigXPubItem::from(v))
                    .collect::<Vec<MultiSigXPubItem>>(),
            )
            .c_ptr(),
            verify_code: convert_c_char(value.verify_code.clone()),
            config_text: convert_c_char(value.config_text.clone()),
            network: value.get_network_u32(),
        }
    }
}

impl Into<MultiSigWalletConfig> for &mut MultiSigWallet {
    fn into(self) -> MultiSigWalletConfig {
        MultiSigWalletConfig {
            creator: recover_c_char(self.creator),
            name: recover_c_char(self.name),
            threshold: self.threshold,
            total: self.total,
            derivations: {
                let rebuilt = unsafe {
                    let ptr = (*self.derivations).data;
                    let size = (*self.derivations).size;
                    let cap = (*self.derivations).cap;

                    let ptr = ptr as PtrT<PtrString>;
                    Vec::from_raw_parts(ptr, size, cap)
                };

                rebuilt
                    .iter()
                    .map(|v| recover_c_char(*v))
                    .collect::<Vec<_>>()
            },
            format: recover_c_char(self.format),

            xpub_items: {
                let rebuilt = unsafe {
                    let ptr = (*self.derivations).data;
                    let size = (*self.derivations).size;
                    let cap = (*self.derivations).cap;

                    let ptr = ptr as PtrT<MultiSigXPubItem>;
                    Vec::from_raw_parts(ptr, size, cap)
                };
                rebuilt.iter().map(|v| v.into()).collect::<Vec<_>>()
            },
            verify_code: recover_c_char(self.verify_code),
            config_text: recover_c_char(self.config_text),
            network: if self.network == 0 {
                Network::MainNet
            } else {
                Network::TestNet
            },
        }
    }
}

impl Free for MultiSigWallet {
    fn free(&self) {
        free_str_ptr!(self.creator);
        free_str_ptr!(self.name);
        free_str_ptr!(self.policy);
        free_vec!(self.derivations);
        free_str_ptr!(self.format);
        free_vec!(self.xpub_items);
        free_str_ptr!(self.verify_code);
        free_str_ptr!(self.config_text);
    }
}

impl_c_ptr!(MultiSigWallet);

make_free_method!(MultiSigWallet);

#[repr(C)]
pub struct MultisigSignResult {
    pub ur_result: Ptr<UREncodeResult>,
    pub sign_status: PtrString,
    pub is_completed: bool,
    pub psbt_hex: PtrBytes,
    pub psbt_len: u32,
}

impl Free for MultisigSignResult {
    fn free(&self) {
        free_str_ptr!(self.sign_status);
        unsafe {
            if !self.psbt_hex.is_null() {
                let _x = Box::from_raw(self.psbt_hex);
            }
        }
        //do not free ur_result because it will be released by itself;
    }
}

impl_c_ptr!(MultisigSignResult);
make_free_method!(MultisigSignResult);
