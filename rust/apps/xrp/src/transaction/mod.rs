use crate::errors::{XRPError, R};
use alloc::format;
use alloc::string::{String, ToString};
use bytes::BytesMut;
use rippled_binary_codec;
use rippled_binary_codec::definition_fields::DefinitionFields;
use third_party::cryptoxide::hashing;
use third_party::hex;
use third_party::secp256k1::ecdsa::Signature;
use third_party::serde_json::{from_slice, to_string, Value};

pub struct WrappedTxData {
    pub(crate) tx_data: Value,
    pub(crate) tx_hex: [u8; 32],
    pub(crate) signing_pubkey: String,
    pub(crate) definition_fields: DefinitionFields,
}

impl WrappedTxData {
    pub fn from_raw(input: &[u8]) -> R<WrappedTxData> {
        let definition_fields = DefinitionFields::new();
        let tx_data: Value = from_slice(input)?;
        let signing_pubkey = tx_data["SigningPubKey"].as_str().unwrap_or(&"").to_string();
        if let Some(tag) = tx_data["DestinationTag"].as_i64() {
            if tag > 0xffffffff || tag < 0 {
                return Err(XRPError::SignFailure(format!("invalid tag {:?}", tag)));
            }
        }
        let serialized_tx: String = rippled_binary_codec::serialize::serialize_tx(
            tx_data.to_string(),
            true,
            Some(&definition_fields),
        )
        .ok_or(XRPError::SignFailure("serialize tx failed".to_string()))?;
        let mut tx_hex = BytesMut::new();
        tx_hex.extend_from_slice(&hex::decode("53545800")?);
        tx_hex.extend_from_slice(&hex::decode(serialized_tx)?);
        let tx_hex = hashing::sha512(&tx_hex)[..32].try_into().map_err(|_e| {
            XRPError::InvalidData(format!(
                "generate unsigned tx hash failed, data is {:?}",
                hex::encode(tx_hex)
            ))
        })?;
        Ok(WrappedTxData {
            tx_data,
            tx_hex,
            signing_pubkey,
            definition_fields,
        })
    }

    pub fn generate_signed_tx(&mut self, signature: &[u8; 64]) -> R<String> {
        let sig = Signature::from_compact(signature).map_err(|_e| {
            XRPError::SignFailure(format!("invalid signature {:?}", hex::encode(signature)))
        })?;
        self.tx_data["TxnSignature"] =
            Value::from(Some(sig.serialize_der().to_string().to_uppercase()));
        rippled_binary_codec::serialize::serialize_tx(
            to_string(&self.tx_data)?,
            false,
            Some(&self.definition_fields),
        )
        .ok_or(XRPError::SignFailure("serialize tx failed".to_string()))
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use alloc::string::String;
    use third_party::serde_json::from_str;

    #[test]
    fn test_generate_unsigned_tx_1() {
        let input = r#"{
          "Sequence": 1,
          "Fee": "20",
          "Account": "rL5BYSLd89uzj4i4J47iLQg9HwmeXE7eCt",
          "Destination": "rHfof1xNbEtJYsXN8MUbnf9iFixCEY84kf",
          "DestinationTag": 1700373364,
          "Amount": "1000000",
          "TransactionType": "Payment",
          "Flags": 2147483648,
          "SigningPubKey": "0272DB8641A03008C27EBFBA2234C5BF93952C41FAE5A6295356CDC3991171EBE0"
        }"#;
        let v: Value = from_str(input).unwrap();
        let input_bytes = v.to_string().into_bytes();
        let wrapped_tx = WrappedTxData::from_raw(input_bytes.as_slice()).unwrap();
        let expected =
            String::from("8c0f55afb00fe6b2b4427dc158c03e6a2fd887d4b694a641d5173189f3cf933f");
        assert_eq!(expected, hex::encode(wrapped_tx.tx_hex));
    }

    #[test]
    fn test_generate_unsigned_tx_2() {
        let input = r#"{"TransactionType":"AccountDelete","Fee":"2000000","Flags":2147483648,"Destination":"rNp5zaiaR3maZ8zALz5CWnqRYXWkeGhteS","Account":"rwEJf6YSKALUaxRhvJ1S81PPmXzWhDW8on","Sequence":23159180,"LastLedgerSequence":23164152,"SigningPubKey":"02B87CEB1507849B6473773155827C0B8C15CB311C6876FBD7FAB95F06D3E18E39"}"#;
        let v: Value = from_str(input).unwrap();
        let input_bytes = v.to_string().into_bytes();
        let wrapped_tx = WrappedTxData::from_raw(input_bytes.as_slice()).unwrap();
        let expected =
            String::from("5da71e4d6c09841d1b46d2c7b4317a92cc352139a73e21137e440a2f0b89aa53");
        assert_eq!(expected, hex::encode(wrapped_tx.tx_hex));
    }

    #[test]
    fn test_generate_unsigned_tx_3() {
        //xrp toolkit signrequest
        let input = "7B225472616E73616374696F6E54797065223A225061796D656E74222C22416D6F756E74223A223130303030303030222C2244657374696E6174696F6E223A2272396A79597745503472545278474341636857724C767742754A4646573853734D4A222C22466C616773223A323134373438333634382C224163636F756E74223A227247556D6B794C627671474633687758347177474864727A4C6459325170736B756D222C22466565223A223132222C2253657175656E6365223A37393939313835372C224C6173744C656467657253657175656E6365223A38303730373430342C225369676E696E675075624B6579223A22303346354335424231443139454337313044334437464144313939414631304346384243314431313334384535423337363543304230423943304245433332383739227D";
        let v: Value = from_slice(&hex::decode(input).unwrap().as_slice()).unwrap();
        let wrapped_tx = WrappedTxData::from_raw(v.to_string().into_bytes().as_slice()).unwrap();
        let expected =
            String::from("88dfb47b27747e247bfaeade10c1cdecc64ca2298940cfe8e1b222971be8f41e");
        assert_eq!(expected, hex::encode(wrapped_tx.tx_hex));
    }

    #[test]
    fn test_signed_tx_1() {
        let input = r#"{
          "Sequence": 1,
          "Fee": "20",
          "Account": "rL5BYSLd89uzj4i4J47iLQg9HwmeXE7eCt",
          "Destination": "rHfof1xNbEtJYsXN8MUbnf9iFixCEY84kf",
          "DestinationTag": 1700373364,
          "Amount": "1000000",
          "TransactionType": "Payment",
          "Flags": 2147483648,
          "SigningPubKey": "0272DB8641A03008C27EBFBA2234C5BF93952C41FAE5A6295356CDC3991171EBE0"
        }"#;
        let v: Value = from_str(input).unwrap();
        let input_bytes = v.to_string().into_bytes();
        let mut wrapped_tx = WrappedTxData::from_raw(input_bytes.as_slice()).unwrap();
        let signature: [u8; 64] = hex::decode("71cf4229e7296b07fa6f1287f67a02605d50a73da607471dd3e7ff3a01d61ebb136220129e1db50367b2df2f219c79dcc1a5da5169e1a3c85c8633727c757937").unwrap().try_into().unwrap();
        let signed_tx = wrapped_tx.generate_signed_tx(&signature).unwrap();
        let expected_tx_hex = "120000228000000024000000012E6559A3746140000000000F424068400000000000001473210272DB8641A03008C27EBFBA2234C5BF93952C41FAE5A6295356CDC3991171EBE074463044022071CF4229E7296B07FA6F1287F67A02605D50A73DA607471DD3E7FF3A01D61EBB0220136220129E1DB50367B2DF2F219C79DCC1A5DA5169E1A3C85C8633727C7579378114D8343E8E1F27B467B651748B8396A52C9185D9F98314B0CB0194B32F22136D1FF5A01E45FB2FED2C3F75";
        assert_eq!(expected_tx_hex, signed_tx.as_str());
    }

    #[test]
    fn test_signed_tx_2() {
        let input = r#"{"TransactionType":"Payment","Amount":"14000","Destination":"rpYEGnKLe7ChbFdw9DBx5JeWVL1637cVug","Flags":2147483648,"Account":"rndm7RphBZG6CpZvKcG9AjoFbSvcKhwLCx","Fee":"12","Sequence":226,"LastLedgerSequence":65942078,"SigningPubKey":"03294E766FD584C9538911828EE981C4A73DE0EAAD5AF08C377869C38477D2618F"}"#;
        let v: Value = from_str(input).unwrap();
        let input_bytes = v.to_string().into_bytes();
        let mut wrapped_tx = WrappedTxData::from_raw(input_bytes.as_slice()).unwrap();
        let signature: [u8; 64] = hex::decode("937632fa33c965f34ea9af4259a33d3751da2f90b4f3e850b774d48aaf0e0ef64503c09254eec3b1ae01c1130a4d0a3fcc9cbbe56853309a6b79ce51644c713d").unwrap().try_into().unwrap();
        let signed_tx = wrapped_tx.generate_signed_tx(&signature).unwrap();
        let expected_tx_hex = "120000228000000024000000E2201B03EE323E6140000000000036B068400000000000000C732103294E766FD584C9538911828EE981C4A73DE0EAAD5AF08C377869C38477D2618F74473045022100937632FA33C965F34EA9AF4259A33D3751DA2F90B4F3E850B774D48AAF0E0EF602204503C09254EEC3B1AE01C1130A4D0A3FCC9CBBE56853309A6B79CE51644C713D811432D49A06A7BD5ED01DD0989E783D441D15E79888831410E0EC963C9185C0D659AFC1FBBBAAF257902E3F";
        assert_eq!(expected_tx_hex, signed_tx.as_str());
    }

    #[test]
    fn test_signed_tx_3() {
        let input = r#"{"TransactionType":"AccountDelete","Fee":"2000000","Flags":2147483648,"Destination":"rNp5zaiaR3maZ8zALz5CWnqRYXWkeGhteS","Account":"rwEJf6YSKALUaxRhvJ1S81PPmXzWhDW8on","Sequence":23159180,"LastLedgerSequence":23164152,"SigningPubKey":"02B87CEB1507849B6473773155827C0B8C15CB311C6876FBD7FAB95F06D3E18E39"}"#;
        let v: Value = from_str(input).unwrap();
        let input_bytes = v.to_string().into_bytes();
        let mut wrapped_tx = WrappedTxData::from_raw(input_bytes.as_slice()).unwrap();
        let signature: [u8; 64] = hex::decode("03514388d1649f1b5aea94a400e955b169103d242a786c2164179d09db85b77a16fc0b47005b7045592e0e98f2007a73fd7f147b349ae75df43c14962693a7e6").unwrap().try_into().unwrap();
        let signed_tx = wrapped_tx.generate_signed_tx(&signature).unwrap();
        let expected_tx_hex = "1200152280000000240161618C201B016174F86840000000001E8480732102B87CEB1507849B6473773155827C0B8C15CB311C6876FBD7FAB95F06D3E18E3974463044022003514388D1649F1B5AEA94A400E955B169103D242A786C2164179D09DB85B77A022016FC0B47005B7045592E0E98F2007A73FD7F147B349AE75DF43C14962693A7E68114656D3E2961EFABDED0C9CDCFB39FC78D01E9A77683148EED191963FEB29D532F04958BFA087A45F742C7";
        assert_eq!(expected_tx_hex, signed_tx.to_string());
    }

    #[test]
    fn test_serialize_tx_1() {
        // USD payment
        let input = r#"{
          "TransactionType" : "Payment",
          "Account" : "rf1BiGeXwwQoi8Z2ueFYTEXSwuJYfV2Jpn",
          "Destination" : "ra5nK24KXen9AHvsdFTKHSANinZseWnPcX",
          "Amount" : {
             "currency" : "USD",
             "value" : "1",
             "issuer" : "rf1BiGeXwwQoi8Z2ueFYTEXSwuJYfV2Jpn"
          },
          "Fee": "12",
          "Flags": 2147483648,
          "Sequence": 2
        }"#;
        let v: Value = from_str(input).unwrap();
        let input_bytes = v.to_string().into_bytes();
        let wrapped_tx = WrappedTxData::from_raw(input_bytes.as_slice()).unwrap();
        let definition_fields = DefinitionFields::new();
        let serialized = rippled_binary_codec::serialize::serialize_tx(
            to_string(&wrapped_tx.tx_data).unwrap(),
            true,
            Some(&definition_fields),
        )
        .unwrap();
        assert_eq!("1200002280000000240000000261D4838D7EA4C6800000000000000000000000000055534400000000004B4E9C06F24296074F7BC48F92A97916C6DC5EA968400000000000000C81144B4E9C06F24296074F7BC48F92A97916C6DC5EA983143E9D4A2B8AA0780F682D136F7A56D6724EF53754", serialized);
    }

    #[test]
    fn test_serialize_tx_2() {
        // XRP payment
        let input = r#"{
            "TransactionType": "Payment",
            "Amount": "10000000",
            "Destination": "rDxQoYzcQrpzVHuT4Wx6bacJYXyGTEtbvm",
            "Flags": 2147483648,
            "Account": "rGUmkyLbvqGF3hwX4qwGHdrzLdY2Qpskum",
            "Fee": "12",
            "Sequence": 79991865,
            "LastLedgerSequence": 80815479,
            "SigningPubKey": "03F5C5BB1D19EC710D3D7FAD199AF10CF8BC1D11348E5B3765C0B0B9C0BEC32879"
        }"#;
        let v: Value = from_str(input).unwrap();
        let input_bytes = v.to_string().into_bytes();
        let wrapped_tx = WrappedTxData::from_raw(input_bytes.as_slice()).unwrap();
        let definition_fields = DefinitionFields::new();
        let serialized = rippled_binary_codec::serialize::serialize_tx(
            to_string(&wrapped_tx.tx_data).unwrap(),
            true,
            Some(&definition_fields),
        )
        .unwrap();
        assert_eq!("12000022800000002404C49439201B04D1257761400000000098968068400000000000000C732103F5C5BB1D19EC710D3D7FAD199AF10CF8BC1D11348E5B3765C0B0B9C0BEC328798114A6C3D314FB5418627AB22D9DDF6C18AED5F6CA8983148E1C41B8BEC53377243BB6AD958F3F35B046F330", serialized);
    }

    #[test]
    fn test_serialize_tx_3() {
        // CheckCash
        let input = r#"{
            "Account": "rfkE1aSy9G8Upk4JssnwBxhEv5p4mn2KTy",
            "TransactionType": "CheckCash",
            "Amount": "100000000",
            "CheckID": "838766BA2B995C00744175F69A1B11E32C3DBC40E64801A4056FCBD657F57334",
            "Fee": "12"
        }"#;
        let v: Value = from_str(input).unwrap();
        let input_bytes = v.to_string().into_bytes();
        let wrapped_tx = WrappedTxData::from_raw(input_bytes.as_slice()).unwrap();
        let definition_fields = DefinitionFields::new();
        let serialized = rippled_binary_codec::serialize::serialize_tx(
            to_string(&wrapped_tx.tx_data).unwrap(),
            true,
            Some(&definition_fields),
        )
        .unwrap();
        assert_eq!("1200115018838766BA2B995C00744175F69A1B11E32C3DBC40E64801A4056FCBD657F57334614000000005F5E10068400000000000000C811449FF0C73CA6AF9733DA805F76CA2C37776B7C46B", serialized);
    }

    #[test]
    fn test_serialize_tx_4() {
        // Account Set
        let input = r#"{
            "TransactionType": "AccountSet",
            "Account" : "rf1BiGeXwwQoi8Z2ueFYTEXSwuJYfV2Jpn",
            "Fee": "12",
            "Sequence": 5,
            "Domain": "6578616D706C652E636F6D",
            "SetFlag": 5,
            "MessageKey": "03AB40A0490F9B7ED8DF29D246BF2D6269820A0EE7742ACDD457BEA7C7D0931EDB"
        }"#;
        let v: Value = from_str(input).unwrap();
        let input_bytes = v.to_string().into_bytes();
        let wrapped_tx = WrappedTxData::from_raw(input_bytes.as_slice()).unwrap();
        let serialized = rippled_binary_codec::serialize::serialize_tx(
            to_string(&wrapped_tx.tx_data).unwrap(),
            true,
            None,
        )
        .unwrap();
        assert_eq!("120003240000000520210000000568400000000000000C722103AB40A0490F9B7ED8DF29D246BF2D6269820A0EE7742ACDD457BEA7C7D0931EDB770B6578616D706C652E636F6D81144B4E9C06F24296074F7BC48F92A97916C6DC5EA9", serialized);
    }

    #[test]
    fn test_serialize_tx_5() {
        // CheckCancel
        let input = r#"{
            "Account": "rUn84CUYbNjRoTQ6mSW7BVJPSVJNLb1QLo",
            "TransactionType": "CheckCancel",
            "CheckID": "49647F0D748DC3FE26BDACBC57F251AADEFFF391403EC9BF87C97F67E9977FB0",
            "Sequence": 5,
            "Fee": "12"
        }"#;
        let v: Value = from_str(input).unwrap();
        let input_bytes = v.to_string().into_bytes();
        let wrapped_tx = WrappedTxData::from_raw(input_bytes.as_slice()).unwrap();
        let serialized = rippled_binary_codec::serialize::serialize_tx(
            to_string(&wrapped_tx.tx_data).unwrap(),
            true,
            None,
        )
        .unwrap();
        assert_eq!("1200122400000005501849647F0D748DC3FE26BDACBC57F251AADEFFF391403EC9BF87C97F67E9977FB068400000000000000C81147990EC5D1D8DF69E070A968D4B186986FDF06ED0", serialized);
    }

    #[test]
    fn test_serialize_tx_6() {
        // CheckCreate
        let input = r#"{
          "TransactionType": "CheckCreate",
          "Account": "rUn84CUYbNjRoTQ6mSW7BVJPSVJNLb1QLo",
          "Destination": "rfkE1aSy9G8Upk4JssnwBxhEv5p4mn2KTy",
          "SendMax": "100000000",
          "Expiration": 570113521,
          "InvoiceID": "6F1DFD1D0FE8A32E40E1F2C05CF1C15545BAB56B617F9C6C2D63A6B704BEF59B",
          "DestinationTag": 1,
          "Fee": "12"
        }"#;
        let v: Value = from_str(input).unwrap();
        let input_bytes = v.to_string().into_bytes();
        let wrapped_tx = WrappedTxData::from_raw(input_bytes.as_slice()).unwrap();
        let serialized = rippled_binary_codec::serialize::serialize_tx(
            to_string(&wrapped_tx.tx_data).unwrap(),
            true,
            None,
        )
        .unwrap();
        assert_eq!("1200102A21FB3DF12E0000000150116F1DFD1D0FE8A32E40E1F2C05CF1C15545BAB56B617F9C6C2D63A6B704BEF59B68400000000000000C694000000005F5E10081147990EC5D1D8DF69E070A968D4B186986FDF06ED0831449FF0C73CA6AF9733DA805F76CA2C37776B7C46B", serialized);
    }
}
