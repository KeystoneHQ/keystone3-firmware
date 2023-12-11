#![no_std]

extern crate alloc;
use alloc::boxed::Box;
use alloc::string::{String, ToString};
use alloc::vec;
use alloc::vec::Vec;

use cty::c_char;
use third_party::hex;

use third_party::ur_registry::bytes::Bytes;
use third_party::ur_registry::crypto_key_path::{CryptoKeyPath, PathComponent};
use third_party::ur_registry::crypto_psbt::CryptoPSBT;
use third_party::ur_registry::traits::RegistryItem;

use common_rust_c::errors::ErrorCodes;
use common_rust_c::ffi::CSliceFFI;
use common_rust_c::structs::ExtendedPublicKey;
use common_rust_c::types::PtrDecoder;
use common_rust_c::ur::{
    decode_ur, receive, UREncodeResult, URParseMultiResult, URParseResult, URType, ViewType,
    FRAGMENT_MAX_LENGTH_DEFAULT,
};
use common_rust_c::utils::{convert_c_char, recover_c_char};

use wallet_rust_c::get_connect_blue_wallet_ur;

#[no_mangle]
pub extern "C" fn test_get_crypto_psbt() -> *mut URParseResult {
    let psbt = CryptoPSBT::new(hex::decode("70736274ff01005202000000016d41e6873468f85aff76d7709a93b47180ea0784edaac748228d2c474396ca550000000000fdffffff01a00f0000000000001600146623828c1f87be7841a9b1cc360d38ae0a8b6ed0000000000001011f6817000000000000160014d0c4a3ef09e997b6e99e397e518fe3e41a118ca1220602e7ab2537b5d49e970309aae06e9e49f36ce1c9febbd44ec8e0d1cca0b4f9c3191873c5da0a54000080010000800000008000000000000000000000").unwrap());
    URParseResult::single(ViewType::BtcTx, URType::CryptoPSBT, psbt).c_ptr()
}

#[no_mangle]
pub extern "C" fn test_get_btc_keystone_bytes() -> *mut URParseResult {
    let bytes = Bytes::new(hex::decode("1f8b0800000000000003ad8e3f6b935114c689510959123b854ce545500a21f79e7bcefdb36943a91d14c5a063b9e7dc7b108c491b0df912ee2e05c12fe0ee8710fc061d9dc4b1ddfa429742d7c2333c3cc3eff9f5eeed0cdeac67ab52775faf575f56b25a8ccfbbedda0b4ea864939b3fddfea3fdf9ecf8d5f3f9d1bb83e3b70787ef8fe63b8f45127aae3089156582c075921dd809f94c2e4751b27ef7e7bff35f97e6690fbe767bbf47c31ff79bb34eff998fc8ce8126a7925204086c5995c187546c4a89c0327bb462a2538b1923615b8ca9c4e8717cd8df37ce9942a8c5948260b2afa4b1380fa8a40e2348ae8e8cb6445213c8b112d7aa49a0249bd5c9e82236834fd3884fa6e63a53d37c6ff5587d0a106d604915b268ca56347b66eb2d5355eba2a10c364bb0220801ab27058e4cdc3e0e3be3177722f8edef835b867bb3fe1e8b3d8de2f5f3872d94c576b30cf5e3329d6ed505d9c07a1988362772e2eb62f8ffece1a8d30c5ede80d8a9b90290f88bd8f6010000").unwrap());
    URParseResult::single(ViewType::BtcNativeSegwitTx, URType::Bytes, bytes).c_ptr()
}

fn get_ur_encode_result_for_test(data: Vec<u8>, length: usize, tag: String) -> *mut UREncodeResult {
    let result = third_party::ur_parse_lib::keystone_ur_encoder::probe_encode(&data, length, tag);
    match result {
        Ok(result) => {
            if result.is_multi_part {
                UREncodeResult::multi(result.data, result.encoder.unwrap()).c_ptr()
            } else {
                UREncodeResult::single(result.data).c_ptr()
            }
        }
        Err(e) => UREncodeResult::error(ErrorCodes::BitcoinInvalidInput, e.to_string()).c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn test_encode_crypto_psbt() -> *mut UREncodeResult {
    let psbt = CryptoPSBT::new(hex::decode("70736274ff01005202000000016d41e6873468f85aff76d7709a93b47180ea0784edaac748228d2c474396ca550000000000fdffffff01a00f0000000000001600146623828c1f87be7841a9b1cc360d38ae0a8b6ed0000000000001011f6817000000000000160014d0c4a3ef09e997b6e99e397e518fe3e41a118ca1220602e7ab2537b5d49e970309aae06e9e49f36ce1c9febbd44ec8e0d1cca0b4f9c3191873c5da0a54000080010000800000008000000000000000000000").unwrap());
    let result = get_ur_encode_result_for_test(
        psbt.try_into().unwrap(),
        FRAGMENT_MAX_LENGTH_DEFAULT.clone(),
        CryptoPSBT::get_registry_type().get_type(),
    );
    result
}

#[no_mangle]
pub extern "C" fn test_decode_crypto_psbt() -> *mut URParseResult {
    let _ur = "ur:crypto-psbt/hdcxlkahssqzwfvslofzoxwkrewngotktbmwjkwdcmnefsaaehrlolkskncnktlbaypkvoonhknt";
    let ur = "UR:CRYPTO-PSBT/HDRKJOJKIDJYZMADAEGMAOAEAEAEADJNFPVALTEEISYAHTZMKOTSJONYMUQZJSLAWDATLRWEPKSTFDCPLGDWFLFXMTSGGOAEAEAEAEAEZCZMZMZMADNBBSAEAEAEAEAEAECMAEBBIYCNLFLKCTLTRNKSFPPTPASFENBTETPLBKLUJTTIAEAEAEAEAEADADCTISCHAEAEAEAEAEAECMAEBBTISSOTWSASWLMSRPWLNNESKBGYMYVLVECYBYLKOYCPAMAOVDPYDAEMRETYNNMSAXASPKVTJTNNGAWFJZVYSOZERKTYGLSPVTTTSFNBQZYTSRCFCSJKSKTNBKGHAEAELAADAEAELAAEAEAELAAEAEAEAEAEAEAEAEAEAEMNSFFGMW";
    let ptr = decode_ur(ur.to_string()).c_ptr();
    ptr
}

#[no_mangle]
pub extern "C" fn test_decode_btc_keystone_sign_result() -> *mut URParseResult {
    let ur = "UR:BYTES/HKAOATCTLUAYAEAEAEAEAEAEAXFSMOFSIMCKFPBNLNCLBEIATDAAHHRDJPMETBCXSNISDKGTPKLAVLUEESSAVSPEBKCSGSJTMECWOXSGFEJPRLVSLUGTAARKQDWPSFFDWSZOFDTSWSJTFMKBKGKKKSMNRFKGKNKKZEWNWPTKURJLLBHLYKURJEADKKKEZTSGBSNELBHYKIYAEETTDPCETORYBSLTKGSAONYLUYTORFTSNYGAOTDYYNNLEMHEAYEODKYLAETDKPMWPDRTTODRDLFRLROXPKSFMEBDTKVOIAPDBGLYFDWLBGNSHYZCLDJPZOZOFSBNKSLGHSGOGYREDEWDJZLSMWDKHPNDMWWPDKJTTNSAFXGTPSHHASRPFGOLAMIEBTIEIYONAAWPBBPESELTIEAOBYLKADJEBYNDJTUEIYPKDTJSNSHDDMVOSSRDMTVENYZOPSDPIDGMNDJKROAYLKBWSBHLYLHYDNVOMOIAQZJLTEMOFSDPQDQDBSNTENJELKAEEMFZHHUYHLGDDMGEKEOEPRNYENOYSWCMKNAMVYIYRTLYDYQDDLHGCTGAJZFECHLDDERKVWJNIYDTTDBELEHKHHMWMUTOWTCFNBJPGHRKMNYLIMFXWLSAVABGFXAXKIDAWMETRPEYKNVLAYVDNLOXCXCPAACMCHFGBTSGPDGUFYGAESEECAJLCNWMFYLSKIBDWYZCOLQZTNHSOTEOLPOESWPAMWSOTNTLAHFTCAWDKOHDRLPKZOISIMTTKEENMNCENLDEVOFPGRKGIEVAAEDLWYDWRTCLBDUOKKWDMEHYIOFTPSGLKEFWAEOLNBEMTTSETLHLCELGHLTASGHTDNIMPKWPJKHNVSDNDICNHYLYUODPTSTNNDDAJEAHOTNNRHADPYKIGLETUYWNWTFPDWJSDKMKETLDYNMKTLSRAEZCKNVLAAMKBKQDLGZEBSVEKBSPAYJOVWJTOLTIBZDPESRFATKGWPIHCNTNAEBKNTLFFSQDDLZTLURKFHGWLBADESSERKZTEOAXAEAEJTMNRPEC";
    let ptr = decode_ur(ur.to_string()).c_ptr();
    ptr
}

#[no_mangle]
pub extern "C" fn test_decode_keystone_sign_request() -> *mut URParseResult {
    let ur = "UR:BYTES/HKADHHCTLUAYAEAEAEAEAEAEAXGOLGRKGESRGDAEFZTETICLJYINWEGHFTMDCXDESKMHZOKBCXLFGWBEPYMHHTLKTNWEVAFMQZOEFGPSECVOWPDIVSPKHNDIRSSAPLFTYAATVOKTRONLGOETVLVYNSRTJLTBZOTSCWRHPANTVEFTRSSOKPKBUEZEYNAXRSCFGDSSASVYAELTHEKBPMRDRHENTPJTTOFRGRMWEOBAFYCSDYCFBYIAHLDKDKPABYTKCSJPFZDNDSMHJZKGNTWLTEWPYLADDMAMSRFLFHYANSJERFHFUYHYYAHHPTPMIMLYDESGAYKOBEHPDECAMDCEBGVLBBFYGMBDBABZNBGLHNCXISAMLDDSNYCYLPNLLYCYFTLRROBBNSCWTPWTHTJLFHTEGEROBAMHPFLKMDBAHFCEAOPDREAEMTINGUAMADAEHHGRASOYLRTOAYIEAAREFGBDLEEOVYNBSAOEKEIDFGMWCMUTZSFEGSSPFWGSCHIDGDBGLSWYGMDPFNRDCENSVSEERYNEWKLUSRCPWLUTVWBWJZTLCSYLBSKTTEJYROJKDYDRJZIAYNPFTDWZFWPERKHHTNLPFTFRDPFGZOIAMSVTPMVLTYHLHLMNMOHPPRLBEECSDSASRKVSYKWSPRSWSBLTJEGOSAZSUEZMGOVDFSYTAXIMETWZMWKOADAEAEHSWYCHGS";
    let ptr = decode_ur(ur.to_string()).c_ptr();
    ptr
}

#[no_mangle]
pub extern "C" fn test_decode_crypto_psbt_1() -> *mut URParseResult {
    let ur1 = "ur:crypto-psbt/1-3/lpadaxcfaxiacyvwhdfhndhkadclhkaxhnlkahssqzwfvslofzoxwkrewngotktbmwjkwdcmnefsaaehrlolkskncnktlbaypklkahssqzwfvslofzoxwkrewngotktbmwjkwdcmnefsaaehrlolkskncnktlbaypklkahssqzwfvslofzoxwkrewngotktbmwjkwdcmnefsaaehrlolkskncnktlbaypklkahssqzwfvslofzoxwkrewngotktbmwjkwdcmnefsaaehrlolkskncnktlbaypklkahssqzwfvslofzoxwkrewngotktbmwjkwdcmnefsaaehrlolkskncnktlbaypklkahssqzwfvslofzoxwkrewngotktbmwjkwdcmnefsaaehrlolkskncnktlbaypklkahssqzwfvslofzoxwkrewngotktbmwjkwdcmnefsaaehrlolkskncnktlbaypklkahssqzwfvslofzoxwkrewngotktbmwjkwdcmnefsaaehrlolkskncnktlbaypklkahssqzwfvslofzoxwkrewngotktbmwjkwdcmnefsaaehrlolkskncnktlbnychpmiy";
    let ptr = decode_ur(ur1.to_string()).c_ptr();
    ptr
}

#[no_mangle]
pub extern "C" fn test_decode_crypto_psbt_2(decoder: PtrDecoder) -> *mut URParseMultiResult {
    let ur2 = "ur:crypto-psbt/2-3/lpaoaxcfaxiacyvwhdfhndhkadclaypklkahssqzwfvslofzoxwkrewngotktbmwjkwdcmnefsaaehrlolkskncnktlbaypklkahssqzwfvslofzoxwkrewngotktbmwjkwdcmnefsaaehrlolkskncnktlbaypklkahssqzwfvslofzoxwkrewngotktbmwjkwdcmnefsaaehrlolkskncnktlbaypklkahssqzwfvslofzoxwkrewngotktbmwjkwdcmnefsaaehrlolkskncnktlbaypklkahssqzwfvslofzoxwkrewngotktbmwjkwdcmnefsaaehrlolkskncnktlbaypklkahssqzwfvslofzoxwkrewngotktbmwjkwdcmnefsaaehrlolkskncnktlbaypklkahssqzwfvslofzoxwkrewngotktbmwjkwdcmnefsaaehrlolkskncnktlbaypklkahssqzwfvslofzoxwkrewngotktbmwjkwdcmnefsaaehrlolkskncnktlbaypklkahssqzwfvslofzoxwkrewngotktbmwjkwdcmnefsaaehrlolkskncnktlbaylbntahvo";
    receive(convert_c_char(ur2.to_string()), decoder)
}

#[no_mangle]
pub extern "C" fn test_decode_crypto_psbt_3(decoder: PtrDecoder) -> *mut URParseMultiResult {
    let ur3 = "ur:crypto-psbt/3-3/lpaxaxcfaxiacyvwhdfhndhkadclpklkahssqzwfvslofzoxwkrewngotktbmwjkwdcmnefsaaehrlolkskncnktlbaypklkahssqzwfvslofzoxwkrewngotktbmwjkwdcmnefsaaehrlolkskncnktlbaypklkahssqzwfvslofzoxwkrewngotktbmwjkwdcmnefsaaehrlolkskncnktlbaypklkahssqzwfvslofzoxwkrewngotktbmwjkwdcmnefsaaehrlolkskncnktlbaypklkahssqzwfvslofzoxwkrewngotktbmwjkwdcmnefsaaehrlolkskncnktlbaypklkahssqzwfvslofzoxwkrewngotktbmwjkwdcmnefsaaehrlolkskncnktlbaypklkahssqzwfvslofzoxwkrewngotktbmwjkwdcmnefsaaehrlolkskncnktlbaypklkahssqzwfvslofzoxwkrewngotktbmwjkwdcmnefsaaehrlolkskncnktlbaypklkahssqzwfvslofzoxwkrewngotktbmwjkwdcmnefsaaehrlolkskncnktlbaypknseoskve";
    receive(convert_c_char(ur3.to_string()), decoder)
}

#[no_mangle]
pub extern "C" fn test_connect_blue_wallet() -> *mut UREncodeResult {
    let mfp: [u8; 4] = [115, 197, 218, 10];

    let mfp = Box::into_raw(Box::new(mfp)) as *mut u8;
    let length = 4;
    let legacy_x_pub = "xpub6CatWdiZiodmUeTDp8LT5or8nmbKNcuyvz7WyksVFkKB4RHwCD3XyuvPEbvqAQY3rAPshWcMLoP2fMFMKHPJ4ZeZXYVUhLv1VMrjPC7PW6V";
    let nested_x_pub = "xpub6BosfCnifzxcFwrSzQiqu2DBVTshkCXacvNsWGYJVVhhawA7d4R5WSWGFNbi8Aw6ZRc1brxMyWMzG3DSSSSoekkudhUd9yLb6qx39T9nMdj";
    let native_x_pub = "xpub6C6nQwHaWbSrzs5tZ1q7m5R9cPK9eYpNMFesiXsYrgc1P8bvLLAet9JfHjYXKjToD8cBRswJXXbbFpXgwsswVPAZzKMa1jUp2kVkGVUaJa7";

    let legacy_x_pub = convert_c_char(legacy_x_pub.to_string());
    let nested_x_pub = convert_c_char(nested_x_pub.to_string());
    let native_x_pub = convert_c_char(native_x_pub.to_string());

    let k1 = ExtendedPublicKey {
        path: convert_c_char("m/84'/0'/0'".to_string()),
        xpub: native_x_pub,
    };
    let k2 = ExtendedPublicKey {
        path: convert_c_char("m/49'/0'/0'".to_string()),
        xpub: nested_x_pub,
    };
    let k3 = ExtendedPublicKey {
        path: convert_c_char("m/44'/0'/0'".to_string()),
        xpub: legacy_x_pub,
    };

    let mut keys = [k1, k2, k3];
    let ptr = keys.as_mut_ptr();
    let extended_public_keys = CSliceFFI {
        data: ptr,
        size: keys.len(),
    }
    .c_ptr();

    get_connect_blue_wallet_ur(mfp, length, extended_public_keys)
}
