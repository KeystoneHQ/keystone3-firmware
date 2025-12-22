# Keystone DevKit Hello World

## Overview

This is a Hello World example program for the Keystone DevKit, developed based on the MH1903 chip. This project demonstrates how to run the most basic firmware program on the Keystone hardware wallet development kit.

This project is streamlined from the complete Keystone3 firmware project, specifically designed for learning and demonstration purposes on the Keystone DevKit.

## Features

- System clock initialization
- LCD screen initialization
- Display "Hello World" text in the center of the screen using LVGL
- FreeRTOS-based task scheduling

## Getting Started

### Build Requirements

- ARM GCC Toolchain (arm-none-eabi-gcc)
- CMake 3.10 or higher

### Build Instructions

#### Windows

```bash
mkdir build
cd build
cmake .. -G "Unix Makefiles"
make
```

#### Linux / Mac

```bash
mkdir build
cd build
cmake ..
make
```


### Project Structure

```
.
├── src/
│   ├── main.c                      # Main entry file
│   ├── tasks/
│   │   ├── helloworld_task.c       # Hello World display task
│   │   └── helloworld_task.h
│   ├── driver/                     # Hardware drivers
│   └── config/                     # Configuration files
├── external/
│   ├── FreeRTOS/                   # FreeRTOS source code
│   ├── lvgl/                       # LVGL graphics library
│   └── mh1903_lib/                 # MH1903 chip library
├── CMakeLists.txt                  # CMake configuration
├── firmware.cmake                  # Firmware build configuration
└── mh1903b.ld                      # Linker script

### Output Files

After successful compilation, the following files will be generated in the `build` directory:
- `mh1903.elf` - ELF format firmware
- `mh1903.bin` - Binary firmware (for flashing)
- `mh1903.hex` - HEX format firmware

### Flashing

Use a supported flashing tool to flash `mh1903.bin` to your device.

---

## Original Full Firmware Project Information

Below is the build instructions for the original Keystone3 complete firmware project (now simplified to Hello World example):

### Installation

#### MacOS

Follow these steps to set up your development environment on MacOS:

```bash
# Install GCC
brew install armmbed/formulae/arm-none-eabi-gcc
# If you encounter issues with Brew when installing GCC, switch to manual installation:
# Visit https://developer.arm.com/downloads/-/gnu-rm, and select the `9-2020-q2-update`

# Install Rust
# For instructions, visit https://www.rust-lang.org/tools/install
rustup install nightly-2025-05-01
rustup target add thumbv7em-none-eabihf
cargo install bindgen-cli
cargo install cbindgen

# Clone the repository
git clone https://github.com/KeystoneHQ/keystone3-firmware
cd keystone3-firmware
git -c submodule.keystone3-firmware-release.update=none submodule update --init --recursive
```

#### Docker

Alternatively, use Docker to build the required environment:

```bash
docker build -t keystone3-baker:local .
```

### Building the Firmware

Here's how to build the Keystone3 Firmware:

#### Building firmware

```bash
# Run the build script at the root of the project.
python3 build.py
```

## Code Structure

The Keystone3 firmware is built with Rust and C and uses FreeRTOS as the underlying OS. This firmware runs on the MH1903 MCU and incorporates three secure elements (Microchip DS28C50, Maxim MAX32520, and Microchip ATECC608A). In this section, we will introduce the code structure and the main components of the firmware.

- docs: This directory contains the documentation for the Keystone3 Firmware.
- external: This directory houses the essential external dependencies for the Keystone3 Firmware, including libraries from MH1903, Microchip, and various open-source libraries such as cjson, ctaes, and others. It's important to note that the MH1903 library is incorporated as a pre-compiled library rather than source code due to intellectual property restrictions. At present, only the [QRCode library](https://github.com/KeystoneHQ/keystone3-firmware/blob/master/external/mh1903_lib/MHSCPU_Driver/lib/MH1903_QRDecodeLib.a) is utilized, taking advantage of the hardware optimization capabilities specific to this MCU for enhanced performance.
- hardware: This directory contains hardware-related files and schematics for the Keystone3 device.
- images: This directory contains the image assets used in the firmware's user interface.
- lv_img_converter: This directory contains the tool script for converting images to a format compatible with the LVGL graphics library.
- rust: This directory contains the Rust code for blockchain support. Most of the blockchain-related functionality is implemented in Rust, including transaction signing, address generation, and cryptographic operations.
- src: This directory contains the main C source code for the firmware, including the FreeRTOS implementation, device drivers, and the core application logic.
- test: This directory contains test scripts and commands for local development and testing.
- tools: This directory contains various tools used in the development process, including the astyle tool for code formatting, tools related to check the integrity of the firmware.
- ui_simulator: This directory contains the UI simulator for the firmware. For more details about the UI simulator, please check the [Simulator Documentation](docs/SIMULATOR.md).

## Simulator

Please follow this [Doc](docs/SIMULATOR.md).

## Contributing

We welcome contributions! Here's how you can contribute:

-   Fork the repository.
-   Create your feature branch: `git checkout -b feature/xxx`.
-   Commit your changes: `git commit -m 'Add some xxx'`.
-   Push to the branch: `git push origin feature/xxx`.
-   Submit a pull request.

Before submitting, ensure your code follows our formatting standards:


## FAQ

Q. How to build and verify the firmware?

A. Please check the detail guide on `docs/verify.md`

## License

Please see the LICENSE.md file for details.

## Contact

For support or inquiries, please contact us at eng@keyst.one
