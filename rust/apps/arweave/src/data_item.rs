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
        let mut avro_bytes = serial;
        let mut tags = vec![];

        loop {
            let block_count = avro_decode_long(&mut avro_bytes)?;
            if block_count == 0 {
                break;
            }

            let item_count = if block_count < 0 {
                let item_count = block_count
                    .checked_abs()
                    .ok_or_else(|| ArweaveError::AvroError("Invalid block count".to_string()))?
                    as u64;
                let block_size = avro_decode_long(&mut avro_bytes)?;
                let block_size = usize::try_from(block_size).map_err(|_| {
                    ArweaveError::AvroError("Invalid negative block size".to_string())
                })?;
                let mut block = avro_take(&mut avro_bytes, block_size)?;
                avro_decode_tags(&mut block, item_count, &mut tags)?;
                if !block.is_empty() {
                    return Err(ArweaveError::AvroError(
                        "Avro tag block contains trailing bytes".to_string(),
                    ));
                }
                continue;
            } else {
                block_count as u64
            };

            avro_decode_tags(&mut avro_bytes, item_count, &mut tags)?;
        }

        if !avro_bytes.is_empty() {
            return Err(ArweaveError::AvroError(
                "Avro tags contain trailing bytes".to_string(),
            ));
        }

        let len = i64::try_from(tags.len())
            .map_err(|_| ArweaveError::AvroError("Too many tags".to_string()))?;
        Ok(Tags { len, data: tags })
    }

    pub(crate) fn as_slice(&self) -> &[Tag] {
        &self.data
    }
}

impl_public_struct!(Tag {
    name: String,
    value: String
});

impl Tag {
    pub(crate) fn name(&self) -> &str {
        &self.name
    }

    pub(crate) fn value(&self) -> &str {
        &self.value
    }
}

fn avro_take<'a>(reader: &mut &'a [u8], len: usize) -> Result<&'a [u8]> {
    if len > reader.len() {
        return Err(ArweaveError::AvroError(
            "Unexpected end of Avro data".to_string(),
        ));
    }
    let (value, rest) = reader.split_at(len);
    *reader = rest;
    Ok(value)
}

fn avro_decode_tags(reader: &mut &[u8], count: u64, tags: &mut Vec<Tag>) -> Result<()> {
    for _ in 0..count {
        let name = avro_decode_string(reader)?;
        let value = avro_decode_string(reader)?;
        tags.push(Tag { name, value });
    }
    Ok(())
}

fn avro_decode_long(reader: &mut &[u8]) -> Result<i64> {
    let mut i = 0u64;

    let mut j = 0u32;
    loop {
        if j >= 10 {
            return Err(ArweaveError::AvroError("Integer overflow".to_string()));
        }

        let head = *avro_take(reader, 1)?
            .first()
            .ok_or_else(|| ArweaveError::AvroError("Unexpected end of Avro data".to_string()))?;
        if j == 9 && head > 1 {
            return Err(ArweaveError::AvroError("Integer overflow".to_string()));
        }
        i |= u64::from(head & 0x7f) << (j * 7);
        if head & 0x80 == 0 {
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

fn avro_decode_string(reader: &mut &[u8]) -> Result<String> {
    let len = avro_decode_long(reader)?;
    let len = usize::try_from(len)
        .map_err(|_| ArweaveError::AvroError("Invalid negative string length".to_string()))?;
    let buf = avro_take(reader, len)?;
    String::from_utf8(buf.to_vec()).map_err(|e| ArweaveError::AvroError(format!("{e}")))
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

#[allow(unused)]
enum SignatureType {
    ARWEAVE = 1,
    ED25519,
    ETHEREUM,
    SOLANA,
    INJECTEDAPTOS,
    MULTIAPTOS,
    TYPEDETHEREUM,
}

fn data_item_take(reader: &mut Vec<u8>, len: usize) -> Result<Vec<u8>> {
    if len > reader.len() {
        return Err(ArweaveError::ParseTxError(
            "Unexpected end of DataItem".to_string(),
        ));
    }
    Ok(reader.drain(..len).collect())
}

impl DataItem {
    pub(crate) fn tags_ref(&self) -> &Tags {
        &self.tags
    }

    pub fn deserialize(binary: &[u8]) -> Result<Self> {
        let mut reader = binary.to_vec();
        let signature_type =
            u16::from_le_bytes(data_item_take(&mut reader, 2)?.try_into().map_err(|_| {
                ArweaveError::ParseTxError("Invalid DataItem signature_type".to_string())
            })?);

        if signature_type != SignatureType::ARWEAVE as u16 {
            return Err(ArweaveError::NotSupportedError);
        }
        //ar signature length is 512
        let signature = data_item_take(&mut reader, 512)?;
        //ar pubkey length is 512
        let raw_owner = data_item_take(&mut reader, 512)?;
        let owner = generate_address(raw_owner.clone())?;

        let has_target = data_item_take(&mut reader, 1)?[0];
        let (raw_target, target) = if has_target > 0 {
            let raw_target = data_item_take(&mut reader, 32)?;
            (raw_target.clone(), Some(base64_url(raw_target.clone())))
        } else {
            (vec![], None)
        };

        let has_anchor = data_item_take(&mut reader, 1)?[0];
        let (raw_anchor, anchor) = if has_anchor > 0 {
            let raw_anchor = data_item_take(&mut reader, 32)?;
            (raw_anchor.clone(), Some(base64_url(raw_anchor.clone())))
        } else {
            (vec![], None)
        };

        let tags_number =
            u64::from_le_bytes(data_item_take(&mut reader, 8)?.try_into().map_err(|_| {
                ArweaveError::ParseTxError("Invalid DataItem tags_number".to_string())
            })?);

        let tags_bytes_number =
            u64::from_le_bytes(data_item_take(&mut reader, 8)?.try_into().map_err(|_| {
                ArweaveError::ParseTxError("Invalid DataItem tags_number".to_string())
            })?);

        let tags_bytes_len = usize::try_from(tags_bytes_number).map_err(|_| {
            ArweaveError::ParseTxError("DataItem tags byte length is too large".to_string())
        })?;
        if tags_bytes_len > reader.len() {
            return Err(ArweaveError::ParseTxError(
                "DataItem tags exceed remaining input".to_string(),
            ));
        }
        let raw_tags: Vec<u8> = reader.drain(..tags_bytes_len).collect();
        let tags = Tags::deserialize(&raw_tags)?;
        if tags.as_slice().len() as u64 != tags_number {
            return Err(ArweaveError::ParseTxError(format!(
                "DataItem tags count mismatch: expected {tags_number}, decoded {}",
                tags.as_slice().len()
            )));
        }

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

    use super::{ArweaveError, DataItem, Tags};
    use alloc::string::ToString;
    use alloc::vec::Vec;

    use hex;

    #[test]
    fn test_reject_truncated_data_item_fields() {
        for has_target in [false, true] {
            for has_anchor in [false, true] {
                let mut binary = Vec::new();
                binary.extend_from_slice(&1u16.to_le_bytes());
                binary.extend_from_slice(&[0u8; 512]);
                binary.extend_from_slice(&[0u8; 512]);
                binary.push(u8::from(has_target));
                if has_target {
                    binary.extend_from_slice(&[0u8; 32]);
                }
                binary.push(u8::from(has_anchor));
                if has_anchor {
                    binary.extend_from_slice(&[0u8; 32]);
                }
                binary.extend_from_slice(&0u64.to_le_bytes());
                binary.extend_from_slice(&1u64.to_le_bytes());
                let header_len = binary.len();
                binary.push(0);

                assert!(DataItem::deserialize(&binary).is_ok());
                for len in 0..header_len {
                    let error = DataItem::deserialize(&binary[..len]).unwrap_err();
                    assert!(
                        matches!(error, ArweaveError::ParseTxError(_)),
                        "target={has_target}, anchor={has_anchor}, length={len}: {error}"
                    );
                }
                assert!(DataItem::deserialize(&binary[..header_len]).is_err());
            }
        }
    }

    #[test]
    fn test_reject_malformed_avro_blocks() {
        let cases: &[(&[u8], &str)] = &[
            (
                &[0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01],
                "Invalid block count",
            ),
            (&[0x01, 0x01], "Invalid negative block size"),
            (&[0x02, 0x01], "Invalid negative string length"),
            (&[0x02, 0x02, 0xff], "utf-8"),
            (&[0x02, 0x04, b'A'], "Unexpected end of Avro data"),
            (
                &[0x01, 0x06, 0x00, 0x00, 0x00, 0x00],
                "Avro tag block contains trailing bytes",
            ),
        ];
        for (serial, message) in cases {
            let error = Tags::deserialize(serial).unwrap_err();
            assert!(matches!(error, ArweaveError::AvroError(_)));
            assert!(error.to_string().contains(message), "{error}");
        }
    }

    #[test]
    fn test_parse_tags_across_multiple_avro_blocks() {
        let serial = [
            0x02, 0x12, b'R', b'e', b'c', b'i', b'p', b'i', b'e', b'n', b't', 0x02, b'A', 0x02,
            0x10, b'Q', b'u', b'a', b'n', b't', b'i', b't', b'y', 0x04, b'1', b'0', 0x00,
        ];

        let tags = Tags::deserialize(&serial).unwrap();

        assert_eq!(tags.get_len(), 2);
        assert_eq!(tags.get_data()[0].get_name(), "Recipient");
        assert_eq!(tags.get_data()[0].get_value(), "A");
        assert_eq!(tags.get_data()[1].get_name(), "Quantity");
        assert_eq!(tags.get_data()[1].get_value(), "10");
    }

    #[test]
    fn test_parse_negative_count_avro_block() {
        let serial = [
            0x01, 0x18, 0x12, b'R', b'e', b'c', b'i', b'p', b'i', b'e', b'n', b't', 0x02, b'A',
            0x00,
        ];

        let tags = Tags::deserialize(&serial).unwrap();

        assert_eq!(tags.get_len(), 1);
        assert_eq!(tags.get_data()[0].get_name(), "Recipient");
    }

    #[test]
    fn test_reject_trailing_avro_tag_bytes() {
        assert!(Tags::deserialize(&[0x00, 0x02]).is_err());
    }

    #[test]
    fn test_reject_header_tag_count_mismatch() {
        let raw_tags = [0x02, 0x02, b'A', 0x02, b'B', 0x00];
        let mut binary = Vec::new();
        binary.extend_from_slice(&1u16.to_le_bytes());
        binary.extend_from_slice(&[0u8; 512]);
        binary.extend_from_slice(&[0u8; 512]);
        binary.push(0); // no target
        binary.push(0); // no anchor
        binary.extend_from_slice(&2u64.to_le_bytes()); // header claims two tags
        binary.extend_from_slice(&(raw_tags.len() as u64).to_le_bytes());
        binary.extend_from_slice(&raw_tags); // Avro payload contains one tag

        let error = DataItem::deserialize(&binary).unwrap_err();
        assert!(error.to_string().contains("tags count mismatch"));
    }

    #[test]
    fn test_parse_data_item() {
        //01000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000a999bac8b7906c0bc94f7d163ea9e7fe6ef34045b6a27035e5298aaaddeea05355c50efd30da262c97a68b5aa7219726754bf8501818429e60b9f8175ed66a23349757dc8b3f126abc199272c91174bdb96a9a13aad43b9b6195583188c222002d29b105169dc237dccb0e371895aa10b9263e0b6fbe2d03d3a0073fa7f278ecfa890e75a3fe812ca86eb44f134a7edaa664a5582e72fa43b7accdfeb03f0492c118235b9ff7784106ca1a2f6e7bc4bcc6e1ed98775b7c023a1ae1e332f42e3183ab17c43c58e6605353a47331452ebf659fb267d27492b961ecdafcde9657a0a623aec761f6b3130f89ff7136cae26ebc58aaaa0c6c2264d8e0aa7c78cb46b5210cd69be2ffca64fd3cb0990116034c582828dd22d0235edf9ad999ef0b25afbcab802330d03e9653eff2dbee7f9e0a695a63e04d2aaef73152c255a1d8e5f9cc525cbcfd796ffff337f21d846ae7091037e2bfd06efaf262375100323335e62c79ca63aa31226e3655acab5f2861913630be567210d3d0d5b0f0a6bdc7edfc986e9c14b28b9d32deab5041872a26f8b95341a8cdf6326207d0c2f728ef85554f18c9e285c9f3e01e1d1cb1adf2546eeb9ddfc81a51b0fdf94c9f9116adcd5878815d21038968cbef2b51cc4a27fb1911008c6d1d587830645aca9ca775cf1d67dd9901aadb830a1e8abe0548a47619b8d80083316a645c646820640067653101c54f73164ab75f6650ea8970355bebd6f5162237379174d6afbc4a403e9d875d000800000000000000b100000000000000100c416374696f6e105472616e7366657212526563697069656e745671667a34427465626f714d556f4e536c74457077394b546462663736665252446667783841693644474a77105175616e746974791631303030303030303030301a446174612d50726f746f636f6c04616f0e56617269616e740e616f2e544e2e3108547970650e4d6573736167650653444b12616f636f6e6e65637418436f6e74656e742d5479706514746578742f706c61696e0037373037
        let binary = hex::decode("01000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000a999bac8b7906c0bc94f7d163ea9e7fe6ef34045b6a27035e5298aaaddeea05355c50efd30da262c97a68b5aa7219726754bf8501818429e60b9f8175ed66a23349757dc8b3f126abc199272c91174bdb96a9a13aad43b9b6195583188c222002d29b105169dc237dccb0e371895aa10b9263e0b6fbe2d03d3a0073fa7f278ecfa890e75a3fe812ca86eb44f134a7edaa664a5582e72fa43b7accdfeb03f0492c118235b9ff7784106ca1a2f6e7bc4bcc6e1ed98775b7c023a1ae1e332f42e3183ab17c43c58e6605353a47331452ebf659fb267d27492b961ecdafcde9657a0a623aec761f6b3130f89ff7136cae26ebc58aaaa0c6c2264d8e0aa7c78cb46b5210cd69be2ffca64fd3cb0990116034c582828dd22d0235edf9ad999ef0b25afbcab802330d03e9653eff2dbee7f9e0a695a63e04d2aaef73152c255a1d8e5f9cc525cbcfd796ffff337f21d846ae7091037e2bfd06efaf262375100323335e62c79ca63aa31226e3655acab5f2861913630be567210d3d0d5b0f0a6bdc7edfc986e9c14b28b9d32deab5041872a26f8b95341a8cdf6326207d0c2f728ef85554f18c9e285c9f3e01e1d1cb1adf2546eeb9ddfc81a51b0fdf94c9f9116adcd5878815d21038968cbef2b51cc4a27fb1911008c6d1d587830645aca9ca775cf1d67dd9901aadb830a1e8abe0548a47619b8d80083316a645c646820640067653101c54f73164ab75f6650ea8970355bebd6f5162237379174d6afbc4a403e9d875d000800000000000000b100000000000000100c416374696f6e105472616e7366657212526563697069656e745671667a34427465626f714d556f4e536c74457077394b546462663736665252446667783841693644474a77105175616e746974791631303030303030303030301a446174612d50726f746f636f6c04616f0e56617269616e740e616f2e544e2e3108547970650e4d6573736167650653444b12616f636f6e6e65637418436f6e74656e742d5479706514746578742f706c61696e0037373037").unwrap();
        let result = DataItem::deserialize(&binary).unwrap();
        assert_eq!(result.signature_type, 1);
        assert_eq!(result.owner, "nSkowCiV4VBZJVyI2UK2wT_6g9LVX5BLZvYSTjd0bVQ");
        assert_eq!(
            result.target.unwrap(),
            "xU9zFkq3X2ZQ6olwNVvr1vUWIjc3kXTWr7xKQD6dh10"
        );
        assert_eq!(result.anchor, None);
        assert_eq!(result.tags.len, 8);
        assert_eq!(result.tags.data.first().unwrap().name, "Action");
        assert_eq!(result.tags.data.first().unwrap().value, "Transfer");
        assert_eq!(result.tags.data.get(7).unwrap().name, "Content-Type");
        assert_eq!(result.tags.data.get(7).unwrap().value, "text/plain");
    }
}
