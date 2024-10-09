use crate::{
    base64_url,
    deep_hash::deep_hash,
    errors::{ArweaveError, Result},
    generate_address,
    transaction::DeepHashItem,
};
use alloc::{
    string::{String, ToString},
    vec::Vec,
};
use app_utils::impl_public_struct;



impl_public_struct!(Tags {
    len: i64,
    data: Vec<Tag>
});

impl Tags {
    pub fn deserialize(serial: &[u8]) -> Result<Self> {
        let mut avro_bytes = serial.to_vec();
        let len = avro_decode_long(&mut avro_bytes)?;
        let mut tags = vec![];
        for _i in 0..len {
            let name = avro_decode_string(&mut avro_bytes)?;
            let value = avro_decode_string(&mut avro_bytes)?;
            tags.push(Tag { name, value })
        }
        return Ok(Tags { len, data: tags });
    }
}

impl_public_struct!(Tag {
    name: String,
    value: String
});

fn avro_decode_long(reader: &mut Vec<u8>) -> Result<i64> {
    let mut i = 0u64;
    let mut buf = [0u8; 1];

    let mut j = 0;
    loop {
        if j > 9 {
            // if j * 7 > 64
            return Err(ArweaveError::AvroError(format!("Integer overflow")));
        }
        let head = reader.remove(0);
        buf[0] = head;
        i |= (u64::from(buf[0] & 0x7F)) << (j * 7);
        if (buf[0] >> 7) == 0 {
            break;
        } else {
            j += 1;
        }
    }
    Ok(if i & 0x1 == 0 {
        (i >> 1) as i64
    } else {
        !(i >> 1) as i64
    })
}

fn avro_decode_string(reader: &mut Vec<u8>) -> Result<String> {
    let len = avro_decode_long(reader)?;
    let buf = reader.drain(..len as usize).collect();
    Ok(
        String::from_utf8(buf)
            .map_err(|e| ArweaveError::AvroError(format!("{}", e.to_string())))?,
    )
}

impl_public_struct!(DataItem {
    signature_type: u16,
    signature: Vec<u8>,
    owner: String,
    raw_owner: Vec<u8>,
    target: Option<String>,
    raw_target: Vec<u8>,
    anchor: Option<String>,
    raw_anchor: Vec<u8>,
    tags_number: u64,
    tags_bytes_number: u64,
    tags: Tags,
    raw_tags: Vec<u8>,
    data: String,
    raw_data: Vec<u8>
});

enum SignatureType {
    ARWEAVE = 1,
    ED25519,
    ETHEREUM,
    SOLANA,
    INJECTEDAPTOS,
    MULTIAPTOS,
    TYPEDETHEREUM,
}

impl DataItem {
    pub fn deserialize(binary: &[u8]) -> Result<Self> {
        let mut reader = binary.to_vec();
        let signature_type =
            u16::from_le_bytes(reader.drain(..2).collect::<Vec<u8>>().try_into().map_err(
                |_| ArweaveError::ParseTxError(format!("Invalid DataItem signature_type")),
            )?);

        if signature_type != SignatureType::ARWEAVE as u16 {
            return Err(ArweaveError::NotSupportedError);
        }
        //ar signature length is 512
        let signature = reader.drain(..512).collect();
        //ar pubkey length is 512
        let raw_owner: Vec<u8> = reader.drain(..512).collect();
        let owner = generate_address(raw_owner.clone())?;

        let has_target = reader.remove(0);
        let (raw_target, target) = if has_target > 0 {
            let raw_target: Vec<u8> = reader.drain(..32).collect();
            (raw_target.clone(), Some(base64_url(raw_target.clone())))
        } else {
            (vec![], None)
        };

        let has_anchor = reader.remove(0);
        let (raw_anchor, anchor) = if has_anchor > 0 {
            let raw_anchor: Vec<u8> = reader.drain(..32).collect();
            (raw_anchor.clone(), Some(base64_url(raw_anchor.clone())))
        } else {
            (vec![], None)
        };

        let tags_number =
            u64::from_le_bytes(reader.drain(..8).collect::<Vec<u8>>().try_into().map_err(
                |_| ArweaveError::ParseTxError(format!("Invalid DataItem tags_number")),
            )?);

        let tags_bytes_number =
            u64::from_le_bytes(reader.drain(..8).collect::<Vec<u8>>().try_into().map_err(
                |_| ArweaveError::ParseTxError(format!("Invalid DataItem tags_number")),
            )?);

        let raw_tags: Vec<u8> = reader.drain(..tags_bytes_number as usize).collect();
        let tags = Tags::deserialize(&raw_tags)?;

        let raw_data = reader.clone();
        let data = base64_url(raw_data.clone());

        Ok(Self {
            signature_type,
            signature,
            owner,
            raw_owner,
            target,
            raw_target,
            anchor,
            raw_anchor,
            tags_number,
            tags_bytes_number,
            tags,
            raw_tags,
            data,
            raw_data,
        })
    }

    pub fn deep_hash(&self) -> Result<Vec<u8>> {
        let mut items = vec![];
        items.push(DeepHashItem::Blob(b"dataitem".to_vec()));
        items.push(DeepHashItem::Blob(b"1".to_vec()));
        items.push(DeepHashItem::Blob(
            self.signature_type.to_string().as_bytes().to_vec(),
        ));
        items.push(DeepHashItem::Blob(self.raw_owner.clone()));
        items.push(DeepHashItem::Blob(self.raw_target.clone()));
        items.push(DeepHashItem::Blob(self.raw_anchor.clone()));
        items.push(DeepHashItem::Blob(self.raw_tags.clone()));
        items.push(DeepHashItem::Blob(self.raw_data.clone()));
        deep_hash(DeepHashItem::List(items)).map(|v| v.to_vec())
    }
}

#[cfg(test)]
mod tests {
    

    use super::DataItem;
    
    use third_party::hex;

    #[test]
    fn test_parse_data_item() {
        //01000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000a999bac8b7906c0bc94f7d163ea9e7fe6ef34045b6a27035e5298aaaddeea05355c50efd30da262c97a68b5aa7219726754bf8501818429e60b9f8175ed66a23349757dc8b3f126abc199272c91174bdb96a9a13aad43b9b6195583188c222002d29b105169dc237dccb0e371895aa10b9263e0b6fbe2d03d3a0073fa7f278ecfa890e75a3fe812ca86eb44f134a7edaa664a5582e72fa43b7accdfeb03f0492c118235b9ff7784106ca1a2f6e7bc4bcc6e1ed98775b7c023a1ae1e332f42e3183ab17c43c58e6605353a47331452ebf659fb267d27492b961ecdafcde9657a0a623aec761f6b3130f89ff7136cae26ebc58aaaa0c6c2264d8e0aa7c78cb46b5210cd69be2ffca64fd3cb0990116034c582828dd22d0235edf9ad999ef0b25afbcab802330d03e9653eff2dbee7f9e0a695a63e04d2aaef73152c255a1d8e5f9cc525cbcfd796ffff337f21d846ae7091037e2bfd06efaf262375100323335e62c79ca63aa31226e3655acab5f2861913630be567210d3d0d5b0f0a6bdc7edfc986e9c14b28b9d32deab5041872a26f8b95341a8cdf6326207d0c2f728ef85554f18c9e285c9f3e01e1d1cb1adf2546eeb9ddfc81a51b0fdf94c9f9116adcd5878815d21038968cbef2b51cc4a27fb1911008c6d1d587830645aca9ca775cf1d67dd9901aadb830a1e8abe0548a47619b8d80083316a645c646820640067653101c54f73164ab75f6650ea8970355bebd6f5162237379174d6afbc4a403e9d875d000800000000000000b100000000000000100c416374696f6e105472616e7366657212526563697069656e745671667a34427465626f714d556f4e536c74457077394b546462663736665252446667783841693644474a77105175616e746974791631303030303030303030301a446174612d50726f746f636f6c04616f0e56617269616e740e616f2e544e2e3108547970650e4d6573736167650653444b12616f636f6e6e65637418436f6e74656e742d5479706514746578742f706c61696e0037373037
        let binary = hex::decode("01000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000a999bac8b7906c0bc94f7d163ea9e7fe6ef34045b6a27035e5298aaaddeea05355c50efd30da262c97a68b5aa7219726754bf8501818429e60b9f8175ed66a23349757dc8b3f126abc199272c91174bdb96a9a13aad43b9b6195583188c222002d29b105169dc237dccb0e371895aa10b9263e0b6fbe2d03d3a0073fa7f278ecfa890e75a3fe812ca86eb44f134a7edaa664a5582e72fa43b7accdfeb03f0492c118235b9ff7784106ca1a2f6e7bc4bcc6e1ed98775b7c023a1ae1e332f42e3183ab17c43c58e6605353a47331452ebf659fb267d27492b961ecdafcde9657a0a623aec761f6b3130f89ff7136cae26ebc58aaaa0c6c2264d8e0aa7c78cb46b5210cd69be2ffca64fd3cb0990116034c582828dd22d0235edf9ad999ef0b25afbcab802330d03e9653eff2dbee7f9e0a695a63e04d2aaef73152c255a1d8e5f9cc525cbcfd796ffff337f21d846ae7091037e2bfd06efaf262375100323335e62c79ca63aa31226e3655acab5f2861913630be567210d3d0d5b0f0a6bdc7edfc986e9c14b28b9d32deab5041872a26f8b95341a8cdf6326207d0c2f728ef85554f18c9e285c9f3e01e1d1cb1adf2546eeb9ddfc81a51b0fdf94c9f9116adcd5878815d21038968cbef2b51cc4a27fb1911008c6d1d587830645aca9ca775cf1d67dd9901aadb830a1e8abe0548a47619b8d80083316a645c646820640067653101c54f73164ab75f6650ea8970355bebd6f5162237379174d6afbc4a403e9d875d000800000000000000b100000000000000100c416374696f6e105472616e7366657212526563697069656e745671667a34427465626f714d556f4e536c74457077394b546462663736665252446667783841693644474a77105175616e746974791631303030303030303030301a446174612d50726f746f636f6c04616f0e56617269616e740e616f2e544e2e3108547970650e4d6573736167650653444b12616f636f6e6e65637418436f6e74656e742d5479706514746578742f706c61696e0037373037").unwrap();
        let result = DataItem::deserialize(&binary).unwrap();
        println!("{:?}", result);
        assert_eq!(result.signature_type, 1);
        assert_eq!(result.owner, "nSkowCiV4VBZJVyI2UK2wT_6g9LVX5BLZvYSTjd0bVQ");
        assert_eq!(
            result.target.unwrap(),
            "xU9zFkq3X2ZQ6olwNVvr1vUWIjc3kXTWr7xKQD6dh10"
        );
        assert_eq!(result.anchor, None);
        assert_eq!(result.tags.len, 8);
        assert_eq!(result.tags.data.get(0).unwrap().name, "Action");
        assert_eq!(result.tags.data.get(0).unwrap().value, "Transfer");
        assert_eq!(result.tags.data.get(7).unwrap().name, "Content-Type");
        assert_eq!(result.tags.data.get(7).unwrap().value, "text/plain");
    }
}
