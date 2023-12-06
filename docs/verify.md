# Comprehensive Guide to Building and Verifying Keystone3 Firmware

This document offers a detailed walkthrough for building and verifying the Keystone3 firmware from source code. It focuses on ensuring the firmware's integrity and alignment with the specific release version, which is crucial for maintaining the reliability and security of the hardware wallet.

## Introduction

Building and verifying firmware are critical steps in software development, especially for hardware wallets like Keystone3. This guide targets developers and technicians who need to compile firmware from the source and verify its authenticity and integrity.

## Prerequisites

- Keystone3 firmware version 1.2.0 or higher.
- Basic knowledge of command-line operations.
- Docker installed on your machine.
- Internet access to clone the repository and pull Docker images.
- A robust Linux machine: The build process requires a Linux environment. Opt for a system with at least 16GB of memory for adequate compatibility and performance; a 32GB memory setup is highly recommended for optimal results.

## Detailed Steps for Building and Verifying the Firmware

### 1. Clone and Prepare the Repository

Begin by cloning the Keystone3 firmware repository to copy the entire firmware codebase onto your local machine.

**Commands:**

```bash
git clone https://github.com/KeystoneHQ/keystone3-firmware --recursive
git checkout tags/<release_tag_name>
```

**Explanation:**

- `git clone` copies the repository.
- `--recursive` ensures all submodules are also cloned.
- `git checkout tags/<release_tag_name>` switches to the specified release version.

### 2. Build the Docker Image

A consistent build environment is key. Docker helps by creating a container with all necessary dependencies and tools.

#### a. Build Locally

**Commands:**

```bash
docker build -t keystone3-baker:local .
```

**Explanation:**

- This builds a Docker image based on the provided Dockerfile.
- The tag `keystone3-baker:local` identifies your local build.

#### b. Use Pre-built Image

**Commands:**

```bash
docker pull keystonedockerhub/keystone3_baker:1.0.0
```

**Explanation:**

- This pulls a pre-built Docker image from Docker Hub.
- It saves time and ensures a standardized environment.

### 3. Execute the Build Process

Use the Docker image to compile the firmware.

**Commands:**

```bash
docker run -v $(pwd):/project-pillar-firmware keystone3_baker:1.0.0 python3 build.py -e production
```

**Explanation:**

- `docker run` creates a container from the Docker image.
- `-v $(pwd):/project-pillar-firmware` mounts your directory to the container.
- `python3 build.py -e production` initiates the build process.

**Note:** The build process compiles the source code into `mh1903.bin`, which may take some time.

### 4. Verify the Firmware Checksum

Verifying the firmware's integrity is crucial.

**Commands:**

```bash
sha256sum mh1903.bin
```

**Explanation:**

- `sha256sum` generates a hash for `mh1903.bin`.
- Compare this hash with the one on your Keystone3 device to confirm authenticity. For instructions on obtaining this value from the device, refer to [this tutorial](). To understand how the value is calculated, see the code [here]().

## Optional: Cross-Verification with Release Page

Enhance security by comparing `keystone3.bin` with the version on Keystone3's official release page.

**Why This Matters:** Cross-verification adds a layer of security, ensuring the built firmware matches the officially released version and mitigating security threats.