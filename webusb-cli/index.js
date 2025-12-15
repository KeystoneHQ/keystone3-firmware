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
  .description('Send K1 public key to Keystone device')
  .option('-k, --key <publicKey>', 'K1 public key (hex format, 65 bytes uncompressed or 33 bytes compressed)')
  .option('-v, --vendor-id <id>', `USB Vendor ID (default: ${CLI_DEFAULTS.vendorId})`, CLI_DEFAULTS.vendorId)
  .option('-p, --product-id <id>', `USB Product ID (default: ${CLI_DEFAULTS.productId})`, CLI_DEFAULTS.productId)
  .action(async (options) => {
    try {
      const keystone = new KeystoneUSB();
      
      // 如果没有提供公钥，交互式输入
      let publicKey = options.key;
      if (!publicKey) {
        const answers = await inquirer.prompt([
          {
            type: 'input',
            name: 'publicKey',
            message: 'Enter K1 public key (hex format):',
            validate: (input) => {
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
        publicKey = answers.publicKey;
      }

      // 清理公钥格式
      publicKey = publicKey.replace(/^0x/, '').replace(/\s/g, '');
      
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
      
      // 发送公钥
      spinner.start('Sending K1 public key...');
      const response = await keystone.sendPublicKey(publicKey);
      spinner.succeed('Public key sent successfully');
      
      console.log(chalk.green('\n✓ Response from device:'));
      console.log(chalk.gray(`  Status: ${response.success ? 'Success' : 'Failed'}`));
      console.log(chalk.gray(`  Message: ${response.statusMessage}`));
      console.log(chalk.gray(`  Command Type: 0x${response.commandType?.toString(16) || 'N/A'}`));
      console.log(chalk.gray(`  Request ID: ${response.requestId || 'N/A'}`));
      if (response.totalPackets) {
        console.log(chalk.gray(`  Total Packets: ${response.totalPackets}`));
      }
      if (response.payload && response.payload.length > 0) {
        console.log(chalk.gray(`  Payload Length: ${response.payload.length} bytes`));
        console.log(chalk.gray(`  Payload (hex): ${response.payloadHex || response.payload.toString('hex')}`));
        
        // Try to parse as UTF-8 string (might be JSON)
        try {
          const payloadStr = response.payload.toString('utf8');
          if (payloadStr.includes('{') || payloadStr.includes('[')) {
            console.log(chalk.gray(`  Payload (JSON): ${payloadStr}`));
          } else if (payloadStr.match(/^[\x20-\x7E\s]*$/)) {
            console.log(chalk.gray(`  Payload (text): ${payloadStr}`));
          }
        } catch (e) {
          // Not valid UTF-8, ignore
        }
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
