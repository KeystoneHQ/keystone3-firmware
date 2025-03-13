## Zcash App
This crate is the Zcash application layer implementation.

Provides address generation, PCZT checking/parsing/signing functionalities.

### PCZT

PCZT stands for `Partially Created Zcash Transaction`, similar to Bitcoin's `Partially Signed Bitcoin Transaction`. It's called "Created" rather than "Signed" due to the zero-knowledge proof component in Zcash transactions.

Source: https://github.com/zcash/librustzcash/tree/main/pczt

PCZT is not well documented at the moment. In simple terms, it involves several roles:
`creator`, `signer`, `prover`, `combinator`, and `tx_extractor`. Multiple roles can be fulfilled by the same entity, such as a software wallet.

The PCZT lifecycle consists of:

1. Creation: Typically occurs in a software wallet (like Zashi). During this step, the software wallet determines transaction parameters such as `inputs` and `outputs` to compose the PCZT, then exports it to a signer.
2. Signing: The signer (which could be a hardware wallet or the software wallet itself) receives the PCZT, verifies and signs each spend, and writes the signatures back into the PCZT. The signed PCZT is then returned to the combinator. A single PCZT may require multiple signers.
   For Keystone, the signing process is divided into three phases:
   1. Checking: We perform basic validation of transaction data. If a transaction is deemed potentially harmful at this stage, we abort the signing process and alert the user.
      - For Transparent bundles: We verify that input addresses match the `zip32derivation` declared in the PCZT
      - For Sapling Bundles: We do not support checking for Sapling components
      - For Orchard Bundles: We verify the spend's `nullifier` and `rk`, as well as the output's `note_commitment`
   2. Parsing: We analyze the transaction data as thoroughly as possible and display the parsed transaction on the Keystone screen for user verification.
      - For Sapling Bundles: Although we don't support signing Sapling Spends, Sapling outputs are allowed in transactions, and we attempt to parse them as thoroughly as possible to ensure transaction amount information doesn't cause confusion
   3. Signing: When the user chooses to sign, we sign all spends in the PCZT that Keystone is capable of signing.
3. Proving: The prover calculates proofs for each input in the PCZT, writes these proofs into the PCZT, and returns the proof-containing PCZT to the combinator. For Sapling Spends, proof generation requires signer participation, but Keystone lacks the necessary computational capacity, so it doesn't support Sapling spends.
4. Combination: The combinator collects PCZTs from signers and provers, merges the signatures and proofs into a single PCZT, and passes the result to the tx_extractor.
5. Tx Extraction: The on-chain Zcash transaction format is extracted from the PCZT and passed to the software wallet for broadcasting.

#### Current Workflow:
1. Keystone generates the Zcash account's UFVK ([code](/rust/keystore/src/algorithms/zcash/mod.rs)), then creates a Zcash-UFVK QR code to be scanned by Zashi Wallet ([code](/rust/rust_c/src/wallet/cypherpunk_wallet/zcash.rs)).
2. Zashi Wallet creates a PCZT and uses the keystone-sdk to generate a Zcash-PCZT QR code containing the PCZT data.
3. Keystone scans the QR code displayed by Zashi Wallet and processes it:
   - Parses the QR code to obtain a ParseResult (https://github.com/KeystoneHQ/keystone3-firmware/blob/master/rust/rust_c/src/common/ur.rs#L714)
   - The frontend retrieves the Zcash-PCZT object from ParseResult and validates it (https://github.com/KeystoneHQ/keystone3-firmware/blob/master/src/ui/gui_chain/multi/cypherpunk/gui_zcash.c#L308)
   - The frontend parses the Zcash-PCZT object (https://github.com/KeystoneHQ/keystone3-firmware/blob/master/src/ui/gui_chain/multi/cypherpunk/gui_zcash.c#L41)
   - After user authorization, the Zcash-PCZT object is signed (https://github.com/KeystoneHQ/keystone3-firmware/blob/master/src/ui/gui_chain/multi/cypherpunk/gui_zcash.c#L321)
4. Zashi Wallet scans the signed Zcash-PCZT from Keystone, extracts the transaction, and broadcasts it to the network.

### QR Protocols

From Keystone's perspective, we need a QR protocol to encode both the UFVK (Unified Full Viewing Key) of a Zcash account and the PCZT.

Protocol definition: [docs](/docs/protocols/ur_registrys/zcash.md)

Protocol implementation: https://github.com/KeystoneHQ/keystone-sdk-rust/tree/master/libs/ur-registry/src/zcash

