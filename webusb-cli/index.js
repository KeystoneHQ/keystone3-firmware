#!/usr/bin/env node

import { program } from 'commander';
import chalk from 'chalk';
import ora from 'ora';
import inquirer from 'inquirer';
import { KeystoneUSB } from './lib/keystone-usb.js';
import { CLI_DEFAULTS } from './lib/constants.js';

program
  .name('keystone-usb')
  .description('CLI tool to communicate with Keystone device via USB')
  .version('1.0.0');

// 发送 K1 公钥命令
program
  .command('send-pubkey')
  .description('Send K1 public key with signature to Keystone device')
  .option('-k, --key <publicKey>', 'K1 public key (hex format, 65 bytes uncompressed or 33 bytes compressed)')
  .option('-s, --secret <privateKey>', 'Private key for signing (hex format, 32 bytes)')
  .option('-v, --vendor-id <id>', `USB Vendor ID (default: ${CLI_DEFAULTS.vendorId})`, CLI_DEFAULTS.vendorId)
  .option('-p, --product-id <id>', `USB Product ID (default: ${CLI_DEFAULTS.productId})`, CLI_DEFAULTS.productId)
  .action(async (options) => {
    try {
      const keystone = new KeystoneUSB();
      
      // 如果没有提供公钥和私钥，交互式输入
      let publicKey = options.key;
      let privateKey = options.secret;
      
      if (!publicKey || !privateKey) {
        const answers = await inquirer.prompt([
          {
            type: 'input',
            name: 'privateKey',
            message: 'Enter private key (hex format, 32 bytes):',
            when: !privateKey,
            validate: (input) => {
              const cleaned = input.replace(/^0x/, '').replace(/\s/g, '');
              if (cleaned.length !== 64) {
                return 'Private key must be 32 bytes (64 hex characters)';
              }
              if (!/^[0-9a-fA-F]+$/.test(cleaned)) {
                return 'Private key must be valid hex string';
              }
              return true;
            }
          },
          {
            type: 'input',
            name: 'publicKey',
            message: 'Enter K1 public key (hex format, or leave empty to derive from private key):',
            when: !publicKey,
            validate: (input) => {
              if (!input || input.trim() === '') {
                return true; // Allow empty, will derive from private key
              }
              const cleaned = input.replace(/^0x/, '').replace(/\s/g, '');
              if (cleaned.length !== 66 && cleaned.length !== 130) {
                return 'Public key must be 33 bytes (compressed) or 65 bytes (uncompressed) in hex format';
              }
              if (!/^[0-9a-fA-F]+$/.test(cleaned)) {
                return 'Public key must be valid hex string';
              }
              return true;
            }
          }
        ]);
        
        privateKey = privateKey || answers.privateKey;
        publicKey = publicKey || answers.publicKey;
      }

      // 清理格式
      privateKey = privateKey.replace(/^0x/, '').replace(/\s/g, '');
      
      // 如果没有提供公钥，从私钥派生
      if (!publicKey || publicKey.trim() === '') {
        const { getPublicKey } = await import('./lib/crypto.js');
        const privKeyBuffer = Buffer.from(privateKey, 'hex');
        const pubKeyBuffer = getPublicKey(privKeyBuffer, true); // 使用压缩格式
        publicKey = pubKeyBuffer.toString('hex');
        console.log(chalk.blue('\n✓ Public key derived from private key'));
      } else {
        publicKey = publicKey.replace(/^0x/, '').replace(/\s/g, '');
      }
      
      console.log(chalk.blue('\n🔌 Connecting to Keystone device...\n'));
      
      const vendorId = parseInt(options.vendorId);
      const productId = parseInt(options.productId);
      
      const spinner = ora('Searching for device...').start();
      
      // 连接设备
      await keystone.connect(vendorId, productId);
      spinner.succeed('Device connected');
      
      // 显示设备信息
      const deviceInfo = keystone.getDeviceInfo();
      console.log(chalk.gray('Device Info:'));
      console.log(chalk.gray(`  Vendor ID: 0x${deviceInfo.vendorId.toString(16).padStart(4, '0')}`));
      console.log(chalk.gray(`  Product ID: 0x${deviceInfo.productId.toString(16).padStart(4, '0')}`));
      console.log(chalk.gray(`  Manufacturer: ${deviceInfo.manufacturer}`));
      console.log(chalk.gray(`  Product: ${deviceInfo.product}\n`));
      
      // 发送公钥（带签名）
      spinner.start('Sending K1 public key with signature...');
      const response = await keystone.sendPublicKey(publicKey, privateKey);
      spinner.succeed('Public key exchange completed');
      
      // Check if response is a success status code
      // Success codes: 0 (RSP_SUCCESS_CODE), 0x16 (PRS_SET_PUBKEY_VERIFY_SUCCESS), 0x17 (PRS_SET_PUBKEY_SET_SUCCESS)
      const isSuccess = response.status === 0x00000000 || response.status === 0x00000016 || response.status === 0x00000017;
      
      console.log(chalk.green('\n✓ Signature Verification Result:'));
      console.log(chalk.gray(`  Status: ${isSuccess ? '✓ Success' : '✗ Failed'}`));
      console.log(chalk.gray(`  Message: ${response.statusMessage}`));
      
      if (isSuccess) {
        console.log(chalk.blue('\n✓ Public key signature verified successfully!'));
        console.log(chalk.gray('  The device confirmed that you own the private key.'));
      } else {
        console.log(chalk.yellow('\n⚠ Signature verification failed.'));
        console.log(chalk.yellow('  Possible reasons:'));
        console.log(chalk.yellow('    - Public key and private key do not match'));
        console.log(chalk.yellow('    - Invalid signature format'));
        console.log(chalk.yellow('    - Data packet corruption'));
      }
      
      // 断开连接
      await keystone.disconnect();
      console.log(chalk.blue('\n✓ Disconnected from device\n'));
      
    } catch (error) {
      console.error(chalk.red('\n✗ Error:'), error.message);
      process.exit(1);
    }
  });

// 列出可用设备
program
  .command('list-devices')
  .description('List all available USB devices')
  .action(async () => {
    try {
      const keystone = new KeystoneUSB();
      const devices = await keystone.listDevices();
      
      if (devices.length === 0) {
        console.log(chalk.yellow('\nNo USB devices found\n'));
        return;
      }
      
      console.log(chalk.blue('\n📱 Available USB Devices:\n'));
      devices.forEach((device, index) => {
        console.log(chalk.white(`${index + 1}. ${device.manufacturer || 'Unknown'} - ${device.product || 'Unknown'}`));
        console.log(chalk.gray(`   Vendor ID: 0x${device.vendorId.toString(16).padStart(4, '0')}`));
        console.log(chalk.gray(`   Product ID: 0x${device.productId.toString(16).padStart(4, '0')}\n`));
      });
      
    } catch (error) {
      console.error(chalk.red('\n✗ Error:'), error.message);
      process.exit(1);
    }
  });

// 获取设备版本
// program
//   .command('version')
//   .description('Get Keystone device firmware version')
//   .option('-v, --vendor-id <id>', `USB Vendor ID (default: ${CLI_DEFAULTS.vendorId})`, CLI_DEFAULTS.vendorId)
//   .option('-p, --product-id <id>', `USB Product ID (default: ${CLI_DEFAULTS.productId})`, CLI_DEFAULTS.productId)
//   .action(async (options) => {
//     try {
//       const keystone = new KeystoneUSB();
//       const spinner = ora('Connecting to device...').start();
//       
//       const vendorId = parseInt(options.vendorId);
//       const productId = parseInt(options.productId);
//       
//       await keystone.connect(vendorId, productId);
//       spinner.succeed('Device connected');
//       
//       spinner.start('Getting version...');
//       const version = await keystone.getVersion();
//       spinner.succeed('Version retrieved');
//       
//       if (version) {
//         console.log(chalk.green('\n✓ Device Version:'));
//         console.log(chalk.gray(`  Firmware: v${version.version}`));
//         console.log(chalk.gray(`  Major: ${version.major}`));
//         console.log(chalk.gray(`  Minor: ${version.minor}`));
//         console.log(chalk.gray(`  Patch: ${version.patch}\n`));
//       } else {
//         console.log(chalk.yellow('\n⚠ Could not retrieve version\n'));
//       }
//       
//       await keystone.disconnect();
//       
//     } catch (error) {
//       console.error(chalk.red('\n✗ Error:'), error.message);
//       process.exit(1);
//     }
//   });

// 获取设备状态
program
  .command('status')
  .description('Get Keystone device status')
  .option('-v, --vendor-id <id>', `USB Vendor ID (default: ${CLI_DEFAULTS.vendorId})`, CLI_DEFAULTS.vendorId)
  .option('-p, --product-id <id>', `USB Product ID (default: ${CLI_DEFAULTS.productId})`, CLI_DEFAULTS.productId)
  .action(async (options) => {
    try {
      const keystone = new KeystoneUSB();
      const spinner = ora('Connecting to device...').start();
      
      const vendorId = parseInt(options.vendorId);
      const productId = parseInt(options.productId);
      
      await keystone.connect(vendorId, productId);
      spinner.succeed('Device connected');
      
      spinner.start('Getting status...');
      const status = await keystone.getStatus();
      spinner.succeed('Status retrieved');
      
      console.log(chalk.green('\n✓ Device Information:'));
      
      if (status.tlvArray && status.tlvArray.length > 0) {
        status.tlvArray.forEach((tlv) => {
          let label = 'Unknown';
          let value = tlv.value ? tlv.value.toString('utf8').replace(/\0/g, '') : '';
          
          switch(tlv.type) {
            case 1: label = 'Model'; break;
            case 2: label = 'Serial Number'; break;
            case 3: label = 'Hardware Version'; break;
            case 4: label = 'Firmware Version'; break;
            case 5: label = 'Boot Version'; break;
            default: label = `Type ${tlv.type}`;
          }
          
          console.log(chalk.gray(`  ${label}: ${chalk.white(value)}`));
        });
      } else {
        console.log(chalk.yellow('  No device information available'));
      }
      
      console.log(chalk.gray(`\n  Protocol Status: ${status.statusMessage}`));
      
      await keystone.disconnect();
      
    } catch (error) {
      console.error(chalk.red('\n✗ Error:'), error.message);
      process.exit(1);
    }
  });

// 交互式模式
program
  .command('interactive')
  .alias('i')
  .description('Interactive mode for device communication')
  .action(async () => {
    try {
      const keystone = new KeystoneUSB();
      
      // 选择操作
      const { action } = await inquirer.prompt([
        {
          type: 'list',
          name: 'action',
          message: 'What would you like to do?',
          choices: [
            { name: 'Send K1 Public Key', value: 'send-pubkey' },
            { name: 'Get Device Version', value: 'version' },
            { name: 'Get Device Status', value: 'status' },
            { name: 'List Devices', value: 'list' },
            { name: 'Exit', value: 'exit' }
          ]
        }
      ]);
      
      if (action === 'exit') {
        console.log(chalk.blue('\nGoodbye! 👋\n'));
        return;
      }
      
      if (action === 'list') {
        program.parse(['node', 'index.js', 'list-devices']);
        return;
      }
      
      if (action === 'version') {
        program.parse(['node', 'index.js', 'version']);
        return;
      }
      
      if (action === 'status') {
        program.parse(['node', 'index.js', 'status']);
        return;
      }
      
      if (action === 'send-pubkey') {
        const { publicKey, vendorId, productId } = await inquirer.prompt([
          {
            type: 'input',
            name: 'publicKey',
            message: 'Enter K1 public key (hex):',
            validate: (input) => {
              const cleaned = input.replace(/^0x/, '').replace(/\s/g, '');
              if (cleaned.length !== 66 && cleaned.length !== 130) {
                return 'Invalid length. Must be 33 or 65 bytes in hex';
              }
              if (!/^[0-9a-fA-F]+$/.test(cleaned)) {
                return 'Must be valid hex string';
              }
              return true;
            }
          },
          {
            type: 'input',
            name: 'vendorId',
            message: `Vendor ID (hex, press Enter for default ${CLI_DEFAULTS.vendorId}):`,
            default: CLI_DEFAULTS.vendorId
          },
          {
            type: 'input',
            name: 'productId',
            message: `Product ID (hex, press Enter for default ${CLI_DEFAULTS.productId}):`,
            default: CLI_DEFAULTS.productId
          }
        ]);
        
        program.parse(['node', 'index.js', 'send-pubkey', '-k', publicKey, '-v', vendorId, '-p', productId]);
      }
      
    } catch (error) {
      console.error(chalk.red('\n✗ Error:'), error.message);
      process.exit(1);
    }
  });

program.parse();
