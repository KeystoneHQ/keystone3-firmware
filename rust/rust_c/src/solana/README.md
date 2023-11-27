# Solana Interfaces

## Methods

### `solana_check` => [TransactionCheckResult]() \*

| parameters         | type     | comment                                   |
|:-------------------|:---------|:------------------------------------------|
| ptr                | void \*  | Pointer of a `SolanaSignRequest` instance |
| master_fingerprint | uint8 \* | 4 bytes bip32 master fingerprint          |
| length             | u32      | length of master_fingerprint              |

### `solana_parse_tx` => [TransactionParseResult]()<[DisplaySolanaTx](#display_solana_tx)> \*

| parameters | type    | comment                                   |
|:-----------|:--------|:------------------------------------------|
| ptr        | void \* | Pointer of a `SolanaSignRequest` instance |

### `solana_sign_tx` => [UREncodeResult]() \*

| parameters | type     | comment                                    |
|:-----------|:---------|:-------------------------------------------|
| ptr        | void \*  | Pointer of a `SolananSignRequest` instance |
| seed       | uint8 \* | seed of current wallet                     |
| seed_len   | uint32   | length of seed                             |

## Structs

### <a id="display_solana_tx">DisplaySolanaTx</a>

| field    | type                                                      | comment                                                       |
|:---------|:----------------------------------------------------------|:--------------------------------------------------------------|
| overview | [DisplaySolanaTxOverview](#display_solana_tx_overview) \* | the specific values on overview page                          |
| detail   | char \*                                                   | the [json string](#detail_json_string_example) on detail page |
| network  | char \*                                                   | network text, "Solana Mainnet"                                |

### <a id="display_solana_tx_overview">DisplaySolanaTxOverview</a>

| field          | type                                                                      | comment                                                                                                                         |
|:---------------|:--------------------------------------------------------------------------|:--------------------------------------------------------------------------------------------------------------------------------|
| display_type   | char \*                                                                   | the overview display type, `"Transfer"`, `"Vote"` `"General"` or `"Unknown"`                                                    |
| main_action    | char \*                                                                   | action text, `"SOL Transfer"` for `"Transfer"` display type, `"Vote"` for `"Vote"` display type, `Null` for other display types |
| transfer_value | char \*                                                                   | available for `"Transfer"` display type                                                                                         |
| transfer_from  | char \*                                                                   | available for `"Transfer"` display type                                                                                         |
| transfer_to    | char \*                                                                   | available for `"Transfer"` display type                                                                                         |
| votes_on       | [DisplaySolanaTxOverviewVotesOn](#display_solana_tx_overview_votes_on) \* | available for `"Vote"` display type, a list of slots                                                                            |
| vote_account   | char \*                                                                   | available for `"Vote"` display type                                                                                             |
| general        | [DisplaySolanaTxOverviewGeneral](#display_solana_tx_overview_general) \*  | available for `"General"` display type, a list of general overview for programs                                                 |

### <a id="display_solana_tx_overview_votes_on">DisplaySolanaTxOverviewVotesOn</a>

| field | type    | comment          |
|:------|:--------|:-----------------|
| slot  | char \* | slot to be voted |

### <a id="display_solana_tx_overview_general">DisplaySolanaTxOverviewGeneral</a>

| field   | type    | comment      |
|:--------|:--------|:-------------|
| program | char \* | program name |
| method  | char \* | method_name  |

## <a id="detail_json_string_example">DetailJsonStringExample</a>

### Transfer Detail Example

```json
[
  {
    "program": "System",
    "method": "Transfer",
    "value": "0.000005 SOL",
    "from": "A7dxsCbMy5ktZwQUgsQhVxsoJpx6wPAZYEcccQVjWnkE",
    "to": "2vCzt15qsXSCsf5k6t6QF9DiQSpE7kPTg3PdvFZtm2Tr"
  }
]
```

### Vote Detail Example

```json
[
  {
    "program": "Vote",
    "method": "Vote",
    "vote_account": "8KokPGKTfneh2YZtd7iFMqnAsdoErKRmLTFrtHmTGUsB",
    "sysvar_slot_hashes": "SysvarS1otHashes111111111111111111111111111",
    "sysvar_clock": "SysvarC1ock11111111111111111111111111111111",
    "vote_authority_pubkey": "2nayumaE7p4mU3skEjDJQHFr2NRJ6YTVMyqbcmApCbGo",
    "slots": [
      "197833586",
      "197833587"
    ],
    "hash": "4SUQLyPHVoyuPLLtJZazzywhYf3AehDEaB8wZAwtVQ1o",
    "timestamp": "1685932975"
  }
]
```

### General Detail Example

```json
[
  {
    "program": "System",
    "method": "CreateAccount",
    "funding_account": "STEPNq2UGeGSzCyGVr2nMQAzf8xuejwqebd84wcksCK",
    "new_account": "CdV4w55UDTvcza5d6V2Y6m7TF9Xmq9MHPUBYMe9WtptL",
    "amount": "1461600",
    "space": "82",
    "owner": "TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA"
  },
  {
    "program": "Token",
    "method": "InitializeMint",
    "mint": "CdV4w55UDTvcza5d6V2Y6m7TF9Xmq9MHPUBYMe9WtptL",
    "sysver_rent": "SysvarRent111111111111111111111111111111111",
    "mint_authority_pubkey": "STEPNq2UGeGSzCyGVr2nMQAzf8xuejwqebd84wcksCK",
    "freeze_authority_pubkey": "STEPNq2UGeGSzCyGVr2nMQAzf8xuejwqebd84wcksCK",
    "decimals": 0
  },
  {
    "program": "Unknown",
    "reason": "Unable to parse instruction, Program `metaqbxxUerdq28cj1RbAWkYQm3ybzjb6a8bt518x1s` is not supported yet"
  },
  {
    "program": "Unknown",
    "reason": "Unable to parse instruction, Program `ATokenGPvbdGVxr1b2hvZbsiqW5xWH25efTNsLJA8knL` is not supported yet"
  },
  {
    "program": "Token",
    "method": "MintTo",
    "mint": "CdV4w55UDTvcza5d6V2Y6m7TF9Xmq9MHPUBYMe9WtptL",
    "mint_to_account": "DG3Za1KX8Tj1TeZJy2U9nDa8qX3tyZCBYNehNy4fFsnQ",
    "mint_authority_pubkey": "STEPNq2UGeGSzCyGVr2nMQAzf8xuejwqebd84wcksCK",
    "amount": "1"
  },
  {
    "program": "Unknown",
    "reason": "Unable to parse instruction, Program `metaqbxxUerdq28cj1RbAWkYQm3ybzjb6a8bt518x1s` is not supported yet"
  }
]
```

### Unknown Detail Example

```json
{
  "accounts": [
    "2nkVYBJQg5UavBjvBT4jcahJamnFyr8wzPhzabDaRnBD",
    "7GgPYjS5Dza89wV6FpZ23kUJRG5vbQ1GM25ezspYFSoE",
    "8szGkuLTAux9XMgZ2vtY39jVSowEcpBfFfD8hXSEqdGC",
    "Du3Ysj1wKbxPKkuPPnvzQLQh8oMSVifs3jGZjJWXFmHN",
    "HgsT81nSFHBB1hzRP9KEsPE5EEVCPwyCsUao2FFuL72i",
    "mSoLzYCxHdYgdzU16g5QSh3i5K3z3KZK7ytfqcJm7So",
    "SLRjonR3SeJX74EqK1LgqK227D8kK2SEwXc3MKegyvh",
    "UefNb6z6yvArqe4cJHTXCqStRsKmWhGxnZzuHbikP5Q",
    "11111111111111111111111111111111",
    "3JLPCS1qM2zRw3Dp6V4hZnYHd4toMNPkNesXdX9tg6KM",
    "EyaSjUtSgo9aRD1f8LWXwdvkpDTmXAW54yoSHZRF14WL",
    "MarBmsSgKXdrN1egZf5sqe1TMai9K1rChYNDJgjq7aD",
    "mRefx8ypXNxE59NhoBqwqb3vTvjgf8MYECp4kgJWiDY",
    "TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA"
  ],
  "block_hash": "2qQSUu3YPsSYKTVN71jgGpmqZFwTzFjjE8WYsGvSm8hE",
  "header": {
    "num_readonly_signed_accounts": 0,
    "num_readonly_unsigned_accounts": 6,
    "num_required_signatures": 1
  },
  "instructions": [
    {
      "account_indexes": [
        2,
        5,
        7,
        1,
        10,
        3,
        0,
        4,
        9,
        8,
        13,
        11,
        6
      ],
      "accounts": "8szGkuLTAux9XMgZ2vtY39jVSowEcpBfFfD8hXSEqdGC,mSoLzYCxHdYgdzU16g5QSh3i5K3z3KZK7ytfqcJm7So,UefNb6z6yvArqe4cJHTXCqStRsKmWhGxnZzuHbikP5Q,7GgPYjS5Dza89wV6FpZ23kUJRG5vbQ1GM25ezspYFSoE,EyaSjUtSgo9aRD1f8LWXwdvkpDTmXAW54yoSHZRF14WL,Du3Ysj1wKbxPKkuPPnvzQLQh8oMSVifs3jGZjJWXFmHN,2nkVYBJQg5UavBjvBT4jcahJamnFyr8wzPhzabDaRnBD,HgsT81nSFHBB1hzRP9KEsPE5EEVCPwyCsUao2FFuL72i,3JLPCS1qM2zRw3Dp6V4hZnYHd4toMNPkNesXdX9tg6KM,11111111111111111111111111111111,TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA,MarBmsSgKXdrN1egZf5sqe1TMai9K1rChYNDJgjq7aD,SLRjonR3SeJX74EqK1LgqK227D8kK2SEwXc3MKegyvh",
      "data": "WuE7HjnsyebB3jriTd85Ef",
      "program": "Unknown",
      "program_account": "mRefx8ypXNxE59NhoBqwqb3vTvjgf8MYECp4kgJWiDY",
      "program_index": 12,
      "reason": "Unable to parse instruction, Program `mRefx8ypXNxE59NhoBqwqb3vTvjgf8MYECp4kgJWiDY` is not supported yet"
    }
  ]
}
```