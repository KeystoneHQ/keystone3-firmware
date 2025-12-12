#!/usr/bin/env node

import { KeystoneUSB } from '../lib/keystone-usb.js';
import { DEFAULT_VENDOR_ID, DEFAULT_PRODUCT_ID } from '../lib/constants.js';

/**
 * 示例：获取设备状态
 */
async function main() {
  const keystone = new KeystoneUSB();
  
  try {
    console.log('🔌 连接到设备...');
    await keystone.connect(DEFAULT_VENDOR_ID, DEFAULT_PRODUCT_ID);
    console.log('✓ 已连接\n');
    
    console.log('📊 获取设备状态...');
    const status = await keystone.getStatus();
    
    console.log('✓ 状态信息:');
    console.log(`  成功: ${status.success}`);
    console.log(`  状态码: 0x${status.statusCode.toString(16).padStart(2, '0')}`);
    console.log(`  消息: ${status.statusMessage}`);
    
    if (status.dataHex && status.dataHex.length > 0) {
      console.log(`  数据 (hex): ${status.dataHex}`);
      console.log(`  数据长度: ${status.data.length} bytes`);
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
