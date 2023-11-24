#!/bin/bash

CWD=`pwd`

FMT_FILE=$CWD/.rustfmt.toml

DIRS=("apps/bitcoin" "apps/cardano" "apps/solana" "apps/sui" "apps/wallets" "apps/ethereum" "apps/tron" "apps/near" "apps/xrp" "apps/cosmos" "apps/aptos" "apps/utils" "keystore" "rust_c" "rust_c/src/aptos" "rust_c/src/bitcoin" "rust_c/src/cardano" "rust_c/src/common" "rust_c/src/cosmos" "rust_c/src/ethereum" "rust_c/src/kt_allocator" "rust_c/src/near" "rust_c/src/solana" "rust_c/src/sui" "rust_c/src/test_cmd" "rust_c/src/test_cmd/src/btc_test_cmd" "rust_c/src/test_cmd/src/general_test_cmd" "rust_c/src/tron" "rust_c/src/wallet" "rust_c/src/wallet/src/btc_only_wallet" "rust_c/src/wallet/src/multi_coins_wallet" "rust_c/src/xrp" "third_party")

command=$1

format () {
    for dir in ${DIRS[@]}; do
        echo "copy rustfml.toml to ${dir}"
        cp $FMT_FILE $CWD/$dir
        echo "run cargo fmt"
        cd $CWD/$dir && cargo fmt
    done
}

test () {
    for dir in ${DIRS[@]}; do
        echo "run cargo test"
        cd $CWD/$dir && cargo test
    done
}

case $command in
    format)
        format
        ;;
    test)
        test
        ;;
    *)
        echo "invalid command ${command}"
        ;;
esac
