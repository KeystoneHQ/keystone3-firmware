# Near Interfaces

## Methods

### `near_check` => [TransactionCheckResult]() \*

| parameters         | type     | comment                                 |
|:-------------------|:---------|:----------------------------------------|
| ptr                | void \*  | Pointer of a `NearSignRequest` instance |
| master_fingerprint | uint8 \* | 4 bytes bip32 master fingerprint        |
| length             | u32      | length of master_fingerprint            |

### `near_parse_tx` => [TransactionParseResult]()<[DisplayNearTx](#display_near_tx)> \*

| parameters | type    | comment                                 |
|:-----------|:--------|:----------------------------------------|
| ptr        | void \* | Pointer of a `NearSignRequest` instance |

### `near_sign_tx` => [UREncodeResult]() \*

| parameters | type     | comment                                 |
|:-----------|:---------|:----------------------------------------|
| ptr        | void \*  | Pointer of a `NearSignRequest` instance |
| seed       | uint8 \* | seed of current wallet                  |
| seed_len   | uint32   | length of seed                          |

## Structs

### <a id="display_near_tx">DisplayNearTx</a>

| field    | type                                                  | comment                                                       |
|:---------|:------------------------------------------------------|:--------------------------------------------------------------|
| overview | [DisplayNearTxOverview](#display_near_tx_overview) \* | the specific values on overview page                          |
| detail   | char \*                                               | the [json string](#detail_json_string_example) on detail page |
| network  | char \*                                               | network text, "Near Mainnet"                                  |

### <a id="display_near_tx_overview">DisplayNearTxOverview</a>

| field          | type                                                                              | comment                                                   |
|:---------------|:----------------------------------------------------------------------------------|:----------------------------------------------------------|
| display_type   | char \*                                                                           | the overview display type, `"Transfer"`, or `"General"`   |
| main_action    | char \*                                                                           | action text                                               |
| transfer_value | char \*                                                                           | available for `"Transfer"` display type                   |
| from  | char \*                                                                           | available for `"Transfer"` display type                   |
| to    | char \*                                                                           | available for `"Transfer"` display type                   |
| action_list    | [DisplayNearTxOverviewGeneralAction](#display_near_tx_overview_general_action) \* | available for `"General"` display type, a list of actions |

### <a id="display_near_tx_overview_general_action">DisplayNearTxOverviewGeneralAction</a>

| field  | type    | comment     |
|:-------|:--------|:------------|
| action | char \* | action name |

## <a id="detail_json_string_example">DetailJsonStringExample</a>

### Transfer Detail Example

```json
[
  {
    "From": "sweat_welcome.near",
    "To": "31824fbf2435fb1eca4dfc39774183cb25f13b10352d5d5327616b59c35eae9f"
  },
  {
    "Action": "Transfer",
    "Value": "123 Yocto"
  }
]
```

### General Detail Example

```json
[
  {
    "From": "test.near",
    "To": "whatever.near"
  },
  {
    "Action": "Create Account"
  },
  {
    "Action": "Deploy Contract"
  },
  {
    "Action": "Function Call",
    "Deposit Value": "1000000 Yocto",
    "Method": "qqq",
    "Prepaid Gas": "0.000000001 TGas"
  },
  {
    "Action": "Transfer",
    "Value": "123 Yocto"
  },
  {
    "Action": "Stake",
    "Public Key": "ed25519:4LGE55cdv7DRfErMovJY3Hm6jQrzRFyGfGpAW9vnb8Sz",
    "Stake Amount": "1000000 Yocto"
  },
  {
    "Access Key Allowance": "0",
    "Access Key Method Names": [
      "www"
    ],
    "Access Key Nonce": "0",
    "Access Key Receiver ID": "zzz",
    "Action": "Add Key",
    "Public Key": "ed25519:4LGE55cdv7DRfErMovJY3Hm6jQrzRFyGfGpAW9vnb8Sz"
  },
  {
    "Action": "Delete Key",
    "Public Key": "ed25519:4LGE55cdv7DRfErMovJY3Hm6jQrzRFyGfGpAW9vnb8Sz"
  },
  {
    "Action": "Delete Account",
    "Beneficiary ID": "123"
  }
]
```