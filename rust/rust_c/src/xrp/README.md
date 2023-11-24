# Xrp Interfaces

## Methods

### `xrp_parse_tx` => [TransactionParseResult]()<[DisplayXrpTx](#display_xrp_tx)> \*

| parameters | type    | comment                                |
|:-----------|:--------|:---------------------------------------|
| ptr        | void \* | Pointer of a `XrpSignRequest` instance |

### `xrp_sign_tx` => [UREncodeResult]() \*

| parameters | type    | comment                                                                             |
|:-----------|:--------|:------------------------------------------------------------------------------------|
| ptr        | void \* | Pointer of a `SolananSignRequest` instance                                          |
| hd_paths   | void \* | Pointer of a `CSliceFFI<XRPHDPath>` instance, the [list of HD paths](#xrp_hd_path)) |
| seed       | uint8   | the account seed of current wallet                                                  |

## Structs

### <a id="display_xrp_tx">DisplayXrpTx</a>

| field    | type                                                | comment                                                       |
|:---------|:----------------------------------------------------|:--------------------------------------------------------------|
| overview | [DisplayXrpTxOverview](#display_xrp_tx_overview) \* | the specific values on overview page                          |
| detail   | char \*                                             | the [json string](#detail_json_string_example) on detail page |
| network  | char \*                                             | network text, "XRP Mainnet"                                   |

### <a id="display_xrp_tx_overview">DisplayXrpTxOverview</a>

| field            | type    | comment                                                                                                |
|:-----------------|:--------|:-------------------------------------------------------------------------------------------------------|
| display_type     | char \* | the overview display type, `"Payment"` or `"General"`                                                  |
| transaction_type | char \* | transaction type                                                                                       |
| fee              | char \* | transaction fee                                                                                        |
| sequence         | char \* | available for `"General"` transaction type, the sequence number of the account sending the transaction |
| value            | char \* | available for `"Payment"` transaction type                                                             |
| from             | char \* | available for `"Payment"` transaction type                                                             |
| to               | char \* | available for `"Payment"` transaction type                                                             |

### <a id="xrp_hd_path">XRPHDPath</a>

| field | type    | comment                                                                                                    |
|:------|:--------|:-----------------------------------------------------------------------------------------------------------|
| path  | char \* | hd path the wallet have synced, used to determine which private key should be used to sign the transaction |


## <a id="detail_json_string_example">DetailJsonStringExample</a>

### General Detail Example

```json
{
  "Account": "rUn84CUYbNjRoTQ6mSW7BVJPSVJNLb1QLo",
  "CheckID": "49647F0D748DC3FE26BDACBC57F251AADEFFF391403EC9BF87C97F67E9977FB0",
  "Fee": "12",
  "Sequence": 5,
  "TransactionType": "CheckCancel"
}
```

### Payment Detail Example

```json
{
  "Account": "rGUmkyLbvqGF3hwX4qwGHdrzLdY2Qpskum",
  "Amount": "10000000",
  "Destination": "rDxQoYzcQrpzVHuT4Wx6bacJYXyGTEtbvm",
  "Fee": "12",
  "Flags": 2147483648,
  "LastLedgerSequence": 80882602,
  "Sequence": 79991865,
  "SigningPubKey": "03F5C5BB1D19EC710D3D7FAD199AF10CF8BC1D11348E5B3765C0B0B9C0BEC32879",
  "TransactionType": "Payment"
}
```