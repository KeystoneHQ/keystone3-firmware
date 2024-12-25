#!/bin/bash

set -e
set -o pipefail

BUILD_FOLDER="$(pwd)/build"
BUILD_SIMULATOR_FOLDER="$(pwd)/build_simulator"
TOOLS_FOLDER="$(pwd)/tools"
MAKE_OAT_FILE_PATH="${TOOLS_FOLDER}/ota_file_maker"
MAKE_PADDING_FILE_PATH="${TOOLS_FOLDER}/padding_bin_file"
ASTYLE_PATH="${TOOLS_FOLDER}/AStyle.sh"
PACK_PATH="$(pwd)/pack.sh"
LANGUAGE_PATH="$(pwd)/src/ui/lv_i18n"
LANGUAGE_SCRIPT="python3 data_loader.py"
RUST_C_PATH="$(pwd)/rust/rust_c"

declare -A build_options=(
    ["log"]=false
    ["copy"]=false
    ["production"]=false
    ["screen"]=false
    ["debug"]=false
    ["format"]=false
    ["release"]=false
    ["rebuild"]=false
    ["btc_only"]=false
    ["cyberpunk"]=false
    ["simulator"]=false
    ["language"]=false
    ["clean"]=false
)

for arg in "$@"; do
    if [[ "$arg" == "format" ]]; then
        pushd "$TOOLS_FOLDER"
        echo "Formatting files..."
        bash "$ASTYLE_PATH"
        popd
    else
        echo "Building with option: $arg"
        build_options["$arg"]=true
    fi
done

echo "Building with options: ${build_options[@]}"

if [[ "${build_options[rebuild]}" == true ]]; then
    if [[ -d "$BUILD_FOLDER" ]]; then
        rm -rf "$BUILD_FOLDER"
    fi
    pushd "$RUST_C_PATH"
    cargo clean
    popd
fi

mkdir -p "$BUILD_FOLDER"

if [[ ! -f "$BUILD_FOLDER/padding_bin_file.py" ]]; then
    cp "$MAKE_PADDING_FILE_PATH/padding_bin_file.py" "$BUILD_FOLDER/padding_bin_file.py"
fi

execute_build() {
    if [[ "${build_options[language]}" == true ]]; then
        pushd "$LANGUAGE_PATH"
        $LANGUAGE_SCRIPT
        popd
    fi

    cmake_parm=""
    if [[ "${build_options[production]}" == true ]]; then
        cmake_parm="${cmake_parm} -DBUILD_PRODUCTION=true"
    fi
    if [[ "${build_options[btc_only]}" == true ]]; then
        cmake_parm="${cmake_parm} -DBTC_ONLY=true"
    fi
    if [[ "${build_options[cyberpunk]}" == true ]]; then
        cmake_parm="${cmake_parm} -DCYBERPUNK=true"
    fi
    if [[ "${build_options[screen]}" == true ]]; then
        cmake_parm="${cmake_parm} -DENABLE_SCREEN_SHOT=true"
    fi
    if [[ "${build_options[debug]}" == true ]]; then
        cmake_parm="${cmake_parm} -DDEBUG_MEMORY=true"
    fi

    echo "Building project.............."
    echo "cmake_parm: $cmake_parm"
    if [[ "${build_options[simulator]}" == true ]]; then
        echo "Building project---------------"
        mkdir -p "$BUILD_SIMULATOR_FOLDER"
        pushd "$BUILD_SIMULATOR_FOLDER"
        cmake -G "Unix Makefiles" -DBUILD_TYPE=Simulator $cmake_parm ..
        make -j16
        popd
    else
        echo "Building project.............."
        mkdir -p "$BUILD_SIMULATOR_FOLDER"
        pushd "$BUILD_SIMULATOR_FOLDER"
        cmake -G "Unix Makefiles" -DBUILD_TYPE=Simulator $cmake_parm ..
        make -j16
        popd
        # pushd "$BUILD_FOLDER"
        # cmake -G "Unix Makefiles" $cmake_parm ..
        # if [[ "${build_options[log]}" == true ]]; then
        #     make -j16 > makefile.log 2>&1
        # else
        #     make -j16
        # fi
        # python3 padding_bin_file.py mh1903.bin
        # popd
    fi

    if [[ "${build_options[copy]}" == true ]]; then
        echo "Generating pillar.bin file..."
        pushd "$MAKE_OAT_FILE_PATH"
        echo "Generating OTA files..."
        bash make_ota_file.sh "$(pwd)/build/pillar.bin"
        bash make_ota_file.sh "$(pwd)/build/keystone3.bin"
        bash make_ota_file.sh "F:/pillar.bin"
        popd
    elif [[ "${build_options[release]}" == true ]]; then
        pushd "$MAKE_OAT_FILE_PATH"
        echo "Generating release files..."
        bash make_ota_file.sh "$(pwd)/build/pillar.bin"
        bash make_ota_file.sh "$(pwd)/build/keystone3.bin"
        popd
    elif [[ "${build_options[simulator]}" == true ]]; then
        ./build/simulator.exe
    fi
}

execute_build
