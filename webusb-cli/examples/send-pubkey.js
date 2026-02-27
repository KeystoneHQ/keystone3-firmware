#!/usr/bin/env node

import { KeystoneUSB } from '../lib/keystone-usb.js';
import { DEFAULT_VENDOR_ID, DEFAULT_PRODUCT_ID } from '../lib/constants.js';
import { generatePrivateKey, getPublicKey } from '../lib/crypto.js';

/**
 * 示例：发送公钥并验证签名
 * 
 * 这个示例展示如何：
 * 1. 连接到 Keystone 设备
 * 2. 生成或使用已有的密钥对
 * 3. 用私钥对公钥进行签名（自签名）
 * 4. 发送公钥和签名到设备
 * 5. 设备使用公钥验证签名，确认发送方拥有私钥
 */
async function main() {
  const keystone = new KeystoneUSB();
  
  // 生成测试密钥对（实际使用中应该使用已有的密钥）
  console.log('🔑 Generating test key pair...\n');
  const privateKey = generatePrivateKey();
  const publicKey = getPublicKey(privateKey, true); // 使用压缩格式
  
  console.log(`Private Key: ${privateKey.toString('hex')}`);
  console.log(`Public Key: ${publicKey.toString('hex')}\n`);
  
  // 注意：设备会使用接收到的公钥来验证签名（自签名验证）
  // 这证明发送方确实拥有对应的私钥
  
  try {
    console.log('🔌 连接到设备...');
    await keystone.connect(DEFAULT_VENDOR_ID, DEFAULT_PRODUCT_ID);
    console.log('✓ 已连接\n');
    
    console.log('📤 发送公钥和签名到设备...');
    
    const response = await keystone.sendPublicKey(publicKey, privateKey);
    
    if (response.success) {
      console.log('\n✓ 签名验证成功！');
      console.log(`  状态: ${response.statusMessage}`);
      console.log('\n✓ 设备确认你拥有该公钥对应的私钥。');
      console.log('✓ 身份验证通过。');
    } else {
      console.log('\n✗ 签名验证失败');
      console.log(`  错误: ${response.statusMessage}`);
      console.log('\n可能的原因:');
      console.log('  1. 公钥和私钥不匹配');
      console.log('  2. 签名算法错误');
      console.log('  3. 数据格式错误');
    }
    
  } catch (error) {
    console.error('❌ 错误:', error.message);
    process.exit(1);
  } finally {
    await keystone.disconnect();
    console.log('\n✓ 已断开连接');
  }
}

main();
