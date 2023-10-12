#!/bin/bash

CWD=`pwd`

FMT_FILE=$CWD/.rustfmt.toml

DIRS=("apps/bitcoin" "apps/cardano" "apps/solana" "apps/sui" "apps/wallets" "apps/ethereum" "apps/tron" "apps/near" "apps/xrp" "apps/cosmos" "apps/aptos" "apps/utils" "keystore" "rust_c" "third_party")


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

if [ $command == "format" ];
then
    format
elif [ $command == "test" ];
then 
    test
else
    echo "invalid command ${command}"
fi