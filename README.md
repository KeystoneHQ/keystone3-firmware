# project-pillar-firmware

[![Build status](https://badge.buildkite.com/ee12be3737c11790cf3593a2573080b73fd8ae44cc549a69fe.svg)](https://buildkite.com/keystonehq/keystone-g3-project-pillar-firmware)

Artifacts has been uploaded to s3 bucket ```keystone-g3-firmware```, search in 1P ```AWS S3 Access``` to got permission for downloading.

# Developing Environment

## MacOS

### Setup

#### 1. Install GCC
- Auto install: `brew install armmbed/formulae/arm-none-eabi-gcc` may have unpredictable bugs
- Mannual install: https://developer.arm.com/downloads/-/gnu-rm, select `9-2020-q2-update`, download and config it to your $PATH

#### 2. Install Rust
- https://www.rust-lang.org/tools/install
- `rustup install nightly-2023-06-26`
- `rustup target add thumbv7em-none-eabihf`

## [TBD]WINDOWS


## Code Formating

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

### Build bin with docker

```shell
aws ecr get-login-password --region eu-central-1 | docker login --username AWS --password-stdin 623147552995.dkr.ecr.eu-central-1.amazonaws.com
.ci/build.sh
```

## Build base docker image

### Build on local

```shell
docker build -t project-pillar-base:local .
```

### How to use `buildkite` to build signed firmware
1. Open BuildKit：https://buildkite.com/keystonehq/keystone-g3-project-pillar-firmware， please find the username and password in 1P.
2. Click the `New Build` Green Button to create a new build.
3. if you would like to build the dev version, on the message box type `release-firmware-dev`. If you would like to build the production version, type `release-firmware-production`.
4. Choose the branch to build like `master` or `release/***`.
5. After the CI done, all the firmware will be listed on the S3 bucket `keystone-g3-firmware` the folder will be named as branch name and the subfolder will be the build number of buildkite. eg. `master/107/*`
6. You will find the s3 username/password on 1password.

### How to use `buildkite` to build unsigned firmware
1. Open BuildKit：https://buildkite.com/keystonehq/keystone-g3-project-pillar-firmware， please find the username and password in 1P
2. Click the `New Build` Green Button to create a new build
3. If you would like to build the dev version, on the message box type `debug-firmware-dev`. If you would like to build the production version, type `debug-firmware-production`
4. Choose the branch to build like `master` or `release/***`.
5. Download the firmware in `Buildkite` artifacts
