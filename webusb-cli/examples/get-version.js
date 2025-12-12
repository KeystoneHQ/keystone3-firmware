#!/usr/bin/env node

import { KeystoneUSB } from '../lib/keystone-usb.js';
import { DEFAULT_VENDOR_ID, DEFAULT_PRODUCT_ID } from '../lib/constants.js';

/**
 * 示例：获取设备版本信息
 */
async function main() {
  const keystone = new KeystoneUSB();
  
  try {
    console.log('🔌 连接到设备...');
    await keystone.connect(DEFAULT_VENDOR_ID, DEFAULT_PRODUCT_ID);
    console.log('✓ 已连接\n');
    
    // 获取设备信息
    const info = keystone.getDeviceInfo();
    console.log('📱 设备信息:');
    console.log(`  制造商: ${info.manufacturer}`);
    console.log(`  产品: ${info.product}`);
    console.log(`  序列号: ${info.serialNumber}\n`);
    
    // 获取版本
    console.log('📋 获取固件版本...');
    const version = await keystone.getVersion();
    
    if (version) {
      console.log('✓ 固件版本:', version.version);
      console.log(`  主版本号: ${version.major}`);
      console.log(`  次版本号: ${version.minor}`);
      console.log(`  补丁号: ${version.patch}`);
    } else {
      console.log('⚠ 无法获取版本信息');
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
