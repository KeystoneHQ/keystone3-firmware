use zcash_vendor::{
    zcash_keys::{
        address::Address,
        keys::{ReceiverRequirement, UnifiedAddressRequest, UnifiedFullViewingKey},
    },
    zcash_protocol::consensus,
    zip32::DiversifierIndex,
};

#[derive(Debug, PartialEq, Eq)]
pub enum VerificationResult {
    Owned,
    NotOwned,
    InvalidAddress,
    UnsupportedReceiver,
    KeyUnavailable,
}

pub fn verify<P: consensus::Parameters>(params: &P, ufvk: &str, text: &str) -> VerificationResult {
    use VerificationResult::*;
    let normalized;
    let text = if ["u1", "utest1", "uregtest1"].iter().any(|prefix| {
        text.get(..prefix.len())
            .is_some_and(|head| head.eq_ignore_ascii_case(prefix))
    }) && text.bytes().any(|b| b.is_ascii_uppercase())
    {
        if text.bytes().any(|b| b.is_ascii_lowercase()) {
            return InvalidAddress;
        }
        normalized = text.to_ascii_lowercase();
        normalized.as_str()
    } else {
        text
    };
    let ua = match Address::decode(params, text) {
        Some(Address::Unified(ua)) => ua,
        Some(_) => return UnsupportedReceiver,
        None => return InvalidAddress,
    };
    if !ua.unknown().is_empty() || ua.has_sapling() {
        return UnsupportedReceiver;
    }
    let receiver = match ua.orchard() {
        Some(receiver) => receiver,
        None => return UnsupportedReceiver,
    };
    let ufvk = match UnifiedFullViewingKey::decode(params, ufvk) {
        Ok(ufvk) => ufvk,
        Err(_) => return KeyUnavailable,
    };
    let incoming = ufvk.to_unified_incoming_viewing_key();
    let orchard = match incoming.orchard().as_ref() {
        Some(orchard) => orchard,
        None => return KeyUnavailable,
    };
    let index = match orchard.diversifier_index(receiver) {
        Some(index) => DiversifierIndex::from(*index.as_bytes()),
        None => return NotOwned,
    };
    let request = UnifiedAddressRequest::custom(
        ReceiverRequirement::Require,
        ReceiverRequirement::Omit,
        if ua.has_transparent() {
            ReceiverRequirement::Require
        } else {
            ReceiverRequirement::Omit
        },
    )
    .expect("Orchard is required");
    match incoming.address(index, request) {
        Ok(expected)
            if expected.orchard() == ua.orchard() && expected.transparent() == ua.transparent() =>
        {
            Owned
        }
        _ => NotOwned,
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use alloc::{vec, vec::Vec};
    use keystore::algorithms::zcash::derive_ufvk;
    use zcash_vendor::zip32::Scope;
    use zcash_vendor::{
        zcash_address::unified::{self, Container, Encoding, Receiver},
        zcash_protocol::consensus::{MainNetwork, NetworkType, TestNetwork},
    };

    #[test]
    fn official_mainnet_vector_is_owned() {
        let ufvk = "uview1dgucetjuudl3msv6tcecj0getnxsdpk5j60h74nckdtrrqs5p4gvf8a3s3lflxednpr774sx7tyaqfxw0s8mnnl7nk7h4m5m3y8xl7efgjprar8sdq6k3mha9l9cln5wy3s6k4g7ma0tc7y30x5xwl7ezxrzhvnrgd8q9zzh4l088mta932ml7wll39y3s82qmse58ulr7p6rj59lh9v0g7evcpghu3ntnh6jr2jzj2vdc904ahhlkq6sn0jdyt3l4s0ch6af7nvh4emlspt32hgk5t59mg3nuxnjqpvpx26vwcfvpuhm8vmpy0ray4mhwhtz9de8gvwg423rpw7l899fvyu0wmr5lp5lu46ckufa2gpt9msjpxxach5cnuwaxph00e65tfmelxyed78t57t4g7f8v9q9fq0378g23yql2t9tp78g3mp0xl30q623rmxglhknl79e7ltwurfvpw7w4k57yzv97exq9h0fv0urgvffu6dqmk5";
        let address = "u1ukslldhknrzmvpdmn03u03edgfy976w3muurfs9asvh3n9uh9h6sgle6m7yjgf3wafxtvke08u735v4nd3kjqnyulw7cvxh6ke357knyjudgqtes6kcw7y28e6kewr03pjah5mh26na";
        assert_eq!(
            verify(&MainNetwork, ufvk, address),
            VerificationResult::Owned
        );
        assert_eq!(
            verify(&MainNetwork, ufvk, &address.to_ascii_uppercase()),
            VerificationResult::Owned
        );
        assert_eq!(
            verify(&TestNetwork, ufvk, address),
            VerificationResult::InvalidAddress
        );
    }

    #[test]
    fn uppercase_addresses_are_owned_but_mixed_case_is_invalid() {
        let key = key(1);
        let ufvk = key.encode(&MainNetwork).unwrap();
        let (address, _) = key.default_address(UnifiedAddressRequest::ORCHARD).unwrap();
        let text = address.encode(&MainNetwork);
        assert_eq!(
            verify(&MainNetwork, &ufvk, &text.to_ascii_uppercase()),
            VerificationResult::Owned
        );
        let mixed = alloc::format!("U{}", &text[1..]);
        assert_eq!(
            verify(&MainNetwork, &ufvk, &mixed),
            VerificationResult::InvalidAddress
        );
        let mut broken = text.to_ascii_uppercase();
        broken.pop();
        broken.push('!');
        assert_eq!(
            verify(&MainNetwork, &ufvk, &broken),
            VerificationResult::InvalidAddress
        );
        let testnet = address.encode(&TestNetwork).to_ascii_uppercase();
        assert_eq!(
            verify(&MainNetwork, &ufvk, &testnet),
            VerificationResult::InvalidAddress
        );
    }

    fn key(seed: u8) -> UnifiedFullViewingKey {
        UnifiedFullViewingKey::decode(
            &MainNetwork,
            &derive_ufvk(&MainNetwork, &[seed; 32], "m/32'/133'/0'").unwrap(),
        )
        .unwrap()
    }

    fn check(key: &UnifiedFullViewingKey, receivers: Vec<Receiver>) -> VerificationResult {
        let text = unified::Address::try_from_items(
            unified::Revision::R0,
            receivers.into_iter().map(unified::Uitem::Data).collect(),
        )
        .unwrap()
        .encode(&NetworkType::Main);
        verify(&MainNetwork, &key.encode(&MainNetwork).unwrap(), &text)
    }

    #[test]
    fn default_and_rotated_addresses_are_owned() {
        let key = key(1);
        for i in [0u32, 1, 42, 1729704000] {
            let address = key
                .address(i.into(), UnifiedAddressRequest::AllAvailableKeys)
                .unwrap();
            assert_eq!(
                verify(
                    &MainNetwork,
                    &key.encode(&MainNetwork).unwrap(),
                    &address.encode(&MainNetwork)
                ),
                VerificationResult::Owned
            );
        }
    }

    #[test]
    fn orchard_only_supports_full_diversifier_range() {
        let key = key(1);
        let index = DiversifierIndex::from([0xff; 11]);
        let address = key.address(index, UnifiedAddressRequest::ORCHARD).unwrap();
        assert_eq!(
            verify(
                &MainNetwork,
                &key.encode(&MainNetwork).unwrap(),
                &address.encode(&MainNetwork)
            ),
            VerificationResult::Owned
        );
    }

    #[test]
    fn other_wallet_and_internal_address_are_not_owned() {
        let key = key(1);
        let other = key
            .address(0u32.into(), UnifiedAddressRequest::AllAvailableKeys)
            .unwrap();
        assert_eq!(
            verify(
                &MainNetwork,
                &self::key(2).encode(&MainNetwork).unwrap(),
                &other.encode(&MainNetwork)
            ),
            VerificationResult::NotOwned
        );
        let internal = key
            .orchard()
            .unwrap()
            .to_ivk(Scope::Internal)
            .address_at(0u32);
        assert_eq!(
            check(
                &key,
                vec![Receiver::Orchard(internal.to_raw_address_bytes())]
            ),
            VerificationResult::NotOwned
        );
    }

    #[test]
    fn mixed_receivers_and_wrong_transparent_index_are_rejected() {
        let key = key(1);
        let ua = key
            .address(0u32.into(), UnifiedAddressRequest::AllAvailableKeys)
            .unwrap();
        let orchard = Receiver::Orchard(ua.orchard().unwrap().to_raw_address_bytes());
        for other in [
            self::key(2)
                .address(0u32.into(), UnifiedAddressRequest::AllAvailableKeys)
                .unwrap(),
            key.address(1u32.into(), UnifiedAddressRequest::AllAvailableKeys)
                .unwrap(),
        ] {
            let (_, _, raw) = unified::Address::decode(&other.encode(&MainNetwork)).unwrap();
            let transparent = raw
                .items_as_parsed()
                .iter()
                .find_map(|item| match item {
                    unified::Uitem::Data(r @ Receiver::P2pkh(_)) => Some(r.clone()),
                    _ => None,
                })
                .unwrap();
            assert_eq!(
                check(&key, vec![orchard.clone(), transparent]),
                VerificationResult::NotOwned
            );
        }
        assert_eq!(
            check(&key, vec![orchard, Receiver::P2sh([1; 20])]),
            VerificationResult::NotOwned
        );
    }

    #[test]
    fn unverified_receivers_never_report_success() {
        let key = key(1);
        let ua = key
            .address(0u32.into(), UnifiedAddressRequest::ORCHARD)
            .unwrap();
        let orchard = Receiver::Orchard(ua.orchard().unwrap().to_raw_address_bytes());
        assert_eq!(
            check(
                &key,
                vec![
                    orchard.clone(),
                    Receiver::Unknown {
                        typecode: 65535,
                        data: vec![1; 20]
                    }
                ]
            ),
            VerificationResult::UnsupportedReceiver
        );
        assert_eq!(
            check(&key, vec![orchard, Receiver::Sapling([0; 43])]),
            VerificationResult::UnsupportedReceiver
        );
        assert_ne!(
            check(&key, vec![Receiver::Sapling([0; 43])]),
            VerificationResult::Owned
        );
    }

    #[test]
    fn invalid_encoding_network_and_viewing_key_are_rejected() {
        let key = key(1);
        let ua = key
            .address(0u32.into(), UnifiedAddressRequest::ORCHARD)
            .unwrap();
        let ufvk = key.encode(&MainNetwork).unwrap();
        for text in ["", "u1invalid", "zcash:u1invalid"] {
            assert_eq!(
                verify(&MainNetwork, &ufvk, text),
                VerificationResult::InvalidAddress
            );
        }
        assert_eq!(
            verify(&MainNetwork, &ufvk, &ua.encode(&TestNetwork)),
            VerificationResult::InvalidAddress
        );
        assert_eq!(
            verify(&MainNetwork, "", &ua.encode(&MainNetwork)),
            VerificationResult::KeyUnavailable
        );
        let mut corrupt = ua.encode(&MainNetwork).into_bytes();
        let last = corrupt.len() - 1;
        corrupt[last] = if corrupt[last] == b'q' { b'p' } else { b'q' };
        assert_eq!(
            verify(&MainNetwork, &ufvk, core::str::from_utf8(&corrupt).unwrap()),
            VerificationResult::InvalidAddress
        );
    }
}
