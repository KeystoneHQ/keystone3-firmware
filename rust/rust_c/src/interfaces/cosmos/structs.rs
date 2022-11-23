use crate::interfaces::free::{free_ptr_string, Free};
use crate::interfaces::types::{PtrString, PtrT};
use crate::interfaces::utils::convert_c_char;
use crate::{check_and_free_ptr, impl_c_ptr};
use alloc::string::ToString;
use app_cosmos::transaction::overview::MsgOverview;
use app_cosmos::transaction::structs::{CosmosTxDisplayType, ParsedCosmosTx};
use core::ptr::null_mut;
use third_party::serde_json;

#[repr(C)]
pub struct DisplayCosmosTx {
    pub overview: PtrT<DisplayCosmosTxOverview>,
    pub detail: PtrString,
}

#[repr(C)]
pub struct DisplayCosmosTxOverview {
    // `Send`, `Delegate`, `Undelegate`, `Redelegate`, `WithdrawReward`,'Vote', `Transfer`, `Unknown`
    // `Multiple`
    pub display_type: PtrString,
    pub method: PtrString,
    pub network: PtrString,
    // send
    pub send_value: PtrString,
    pub send_from: PtrString,
    pub send_to: PtrString,
    // Delegate
    pub delegate_value: PtrString,
    pub delegate_from: PtrString,
    pub delegate_to: PtrString,
    // Undelegate
    pub undelegate_value: PtrString,
    pub undelegate_to: PtrString,
    pub undelegate_validator: PtrString,
    // Redelegate
    pub redelegate_value: PtrString,
    pub redelegate_to: PtrString,
    pub redelegate_new_validator: PtrString,
    // Withdraw Reward
    pub withdraw_reward_to: PtrString,
    pub withdraw_reward_validator: PtrString,
    // transfer
    pub transfer_from: PtrString,
    pub transfer_to: PtrString,
    pub transfer_value: PtrString,
    // vote
    pub vote_voted: PtrString,
    pub vote_proposal: PtrString,
    pub vote_voter: PtrString,
    // multiple
    pub overview_list: PtrString,
}

impl_c_ptr!(DisplayCosmosTx);
impl_c_ptr!(DisplayCosmosTxOverview);

impl Default for DisplayCosmosTxOverview {
    fn default() -> Self {
        Self {
            display_type: null_mut(),
            method: null_mut(),
            network: null_mut(),
            send_value: null_mut(),
            send_from: null_mut(),
            send_to: null_mut(),
            delegate_value: null_mut(),
            delegate_from: null_mut(),
            delegate_to: null_mut(),
            undelegate_value: null_mut(),
            undelegate_to: null_mut(),
            undelegate_validator: null_mut(),
            redelegate_value: null_mut(),
            redelegate_to: null_mut(),
            redelegate_new_validator: null_mut(),
            withdraw_reward_to: null_mut(),
            withdraw_reward_validator: null_mut(),
            transfer_from: null_mut(),
            transfer_to: null_mut(),
            transfer_value: null_mut(),
            vote_voted: null_mut(),
            vote_voter: null_mut(),
            vote_proposal: null_mut(),
            overview_list: null_mut(),
        }
    }
}

impl Free for DisplayCosmosTx {
    fn free(&self) {
        unsafe {
            check_and_free_ptr!(self.overview);
            free_ptr_string(self.detail);
        }
    }
}
impl Free for DisplayCosmosTxOverview {
    fn free(&self) {
        unsafe {
            free_ptr_string(self.display_type);
            free_ptr_string(self.method);
            free_ptr_string(self.network);
            free_ptr_string(self.send_value);
            free_ptr_string(self.send_from);
            free_ptr_string(self.send_to);
            free_ptr_string(self.delegate_value);
            free_ptr_string(self.delegate_from);
            free_ptr_string(self.delegate_to);
            free_ptr_string(self.undelegate_value);
            free_ptr_string(self.undelegate_to);
            free_ptr_string(self.undelegate_validator);
            free_ptr_string(self.redelegate_value);
            free_ptr_string(self.redelegate_to);
            free_ptr_string(self.redelegate_new_validator);
            free_ptr_string(self.withdraw_reward_to);
            free_ptr_string(self.withdraw_reward_validator);
            free_ptr_string(self.transfer_from);
            free_ptr_string(self.transfer_to);
            free_ptr_string(self.transfer_value);
            free_ptr_string(self.vote_voted);
            free_ptr_string(self.vote_proposal);
            free_ptr_string(self.vote_voter);
            free_ptr_string(self.overview_list);
        }
    }
}

impl From<ParsedCosmosTx> for DisplayCosmosTx {
    fn from(value: ParsedCosmosTx) -> Self {
        DisplayCosmosTx {
            overview: DisplayCosmosTxOverview::from(&value).c_ptr(),
            detail: convert_c_char(value.detail),
        }
    }
}

impl From<&ParsedCosmosTx> for DisplayCosmosTxOverview {
    fn from(value: &ParsedCosmosTx) -> Self {
        let display_type = convert_c_char(value.overview.display_type.to_string());
        match value.overview.display_type {
            CosmosTxDisplayType::Send => {
                if let MsgOverview::Send(overview) = &value.overview.kind[0] {
                    return Self {
                        display_type,
                        method: convert_c_char(overview.method.clone()),
                        network: convert_c_char(value.overview.common.network.clone()),
                        send_value: convert_c_char(overview.value.clone()),
                        send_from: convert_c_char(overview.from.clone()),
                        send_to: convert_c_char(overview.to.clone()),
                        ..DisplayCosmosTxOverview::default()
                    };
                }
            }
            CosmosTxDisplayType::Delegate => {
                if let MsgOverview::Delegate(overview) = &value.overview.kind[0] {
                    return Self {
                        display_type,
                        method: convert_c_char(overview.method.clone()),
                        network: convert_c_char(value.overview.common.network.clone()),
                        delegate_from: convert_c_char(overview.from.clone()),
                        delegate_to: convert_c_char(overview.to.clone()),
                        delegate_value: convert_c_char(overview.value.clone()),
                        ..DisplayCosmosTxOverview::default()
                    };
                }
            }
            CosmosTxDisplayType::Redelegate => {
                if let MsgOverview::Redelegate(overview) = &value.overview.kind[0] {
                    return Self {
                        display_type,
                        method: convert_c_char(overview.method.clone()),
                        network: convert_c_char(value.overview.common.network.clone()),
                        redelegate_to: convert_c_char(overview.to.clone()),
                        redelegate_value: convert_c_char(overview.value.clone()),
                        redelegate_new_validator: convert_c_char(overview.new_validator.clone()),
                        ..DisplayCosmosTxOverview::default()
                    };
                }
            }
            CosmosTxDisplayType::WithdrawReward => {
                if let MsgOverview::WithdrawReward(overview) = &value.overview.kind[0] {
                    return Self {
                        display_type,
                        method: convert_c_char(overview.method.clone()),
                        network: convert_c_char(value.overview.common.network.clone()),
                        withdraw_reward_to: convert_c_char(overview.to.clone()),
                        withdraw_reward_validator: convert_c_char(overview.validator.clone()),
                        ..DisplayCosmosTxOverview::default()
                    };
                }
            }
            CosmosTxDisplayType::Transfer => {
                if let MsgOverview::Transfer(overview) = &value.overview.kind[0] {
                    return Self {
                        display_type,
                        method: convert_c_char(overview.method.clone()),
                        network: convert_c_char(value.overview.common.network.clone()),
                        transfer_to: convert_c_char(overview.to.clone()),
                        transfer_from: convert_c_char(overview.from.clone()),
                        transfer_value: convert_c_char(overview.value.clone()),
                        ..DisplayCosmosTxOverview::default()
                    };
                }
            }
            CosmosTxDisplayType::Vote => {
                if let MsgOverview::Vote(overview) = &value.overview.kind[0] {
                    return Self {
                        display_type,
                        method: convert_c_char(overview.method.clone()),
                        network: convert_c_char(value.overview.common.network.clone()),
                        vote_voted: convert_c_char(overview.voted.clone()),
                        vote_proposal: convert_c_char(overview.proposal.clone()),
                        vote_voter: convert_c_char(overview.voter.clone()),
                        ..DisplayCosmosTxOverview::default()
                    };
                }
            }
            CosmosTxDisplayType::Unknown => {
                return Self {
                    display_type,
                    ..DisplayCosmosTxOverview::default()
                };
            }
            CosmosTxDisplayType::Multiple => {
                let overview_list =
                    serde_json::to_string(&value.overview.kind).unwrap_or("".to_string());
                return Self {
                    display_type,
                    network: convert_c_char(value.overview.common.network.clone()),
                    overview_list: convert_c_char(overview_list),
                    ..DisplayCosmosTxOverview::default()
                };
            }
            CosmosTxDisplayType::Undelegate => {
                if let MsgOverview::Undelegate(overview) = &value.overview.kind[0] {
                    return Self {
                        display_type,
                        method: convert_c_char(overview.method.clone()),
                        network: convert_c_char(value.overview.common.network.clone()),
                        undelegate_to: convert_c_char(overview.to.clone()),
                        undelegate_validator: convert_c_char(overview.validator.clone()),
                        undelegate_value: convert_c_char(overview.value.clone()),
                        ..DisplayCosmosTxOverview::default()
                    };
                }
            }
        }
        DisplayCosmosTxOverview::default()
    }
}
