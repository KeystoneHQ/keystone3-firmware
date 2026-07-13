## Keystone Zcash UR Registries

This protocol is based on the [Uniform Resources](https://github.com/BlockchainCommons/Research/blob/master/papers/bcr-2020-005-ur.md). It describes the data schemas (UR Registries) used in Zcash integrations.

### Introduction

Keystone's QR workflow involves two main steps: linking the wallet and signing data, broken down into three sub-steps:

1. **Wallet Linking:** Keystone generates a QR code with public key info for the Watch-Only wallet to scan and import.
2. **Transaction Creation:** The Watch-Only wallet creates a transaction and generates a QR code for Keystone to scan, parse, and display.
3. **Signing Authorization:** Keystone signs the transaction, displays the result as a QR code for the Watch-Only wallet to scan and broadcast.

The following UR registries support these steps and use the Partially Created Zcash Transaction structure.

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

`zcash-sign-batch` wraps multiple PCZTs into one Keystone approval.
The outer UR registry envelope carries a request id for response correlation and
an opaque `data` field containing the PCZT-owned batch request. The matching
compact response uses `zcash-batch-sig-result`, echoes the request id, and
carries the PCZT-owned response in its own opaque `data` field. It also reports
the signing firmware version once for the entire response.

Batch version 1 is supported by cypherpunk firmware and currently accepts up to
50 PCZTs. The encoded batch data and request id together, and the canonical PCZT
payloads after decoding, must each fit within 512 KiB. The operation is atomic.
If any PCZT is invalid or cannot be signed, Keystone returns an error instead of
a partial result. PCZT entries with identical canonical encodings are rejected.
Every spend must be fully Keystone-owned and use a supported shielded pool,
currently Orchard or Ironwood. Transparent inputs and Sapling spends or outputs
are rejected.

#### Outer UR/CBOR envelopes

Both registry types use definite-length CBOR maps. Firmware requires
`request-id` to be non-empty. Key `1` follows `zcash-pczt` by carrying opaque
transaction data, and key `2` follows `zcash-sign-result` by carrying the
request id. The result's required key `3` is the three-byte firmware version
`[major, minor, build]` that produced the signatures.

```cddl
zcash-sign-batch = {
    1: bytes, ; BatchSignRequest::serialize output
    2: bytes, ; request-id
}

zcash-batch-sig-result = {
    1: bytes, ; BatchSignResponse::serialize output
    2: bytes, ; echoed request-id
    3: bytes, ; firmware version [major, minor, build]
}
```

#### PCZT batch request

The request `data` encoding is
`"PCZB" || batch_version_le || pczt_version_le || postcard_body`. Its Postcard
body contains the PCZTs in request order. Both version fields are four-byte
little-endian integers. Current encoders emit batch version 1 and PCZT version
2. The shared PCZT version applies to every headerless PCZT wire value in the
body.

```rust
struct BatchSignRequestBody {
    pczts: Vec<PcztV2Wire>,
}
```

The exact PCZT wire value is owned by the pinned
[`pczt::roles::signer::batch`](https://github.com/zcash/librustzcash/blob/878db2074ae8ac2682d3e6c61c00f7018b6adc0c/pczt/src/roles/signer/batch.rs)
implementation. There are no request or message ids in the inner payload. PCZT
entries are correlated by position and must be unique within the request.

#### PCZT batch signature response

The response `data` encoding is `"PCZS" || batch_version_le || postcard_body`.
Entry `i` contains the signatures produced for PCZT `i` in the request.

```rust
struct BatchSignResponseBody {
    signatures: Vec<Vec<SpendAuthSignature>>,
}

struct SpendAuthSignature {
    value_pool: ValuePool,
    action_index: u32,
    signature: [u8; 64],
}

enum ValuePool {
    Orchard,
    Ironwood,
}
```

Postcard encodes integer fields inside each body as varints; `ValuePool` uses
the enum indexes Orchard = 0 and Ironwood = 1. Response entry `i` contains the
signatures for request PCZT `i`. Each signature selects an action by value pool
and action index, so the client can apply it to the corresponding unsigned PCZT
without transporting another full PCZT. The outer response echoes the request
id so the application can correlate it with the outstanding batch.
