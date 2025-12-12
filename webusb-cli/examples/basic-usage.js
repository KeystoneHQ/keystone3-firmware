#!/usr/bin/env node

import { KeystoneUSB } from '../lib/keystone-usb.js';
import { DEFAULT_VENDOR_ID, DEFAULT_PRODUCT_ID } from '../lib/constants.js';

async function main() {
  const keystone = new KeystoneUSB();
  
  try {
    console.log('连接到设备...');
    await keystone.connect(DEFAULT_VENDOR_ID, DEFAULT_PRODUCT_ID);
    console.log('已连接');
    
    const info = keystone.getDeviceInfo();
    console.log('设备信息:', info);
    
    // 示例公钥（压缩格式）
    const publicKey = '02' + 'a'.repeat(64);
    console.log('发送公钥:', publicKey.substring(0, 20) + '...');
    
    const response = await keystone.sendPublicKey(publicKey);
    console.log('响应:', response);
    
    if (response.success) {
      console.log('✓ 成功！');
    }
    
  } catch (error) {
    console.error('错误:', error.message);
  } finally {
    await keystone.disconnect();
    console.log('已断开');
  }
}

main();
