
# Developing Environment

## Setup

### MacOS

#### 1. Install GCC
- Auto install: `brew install armmbed/formulae/arm-none-eabi-gcc` may have unpredictable bugs
- Manual install: https://developer.arm.com/downloads/-/gnu-rm, select `9-2020-q2-update`, download and config it to your $PATH

#### 2. Install Rust
- https://www.rust-lang.org/tools/install
- `rustup install nightly-2023-06-26`
- `rustup target add thumbv7em-none-eabihf`

### [TBD]WINDOWS


## Code Formatting

### Download AStyle
`brew install astyle`

### Command

```cd tools && astyle -A3nrUpHcQ --exclude=../src/cm_backtrace/Languages --exclude=../external --exclude=../rust "../*.c" "../*.h" "../*.cpp" && cd ..```

## Build Bin Artifacts

### Build bin on local

```shell
rustup target add thumbv7em-none-eabihf
git submodule update --init --recursive
cargo install bindgen-cli
cargo install cbindgen
python3 build.py
```

## Build base docker image

### Build on local

```shell
docker build -t project-pillar-base:local .
```
