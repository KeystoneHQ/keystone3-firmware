# Sui Interfaces

## Methods

### `sui_check_request` => [TransactionCheckResult]() \*

| parameters         | type     | comment                                |
| :----------------- | :------- | :------------------------------------- |
| ptr                | void \*  | Pointer of a `SuiSignRequest` instance |
| master_fingerprint | uint8 \* | 4 bytes bip32 master fingerprint       |
| length             | u32      | length of master_fingerprint           |

### `sui_parse_intent` => [TransactionParseResult]()<[DisplaySuiIntentMessage](#display_sui_intent)> \*

| parameters | type    | comment                                |
| :--------- | :------ | :------------------------------------- |
| ptr        | void \* | Pointer of a `SuiSignRequest` instance |

### `sui_sign_intent` => [UREncodeResult]() \*

| parameters | type     | comment                                    |
| :--------- | :------- | :----------------------------------------- |
| ptr        | void \*  | Pointer of a `SolananSignRequest` instance |
| seed       | uint8 \* | seed of current wallet                     |
| seed_len   | uint32   | length of seed                             |

## Structs

### <a id="display_sui_intent">DisplaySuiIntentMessage</a>

| field  | type    | comment                                                       |
| :----- | :------ | :------------------------------------------------------------ |
| detail | char \* | the [json string](#detail_json_string_example) on detail page |

## <a id="detail_json_string_example">DetailJsonStringExample</a>

### Transfer Objects Example

```json
{
  "intent": {
    "app_id": "Sui",
    "scope": "TransactionData",
    "version": "V0"
  },
  "value": {
    "V1": {
      "expiration": "None",
      "gas_data": {
        "budget": 2000,
        "owner": "0x0e4d9313fb5b3f166bb6f2aea587edbe21fb1c094472ccd002f34b9d0633c719",
        "payment": [
          [
            "0x280f4809b93ed87cc06f3397cd42a800a1034316e80d05443bce08e810817a96",
            3317,
            "6WFidAcGkUzUEPvSChbMXg9AZUHj3gzJL4U7HkxJA43H"
          ]
        ],
        "price": 1000
      },
      "kind": {
        "ProgrammableTransaction": {
          "commands": [
            {
              "TransferObjects": [
                [
                  {
                    "Input": 1
                  }
                ],
                {
                  "Input": 0
                }
              ]
            }
          ],
          "inputs": [
            {
              "Pure": [
                134, 172, 97, 121, 202, 106, 217, 167, 177, 204, 180, 114, 2,
                208, 106, 224, 154, 19, 30, 102, 48, 153, 68, 146, 42, 249, 199,
                61, 60, 32, 59, 102
              ]
            },
            {
              "Object": {
                "ImmOrOwnedObject": [
                  "0xd833a8eabc697a0b2e23740aca7be9b0b9e1560a39d2f390cf2534e94429f91c",
                  3309,
                  "2gnMwEZqfMY1Q2Ree5iW3cAt7rhauevfBDY74SH3Ef1D"
                ]
              }
            }
          ]
        }
      },
      "sender": "0x0e4d9313fb5b3f166bb6f2aea587edbe21fb1c094472ccd002f34b9d0633c719"
    }
  }
}
```

### Move Call Example

```json
{
  "intent": {
    "app_id": "Sui",
    "scope": "TransactionData",
    "version": "V0"
  },
  "value": {
    "V1": {
      "expiration": "None",
      "gas_data": {
        "budget": 1000,
        "owner": "0x0e4d9313fb5b3f166bb6f2aea587edbe21fb1c094472ccd002f34b9d0633c719",
        "payment": [
          [
            "0x280f4809b93ed87cc06f3397cd42a800a1034316e80d05443bce08e810817a96",
            3317,
            "6WFidAcGkUzUEPvSChbMXg9AZUHj3gzJL4U7HkxJA43H"
          ]
        ],
        "price": 1000
      },
      "kind": {
        "ProgrammableTransaction": {
          "commands": [
            {
              "MoveCall": {
                "arguments": [
                  {
                    "Input": 0
                  },
                  {
                    "Input": 1
                  }
                ],
                "function": "split_vec",
                "module": "pay",
                "package": "0x0000000000000000000000000000000000000000000000000000000000000002",
                "type_arguments": [
                  {
                    "struct": {
                      "address": "0000000000000000000000000000000000000000000000000000000000000002",
                      "module": "sui",
                      "name": "SUI",
                      "type_args": []
                    }
                  }
                ]
              }
            }
          ],
          "inputs": [
            {
              "Object": {
                "ImmOrOwnedObject": [
                  "0xd833a8eabc697a0b2e23740aca7be9b0b9e1560a39d2f390cf2534e94429f91c",
                  3309,
                  "2gnMwEZqfMY1Q2Ree5iW3cAt7rhauevfBDY74SH3Ef1D"
                ]
              }
            },
            {
              "Pure": [1, 64, 66, 15, 0, 0, 0, 0, 0]
            }
          ]
        }
      },
      "sender": "0x0e4d9313fb5b3f166bb6f2aea587edbe21fb1c094472ccd002f34b9d0633c719"
    }
  }
}
```

### Splite Coins Example

```json
{
  "intent": {
    "app_id": "Sui",
    "scope": "TransactionData",
    "version": "V0"
  },
  "value": {
    "V1": {
      "expiration": "None",
      "gas_data": {
        "budget": 100,
        "owner": "0xebe623e33b7307f1350f8934beb3fb16baef0fc1b3f1b92868eec39440938869",
        "payment": [
          [
            "0xa2e3e42930675d9571a467eb5d4b22553c93ccb84e9097972e02c490b4e7a22a",
            12983,
            "2aS93HVFS54TNKfAFunntFgoRMbMCzp1bDfqSTRPRYpg"
          ]
        ],
        "price": 1000
      },
      "kind": {
        "ProgrammableTransaction": {
          "commands": [
            {
              "SplitCoins": [
                "GasCoin",
                [
                  {
                    "Input": 1
                  }
                ]
              ]
            },
            {
              "TransferObjects": [
                [
                  {
                    "Result": 0
                  }
                ],
                {
                  "Input": 0
                }
              ]
            }
          ],
          "inputs": [
            {
              "Pure": [
                31, 249, 21, 165, 233, 227, 47, 219, 224, 19, 85, 53, 182, 198,
                154, 0, 169, 128, 154, 175, 127, 124, 2, 117, 211, 35, 156, 167,
                157, 178, 13, 100
              ]
            },
            {
              "Pure": [16, 39, 0, 0, 0, 0, 0, 0]
            }
          ]
        }
      },
      "sender": "0xebe623e33b7307f1350f8934beb3fb16baef0fc1b3f1b92868eec39440938869"
    }
  }
}
```

### Personal Message Example

```json
{
  "intent": {
    "app_id": "Sui",
    "scope": "PersonalMessage",
    "version": "V0"
  },
  "value": {
    "message": "Hello World!"
  }
}
```
