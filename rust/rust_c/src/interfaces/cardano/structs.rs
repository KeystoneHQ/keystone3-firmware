use crate::interfaces::ffi::VecFFI;
use crate::interfaces::free::{free_ptr_string, Free};
use crate::interfaces::types::{PtrString, PtrT};
use crate::interfaces::utils::convert_c_char;
use crate::{free_str_ptr, free_vec, impl_c_ptr, impl_c_ptrs};
use alloc::boxed::Box;
use alloc::string::ToString;
use alloc::vec::Vec;
use app_cardano::detail::{CardanoDetail, CardanoDetailStakeAction};
use app_cardano::overview::{CardanoHeaderCard, CardanoOverview};
use app_cardano::structs::{CardanoFrom, CardanoTo, ParsedCardanoTx};
use core::ptr::null_mut;
use third_party::itertools::Itertools;

#[repr(C)]
pub struct DisplayCardanoTx {
    pub overview: PtrT<DisplayCardanoOverview>,
    pub detail: PtrT<DisplayCardanoDetail>,
    pub from: PtrT<VecFFI<DisplayCardanoFrom>>,
    pub to: PtrT<VecFFI<DisplayCardanoTo>>,
    pub fee: PtrString,
    pub network: PtrString,
    pub method: PtrString,
}

#[repr(C)]
pub struct DisplayCardanoOverview {
    // `Transfer`, `Stake`, `Withdrawal`
    pub header_type: PtrString,
    //transfer
    pub total_output_amount: PtrString,
    //stake
    pub stake_amount: PtrString,
    pub has_deposit_amount: bool,
    pub deposit_amount: PtrString,
    //withdrawal
    pub reward_amount: PtrString,
    pub has_deposit_reclaim_amount: bool,
    pub deposit_reclaim_amount: PtrString,
    pub has_reward_account: bool,
    pub reward_account: PtrString,
}

#[repr(C)]
pub struct DisplayCardanoDetail {
    pub total_input_amount: PtrString,
    pub total_output_amount: PtrString,
    pub has_deposit_amount: bool,
    pub deposit_amount: PtrString,
    pub has_deposit_reclaim_amount: bool,
    pub deposit_reclaim_amount: PtrString,
    pub has_stake_content: bool,
    pub stake_content: PtrT<VecFFI<DisplayCardanoStakeContent>>,
}

#[repr(C)]
pub struct DisplayCardanoFrom {
    address: PtrString,
    amount: PtrString,
    has_path: bool,
    path: PtrString,
}

#[repr(C)]
pub struct DisplayCardanoTo {
    address: PtrString,
    amount: PtrString,
    has_assets: bool,
    assets_text: PtrString,
}

#[repr(C)]
pub struct DisplayCardanoStakeContent {
    // `Stake`, `Withdrawal`, `Registration`
    pub stake_type: PtrString,
    // Stake
    pub stake_key: PtrString,
    pub pool: PtrString,
    // Registration
    pub registration_stake_key: PtrString,
    // Withdrawal
    pub reward_address: PtrString,
    pub reward_amount: PtrString,
    pub deregistration_stake_key: PtrString,
}

impl_c_ptrs!(
    DisplayCardanoTx,
    DisplayCardanoOverview,
    DisplayCardanoDetail
);

impl Free for DisplayCardanoTx {
    fn free(&self) {
        unsafe {
            let overview = Box::from_raw(self.overview);
            let detail = Box::from_raw(self.detail);
            overview.free();
            detail.free();

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

            free_ptr_string(self.fee);
            free_ptr_string(self.network);
            free_ptr_string(self.method);
        }
    }
}

impl From<ParsedCardanoTx> for DisplayCardanoTx {
    fn from(value: ParsedCardanoTx) -> Self {
        Self {
            overview: DisplayCardanoOverview::from(&value.get_overview()).c_ptr(),
            detail: DisplayCardanoDetail::from(&value.get_detail()).c_ptr(),
            from: VecFFI::from(
                value
                    .get_from()
                    .iter()
                    .map(DisplayCardanoFrom::from)
                    .collect_vec(),
            )
            .c_ptr(),
            to: VecFFI::from(
                value
                    .get_to()
                    .iter()
                    .map(DisplayCardanoTo::from)
                    .collect_vec(),
            )
            .c_ptr(),
            fee: convert_c_char(value.get_fee()),
            network: convert_c_char(value.get_network()),
            method: convert_c_char(value.get_method()),
        }
    }
}

impl Free for DisplayCardanoOverview {
    fn free(&self) {
        free_str_ptr!(self.header_type);
        free_str_ptr!(self.deposit_amount);
        free_str_ptr!(self.deposit_reclaim_amount);
        free_str_ptr!(self.stake_amount);
        free_str_ptr!(self.reward_amount);
        free_str_ptr!(self.reward_account);
        free_str_ptr!(self.total_output_amount);
    }
}

impl Free for DisplayCardanoDetail {
    fn free(&self) {
        unsafe {
            free_str_ptr!(self.total_output_amount);
            free_str_ptr!(self.total_input_amount);
            free_str_ptr!(self.deposit_amount);
            free_str_ptr!(self.deposit_reclaim_amount);
            free_vec!(self.stake_content);
        }
    }
}

impl From<&CardanoOverview> for DisplayCardanoOverview {
    fn from(value: &CardanoOverview) -> Self {
        let (
            header_type,
            total_output_amount,
            stake_amount,
            deposit_amount,
            reward_amount,
            reward_account,
            deposit_reclaim_amount,
        ) = match value.get_header_card() {
            CardanoHeaderCard::Transfer(x) => {
                let header_type = convert_c_char("Transfer".to_string());
                let total_output_amount = convert_c_char(x.get_total_output_amount());
                (
                    header_type,
                    total_output_amount,
                    null_mut(),
                    null_mut(),
                    null_mut(),
                    null_mut(),
                    null_mut(),
                )
            }
            CardanoHeaderCard::Stake(x) => {
                let header_type = convert_c_char("Stake".to_string());
                let stake_amount = convert_c_char(x.get_stake_amount());
                let deposit = x.get_deposit().map(convert_c_char).unwrap_or(null_mut());
                (
                    header_type,
                    null_mut(),
                    stake_amount,
                    deposit,
                    null_mut(),
                    null_mut(),
                    null_mut(),
                )
            }
            CardanoHeaderCard::Withdrawal(x) => {
                let header_type = convert_c_char("Withdrawal".to_string());
                let reward_amount = convert_c_char(x.get_reward_amount());
                let reward_account = x
                    .get_reward_account()
                    .map(convert_c_char)
                    .unwrap_or(null_mut());
                let deposit_reclaim = x
                    .get_deposit_reclaim()
                    .map(convert_c_char)
                    .unwrap_or(null_mut());
                (
                    header_type,
                    null_mut(),
                    null_mut(),
                    null_mut(),
                    reward_amount,
                    reward_account,
                    deposit_reclaim,
                )
            }
        };
        Self {
            header_type,
            total_output_amount,
            stake_amount,
            deposit_amount,
            reward_amount,
            deposit_reclaim_amount,
            reward_account,
            has_deposit_amount: !deposit_amount.is_null(),
            has_deposit_reclaim_amount: !deposit_reclaim_amount.is_null(),
            has_reward_account: !reward_account.is_null(),
        }
    }
}

impl From<&CardanoDetail> for DisplayCardanoDetail {
    fn from(value: &CardanoDetail) -> Self {
        Self {
            total_input_amount: convert_c_char(value.get_total_input_amount()),
            total_output_amount: convert_c_char(value.get_total_output_amount()),
            has_deposit_amount: value.get_deposit().is_some(),
            deposit_amount: value
                .get_deposit()
                .map(convert_c_char)
                .unwrap_or(null_mut()),
            has_deposit_reclaim_amount: value.get_deposit_reclaim().is_some(),
            deposit_reclaim_amount: value
                .get_deposit_reclaim()
                .map(convert_c_char)
                .unwrap_or(null_mut()),
            has_stake_content: value.get_stake_content().is_some(),
            stake_content: value
                .get_stake_content()
                .map(|v| {
                    VecFFI::from(
                        v.iter()
                            .map(|action| match action {
                                CardanoDetailStakeAction::Withdrawal(withdrawal) => {
                                    DisplayCardanoStakeContent {
                                        stake_type: convert_c_char("Withdrawal".to_string()),
                                        stake_key: null_mut(),
                                        pool: null_mut(),
                                        registration_stake_key: null_mut(),
                                        reward_address: withdrawal
                                            .get_reward_address()
                                            .map(convert_c_char)
                                            .unwrap_or(null_mut()),
                                        reward_amount: withdrawal
                                            .get_reward_amount()
                                            .map(convert_c_char)
                                            .unwrap_or(null_mut()),
                                        deregistration_stake_key: withdrawal
                                            .get_deregistration_stake_key()
                                            .map(convert_c_char)
                                            .unwrap_or(null_mut()),
                                    }
                                }
                                CardanoDetailStakeAction::Stake(stake) => {
                                    DisplayCardanoStakeContent {
                                        stake_type: convert_c_char("Stake".to_string()),
                                        stake_key: convert_c_char(stake.get_stake_key()),
                                        pool: convert_c_char(stake.get_pool()),
                                        registration_stake_key: null_mut(),
                                        reward_address: null_mut(),
                                        reward_amount: null_mut(),
                                        deregistration_stake_key: null_mut(),
                                    }
                                }
                                CardanoDetailStakeAction::Registration(registration) => {
                                    DisplayCardanoStakeContent {
                                        stake_type: convert_c_char("Registration".to_string()),
                                        stake_key: null_mut(),
                                        pool: null_mut(),
                                        registration_stake_key: convert_c_char(
                                            registration.get_registration_stake_key(),
                                        ),
                                        reward_address: null_mut(),
                                        reward_amount: null_mut(),
                                        deregistration_stake_key: null_mut(),
                                    }
                                }
                            })
                            .collect_vec(),
                    )
                    .c_ptr()
                })
                .unwrap_or(null_mut()),
        }
    }
}

impl From<&CardanoFrom> for DisplayCardanoFrom {
    fn from(value: &CardanoFrom) -> Self {
        Self {
            address: convert_c_char(value.get_address()),
            amount: convert_c_char(value.get_amount()),
            has_path: value.get_path().is_some(),
            path: value.get_path().map(convert_c_char).unwrap_or(null_mut()),
        }
    }
}

impl Free for DisplayCardanoTo {
    fn free(&self) {
        free_str_ptr!(self.assets_text);
        free_str_ptr!(self.address);
        free_str_ptr!(self.amount);
    }
}

impl Free for DisplayCardanoFrom {
    fn free(&self) {
        free_str_ptr!(self.path);
        free_str_ptr!(self.amount);
        free_str_ptr!(self.address);
    }
}

impl Free for DisplayCardanoStakeContent {
    fn free(&self) {
        free_str_ptr!(self.stake_type);
        free_str_ptr!(self.stake_key);
        free_str_ptr!(self.reward_address);
        free_str_ptr!(self.reward_amount);
        free_str_ptr!(self.pool);
        free_str_ptr!(self.deregistration_stake_key);
        free_str_ptr!(self.registration_stake_key);
    }
}

impl From<&CardanoTo> for DisplayCardanoTo {
    fn from(value: &CardanoTo) -> Self {
        Self {
            address: convert_c_char(value.get_address()),
            amount: convert_c_char(value.get_amount()),
            has_assets: value.get_assets_text().is_some(),
            assets_text: value
                .get_assets_text()
                .map(convert_c_char)
                .unwrap_or(null_mut()),
        }
    }
}
