# Keystone3 Firmware

## Description

The Keystone3 Firmware is an advanced, highly secure software specifically crafted for the Keystone3 product, a state-of-the-art crypto hardware wallet. This project is laser-focused on delivering an exceptionally secure and intuitive user experience. It boasts cutting-edge features like PCI level anti-tamper protection, ensuring the highest security against physical and digital threats. Additionally, it supports Multi Seed Phrase functionality, which enhances security and recovery options, and includes safeguards against blind signing to protect against unauthorized transactions. The firmware also offers extensive support for a wide range of cryptocurrencies, catering to the diverse needs of crypto users.

Its standout features include:

1. Triple-layer security with Three Secure Element Chips, ensuring top-notch protection of your digital assets.
2. Advanced PCI level anti-tamper features, providing robust defense against physical tampering and hacking attempts.
3. A user-friendly interface offering an intuitive user experience, making it accessible even for those new to crypto hardware wallets.

## Getting Started

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
rustup install nightly-2024-07-01
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

#### Building multi-coins firmware

```bash
# Run the build script at the root of the project.
python3 build.py
```

#### Building btc-only firmware

```bash
# Run the build script at the root of the project.
python build.py -t btc_only
```

#### Building img to C file

Please move the "img" file to the corresponding directory under the "images" directory.

The program will convert underscores in the file names to camel case, and add the appropriate prefix to the files.

Here are two ways to run it.

The first way is to execute the command below.

```bash
# Run the build script at the root of the project.
python img_converter.py
```

The second way is already integrated in the "build.py" file, so you can simply use it.

```bash
python3 build.py

# or
python build.py -t btc_only
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
-   Create your feature branch: `git checkout -b feature/AmazingFeature`.
-   Commit your changes: `git commit -m 'Add some AmazingFeature'`.
-   Push to the branch: `git push origin feature/AmazingFeature`.
-   Submit a pull request.

Before submitting, ensure your code follows our formatting standards:

```bash
brew install astyle
cd tools && astyle -A3nrUpHcQ --exclude=../src/cm_backtrace/Languages --exclude=../src/ui/gui_assets "../src/*.c" "../src/*.h" && cd ..
```

## FAQ

Q. How to build and verify the firmware?

A. Please check the detail guide on `docs/verify.md`

## License

Please see the LICENSE.md file for details.

## Contact

For support or inquiries, please contact us at eng@keyst.one
