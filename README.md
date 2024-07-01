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
rustup install nightly-2023-06-26
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
cd tools && astyle -A3nrUpHcQ --exclude=../src/cm_backtrace/Languages "../src/*.c" "../src/*.h" && cd ..
```

## FAQ

Q. How to build and verify the firmware?

A. Please check the detail guide on `docs/verify.md`

## License

Please see the LICENSE.md file for details.

## Contact

For support or inquiries, please contact us at eng@keyst.one
