use alloc::string::ToString;
use alloc::vec::Vec;
use alloc::{boxed::Box, string::String};
use app_cardano::errors::CardanoError;
use app_cardano::structs::{
    CardanoCertificate, CardanoFrom, CardanoTo, CardanoWithdrawal, ParsedCardanoMultiAsset,
    ParsedCardanoNativeAsset, ParsedCardanoSignCip8Data, ParsedCardanoSignData, ParsedCardanoTx,
    VotingProcedure, VotingProposal,
};
use core::ptr::null_mut;
use hex;
use itertools::Itertools;
use ur_registry::cardano::cardano_catalyst_voting_registration::CardanoCatalystVotingRegistrationRequest;

use crate::common::ffi::VecFFI;
use crate::common::free::{free_ptr_string, Free};
use crate::common::structs::TransactionParseResult;
use crate::common::types::{Ptr, PtrString, PtrT};
use crate::common::utils::convert_c_char;
use crate::{free_str_ptr, free_vec, impl_c_ptr, impl_c_ptrs, make_free_method};

#[repr(C)]
pub struct DisplayCardanoSignData {
    pub payload: PtrString,
    pub payload_is_hex: bool,
    pub derivation_path: PtrString,
    pub message_hash: PtrString,
    pub xpub: PtrString,
    pub hash_payload: bool,
}

#[repr(C)]
pub struct DisplayCardanoCatalyst {
    pub nonce: PtrString,
    pub stake_key: PtrString,
    pub rewards: PtrString,
    pub vote_keys: Ptr<VecFFI<PtrString>>,
}

impl TryFrom<CardanoCatalystVotingRegistrationRequest> for DisplayCardanoCatalyst {
    type Error = CardanoError;

    fn try_from(value: CardanoCatalystVotingRegistrationRequest) -> Result<Self, Self::Error> {
        let stake_key = app_cardano::governance::parse_stake_address(value.get_stake_pub())?;
        let rewards = app_cardano::governance::parse_payment_address(value.get_payment_address())?;

        Ok(Self {
            nonce: convert_c_char(value.get_nonce().to_string()),
            stake_key: convert_c_char(stake_key),
            rewards: convert_c_char(rewards),
            vote_keys: VecFFI::from(
                value
                    .get_delegations()
                    .iter()
                    .map(|v| convert_c_char(hex::encode(v.get_pub_key())))
                    .collect_vec(),
            )
            .c_ptr(),
        })
    }
}

#[repr(C)]
pub struct DisplayCardanoTx {
    pub from: PtrT<VecFFI<DisplayCardanoFrom>>,
    pub to: PtrT<VecFFI<DisplayCardanoTo>>,
    pub fee: PtrString,
    pub network: PtrString,
    pub total_input: PtrString,
    pub total_output: PtrString,
    pub ttl: PtrString,
    pub validity_start: PtrString,
    pub withdrawals_total: PtrString,
    pub total_collateral: PtrString,
    pub collateral_return: PtrString,
    pub collateral_inputs_count: u32,
    pub reference_inputs_count: u32,
    pub donation: PtrString,
    pub required_signers_count: u32,
    pub governance_votes_count: u32,
    pub governance_proposals_count: u32,
    pub transaction_is_valid: bool,
    pub certificates: Ptr<VecFFI<DisplayCardanoCertificate>>,
    pub withdrawals: Ptr<VecFFI<DisplayCardanoWithdrawal>>,
    pub auxiliary_data: PtrString,
    pub governance_data: PtrString,
    pub advanced_data: PtrString,
    pub voting_procedures: Ptr<VecFFI<DisplayVotingProcedure>>,
    pub voting_proposals: Ptr<VecFFI<DisplayVotingProposal>>,
    pub mint_assets: Ptr<VecFFI<DisplayCardanoAsset>>,
    pub collateral_assets: Ptr<VecFFI<DisplayCardanoAsset>>,
    pub has_multi_assets: bool,
    pub has_unknown_inputs: bool,
    pub raw_data: PtrString,
}

#[repr(C)]
pub struct DisplayCardanoFrom {
    address: PtrString,
    amount: PtrString,
    has_path: bool,
    path: PtrString,
    transaction_id: PtrString,
    index: u32,
    known: bool,
}

#[repr(C)]
pub struct DisplayCardanoTo {
    address: PtrString,
    amount: PtrString,
    has_assets: bool,
    assets_text: PtrString,
    assets: Ptr<VecFFI<DisplayCardanoAsset>>,
}

#[repr(C)]
pub struct DisplayCardanoAsset {
    name: PtrString,
    name_hex: PtrString,
    policy_id: PtrString,
    amount: PtrString,
}

#[repr(C)]
pub struct DisplayCertField {
    pub label: PtrString,
    pub value: PtrString,
}

#[repr(C)]
pub struct DisplayCardanoCertificate {
    cert_type: PtrString,
    fields: Ptr<VecFFI<DisplayCertField>>,
}

#[repr(C)]
pub struct DisplayVotingProcedure {
    voter_type: PtrString,
    voter: PtrString,
    transaction_id: PtrString,
    index: PtrString,
    vote: PtrString,
}

#[repr(C)]
pub struct DisplayVotingProposal {
    action: PtrString,
    deposit: PtrString,
    reward_account: PtrString,
    anchor_url: PtrString,
    anchor_data_hash: PtrString,
}

#[repr(C)]
pub struct DisplayCardanoWithdrawal {
    address: PtrString,
    amount: PtrString,
}
#[repr(C)]
pub struct DisplayCardanoSignTxHash {
    pub network: PtrString,
    pub path: Ptr<VecFFI<PtrString>>,
    pub tx_hash: PtrString,
    pub address_list: Ptr<VecFFI<PtrString>>,
}

impl DisplayCardanoSignTxHash {
    pub fn new(
        network: String,
        path: Vec<String>,
        tx_hash: String,
        address_list: Vec<String>,
    ) -> Self {
        Self {
            network: convert_c_char(network),
            path: VecFFI::from(path.iter().map(|v| convert_c_char(v.clone())).collect_vec())
                .c_ptr(),
            tx_hash: convert_c_char(tx_hash),
            address_list: VecFFI::from(
                address_list
                    .iter()
                    .map(|v| convert_c_char(v.clone()))
                    .collect_vec(),
            )
            .c_ptr(),
        }
    }
}

impl_c_ptrs!(DisplayCardanoTx);

impl_c_ptrs!(DisplayCardanoCatalyst);

impl_c_ptrs!(DisplayCardanoSignData);

impl_c_ptrs!(DisplayCardanoSignTxHash);

impl Free for DisplayCardanoSignData {
    unsafe fn free(&self) {
        free_str_ptr!(self.payload);
        free_str_ptr!(self.derivation_path);
        free_str_ptr!(self.message_hash);
        free_str_ptr!(self.xpub);
    }
}

impl Free for DisplayCardanoCatalyst {
    unsafe fn free(&self) {
        free_str_ptr!(self.nonce);
        free_str_ptr!(self.stake_key);
        free_str_ptr!(self.rewards);
        free_vec!(self.vote_keys);
    }
}

impl Free for DisplayCardanoSignTxHash {
    unsafe fn free(&self) {
        free_str_ptr!(self.network);
        free_vec!(self.path);
        free_str_ptr!(self.tx_hash);
        free_vec!(self.address_list);
    }
}

impl Free for DisplayCardanoTx {
    unsafe fn free(&self) {
        let x = Box::from_raw(self.from);
        let ve = Vec::from_raw_parts(x.data, x.size, x.cap);
        ve.iter().for_each(|v| {
            v.free();
        });
        let x = Box::from_raw(self.to);
        let ve = Vec::from_raw_parts(x.data, x.size, x.cap);
        ve.iter().for_each(|v| {
            v.free();
        });
        free_vec!(self.withdrawals);
        free_vec!(self.certificates);
        free_vec!(self.voting_procedures);
        free_vec!(self.voting_proposals);
        free_vec!(self.mint_assets);
        free_vec!(self.collateral_assets);

        free_str_ptr!(self.total_input);
        free_str_ptr!(self.total_output);
        free_str_ptr!(self.fee);
        free_str_ptr!(self.network);
        free_str_ptr!(self.ttl);
        free_str_ptr!(self.validity_start);
        free_str_ptr!(self.withdrawals_total);
        free_str_ptr!(self.total_collateral);
        free_str_ptr!(self.collateral_return);
        free_str_ptr!(self.donation);
        free_str_ptr!(self.auxiliary_data);
        free_str_ptr!(self.governance_data);
        free_str_ptr!(self.advanced_data);
        free_str_ptr!(self.raw_data);
    }
}

impl From<ParsedCardanoSignData> for DisplayCardanoSignData {
    fn from(value: ParsedCardanoSignData) -> Self {
        let (payload, payload_is_hex, derivation_path, message_hash, xpub) =
            value.into_display_parts();
        Self {
            payload: convert_c_char(payload),
            payload_is_hex,
            derivation_path: convert_c_char(derivation_path),
            message_hash: convert_c_char(message_hash),
            xpub: convert_c_char(xpub),
            hash_payload: false,
        }
    }
}

impl From<ParsedCardanoSignCip8Data> for DisplayCardanoSignData {
    fn from(value: ParsedCardanoSignCip8Data) -> Self {
        let (payload, payload_is_hex, derivation_path, message_hash, xpub, hash_payload) =
            value.into_display_parts();
        Self {
            payload: convert_c_char(payload),
            payload_is_hex,
            derivation_path: convert_c_char(derivation_path),
            message_hash: convert_c_char(message_hash),
            xpub: convert_c_char(xpub),
            hash_payload,
        }
    }
}

impl From<ParsedCardanoTx> for DisplayCardanoTx {
    fn from(value: ParsedCardanoTx) -> Self {
        let (
            fee,
            total_input,
            total_output,
            from,
            to,
            network,
            ttl,
            validity_start,
            withdrawals_total,
            total_collateral,
            collateral_return,
            collateral_inputs_count,
            reference_inputs_count,
            donation,
            required_signers_count,
            governance_votes_count,
            governance_proposals_count,
            transaction_is_valid,
            certificates,
            withdrawals,
            auxiliary_data,
            governance_data,
            advanced_data,
            voting_procedures,
            voting_proposals,
            mint_assets,
            collateral_assets,
            has_multi_assets,
            has_unknown_inputs,
        ) = value.into_display_parts();
        Self {
            from: VecFFI::from(from.into_iter().map(DisplayCardanoFrom::from).collect_vec())
                .c_ptr(),
            to: VecFFI::from(to.into_iter().map(DisplayCardanoTo::from).collect_vec()).c_ptr(),
            fee: convert_c_char(fee),
            network: convert_c_char(network),
            total_input: convert_c_char(total_input),
            total_output: convert_c_char(total_output),
            ttl: ttl.map(convert_c_char).unwrap_or(null_mut()),
            validity_start: validity_start.map(convert_c_char).unwrap_or(null_mut()),
            withdrawals_total: withdrawals_total.map(convert_c_char).unwrap_or(null_mut()),
            total_collateral: total_collateral.map(convert_c_char).unwrap_or(null_mut()),
            collateral_return: collateral_return.map(convert_c_char).unwrap_or(null_mut()),
            collateral_inputs_count,
            reference_inputs_count,
            donation: donation.map(convert_c_char).unwrap_or(null_mut()),
            required_signers_count,
            governance_votes_count,
            governance_proposals_count,
            transaction_is_valid,
            certificates: VecFFI::from(
                certificates
                    .into_iter()
                    .map(DisplayCardanoCertificate::from)
                    .collect_vec(),
            )
            .c_ptr(),
            withdrawals: VecFFI::from(
                withdrawals
                    .into_iter()
                    .map(DisplayCardanoWithdrawal::from)
                    .collect_vec(),
            )
            .c_ptr(),
            auxiliary_data: auxiliary_data.map(convert_c_char).unwrap_or(null_mut()),
            governance_data: governance_data.map(convert_c_char).unwrap_or(null_mut()),
            advanced_data: advanced_data.map(convert_c_char).unwrap_or(null_mut()),
            voting_procedures: VecFFI::from(
                voting_procedures
                    .into_iter()
                    .map(DisplayVotingProcedure::from)
                    .collect_vec(),
            )
            .c_ptr(),
            voting_proposals: VecFFI::from(
                voting_proposals
                    .into_iter()
                    .map(DisplayVotingProposal::from)
                    .collect_vec(),
            )
            .c_ptr(),
            mint_assets: VecFFI::from(
                mint_assets
                    .into_iter()
                    .map(DisplayCardanoAsset::from)
                    .collect_vec(),
            )
            .c_ptr(),
            collateral_assets: VecFFI::from(
                collateral_assets
                    .into_iter()
                    .map(DisplayCardanoAsset::from)
                    .collect_vec(),
            )
            .c_ptr(),
            has_multi_assets,
            has_unknown_inputs,
            raw_data: null_mut(),
        }
    }
}

impl DisplayCardanoTx {
    pub fn from_with_raw_data(value: ParsedCardanoTx, raw_data: &[u8]) -> Self {
        let mut display = Self::from(value);
        let mut raw = String::with_capacity(raw_data.len().saturating_mul(2));
        append_hex(&mut raw, raw_data);
        display.raw_data = convert_c_char(raw);
        display
    }
}

fn append_hex(output: &mut String, bytes: &[u8]) {
    const HEX: &[u8; 16] = b"0123456789abcdef";
    output.reserve(bytes.len().saturating_mul(2));
    for byte in bytes {
        output.push(HEX[(byte >> 4) as usize] as char);
        output.push(HEX[(byte & 0x0f) as usize] as char);
    }
}

impl From<CardanoFrom> for DisplayCardanoFrom {
    fn from(value: CardanoFrom) -> Self {
        let (address, amount, path, transaction_id, index, known) = value.into_display_parts();
        Self {
            address: convert_c_char(address),
            amount: convert_c_char(amount),
            has_path: path.is_some(),
            path: path.map(convert_c_char).unwrap_or(null_mut()),
            transaction_id: convert_c_char(transaction_id),
            index,
            known,
        }
    }
}

impl Free for DisplayCardanoTo {
    unsafe fn free(&self) {
        free_vec!(self.assets);
        free_str_ptr!(self.assets_text);
        free_str_ptr!(self.address);
        free_str_ptr!(self.amount);
    }
}

impl Free for DisplayCardanoFrom {
    unsafe fn free(&self) {
        free_str_ptr!(self.path);
        free_str_ptr!(self.transaction_id);
        free_str_ptr!(self.amount);
        free_str_ptr!(self.address);
    }
}

impl From<CardanoTo> for DisplayCardanoTo {
    fn from(value: CardanoTo) -> Self {
        let (address, amount, assets_text, assets) = value.into_display_parts();
        Self {
            address: convert_c_char(address),
            amount: convert_c_char(amount),
            has_assets: assets_text.is_some(),
            assets_text: assets_text.map(convert_c_char).unwrap_or(null_mut()),
            assets: VecFFI::from(
                assets
                    .into_iter()
                    .map(DisplayCardanoAsset::from)
                    .collect_vec(),
            )
            .c_ptr(),
        }
    }
}

fn crc8(bytes: &[u8]) -> u8 {
    let mut crc = 0u8;
    for byte in bytes {
        crc ^= byte;
        for _ in 0..8 {
            crc = if crc & 0x80 != 0 {
                (crc << 1) ^ 0x07
            } else {
                crc << 1
            };
        }
    }
    crc
}

fn cip67_asset_name_content(name: &[u8]) -> Option<&[u8]> {
    if name.len() <= 4 || name[0] & 0xf0 != 0 || name[3] & 0x0f != 0 {
        return None;
    }

    let label = [
        (name[0] << 4) | (name[1] >> 4),
        (name[1] << 4) | (name[2] >> 4),
    ];
    let checksum = (name[2] << 4) | (name[3] >> 4);
    (crc8(&label) == checksum).then_some(&name[4..])
}

fn printable_asset_name(name: &[u8]) -> Option<String> {
    let content = cip67_asset_name_content(name).unwrap_or(name);
    core::str::from_utf8(content)
        .ok()
        .filter(|value| !value.chars().any(char::is_control))
        .map(ToString::to_string)
}

impl DisplayCardanoAsset {
    fn new(policy_id: Vec<u8>, name: Vec<u8>, amount: String) -> Self {
        let printable_name = printable_asset_name(&name);
        Self {
            name: printable_name.map(convert_c_char).unwrap_or(null_mut()),
            name_hex: convert_c_char(hex::encode(name)),
            policy_id: convert_c_char(hex::encode(policy_id)),
            amount: convert_c_char(amount),
        }
    }
}

impl From<ParsedCardanoMultiAsset> for DisplayCardanoAsset {
    fn from(value: ParsedCardanoMultiAsset) -> Self {
        let (policy_id, name, amount) = value.into_display_parts();
        Self::new(policy_id, name, amount)
    }
}

impl From<ParsedCardanoNativeAsset> for DisplayCardanoAsset {
    fn from(value: ParsedCardanoNativeAsset) -> Self {
        let (policy_id, name, amount) = value.into_display_parts();
        Self::new(policy_id, name, amount)
    }
}

impl Free for DisplayCardanoAsset {
    unsafe fn free(&self) {
        free_str_ptr!(self.name);
        free_str_ptr!(self.name_hex);
        free_str_ptr!(self.policy_id);
        free_str_ptr!(self.amount);
    }
}

impl From<VotingProcedure> for DisplayVotingProcedure {
    fn from(value: VotingProcedure) -> Self {
        let (voter_type, voter, transaction_id, index, vote) = value.into_display_parts();
        Self {
            voter_type: convert_c_char(voter_type),
            voter: convert_c_char(voter),
            transaction_id: convert_c_char(transaction_id),
            index: convert_c_char(index),
            vote: convert_c_char(vote),
        }
    }
}

impl From<VotingProposal> for DisplayVotingProposal {
    fn from(value: VotingProposal) -> Self {
        let (action, deposit, reward_account, anchor_url, anchor_data_hash) =
            value.into_display_parts();
        Self {
            action: convert_c_char(action),
            deposit: convert_c_char(deposit),
            reward_account: convert_c_char(reward_account),
            anchor_url: convert_c_char(anchor_url),
            anchor_data_hash: convert_c_char(anchor_data_hash),
        }
    }
}

impl From<CardanoCertificate> for DisplayCardanoCertificate {
    fn from(value: CardanoCertificate) -> Self {
        let (cert_type, fields) = value.into_display_parts();
        Self {
            cert_type: convert_c_char(cert_type),
            fields: VecFFI::from(
                fields
                    .into_iter()
                    .map(|v| {
                        let (label, value) = v.into_display_parts();
                        DisplayCertField {
                            label: convert_c_char(label),
                            value: convert_c_char(value),
                        }
                    })
                    .collect_vec(),
            )
            .c_ptr(),
        }
    }
}

impl Free for DisplayCardanoCertificate {
    unsafe fn free(&self) {
        free_str_ptr!(self.cert_type);
        free_vec!(self.fields);
    }
}

impl Free for DisplayVotingProcedure {
    unsafe fn free(&self) {
        free_str_ptr!(self.voter_type);
        free_str_ptr!(self.voter);
        free_str_ptr!(self.transaction_id);
        free_str_ptr!(self.index);
        free_str_ptr!(self.vote);
    }
}

impl Free for DisplayVotingProposal {
    unsafe fn free(&self) {
        free_str_ptr!(self.action);
        free_str_ptr!(self.deposit);
        free_str_ptr!(self.reward_account);
        free_str_ptr!(self.anchor_url);
        free_str_ptr!(self.anchor_data_hash);
    }
}

impl From<CardanoWithdrawal> for DisplayCardanoWithdrawal {
    fn from(value: CardanoWithdrawal) -> Self {
        let (address, amount) = value.into_display_parts();
        Self {
            address: convert_c_char(address),
            amount: convert_c_char(amount),
        }
    }
}

impl Free for DisplayCardanoWithdrawal {
    unsafe fn free(&self) {
        free_str_ptr!(self.address);
        free_str_ptr!(self.amount);
    }
}

impl Free for DisplayCertField {
    unsafe fn free(&self) {
        free_str_ptr!(self.label);
        free_str_ptr!(self.value);
    }
}

make_free_method!(TransactionParseResult<DisplayCardanoTx>);
make_free_method!(TransactionParseResult<DisplayCardanoCatalyst>);
make_free_method!(TransactionParseResult<DisplayCardanoSignData>);
make_free_method!(TransactionParseResult<DisplayCardanoSignTxHash>);

#[cfg(test)]
mod tests {
    use super::{cip67_asset_name_content, printable_asset_name};

    #[test]
    fn displays_raw_printable_asset_name() {
        assert_eq!(printable_asset_name(b"STRIKE").as_deref(), Some("STRIKE"));
    }

    #[test]
    fn displays_printable_content_after_valid_cip67_label() {
        let gens = hex::decode("0014df1047454e53").unwrap();
        assert_eq!(cip67_asset_name_content(&gens), Some(b"GENS".as_slice()));
        assert_eq!(printable_asset_name(&gens).as_deref(), Some("GENS"));
    }

    #[test]
    fn does_not_strip_invalid_cip67_label() {
        let invalid = hex::decode("0014de1047454e53").unwrap();
        assert_eq!(cip67_asset_name_content(&invalid), None);
        assert_eq!(printable_asset_name(&invalid), None);
    }
}
