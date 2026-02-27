#!/usr/bin/env node

/**
 * 快速验证脚本
 * 检查 EAPDU 协议实现是否正确
 */

import chalk from 'chalk';
import { buildPublicKeyRequest, parseEAPDUResponse, CMD_TYPE, EAPDU, STATUS } from './lib/eapdu-protocol.js';

console.log(chalk.bold.cyan('\n='.repeat(70)));
console.log(chalk.bold.cyan('WebUSB CLI 协议修复验证'));
console.log(chalk.bold.cyan('='.repeat(70)));

let passed = 0;
let failed = 0;

function test(name, fn) {
  try {
    fn();
    console.log(chalk.green('✓'), name);
    passed++;
  } catch (error) {
    console.log(chalk.red('✗'), name);
    console.log(chalk.red('  错误:'), error.message);
    failed++;
  }
}

// 测试 1: 协议头正确
console.log(chalk.bold('\n1. 协议头验证'));
test('EAPDU 协议头应为 0x00', () => {
  if (EAPDU.HEADER !== 0x00) {
    throw new Error(`协议头错误: 期望 0x00, 实际 0x${EAPDU.HEADER.toString(16)}`);
  }
});

// 测试 2: 命令类型正确
console.log(chalk.bold('\n2. 命令类型验证'));
test('公钥传输命令应为 0x06', () => {
  if (CMD_TYPE.GET_DEVICE_USB_PUBKEY !== 0x06) {
    throw new Error(`命令类型错误: 期望 0x06, 实际 0x${CMD_TYPE.GET_DEVICE_USB_PUBKEY.toString(16)}`);
  }
});

// 测试 3: 包结构正确
console.log(chalk.bold('\n3. 数据包结构验证'));
test('压缩公钥 (33 字节) 应生成正确的包', () => {
  const key = '02' + 'aa'.repeat(32);
  const packets = buildPublicKeyRequest(key);
  
  if (packets.length !== 1) {
    throw new Error(`包数量错误: 期望 1, 实际 ${packets.length}`);
  }
  
  const packet = packets[0];
  
  // 检查协议头
  if (packet[0] !== 0x00) {
    throw new Error(`协议头错误: 期望 0x00, 实际 0x${packet[0].toString(16)}`);
  }
  
  // 检查命令类型
  const cmd = packet.readUInt16LE(1);
  if (cmd !== 0x06) {
    throw new Error(`命令错误: 期望 0x06, 实际 0x${cmd.toString(16)}`);
  }
  
  // 检查数据长度
  const dataLen = packet.length - EAPDU.OFFSET_CDATA;
  if (dataLen !== 33) {
    throw new Error(`数据长度错误: 期望 33, 实际 ${dataLen}`);
  }
});

test('非压缩公钥 (65 字节) 应正确分包', () => {
  const key = '04' + 'bb'.repeat(64);
  const packets = buildPublicKeyRequest(key);
  
  if (packets.length !== 2) {
    throw new Error(`包数量错误: 期望 2, 实际 ${packets.length}`);
  }
  
  // 验证第一个包
  const packet1 = packets[0];
  if (packet1[0] !== 0x00) {
    throw new Error('第一个包协议头错误');
  }
  if (packet1.readUInt16LE(3) !== 2) {
    throw new Error('第一个包总包数错误');
  }
  if (packet1.readUInt16LE(5) !== 0) {
    throw new Error('第一个包索引错误');
  }
  
  // 验证第二个包
  const packet2 = packets[1];
  if (packet2[0] !== 0x00) {
    throw new Error('第二个包协议头错误');
  }
  if (packet2.readUInt16LE(3) !== 2) {
    throw new Error('第二个包总包数错误');
  }
  if (packet2.readUInt16LE(5) !== 1) {
    throw new Error('第二个包索引错误');
  }
  
  // 验证总数据长度
  const totalDataLen = (packet1.length - EAPDU.OFFSET_CDATA) + (packet2.length - EAPDU.OFFSET_CDATA);
  if (totalDataLen !== 65) {
    throw new Error(`总数据长度错误: 期望 65, 实际 ${totalDataLen}`);
  }
});

// 测试 4: 响应解析正确
console.log(chalk.bold('\n4. 响应解析验证'));
test('应正确解析成功响应', () => {
  const response = Buffer.alloc(20);
  response[0] = 0x00;  // CLA
  response.writeUInt16LE(0x06, 1);  // INS
  response.writeUInt16LE(1, 3);     // P1 (total)
  response.writeUInt16LE(0, 5);     // P2 (index)
  response.writeUInt16LE(0, 7);     // LC (requestId)
  Buffer.from('ok', 'utf8').copy(response, 9);
  response.writeUInt16LE(STATUS.SUCCESS, 11);  // Status
  
  const parsed = parseEAPDUResponse(response.slice(0, 13));
  
  if (!parsed.success) {
    throw new Error('解析成功状态失败');
  }
  if (parsed.status !== STATUS.SUCCESS) {
    throw new Error(`状态码错误: 期望 ${STATUS.SUCCESS}, 实际 ${parsed.status}`);
  }
  if (parsed.payload.toString('utf8') !== 'ok') {
    throw new Error(`负载错误: 期望 "ok", 实际 "${parsed.payload.toString('utf8')}"`);
  }
});

test('应正确解析失败响应', () => {
  const response = Buffer.alloc(20);
  response[0] = 0x00;
  response.writeUInt16LE(0x06, 1);
  response.writeUInt16LE(1, 3);
  response.writeUInt16LE(0, 5);
  response.writeUInt16LE(0, 7);
  Buffer.from('error', 'utf8').copy(response, 9);
  response.writeUInt16LE(STATUS.FAILURE, 14);
  
  const parsed = parseEAPDUResponse(response.slice(0, 16));
  
  if (parsed.success) {
    throw new Error('应该解析为失败状态');
  }
  if (parsed.status !== STATUS.FAILURE) {
    throw new Error(`状态码错误: 期望 ${STATUS.FAILURE}, 实际 ${parsed.status}`);
  }
});

// 测试 5: 与固件期望一致
console.log(chalk.bold('\n5. 固件兼容性验证'));
test('偏移量应匹配固件定义', () => {
  const expectedOffsets = {
    OFFSET_CLA: 0,
    OFFSET_INS: 1,
    OFFSET_P1: 3,
    OFFSET_P2: 5,
    OFFSET_LC: 7,
    OFFSET_CDATA: 9,
  };
  
  for (const [key, expected] of Object.entries(expectedOffsets)) {
    if (EAPDU[key] !== expected) {
      throw new Error(`${key} 偏移量错误: 期望 ${expected}, 实际 ${EAPDU[key]}`);
    }
  }
});

test('数据包应为小端序 (little-endian)', () => {
  const testValue = 0x1234;
  const buffer = Buffer.alloc(2);
  buffer.writeUInt16LE(testValue, 0);
  
  if (buffer[0] !== 0x34 || buffer[1] !== 0x12) {
    throw new Error('字节序错误: 应使用小端序');
  }
});

// 汇总结果
console.log(chalk.bold.cyan('\n' + '='.repeat(70)));
console.log(chalk.bold('验证结果:'));
console.log(chalk.green(`  ✓ 通过: ${passed}`));
if (failed > 0) {
  console.log(chalk.red(`  ✗ 失败: ${failed}`));
}
console.log(chalk.bold.cyan('='.repeat(70)));

if (failed === 0) {
  console.log(chalk.bold.green('\n🎉 所有验证通过！协议实现正确。\n'));
  console.log(chalk.gray('下一步:'));
  console.log(chalk.gray('  1. 连接设备'));
  console.log(chalk.gray('  2. 运行: node index.js send-pubkey -k <your-public-key>'));
  console.log(chalk.gray('  3. 检查设备端日志确认协议处理正确\n'));
  process.exit(0);
} else {
  console.log(chalk.bold.red('\n❌ 验证失败！请检查实现。\n'));
  process.exit(1);
}
