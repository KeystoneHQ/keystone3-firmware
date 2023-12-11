# Cosmos Interfaces

## Methods

### `cosmos_check_tx` => [TransactionCheckResult]() \*

| parameters         | type     | comment                                                       |
|:-------------------|:---------|:--------------------------------------------------------------|
| ptr                | void \*  | Pointer of a `CosmosSignRequest` or `EvmSignRequest` instance |
| ur_type            | URType   | `CosmosSignRequest` or `EvmSignRequest`                       |
| master_fingerprint | uint8 \* | 4 bytes bip32 master fingerprint                              |
| length             | u32      | length of master_fingerprint                                  |

### `cosmos_parse_tx` => [TransactionParseResult]()<[DisplayCosmosTx](#display_cosmos_tx)> \*

| parameters | type    | comment                                                       |
|:-----------|:--------|:--------------------------------------------------------------|
| ptr        | void \* | Pointer of a `CosmosSignRequest` or `EvmSignRequest` instance |
| ur_type    | URType  | `CosmosSignRequest` or `EvmSignRequest`                       |

### `cosmos_sign_tx` => [UREncodeResult]() \*

| parameters | type     | comment                                                       |
|:-----------|:---------|:--------------------------------------------------------------|
| ptr        | void \*  | Pointer of a `CosmosSignRequest` or `EvmSignRequest` instance |
| ur_type    | URType   | `CosmosSignRequest` or `EvmSignRequest`                       |
| seed       | uint8 \* | seed of current wallet                                        |
| seed_len   | uint32   | length of seed                                                |

## Structs

### <a id="display_cosmos_tx">DisplayCosmosTx</a>

| field    | type                                                 | comment                                                                                                  |
|:---------|:-----------------------------------------------------|:---------------------------------------------------------------------------------------------------------|
| overview | [DisplayCosmosOverview](#display_cosmos_overview) \* | the especial values on overview page                                                                     |
| detail   | char \*                                              | the especial values on detail page, this is a json string, contains two fields, `"common"` and `"kind"`. |

### <a id="display_cosmos_overview">DisplayCosmosTxOverview</a>

| field                     | type    | comment                                                                                                                                             |
|:--------------------------|:--------|:----------------------------------------------------------------------------------------------------------------------------------------------------|
| display_type              | char \* | the overview display type, `"Send"`, `"Delegate"` or `"Undelegate"`, `Redelegate`, `Withdraw Reward`, `Vote`, `IBC Transfer`, `Unknown`,`Multiple`. |
| method                    | char \* | transaction method, available for all display type except `Unknown` and `Multiple`                                                                  |
| network                   | char \* | human readable chain_id, available for all display type, default value is `Cosmos Hub`                                                              |
| send_value                | char \* | available for `"Send"` display type                                                                                                                 |
| send_from                 | char \* | available for `"Send"` display type                                                                                                                 |
| send_to                   | char \* | available for `"Send"` display type                                                                                                                 |
| delegate_value            | char \* | available for `"Delegate"` display type                                                                                                             |
| delegate_from             | char \* | available for `"Delegate"` display type                                                                                                             |
| delegate_to               | char \* | available for `"Delegate"` display type                                                                                                             |
| redelegate_value          | char \* | available for `"Redelegate"` display type                                                                                                           |
| redelegate_to             | char \* | available for `"Redelegate"` display type                                                                                                           |
| redelegate_new_validator  | char \* | available for `"Redelegate"` display type                                                                                                           |
| withdraw_reward_to        | char \* | available for `"Withdraw Reward"` display type                                                                                                      |
| withdraw_reward_validator | char \* | available for `"Withdraw Reward"` display type                                                                                                      |
| transfer_value            | char \* | available for `"IBC Transfer"` display type                                                                                                         |
| transfer_from             | char \* | available for `"IBC Transfer"` display type                                                                                                         |
| transfer_to               | char \* | available for `"IBC Transfer"` display type                                                                                                         |
| vote_voted                | char \* | available for `"Vote"` display type                                                                                                                 |
| vote_proposal             | char \* | available for `"Vote"` display type                                                                                                                 |
| vote_voter                | char \* | available for `"Vote"` display type                                                                                                                 |
| overview_list             | char \* | available for `"Multiple"` display type                                                                                                             |

### Send Detail Example

```json
{
  "common": {
    "Network": "Cosmos Hub",
    "Chain ID": "cosmoshub-4",
    "Max Fee": "266.826483 ATOM",
    "Fee": "0.002583 ATOM",
    "Gas Limit": "103301"
  },
  "kind": [
    {
      "Method": "Send",
      "Value": "0.012 ATOM",
      "From": "cosmos17u02f80vkafne9la4wypdx3kxxxxwm6f2qtcj2",
      "To": "cosmos1kwml7yt4em4en7guy6het2q3308u73dff983s3"
    }
  ]
}
```

### Delegate Detail Example

```json
 {
  "common": {
    "Network": "Cosmos Hub",
    "Chain ID": "osmo-test-5",
    "Max Fee": "855583375 uosmo",
    "Fee": "4625 uosmo",
    "Gas Limit": "184991"
  },
  "kind": [
    {
      "Method": "Delegate",
      "Value": "2000000 uosmo",
      "From": "osmo17u02f80vkafne9la4wypdx3kxxxxwm6fzmcgyc",
      "To": "osmovaloper1hh0g5xf23e5zekg45cmerc97hs4n2004dy2t26"
    }
  ]
}
```

### Redelegate Detail Example

```json
{
  "common": {
    "Network": "Cosmos Hub",
    "Chain ID": "osmo-test-5",
    "Max Fee": "2666027676 uosmo",
    "Fee": "8164 uosmo",
    "Gas Limit": "326559"
  },
  "kind": [
    {
      "Method": "Re-delegate",
      "Value": "2000000 uosmo",
      "To": "osmo17u02f80vkafne9la4wypdx3kxxxxwm6fzmcgyc",
      "New Validator": "osmovaloper1vaq0tneq0vmnkk48jxrqlaaefdx8kl2tx06eg9",
      "Old Validator": "osmovaloper1hh0g5xf23e5zekg45cmerc97hs4n2004dy2t26"
    }
  ]
}
```

### Undelegate Detail Example

```json
{
  "common": {
    "Network": "Cosmos Hub",
    "Chain ID": "osmo-test-5",
    "Max Fee": "2261839456 uosmo",
    "Fee": "9512 uosmo",
    "Gas Limit": "237788"
  },
  "kind": [
    {
      "Method": "Undelegate",
      "Value": "2000000 uosmo",
      "To": "osmo17u02f80vkafne9la4wypdx3kxxxxwm6fzmcgyc",
      "Validator": "osmovaloper1hh0g5xf23e5zekg45cmerc97hs4n2004dy2t26"
    }
  ]
}
```

### Vote Detail Example

```json
{
  "common": {
    "Network": "Osmosis",
    "Chain ID": "osmosis-1",
    "Max Fee": "151426044 uosmo",
    "Fee": "1946 uosmo",
    "Gas Limit": "77814"
  },
  "kind": [
    {
      "Method": "Vote",
      "Voter": "osmo17u02f80vkafne9la4wypdx3kxxxxwm6fzmcgyc",
      "Proposal": "#565",
      "Voted": "NO_WITH_VETO"
    }
  ]
}
```

### Withdraw Reward Example

```json
{
  "common": {
    "Network": "Cosmos Hub",
    "Chain ID": "osmo-test-5",
    "Max Fee": "669291691 uosmo",
    "Fee": "4091 uosmo",
    "Gas Limit": "163601"
  },
  "kind": [
    {
      "Method": "Withdraw Reward",
      "To": "osmo17u02f80vkafne9la4wypdx3kxxxxwm6fzmcgyc",
      "Validator": "osmovaloper1hh0g5xf23e5zekg45cmerc97hs4n2004dy2t26"
    }
  ]
}
```

### IBC Transfer Detail Example

```json
{
  "common": {
    "Network": "Osmosis",
    "Chain ID": "osmosis-1",
    "Max Fee": "1137555565 uosmo",
    "Fee": "5333 uosmo",
    "Gas Limit": "213305"
  },
  "kind": [
    {
      "Method": "IBC Transfer",
      "Value": "300000 uosmo",
      "From": "osmo17u02f80vkafne9la4wypdx3kxxxxwm6fzmcgyc",
      "To": "cosmos10dyr9899g6t0pelew4nvf4j5c3jcgv0r73qga5",
      "Source Channel": "channel-0"
    }
  ]
}
```

### Unknown Detail Example
```json
{
  "Network": "Cosmos Hub",
  "Chain ID": "pulsar-2",
  "Message": "Unknown Data"
}
```

### Multiple Detail Example
```json
{
  "common": {
    "Network": "Osmosis",
    "Chain ID": "osmosis-1",
    "Max Fee": "24690000 uosmo",
    "Fee": "2000 uosmo",
    "Gas Limit": "12345"
  },
  "kind": [
    {
      "Method": "Withdraw Reward",
      "To": "mantle1ceeqym2q878ek9337a5m7dp724cap00npx7e0h",
      "Validator": "mantlevaloper1yvrw5z5ec0n9c253hpy5lkq9cufmk8dvjqnfz8"
    },
    {
      "Method": "Withdraw Reward",
      "To": "mantle1ceeqym2q878ek9337a5m7dp724cap00npx7e0h",
      "Validator": "mantlevaloper1gp957czryfgyvxwn3tfnyy2f0t9g2p4pmkjlwt"
    }
  ]
}
```