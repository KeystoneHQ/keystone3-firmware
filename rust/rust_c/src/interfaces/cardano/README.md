# Cardano Interfaces

## Methods

### `cardano_check_tx` => [TransactionCheckResult]() \*

| parameters         | type     | comment                                           |
|:-------------------|:---------|:--------------------------------------------------|
| ptr                | void \*  | Pointer of a `CardanoSignRequest` instance        |
| master_fingerprint | uint8 \* | 4 bytes bip32 master fingerprint                  |
| cardano_xpub       | char \*  | stored 64 bytes bip32-ed25519 extended public key |

### `cardano_parse_tx` => [TransactionParseResult]()<[DisplayCardanoTx](#display_cardano_tx)> \*

| parameters         | type     | comment                                           |
|:-------------------|:---------|:--------------------------------------------------|
| ptr                | void \*  | Pointer of a `CardanoSignRequest` instance        |
| master_fingerprint | uint8 \* | 4 bytes bip32 master fingerprint                  |
| cardano_xpub       | char \*  | stored 64 bytes bip32-ed25519 extended public key |

### `cardano_sign_tx` => [UREncodeResult]() \*

| parameters         | type     | comment                                           |
|:-------------------|:---------|:--------------------------------------------------|
| ptr                | void \*  | Pointer of a `CardanoSignRequest` instance        |
| master_fingerprint | uint8 \* | 4 bytes bip32 master fingerprint                  |
| cardano_xpub       | char \*  | stored 64 bytes bip32-ed25519 extended public key |
| entropy            | uint8 \* | entropy of current wallet bip39 entropy           |
| entropy_len        | uint32   | length of bip39 entropy                           |

## Structs

### <a id="display_cardano_tx">DisplayCardanoTx</a>

| fileld   | type                                                    | comment                                                                                     |
|:---------|:--------------------------------------------------------|:--------------------------------------------------------------------------------------------|
| overview | [DisplayCardanoOverview](#display_cardano_overview) \*  | the especial values on overview page                                                        |
| detail   | [DisplayCardanoDetail](#display_cardano_detail) \*      | the especial values on detail page                                                          |
| from     | VecFFI\<[DisplayCardanoFrom](#display_cardano_from)> \* | the list of from address (not input) in this transaction                                    |
| to       | VecFFI\<[CardanoTo](#display_cardano_to)> \*            | the list of to address (not output) in this transaction                                     |
| fee      | char \*                                                 | formatted fee text, "0.001 ADA"                                                             |
| network  | char \*                                                 | network text, "Cardano Mainnet" or "Cardano Testnet"                                        |
| method   | char \*                                                 | the main purpose of this transaction mostly looks like, "Stake", "Withdrawal" or "Transfer" |

### <a id="display_cardano_overview">DisplayCardanoOverview</a>

| fileld                     | type    | comment                                                             |
|:---------------------------|:--------|:--------------------------------------------------------------------|
| header_type                | char \* | the overview header type, `"Stake"`, `"Withdrawal"` or `"Transfer"` |
| total_output_amount        | char \* | the transaction's outputs' total value text, "1000.0000 ADA"        |
| stake_amount               | char \* | display stake amount of a `Stake` transaction                       |
| has_deposit_amount         | bool    |                                                                     |
| deposit_amount             | char \* | display deposit amount of a `Stake` transaction                     |
| reward_amount              | char \* | the total reward amount in one `Withdrawal` transaction             |
| has_deposit_reclaim_amount | bool    |                                                                     |
| deposit_reclaim_amount     | char \* | deposit reclaim amount of a `Withdrawal` transaction                |
| reward_account             | char \* | the reward address of a `Withdrawal` transaction                    |
| has_reward_account         | bool    |

### <a id="display_cardano_detail">DisplayCardanoDetail</a>

| fileld                     | type                                                                  | comment                                                      |
|:---------------------------|:----------------------------------------------------------------------|:-------------------------------------------------------------|
| total_input_amount         | char \*                                                               | the transaction's inputs' total value text                   |
| total_output_amount        | char \*                                                               | the transaction's outputs' total value text, "1000.0000 ADA" |
| has_deposit_amount         | bool                                                                  |                                                              |
| deposit_amount             | char \*                                                               | display deposit amount of a `Stake` transaction              |
| has_deposit_reclaim_amount | bool                                                                  |                                                              |
| deposit_reclaim_amount     | char \*                                                               | deposit reclaim amount of a `Withdrawal` transaction         |
| has_stake_content          | bool                                                                  |                                                              |
| stake_content              | VecFFI\<[DisplayCardanoStakeContent](#display_cardano_stake_content)> | the list of stake action card                                |

### <a id="display_cardano_stake_content">DisplayCardanoStakeContent</a>

| fileld                   | type    | comment                                                              |
|:-------------------------|:--------|:---------------------------------------------------------------------|
| stake_type               | char \* | The stake action type, `"Stake"`, `"Withdrawal"` or `"Registration"` |
| stake_key                | char \* | only available when `stake_type` is `Stake`                          |
| pool                     | char \* | only available when `stake_type` is `Stake`                          |
| registration_stake_key   | char \* | only available when `stake_type` is `Registration`                   |
| reward_address           | char \* | only available when `stake_type` is `Withdrawal`                     |
| reward_amount            | char \* | only available when `stake_type` is `Withdrawal`                     |
| deregistration_stake_key | char \* | only available when `stake_type` is `Withdrawal`                     |

### <a id="display_cardano_from">DisplayCardanoFrom</a>

| fileld   | type    | comment                                                  |
|:---------|:--------|:---------------------------------------------------------|
| address  | char \* | base address                                             |
| amount   | char \* | total input amount of an address                         |
| has_path | bool    |                                                          |
| path     | char \* | available when having matched utxo in CardanoSignRequest |

### <a id="display_cardano_to">DisplayCardanoTo</a>

| fileld      | type    | comment                           |
|:------------|:--------|:----------------------------------|
| address     | char \* | base address                      |
| amount      | char \* | total output amount of an address |
| has_assets  | bool    |                                   |
| assets_text | char \* | e.g. "9 more assets"              |
