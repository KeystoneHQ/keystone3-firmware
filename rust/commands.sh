#!/bin/bash

CWD=$(pwd)

FMT_FILE=$CWD/.rustfmt.toml

DIRS=(
    "keystore"
    "rust_c"
    "rust_c/src/common"
    "rust_c/src/kt_allocator"
    "sim_qr_reader"
    "tools"
    "zcash_vendor")

TOOLCHAIN_IGNORE=(
    "rust_c/src/common"
    "rust_c/src/kt_allocator"
)

command=$1

format() {
    for dir in ${DIRS[@]}; do
        echo "copy rustfml.toml to ${dir}"
        cp $FMT_FILE $CWD/$dir
        echo "run cargo fmt"
        cd $CWD/$dir && cargo fmt
    done
}

test() {
    for dir in ${DIRS[@]}; do
        echo "run cargo test"
        cd $CWD/$dir && cargo test
    done
}

copy-toolchain() {
    for dir in ${DIRS[@]}; do
        found=false
        for dir2 in ${TOOLCHAIN_IGNORE[@]}; do
            if [ "$dir2" == "$dir" ]; then
                found=true
                break
            fi
        done
        if [ "$found" == true ]; then
            continue
        fi
        echo "run copy toolchain"
        cp $CWD/rust-toolchain $CWD/$dir
    done
}

case $command in
format)
    format
    ;;
test)
    test
    ;;
copy-toolchain)
    copy-toolchain
    ;;
*)
    echo "invalid command ${command}"
    ;;
esac
