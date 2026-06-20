## Keystone Zcash UR Registries

This protocol is based on the [Uniform Resources](https://github.com/BlockchainCommons/Research/blob/master/papers/bcr-2020-005-ur.md). It describes the data schemas (UR Registries) used in Zcash integrations.

### Introduction

Keystone's QR workflow involves two main steps: linking the wallet and signing data, broken down into three sub-steps:

1. **Wallet Linking:** Keystone generates a QR code with public key info for the Watch-Only wallet to scan and import.
2. **Transaction Creation:** The Watch-Only wallet creates a transaction and generates a QR code for Keystone to scan, parse, and display.
3. **Signing Authorization:** Keystone signs the transaction, displays the result as a QR code for the Watch-Only wallet to scan and broadcast.

Two UR Registries are needed for these steps, utilizing the Partially Created Zcash Transaction structure.

### Zcash Accounts

#### Unified Full Viewing Key (UFVK)

UFVK is a standard account expression format in Zcash as per [ZIP-316](https://zips.z.cash/zip-0316). It consists of:

1. Transparent
2. Sprout
3. Sapling
4. Orchard

This protocol focuses on the Transparent, Orchard, and Ironwood components.

#### CDDL for Zcash Accounts

The specification uses CDDL;

```cddl
zcash-accounts = {
    seed-fingerprint: bytes.32, ; the seed fingerprint specified by ZIP-32 to identify the wallet
    accounts: [+ zcash-ufvk],
}

zcash-ufvk = {
    ufvk: text, ; the standard UFVK expression, it may includes transparent, orchard and sapling FVK or not;
    index: uint32, ; the account index
    ? name: text,
}

```

`zcash-ufvk` describes the UFVK of a Zcash account. Each seed has multiple accounts with different indexes. For index 0, `zcash-ufvk` should contain a BIP32 extended public key with path `M/44'/133'/0'` (transparent) and an Orchard FVK with path `M_orchard/32'/133'/0'` (Orchard).

#### CDDL for Zcash PCZT

```cddl
zcash-pczt {
    data: bytes, ; Zcash PCZT, signatures inserted after signing.
}
```

### Zcash Batch Signing

`zcash-sign-batch` wraps multiple signing messages into one Keystone approval.
Version 1 is supported by cypherpunk firmware and currently supports up to 35
mainnet PCZT messages. It requires `atomic` to be `true`; if any message is
invalid or cannot be signed, Keystone returns an error instead of a partial
result. Batch PCZT entries must be fully Keystone-owned spends from supported
shielded pools, currently Orchard or Ironwood. Transparent inputs and Sapling
spends or outputs are rejected.

The 35-message limit is the current batch memory budget for `pczt-v1`. A full
35-message batch using the supported PCZT message shape was measured at about
35% RAM on target hardware, so this version does not define separate byte caps
for request ids, message ids, or payloads. Revisit the limit if new message
kinds or substantially larger payload encodings are added.

Message kinds:

```cddl
pczt-v1 = 1
```

Networks:

```cddl
zcash-mainnet = 1
```

Result statuses:

```cddl
signed = 0
```

#### CDDL for Zcash Sign Batch

```cddl
zcash-sign-batch = {
    1: uint,                 ; version. Must be 1.
    2: bytes,                ; request id. Echoed by zcash-sign-result.
    3: uint,                 ; network. Must be zcash-mainnet.
    4: [1*35 zcash-sign-message],
   ?11: bool,                ; atomic. Defaults to true. Must be true.
}

zcash-sign-message = {
    1: bytes,                ; caller-defined message id. Must be unique.
    2: uint,                 ; message kind. Must be pczt-v1.
    3: bytes,                ; message payload. For pczt-v1 this is raw PCZT bytes. Must be unique in the batch.
   ?6: bytes.32,             ; SHA-256 of payload.
}
```

#### CDDL for Zcash Sign Result

```cddl
zcash-sign-result = {
    1: uint,                 ; version. Matches request version.
    2: bytes,                ; request id from zcash-sign-batch.
    3: [1*35 zcash-sign-message-result],
}

zcash-sign-message-result = {
    1: bytes,                ; message id from zcash-sign-message.
    2: uint,                 ; status. signed = 0.
    3: uint,                 ; message kind from zcash-sign-message.
    4: bytes,                ; signed payload. For pczt-v1 this is signed PCZT bytes.
    6: bytes.32,             ; SHA-256 of signed payload.
}
```
