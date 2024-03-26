## 1.3.2 (2024-03-22)

### What's new

1. Added support for Dymension para-chain under the Cosmos ecosystem

### Improvements

1. Optimized display of certain upgrade notification pages
2. Improved default address display for BTC receiving

### Bug Fixes

1. Fixed issue with device rebooting in certain scenarios


## 1.3.0 (2024-02-29)

### What's new

1. Support for Taproot through Sparrow

### Improvements

1. Added a success notification page for firmware upgrades

### Bug Fixes

1. Fixed an issue where some transaction types for LTC and DASH couldn't be parsed
2. Fixed an issue where certain transaction types for ADA would cause the device to restart after parsing


## 1.2.8 (2024-01-19)

### What's new

1. Introduced the display of BTC's XPUB in the form of a QR code

### Improvements

1. Improved the presentation of Celestia chain under the Cosmos ecosystem and other parallel chains
2. Enhanced the wording in some device messages
3. Optimized the display of icons in certain status bars


## 1.2.6 (2024-01-10)

### What's new

1. Added a feature to generate seed phrases through dice rolling
2. Introduced support for Tron, Bitcoin Cash, Dash, and Litecoin

### Improvements

1. Improved the display of Cosmos and its para-chains on the coin management page

### Bug Fixes

1. Fixed the issue of device restarts caused by insufficient memory


## 1.2.4 (2023-12-21)

### What's new

1. Introduced the Checksum verification feature for firmware update via microSD cards

### Improvements

1. Optimized the feature for open-source code verification of firmware
2. Improved the wording within the device for better clarity
3. Optimized transaction decoding for ETH, Polygon, and BSC networks
4. Enhanced the user experience of fingerprint unlocking for Passphrase wallets

### Bug Fixes

1. Fixed transaction failures in specific scenarios among Cardano and COSMOS networks
2. Resolved fingerprint signature failures in specific scenarios


## 1.2.2 (2023-12-18)

### Improvements

1. Optimized battery charging logic
2. Refined text throughout the app


## 1.2.0 (2023-12-08)

### What's new

1. Added checksum for firmware

### Improvements

1. Improved responsiveness of device to click events after unlocking the screen with an inserted microSD card
2. Enhanced compatibility with microSD cards from major manufacturers (<= 512GB)
3. Optimized certain text messages

### Bug Fixes

1. Fixed an issue with incorrect Value display in the parsing of individual ERC-20 transactions
2. Resolved an issue where signing List NFT transactions on OpenSea would lead to device reboot
3. Addressed the problem of firmware upgrade failure when using a 512GB microSD card


## 1.1.4 (2023-11-24)

### What’s new

1. Added support for ADA through integration with Eternl wallet
2. Added support for connecting and signing transactions with Rabby Wallet via USB
3. Added support for managing ETH and EVM assets using imToken wallet
4. Added support for managing BTC through Sparrow wallet
5. Added support for PlatON transaction decoding on the EVM network

### Improvements

1. Enhanced readability of transaction decoding information for part of ERC20 tokens
2. Improved transaction decoding speed for XRP signatures
3. Optimized logic for incorrect device unlock password prompts
4. Various optimizations at the firmware system level and improvements in text display

### Bug Fixes

1. Fixed the issue where pressing the Enter key twice was required for the passphrase popup in the keyboard
2. Resolved the occasional device restart problem when reading ABI data from an SD card during transaction decoding


## 1.1.2 (2023-11-09)

### Features

1. Added support for XRP

### Improvement

1. Refined text throughout the app


## 1.1.0 (2023-10-31)

### Features

1. Added support for Solana blockchain
2. Added support for SUI blockchain
3. Added support for Aptos blockchain
4. Updated currency icons display
5. Improved the loading page
6. Added BTC address example display

### Improvement

1. Refined text throughout the app

### Bug Fixes

1. Fixed an issue triggered when inserting an SD card in specific scenarios
2. Improved battery curve and battery level display


## 1.0.4 (2023-10-08)

### Features

1. Implemented WebUSB upgrade support


## 1.0.0 (2023-09-26)

### Features

1. Added support for the Cosmos blockchain
2. Introduced Passphrase functionality
3. Implemented hardware self-check during startup
4. Added the ability to zoom in on QR codes
5. Enhanced QR code scanning to improve scanning speed and reliability in certain scenarios

### Bug Fixes

1. Optimized the data interface logic for transaction amounts, resulting in more accurate displays
2. Expanded the hotzone range on the wallet settings page
3. Improved the user experience with the keyboard
4. Resolved issues related to PIN code attempts exceeding specific limits
5. Addressed problems occurring when clicking on the scan code interface
6. Fixed abnormal scenarios triggered during MicroSD card upgrades


## 0.8.8 (2023-08-18)

### Features

1. Enables USB upgrade functionality
2. Supports fingerprint signature capability
3. Keyboard logic optimization

### Bug Fixes

1. Fixes placeholder error with special characters
2. Resolves currency title display issue
