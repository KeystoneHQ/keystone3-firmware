## Build

BUILD_ENVIRONMENT: PRODUCTION, DEBUG, SIMULATOR
BUILD_VARIANT: BTC_ONLY, MULTI_COINS

### Build for production

```bash
cargo build --release -p rust_c --target thumbv7em-none-eabihf --no-default-features --features production-multi-coins
```

```bash
cargo build --release -p rust_c --target thumbv7em-none-eabihf --no-default-features --features production-multi-coins
```

### Build for simulator

```bash
cargo build --release -p rust_c --no-default-features --features simulator-multi-coins
```

```bash
cargo build --release -p rust_c --no-default-features --features simulator-multi-coins
```

### Build for debug

```bash
cargo build -p rust_c --target thumbv7em-none-eabihf --no-default-features --features debug-multi-coins
```

```bash
cargo build -p rust_c --target thumbv7em-none-eabihf --no-default-features --features debug-multi-coins
```
