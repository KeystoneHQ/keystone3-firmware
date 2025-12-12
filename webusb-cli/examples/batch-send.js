#!/usr/bin/env node

import { KeystoneUSB } from '../lib/keystone-usb.js';
import { DEFAULT_VENDOR_ID, DEFAULT_PRODUCT_ID } from '../lib/constants.js';

/**
 * 示例：发送多个公钥
 */
async function main() {
  const keystone = new KeystoneUSB();
  
  // 测试用的公钥列表
  const publicKeys = [
    // 压缩格式公钥
    '02' + 'a'.repeat(64),
    '03' + 'b'.repeat(64),
    // 非压缩格式公钥
    '04' + 'c'.repeat(128),
  ];
  
  try {
    console.log('🔌 连接到设备...');
    await keystone.connect(DEFAULT_VENDOR_ID, DEFAULT_PRODUCT_ID);
    console.log('✓ 已连接\n');
    
    // 逐个发送公钥
    for (let i = 0; i < publicKeys.length; i++) {
      const pubkey = publicKeys[i];
      const isCompressed = pubkey.length === 66;
      
      console.log(`📤 发送公钥 ${i + 1}/${publicKeys.length}`);
      console.log(`  格式: ${isCompressed ? '压缩 (33 bytes)' : '非压缩 (65 bytes)'}`);
      console.log(`  公钥: ${pubkey.substring(0, 20)}...${pubkey.substring(pubkey.length - 10)}`);
      
      const response = await keystone.sendPublicKey(pubkey);
      
      if (response.success) {
        console.log(`  ✓ 成功! 状态: ${response.statusMessage}`);
        if (response.dataHex) {
          console.log(`  响应数据: ${response.dataHex}`);
        }
      } else {
        console.log(`  ✗ 失败! 状态: ${response.statusMessage} (0x${response.statusCode.toString(16)})`);
      }
      
      console.log('');
      
      // 短暂延迟
      await new Promise(resolve => setTimeout(resolve, 100));
    }
    
    console.log('✓ 所有公钥发送完成');
    
  } catch (error) {
    console.error('❌ 错误:', error.message);
    process.exit(1);
  } finally {
    await keystone.disconnect();
    console.log('\n✓ 已断开连接');
  }
}

main();
