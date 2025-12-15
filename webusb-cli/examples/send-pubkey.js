#!/usr/bin/env node

import { KeystoneUSB } from '../lib/keystone-usb.js';
import { DEFAULT_VENDOR_ID, DEFAULT_PRODUCT_ID } from '../lib/constants.js';

/**
 * 示例：发送公钥并建立 ECDH 安全通道
 * 
 * 这个示例展示如何：
 * 1. 连接到 Keystone 设备
 * 2. 发送客户端公钥（33 字节压缩格式或 65 字节未压缩格式）
 * 3. 接收设备公钥（33 字节压缩格式）
 * 4. 使用 ECDH 算法在双方建立共享密钥用于后续加密通信
 */
async function main() {
  const keystone = new KeystoneUSB();
  
  // 示例：使用测试公钥（33 字节压缩格式）
  // 在实际应用中，这应该是你生成的真实公钥
  const testPublicKey = '02' + 'a'.repeat(64); // 压缩格式：02 + 32字节X坐标
  
  try {
    console.log('🔌 连接到设备...');
    await keystone.connect(DEFAULT_VENDOR_ID, DEFAULT_PRODUCT_ID);
    console.log('✓ 已连接\n');
    
    console.log('📤 发送公钥到设备...');
    console.log(`  公钥 (hex): ${testPublicKey}`);
    console.log(`  长度: ${testPublicKey.length / 2} bytes\n`);
    
    const response = await keystone.sendPublicKey(testPublicKey);
    
    if (response.success) {
      console.log('✓ 公钥交换成功！');
      console.log(`  状态: ${response.statusMessage}`);
      
      if (response.payload && response.payload.length > 0) {
        console.log(`\n📥 设备返回公钥:`);
        console.log(`  长度: ${response.payload.length} bytes`);
        console.log(`  格式: ${response.payload.length === 33 ? '压缩' : '未压缩'}`);
        console.log(`  公钥 (hex): ${response.payload.toString('hex')}`);
        
        console.log('\n✓ ECDH 密钥交换完成。现在可以使用共享密钥进行加密通信。');
      } else {
        console.log('\n⚠ 设备没有返回公钥');
      }
    } else {
      console.log('✗ 公钥交换失败');
      console.log(`  错误: ${response.statusMessage}`);
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
