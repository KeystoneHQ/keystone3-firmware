use alloc::string::String;

use app_utils::impl_public_struct;

impl_public_struct!(CardanoOverview {
    header_card: CardanoHeaderCard
});

#[derive(Debug, Clone)]
pub enum CardanoHeaderCard {
    Transfer(CardanoOverviewTransferCard),
    Stake(CardanoOverviewStakeCard),
    Withdrawal(CardanoOverviewWithdrawalCard),
}

impl_public_struct!(CardanoOverviewTransferCard {
    total_output_amount: String
});

impl Default for CardanoHeaderCard {
    fn default() -> Self {
        CardanoHeaderCard::Transfer(CardanoOverviewTransferCard::default())
    }
}

impl_public_struct!(CardanoOverviewStakeCard {
    stake_amount: String,
    deposit: Option<String>
});

impl_public_struct!(CardanoOverviewWithdrawalCard {
    reward_amount: String,
    deposit_reclaim: Option<String>,
    reward_account: Option<String>
});
