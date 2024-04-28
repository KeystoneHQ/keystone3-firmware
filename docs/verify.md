# Comprehensive Guide to Building and Verifying Keystone3 Firmware

This guide provides an in-depth walkthrough for building and verifying the Keystone3 firmware from its source code. It emphasizes the crucial role of maintaining firmware integrity and alignment with specific release versions, essential for the reliability and security of the Keystone3 hardware wallet.

## Introduction

Building and verifying firmware are key steps in software development, especially for specialized devices like the Keystone3 hardware wallet. This guide is designed for developers and technicians who are tasked with compiling firmware from source and verifying its authenticity and integrity.

## Prerequisites

- Keystone3 firmware version 1.2.0 or later.
- Basic proficiency in command-line operations.
- Docker installed on your machine.
- Internet access for cloning the repository and pulling Docker images.
- A robust Linux machine with at least 16GB of memory; 32GB is recommended for optimal performance.

## Detailed Steps for Building and Verifying the Firmware

### 1. Clone and Prepare the Repository

Start by cloning the Keystone3 firmware repository to your local machine.

**Commands:**

```bash
git clone https://github.com/KeystoneHQ/keystone3-firmware
cd keystone3-firmware
git -c submodule.keystone3-firmware-release.update=none submodule update --init --recursive
git checkout tags/<release_tag_name>
```

**Highlights:**

- The `git clone` command retrieves the repository.
- The `--recursive` flag includes all submodules.
- `git checkout tags/<release_tag_name>` switches to a specific firmware version.

### 2. Build the Docker Image

A consistent build environment is essential. Docker facilitates this by creating a container with all necessary dependencies and tools.

#### a. Building Locally

**Commands:**

```bash
docker build -t keystonedockerhub/keystone3_baker:1.0.1 .
```

#### b. Using a Pre-built Image

**Commands:**

```bash
docker pull keystonedockerhub/keystone3_baker:1.0.1
```

### 3. Execute the Build Process

Compile the firmware using Docker.

**Commands:**

```bash
docker run -v $(pwd):/keystone3-firmware keystonedockerhub/keystone3_baker:1.0.1 python3 build.py -e production
```

**Note:** This step compiles the source into the `mh1903.bin` file.

### 4. Verify the Firmware Checksum

An integrity check is essential.

**Commands:**

```bash
sha256sum mh1903.bin
```

**Further Steps:** Compare the generated hash with the display on your device. For detailed instructions, refer [here](https://guide.keyst.one/docs/verify-checksum). To understand the checksum calculation on the device, see the code [here](https://github.com/KeystoneHQ/keystone3-firmware/blob/ce9e8e7e9bc33b46d420f9cfea4329b73426a7cd/src/ui/gui_model/gui_model.c#L1261).

## Optional: In-Depth Verification with Release Page
Before delving into this section, it's important to understand some context. The firmware file available on our release page, named `keystone3.bin`, is derived from the `mh1903.bin` file. This transformation involves compressing the original file and adding our official signature for enhanced security and authenticity.

**Verification Steps:**

1. **Use the Firmware Maker:**
   - Access the `firmware-maker` tool under `<ProjectRoot>/tools/code/firmware-maker`.
   - Build it using `cargo build`.
   - Run `./fmm --source mh1903.bin --destination keystone3-unsigned.bin` to generate `keystone3-unsigned.bin`.

**Note**: The checksum of `keystone3-unsigned.bin` will differ from the official release due to the absence of our signature.

2. **Checksum Verification:**
   - Find the `firmware-checker` tool under `<ProjectRoot>/tools/code/firmware-checker`.
   - Build it with `cargo build`.
   - Run `./fmc --source keystone3-unsigned.bin` to obtain the firmware checksum.
   - This checksum should match the `mh1903.bin` checksum from step 4 and the value on your device.

**Significance:** These verification steps conclusively confirm that `keystone3.bin` is derived from `mh1903.bin`, ensuring the code on your Keystone3 device is authentic and reliable.
