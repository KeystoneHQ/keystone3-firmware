use alloc::string::String;
use alloc::vec::Vec;
use app_utils::impl_public_struct;

impl_public_struct!(CardanoDetail {
    total_input_amount: String,
    total_output_amount: String,
    deposit_reclaim: Option<String>,
    deposit: Option<String>,
    stake_content: Option<Vec<CardanoDetailStakeAction>>
});

#[derive(Clone, Debug)]
pub enum CardanoDetailStakeAction {
    // special scenario
    // user delegation to a pool, contain a Delegation and an optional matched Registration
    Stake(CardanoStake),
    // user withdraw from a pool, contain a Withdrawal and an optional matched Deregistration.
    // we treat a Deregistration as a kind of Withdrawal which reward_amount is 0
    Withdrawal(CardanoWithdrawal),
    //Plain action
    Registration(CardanoRegistration),
}

impl Default for CardanoDetailStakeAction {
    fn default() -> Self {
        Self::Stake(CardanoStake::default())
    }
}

impl_public_struct!(CardanoStake {
    stake_key: String,
    pool: String
});

impl_public_struct!(CardanoRegistration {
    registration_stake_key: String
});

impl_public_struct!(CardanoWithdrawal {
    reward_address: Option<String>,
    reward_amount: Option<String>,
    value: u64,
    deregistration_stake_key: Option<String>
});
