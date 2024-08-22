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

This protocol focuses on the Transparent and Orchard components.

#### CDDL for Zcash Accounts

The specification uses CDDL and includes `crypto-hdkey` and `crypto-key-path` specs defined in https://github.com/BlockchainCommons/Research/blob/master/papers/bcr-2020-007-hdkey.md.

```cddl
zcash-accounts = {
    seed-fingerprint: bytes.32, ; the seed fingerprint specified by ZIP-32 to identify the wallet
    accounts: [+ zcash-ufvk],
    ? origin: text, ; source of data, e.g., Keystone
}

zcash-ufvk = {
    ? transparent: crypto-hdkey,
    orchard: zcash-fvk,
    ? name: text,
}

zcash-fvk = {
    key-path: crypto-key-path,
    key-data: bytes,
}
```

`zcash-ufvk` describes the UFVK of a Zcash account. Each seed has multiple accounts with different indexes. For index 0, `zcash-ufvk` should contain a BIP32 extended public key with path `M/44'/133'/0'` (transparent) and an Orchard FVK with path `M_orchard/32'/133'/0'` (Orchard).

#### CDDL for Zcash PCZT

```cddl
zcash-pczt {
    data: bytes, ; Zcash PCZT, signatures inserted after signing.
}
```
