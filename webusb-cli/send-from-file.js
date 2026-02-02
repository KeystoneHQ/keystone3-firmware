#!/usr/bin/env node

import { KeystoneUSB } from './lib/keystone-usb.js';
import { DEFAULT_VENDOR_ID, DEFAULT_PRODUCT_ID } from './lib/constants.js';
import fs from 'fs';
import chalk from 'chalk';

/**
 * 从文件读取并发送公钥到设备
 */
async function main() {
  const keystone = new KeystoneUSB();
  
  try {
    // 读取私钥文件
    console.log('📖 Reading key files...\n');
    const privateKeyContent = fs.readFileSync('./private.txt', 'utf8');
    const privateKey = privateKeyContent
      .split('\n')
      .find(line => !line.startsWith('#') && line.trim().length > 0)
      .trim();
    
    // 读取公钥文件
    const publicKeyContent = fs.readFileSync('./pubkey.txt', 'utf8');
    const publicKey = publicKeyContent
      .split('\n')
      .find(line => !line.startsWith('#') && line.trim().length > 0)
      .trim();
    
    console.log(chalk.gray(`Private Key: ${privateKey.substring(0, 16)}...`));
    console.log(chalk.gray(`Public Key: ${publicKey.substring(0, 32)}...\n`));
    
    // 连接设备
    console.log('🔌 Connecting to Keystone device...');
    await keystone.connect(DEFAULT_VENDOR_ID, DEFAULT_PRODUCT_ID);
    console.log(chalk.green('✓ Connected\n'));
    
    // 发送公钥和签名
    console.log('📤 Sending public key with signature...');
    const response = await keystone.sendPublicKey(publicKey, privateKey);
    
    console.log(chalk.green('\n✓ Signature Verification Result:'));
    console.log(chalk.gray(`  Status: ${response.success ? chalk.green('✓ Success') : chalk.red('✗ Failed')}`));
    console.log(chalk.gray(`  Message: ${response.statusMessage}`));
    
    if (response.success) {
      console.log(chalk.blue('\n✓ Public key signature verified successfully!'));
      console.log(chalk.gray('  The device confirmed that you own the private key.'));
    } else {
      console.log(chalk.yellow('\n⚠ Signature verification failed.'));
      console.log(chalk.yellow('  Please check that public and private keys match.'));
    }
    
  } catch (error) {
    if (error.code === 'ENOENT') {
      console.error(chalk.red('\n✗ Error: Key files not found!'));
      console.log(chalk.yellow('\nPlease create the following files:'));
      console.log(chalk.gray('  - private.txt (contains your private key in hex)'));
      console.log(chalk.gray('  - pubkey.txt (contains your public key in hex)\n'));
    } else {
      console.error(chalk.red('\n✗ Error:'), error.message);
    }
    process.exit(1);
  } finally {
    await keystone.disconnect();
    console.log(chalk.blue('\n✓ Disconnected from device\n'));
  }
}

main();
