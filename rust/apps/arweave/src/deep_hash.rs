use alloc::vec::Vec;

use crate::transaction::DeepHashItem;
use third_party::cryptoxide::digest::Digest;
use third_party::cryptoxide::sha2::Sha384;

use crate::errors::ArweaveError;

fn concat_u8_48(left: [u8; 48], right: [u8; 48]) -> Result<[u8; 96], ArweaveError> {
    let mut iter = left.into_iter().chain(right);
    let result = [(); 96].map(|_| iter.next().unwrap());
    Ok(result)
}

pub fn hash_sha384(message: &[u8]) -> Result<[u8; 48], ArweaveError> {
    let mut hasher = Sha384::new();
    hasher.input(message);
    let mut result = [0u8; 48];
    hasher.result(&mut result);
    Ok(result)
}

pub fn hash_all_sha384(messages: Vec<&[u8]>) -> Result<[u8; 48], ArweaveError> {
    let hash: Vec<u8> = messages
        .into_iter()
        .map(|m| hash_sha384(m).unwrap())
        .into_iter()
        .flatten()
        .collect();
    let hash = hash_sha384(&hash)?;
    Ok(hash)
}

pub fn deep_hash(deep_hash_item: DeepHashItem) -> Result<[u8; 48], ArweaveError> {
    let hash = match deep_hash_item {
        DeepHashItem::Blob(blob) => {
            let blob_tag = format!("blob{}", blob.len());
            hash_all_sha384(vec![blob_tag.as_bytes(), &blob])?
        }
        DeepHashItem::List(list) => {
            let list_tag = format!("list{}", list.len());
            let mut hash = hash_sha384(list_tag.as_bytes())?;

            for child in list.into_iter() {
                let child_hash = deep_hash(child)?;
                hash = hash_sha384(&concat_u8_48(hash, child_hash)?)?;
            }
            hash
        }
    };
    Ok(hash)
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::transaction::DeepHashItem;
    use alloc::vec;

    #[test]
    fn test_concat_u8_48() {
        let left = [1u8; 48];
        let right = [2u8; 48];
        let result = concat_u8_48(left, right).unwrap();
        assert_eq!(&result[..48], &left);
        assert_eq!(&result[48..], &right);
    }

    #[test]
    fn test_hash_sha384() {
        let message = b"test message";
        let result = hash_sha384(message).unwrap();
        assert_eq!(result.len(), 48);
    }

    #[test]
    fn test_hash_all_sha384() {
        let messages = vec![b"message1".as_ref(), b"message2".as_ref()];
        let result = hash_all_sha384(messages).unwrap();
        assert_eq!(result.len(), 48);
    }

    #[test]
    fn test_deep_hash_blob() {
        let blob = b"test blob";
        let item = DeepHashItem::Blob(blob.to_vec());
        let result = deep_hash(item).unwrap();
        assert_eq!(result.len(), 48);
    }

    #[test]
    fn test_deep_hash_list() {
        let blob1 = DeepHashItem::Blob(b"blob1".to_vec());
        let blob2 = DeepHashItem::Blob(b"blob2".to_vec());
        let list = DeepHashItem::List(vec![blob1, blob2]);
        let result = deep_hash(list).unwrap();
        assert_eq!(result.len(), 48);
    }
}
