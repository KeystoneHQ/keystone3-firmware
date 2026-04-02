# IOTA Implementation

IOTA uses the same underlying Move VM as Sui, therefore this implementation
reuses Sui's core functionality for transaction parsing and signing.

The main differences are:
- IOTA-specific address generation (Blake2b-256)
- IOTA-specific network parameters
- Custom transaction display formatting for IOTA ecosystem