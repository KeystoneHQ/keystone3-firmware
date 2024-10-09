#![no_std]

extern crate alloc;

use alloc::string::ToString;
use alloc::vec;

use cty::c_char;
use third_party::hex;

use third_party::ur_registry::aptos::aptos_sign_request::AptosSignRequest;
use third_party::ur_registry::bytes::Bytes;
use third_party::ur_registry::cardano::cardano_sign_request::CardanoSignRequest;
use third_party::ur_registry::cosmos::cosmos_sign_request::CosmosSignRequest;
use third_party::ur_registry::cosmos::evm_sign_request::EvmSignRequest;
use third_party::ur_registry::crypto_key_path::{CryptoKeyPath, PathComponent};

use third_party::ur_registry::ethereum::eth_sign_request::{DataType, EthSignRequest};
use third_party::ur_registry::near::near_sign_request::NearSignRequest;
use third_party::ur_registry::solana::sol_sign_request::SolSignRequest;
use third_party::ur_registry::sui::sui_sign_request::SuiSignRequest;

use common_rust_c::ur::{QRCodeType, URParseResult, ViewType};
use common_rust_c::utils::recover_c_char;

#[no_mangle]
pub extern "C" fn test_get_bch_keystone_succeed_bytes() -> *mut URParseResult {
    let bytes = Bytes::new(hex::decode("1f8b0800000000000003658dbb4e0241144003361b1a818a58c1c604b3c966e73db39d819858aa7f70e7ce5c88aeac3c43f8173b7b0b3b3fc1da0fe003d4c6c2d8496f72aa539c9334bbc7d78b711d62ff6a51af6aacab9397e6c12656a20ec0207d6ab68e46e3cbee2962a98c8f22775161ae848f3948c1736d404b70489a9bfef3d7fef5979d25379f8de4add37ecfd2c746ebdcb8e049490dca033990e8a3e1589205a7b577b204511a292df1923312a06244a4084c4783e0796fff3348474c781f6df018c879210cd79281333690e58e796ea1645e39415691b0d2f8c3890549569ba84414dc669dfb42a961c1951e16ec40c1b28b56367f704b0ad3c96d35376e5aedeea0ac70b95de16cb3decc02dbceb7eb09ed76a8db1fdf835e23fd97e17f24a9ccb649010000").unwrap());
    URParseResult::single(ViewType::BchTx, QRCodeType::Bytes, bytes).c_ptr()
}

#[no_mangle]
pub extern "C" fn test_get_ltc_keystone_bytes() -> *mut URParseResult {
    let bytes = Bytes::new(hex::decode("1f8b08000000000000030d8fbb4a0341144049b458b64948155285458808cbcee3de79749228421e6250b19ebd33830bc2c09242eded2d2c53f903f67e44bec12fb0b773e15487d39cac3f1a6cdb45f2617ad3a65da2f434f9e87736d392d03be68ab77e7eb4be5b8c4e882ca83a88d204a012441d4a27052f513994ce5044aea65fbf3fdf7fec34ab0fbdec301e7e1e17fb5e7e8ee0d0684384ca684d96b840cb312a3484516a19980062442c30e6410b92d251ed144a2d248f93ab7cce449738a594359e111885ce686e9cf41249591539b0009e230988e02d700c68adafa5d7dd499476fc7e5d0c3615d859256615eba8c4d92c2f36b7ebfae1de37172b4c293d36bbe5563c07b55dbc3497afabd4ce61b85ffe0373cb71792a010000").unwrap());
    URParseResult::single(ViewType::LtcTx, QRCodeType::Bytes, bytes).c_ptr()
}

#[no_mangle]
pub extern "C" fn test_get_dash_keystone_bytes() -> *mut URParseResult {
    let bytes = Bytes::new(hex::decode("1f8b08000000000000030d8dbb4a03411440c962b12445622ab10a8b10092c3b337befccddce1771090889124cec66eecc14625808e617fc033bffc1de4fb0b0b2b4f00bacb573e154a738274d86fdc5f6bcf16134df368f0d370f87cf496b535446b230327b4aba7b17a737f5f088b902ed82ca2900e7a05cc86da9648eda626989234a3d7afdf97efb13c7e9dd47277ddf1f7c4eb2974ef70431ea28a4d29e5dd4c04a685b5949825a90502a563e04e2f66a24392bc8b091100c69e3050c9283df5e76269453ce79b402aa207d44aa5c553a8c3156912080630b467970a50ea0c9b515172acd42954cd26b3be95f1500e302c7856829cac9b89bad366b9c2d97b7bbf9252f96ab7a8610a7f5d4eeaed7f79ba626bf187cf5fe011f6b55132b010000").unwrap());
    URParseResult::single(ViewType::DashTx, QRCodeType::Bytes, bytes).c_ptr()
}

#[no_mangle]
pub extern "C" fn test_get_bch_keystone_bytes() -> *mut URParseResult {
    let bytes = Bytes::new(hex::decode("1f8b08000000000000035d90bd4ac3501886695c4217db4ec5a90d42251072fecfc926ad83a37a07dff96b698d6993b43617a3b7e0ee1588b3ee1d9cc54d0407330beff4c2c3034f180c8eafcb5961dde8aa2ceac214b7271f41fb86921a6e0141f41e748fa6b3cbc1a9311913da914439661246b44b80129c70019c82329e63317afa3c3cffa0b3f0e631085ffbbdb7387ae874cf85b2da33ca8169f00aa8d14e609379098a73ad68062413944a8f338c3c01e69c31de01e24e18d07878f81e475344b476d26a63bdd28408cc290225a4f5122ba4b1840c69a68897cc1349856e4dc8522fb9708c388265dccf53c62629667c92a276298a5137deac7d45e6c6dfcf2921959baf966cd1925bb6ab97cdaedad71b9157ab0c01ef7dbd8ce38b962815b80594db7553e60bbf4596d536afee0c6fda102b261bb129ab6555ee7bbff1b013fdf3e214fd018ef1b3447a010000").unwrap());
    URParseResult::single(ViewType::BchTx, QRCodeType::Bytes, bytes).c_ptr()
}

#[no_mangle]
pub extern "C" fn test_get_tron_keystone_bytes() -> *mut URParseResult {
    let bytes = Bytes::new(hex::decode("1f8b08000000000000031590bf4ac3501c46359452bba871299d4a102a42c8bffbbbf7c6499b1403b6b14d52b42e92e426b5c53636462979029d7d01477707279f40147c0007df41707130856f3870a6f355387ebd9f1a098b1abd34c99230b9acbf70158eaf1099b4db26368427ae5af29c639bdf0e98a652d50fc4500922110121a21efb548c028010142d8814bdbed995106a4a8a0e4d492e26c98defb78ffb3f79a7dcfa5ae505cf21b6359f4447fdc5a1678ce99c9e0dd1558726999b8f269d09ceea82e7b96408dab58bd23c358deccc1fdf38f97cc114ec6746a40e1c41f05cc87b89814edbada9756bda07b3d9893ab2b46eff22746c3c76a6bb2b6a49d129d9b3abfb3e8be3400335f4090d3506818c303042402f0c669851888160504286502c2b408b001d01f5fd40d6286c3c7f3ed46a773fef45486bab5a1ab8a6c7af2d6f395f62ad6c3dfee2c66bef1f257dc3fe50010000").unwrap());
    URParseResult::single(ViewType::TronTx, QRCodeType::Bytes, bytes).c_ptr()
}

#[no_mangle]
pub extern "C" fn test_get_tron_check_failed_keystone_bytes() -> *mut URParseResult {
    let bytes = Bytes::new(hex::decode("1f8b08000000000000030dcfbd4ac34000c071220ea58bdaa9742a41a84bc87d27270e9ab61890c4268d54bb5dee2e26607b508b4a9fa26fe01bf8b128f812be82b383b8161703ffe9bffd1a5bad9d64d1374a77470bb334d2dc7436567d1b1e96540920ec6fabb99da5e7716b5f4a4e58ae91e36b221d8272ed088ca04399a058f8b2a09075f62297909e0b39edb9a0ce05dde79faf8f0d3868048f56c7ce2e86d3b13abb35833089f4f4be2a97ca04554cd8eaa13c9d5ca9d0b6b3315d8d4c9f5c0e83597837884fe6f309ba0e719494328d5995ce90050fe3e671c17c0ab9d2bc904011a031a502f202e414032e19c60c78be209e409aab1cfa9041e603c204821ad588ddd7f5baddfefd7c7aff03e1cbdbd13f2aab0f710f010000").unwrap());
    URParseResult::single(ViewType::TronTx, QRCodeType::Bytes, bytes).c_ptr()
}

#[no_mangle]
pub extern "C" fn test_get_eth_sign_request() -> *mut URParseResult {
    let path1 = PathComponent::new(Some(44), true).unwrap();
    let path2 = PathComponent::new(Some(60), true).unwrap();
    let path3 = PathComponent::new(Some(0), true).unwrap();
    let path4 = PathComponent::new(Some(0), false).unwrap();
    let path5 = PathComponent::new(Some(0), false).unwrap();

    let source_fingerprint: [u8; 4] = [18, 52, 86, 120];
    let components = vec![path1, path2, path3, path4, path5];
    let crypto_key_path = CryptoKeyPath::new(components, Some(source_fingerprint), None);

    let request_id = Some(
        [
            155, 29, 235, 77, 59, 125, 75, 173, 155, 221, 43, 13, 123, 61, 203, 109,
        ]
        .to_vec(),
    );
    let sign_data = [
        248, 73, 128, 134, 9, 24, 78, 114, 160, 0, 130, 39, 16, 148, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 164, 127, 116, 101, 115, 116, 50, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 96, 0, 87, 128, 128, 128,
    ]
    .to_vec();
    let eth_sign_request = EthSignRequest::new(
        request_id,
        sign_data,
        DataType::Transaction,
        Some(1),
        crypto_key_path,
        None,
        Some("metamask".to_string()),
    );
    URParseResult::single(
        ViewType::EthTx,
        QRCodeType::EthSignRequest,
        eth_sign_request,
    )
    .c_ptr()
}

#[no_mangle]
pub extern "C" fn test_get_eth_sign_request_for_personal_message() -> *mut URParseResult {
    let path1 = PathComponent::new(Some(44), true).unwrap();
    let path2 = PathComponent::new(Some(60), true).unwrap();
    let path3 = PathComponent::new(Some(0), true).unwrap();
    let path4 = PathComponent::new(Some(0), false).unwrap();
    let path5 = PathComponent::new(Some(0), false).unwrap();

    let source_fingerprint: [u8; 4] = [18, 52, 86, 120];
    let components = vec![path1, path2, path3, path4, path5];
    let crypto_key_path = CryptoKeyPath::new(components, Some(source_fingerprint), None);

    let request_id = Some(
        [
            155, 29, 235, 77, 59, 125, 75, 173, 155, 221, 43, 13, 123, 61, 203, 109,
        ]
        .to_vec(),
    );

    let sign_data =
        hex::decode("4578616d706c652060706572736f6e616c5f7369676e60206d657373616765").unwrap();
    let eth_sign_request = EthSignRequest::new(
        request_id,
        sign_data,
        DataType::PersonalMessage,
        Some(1),
        crypto_key_path,
        None,
        Some("metamask".to_string()),
    );
    URParseResult::single(
        ViewType::EthPersonalMessage,
        QRCodeType::EthSignRequest,
        eth_sign_request,
    )
    .c_ptr()
}

#[no_mangle]
pub extern "C" fn test_get_sol_sign_request(cbor: *mut c_char) -> *mut URParseResult {
    let cbor_str = recover_c_char(cbor);
    // sol_sign_request Ok(SolSignRequest { request_id: Some([69, 26, 182, 220, 14, 76, 71, 148, 179, 192, 54, 82, 2, 177, 148, 7]), sign_data: [1, 0, 2, 4, 26, 147, 255, 251, 38, 206, 100, 90, 222, 174, 88, 240, 244, 20, 195, 32, 188, 236, 48, 206, 18, 166, 107, 210, 99, 169, 30, 201, 179, 149, 143, 244, 111, 52, 81, 68, 211, 82, 228, 25, 12, 45, 236, 67, 225, 211, 224, 41, 106, 73, 189, 252, 37, 148, 238, 217, 216, 165, 144, 46, 34, 208, 175, 139, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 6, 70, 111, 229, 33, 23, 50, 255, 236, 173, 186, 114, 195, 155, 231, 188, 140, 229, 187, 197, 247, 18, 107, 44, 67, 155, 58, 64, 0, 0, 0, 247, 10, 157, 68, 72, 239, 67, 92, 91, 234, 182, 203, 196, 33, 30, 0, 221, 180, 185, 173, 132, 136, 99, 133, 248, 183, 204, 251, 157, 158, 124, 164, 3, 3, 0, 9, 3, 216, 214, 0, 0, 0, 0, 0, 0, 3, 0, 5, 2, 64, 13, 3, 0, 2, 2, 0, 1, 12, 2, 0, 0, 0, 128, 150, 152, 0, 0, 0, 0, 0], derivation_path: CryptoKeyPath { components: [PathComponent { index: Some(44), wildcard: false, hardened: true }, PathComponent { index: Some(501), wildcard: false, hardened: true }, PathComponent { index: Some(0), wildcard: false, hardened: true }], source_fingerprint: Some([112, 126, 237, 108]), depth: None }, address: None, origin: None, sign_type: Transaction })
    let sol_sign_request = SolSignRequest::try_from(hex::decode(cbor_str).unwrap()).unwrap();
    URParseResult::single(
        ViewType::SolanaTx,
        QRCodeType::SolSignRequest,
        sol_sign_request,
    )
    .c_ptr()
}

#[no_mangle]
pub extern "C" fn test_get_sol_sign_message(cbor: *mut c_char) -> *mut URParseResult {
    let cbor_str = recover_c_char(cbor);
    let sol_sign_message = SolSignRequest::try_from(hex::decode(cbor_str).unwrap()).unwrap();
    URParseResult::single(
        ViewType::SolanaMessage,
        QRCodeType::SolSignRequest,
        sol_sign_message,
    )
    .c_ptr()
}

#[no_mangle]
pub extern "C" fn test_get_sui_sign_request(cbor: *mut c_char) -> *mut URParseResult {
    let cbor_str = recover_c_char(cbor);
    let sign_request = SuiSignRequest::try_from(hex::decode(cbor_str).unwrap()).unwrap();
    URParseResult::single(ViewType::SuiTx, QRCodeType::SuiSignRequest, sign_request).c_ptr()
}

#[no_mangle]
pub extern "C" fn test_get_aptos_sign_request(cbor: *mut c_char) -> *mut URParseResult {
    let cbor_str = recover_c_char(cbor);
    let sign_request = AptosSignRequest::try_from(hex::decode(cbor_str).unwrap()).unwrap();
    URParseResult::single(
        ViewType::AptosTx,
        QRCodeType::AptosSignRequest,
        sign_request,
    )
    .c_ptr()
}

#[no_mangle]
pub extern "C" fn test_get_near_sign_request(cbor: *mut c_char) -> *mut URParseResult {
    let cbor_str = recover_c_char(cbor);
    let near_sign_request = NearSignRequest::try_from(hex::decode(cbor_str).unwrap()).unwrap();
    URParseResult::single(
        ViewType::NearTx,
        QRCodeType::NearSignRequest,
        near_sign_request,
    )
    .c_ptr()
}

#[no_mangle]
pub extern "C" fn test_get_eth_eip1559_sign_request() -> *mut URParseResult {
    let eth_sign_request = EthSignRequest::try_from(hex::decode("a601d825504f755f57cd6942f2bf7d8f90d2ddb2e702583102ef053f8459682f0085037ca764c782520894d6cbd2038a6653993009c56912cb45117ab930d88761069ce3a68a9e80c00304040505d90130a2018a182cf5183cf500f500f400f4021a707eed6c0654fe040716ac4afbba08ee723f3f47d5d814fc48c1").unwrap());
    URParseResult::single(
        ViewType::EthTx,
        QRCodeType::EthSignRequest,
        eth_sign_request,
    )
    .c_ptr()
}

#[no_mangle]
pub extern "C" fn test_get_cardano_sign_request() -> *mut URParseResult {
    let cardano_sign_request = CardanoSignRequest::try_from(hex::decode("a501d825509b1deb4d3b7d4bad9bdd2b0d7b3dcb6d0258a184a400828258204e3a6e7fdcb0d0efa17bf79c13aed2b4cb9baf37fb1aa2e39553d5bd720c5c99038258204e3a6e7fdcb0d0efa17bf79c13aed2b4cb9baf37fb1aa2e39553d5bd720c5c99040182a200581d6179df4c75f7616d7d1fd39cbc1a6ea6b40a0d7b89fea62fc0909b6c370119c350a200581d61c9b0c9761fd1dc0404abd55efc895026628b5035ac623c614fbad0310119c35002198ecb0300a0f5f60382d90899a50158204e3a6e7fdcb0d0efa17bf79c13aed2b4cb9baf37fb1aa2e39553d5bd720c5c990203031a0098968004d90130a2018a19073cf5190717f500f500f400f4021a73c5da0a0578676164647231717938616337717179307674756c796c37776e746d737863367765783830677663796a79333371666672686d37736839323779737835736674757730646c66743035647a3363377265767066376a7830786e6c636a7a336736396d71346166646876d90899a50158204e3a6e7fdcb0d0efa17bf79c13aed2b4cb9baf37fb1aa2e39553d5bd720c5c990204031a0112f6a004d90130a2018a19073cf5190717f500f500f401f4021a73c5da0a057867616464723171797a383536393367346672386335356d667978686165386a3275303470796478726771723733766d77707833617a763464676b797267796c6a35796c326d306a6c70647065737779797a6a7330766877766e6c367867396637737372786b7a39300481d9089ca201581ce557890352095f1cf6fd2b7d1a28e3c3cb029f48cf34ff890a28d17602d90130a2018a19073cf5190717f500f502f400f4021a73c5da0a056e63617264616e6f2d77616c6c6574").unwrap()).unwrap();
    URParseResult::single(
        ViewType::CardanoTx,
        QRCodeType::CardanoSignRequest,
        cardano_sign_request,
    )
    .c_ptr()
}

#[no_mangle]
pub extern "C" fn test_get_xrp_sign_request() -> *mut URParseResult {
    let bytes = Bytes::new(hex::decode("7B225472616E73616374696F6E54797065223A225061796D656E74222C22416D6F756E74223A223130303030303030222C2244657374696E6174696F6E223A2272396A79597745503472545278474341636857724C767742754A4646573853734D4A222C22466C616773223A323134373438333634382C224163636F756E74223A227247556D6B794C627671474633687758347177474864727A4C6459325170736B756D222C22466565223A223132222C2253657175656E6365223A37393939313835372C224C6173744C656467657253657175656E6365223A38303730373430342C225369676E696E675075624B6579223A22303346354335424231443139454337313044334437464144313939414631304346384243314431313334384535423337363543304230423943304245433332383739227D").unwrap());
    URParseResult::single(ViewType::XRPTx, QRCodeType::Bytes, bytes).c_ptr()
}

#[no_mangle]
pub extern "C" fn test_get_xrp_parse_request(cbor: *mut c_char) -> *mut URParseResult {
    let cbor_str = recover_c_char(cbor);
    let xrp_sign_request = Bytes::new(hex::decode(cbor_str).unwrap());
    URParseResult::single(ViewType::XRPTx, QRCodeType::Bytes, xrp_sign_request).c_ptr()
}

#[no_mangle]
pub extern "C" fn test_get_cosmos_sign_request(cbor: *mut c_char) -> *mut URParseResult {
    let cbor_str = recover_c_char(cbor);
    let cosmos_sign_request = CosmosSignRequest::try_from(hex::decode(cbor_str).unwrap()).unwrap();
    URParseResult::single(
        ViewType::CosmosTx,
        QRCodeType::CosmosSignRequest,
        cosmos_sign_request,
    )
    .c_ptr()
}

#[no_mangle]
pub extern "C" fn test_get_cosmos_evm_sign_request(cbor: *mut c_char) -> *mut URParseResult {
    let cbor_str = recover_c_char(cbor);
    let evm_sign_request = EvmSignRequest::try_from(hex::decode(cbor_str).unwrap()).unwrap();
    URParseResult::single(
        ViewType::CosmosEvmTx,
        QRCodeType::EvmSignRequest,
        evm_sign_request,
    )
    .c_ptr()
}
