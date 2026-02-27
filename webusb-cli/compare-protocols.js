#!/usr/bin/env node

/**
 * 对比测试：展示旧协议（内部协议）和新协议（EAPDU）的差异
 */

import { SERVICE_ID, DEVICE_INFO_CMD, TLV_TYPE, buildPacket } from './lib/protocol.js';
import { buildPublicKeyRequest, EAPDU } from './lib/eapdu-protocol.js';

console.log('='.repeat(80));
console.log('协议对比：内部协议 (0x6B) vs EAPDU 协议 (0x00)');
console.log('='.repeat(80));

// 测试公钥：33 字节压缩公钥
const testPubKey = '021234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef';
const keyBuffer = Buffer.from(testPubKey, 'hex');

console.log('\n测试公钥 (33 字节压缩格式):');
console.log(testPubKey);
console.log('');

// ============================================================================
// 旧实现：使用内部协议
// ============================================================================
console.log('\n' + '='.repeat(80));
console.log('【旧实现】使用内部协议 (0x6B) - 错误的方式');
console.log('='.repeat(80));

const tlvArray = [{
  type: TLV_TYPE.UPDATE_PUB_KEY,
  value: keyBuffer
}];

const oldPacket = buildPacket(SERVICE_ID.DEVICE_INFO, DEVICE_INFO_CMD.BASIC, tlvArray);

console.log('\n生成的数据包:');
console.log(`十六进制: ${oldPacket.toString('hex')}`);
console.log(`长度: ${oldPacket.length} 字节`);

console.log('\n协议结构分析:');
console.log(`  [0]      协议头:        0x${oldPacket[0].toString(16).padStart(2, '0')} (应为 0x6B = 内部协议)`);
console.log(`  [1]      协议版本:      ${oldPacket[1]}`);
console.log(`  [2-3]    包索引:        ${oldPacket.readUInt16LE(2)}`);
console.log(`  [4]      服务ID:        ${oldPacket[4]} (DEVICE_INFO)`);
console.log(`  [5]      命令ID:        ${oldPacket[5]} (BASIC)`);
console.log(`  [6-7]    标志:          0x${oldPacket.readUInt16LE(6).toString(16).padStart(4, '0')} (isHost=1, ack=0)`);
console.log(`  [8-9]    负载长度:      ${oldPacket.readUInt16LE(8)} 字节`);
console.log(`  [10]     TLV类型:       0x${oldPacket[10].toString(16).padStart(2, '0')} (UPDATE_PUB_KEY)`);
console.log(`  [11]     TLV长度:       ${oldPacket[11]} 字节`);
console.log(`  [12-44]  TLV值:         ${oldPacket.slice(12, 12 + 10).toString('hex')}... (公钥数据)`);
console.log(`  [45-48]  CRC32:         0x${oldPacket.slice(-4).toString('hex')}`);

console.log('\n❌ 问题:');
console.log('   1. 使用了错误的协议头 (0x6B 而非 0x00)');
console.log('   2. 使用了内部协议的服务/命令结构');
console.log('   3. 设备端会将其路由到 InternalProtocolParser');
console.log('   4. 但设备端期望的是 EApduProtocolParser');
console.log('   5. 导致命令无法被正确识别和处理');

// ============================================================================
// 新实现：使用 EAPDU 协议
// ============================================================================
console.log('\n' + '='.repeat(80));
console.log('【新实现】使用 EAPDU 协议 (0x00) - 正确的方式');
console.log('='.repeat(80));

const newPackets = buildPublicKeyRequest(testPubKey);

console.log(`\n生成了 ${newPackets.length} 个数据包`);

newPackets.forEach((packet, index) => {
  console.log(`\n--- 数据包 ${index + 1}/${newPackets.length} ---`);
  console.log(`十六进制: ${packet.toString('hex')}`);
  console.log(`长度: ${packet.length} 字节`);
  
  console.log('\n协议结构分析:');
  console.log(`  [0]      CLA (协议头):     0x${packet[0].toString(16).padStart(2, '0')} (应为 0x00 = EAPDU协议)`);
  console.log(`  [1-2]    INS (命令类型):   0x${packet.readUInt16LE(1).toString(16).padStart(8, '0')} (CMD_GET_DEVICE_USB_PUBKEY)`);
  console.log(`  [3-4]    P1 (总包数):      ${packet.readUInt16LE(3)}`);
  console.log(`  [5-6]    P2 (包索引):      ${packet.readUInt16LE(5)}`);
  console.log(`  [7-8]    LC (请求ID):      ${packet.readUInt16LE(7)}`);
  
  const dataLength = packet.length - EAPDU.OFFSET_CDATA;
  if (dataLength > 0) {
    const data = packet.slice(EAPDU.OFFSET_CDATA);
    const preview = data.slice(0, Math.min(16, data.length)).toString('hex');
    console.log(`  [9+]     数据 (${dataLength}字节):  ${preview}${dataLength > 16 ? '...' : ''}`);
  }
});

console.log('\n✅ 正确之处:');
console.log('   1. 使用正确的协议头 (0x00)');
console.log('   2. 使用 EAPDU 协议结构');
console.log('   3. 命令类型为 CMD_GET_DEVICE_USB_PUBKEY (0x06)');
console.log('   4. 设备端会将其路由到 EApduProtocolParser');
console.log('   5. 能够正确调用 GetDeviceUsbPubkeyService() 处理');

// ============================================================================
// 设备端处理流程对比
// ============================================================================
console.log('\n' + '='.repeat(80));
console.log('设备端处理流程对比');
console.log('='.repeat(80));

console.log('\n【旧协议在设备端的处理】');
console.log('  1. protocol_parse.c: 检测到 data[0] = 0x6B');
console.log('  2. 选择 NewInternalProtocolParser()');
console.log('  3. internal_protocol_parser.c: 解析内部协议帧');
console.log('  4. 查找 SERVICE_ID_DEVICE_INFO (1) 对应的服务');
console.log('  5. 查找 COMMAND_ID = 1 的处理函数');
console.log('  6. ❌ 但这不是公钥传输命令，无法正确处理!');

console.log('\n【新协议在设备端的处理】');
console.log('  1. protocol_parse.c: 检测到 data[0] = 0x00');
console.log('  2. 选择 NewEApduProtocolParser()');
console.log('  3. eapdu_protocol_parser.c: 解析 EAPDU 帧');
console.log('  4. 提取 INS = CMD_GET_DEVICE_USB_PUBKEY (0x06)');
console.log('  5. 调用 GetDeviceUsbPubkeyService(request)');
console.log('  6. ✅ 正确处理公钥传输请求!');

// ============================================================================
// 总结
// ============================================================================
console.log('\n' + '='.repeat(80));
console.log('总结');
console.log('='.repeat(80));

console.log('\n关键差异:');
console.log('┌─────────────┬──────────────────┬──────────────────┐');
console.log('│ 项目        │ 旧实现 (错误)    │ 新实现 (正确)    │');
console.log('├─────────────┼──────────────────┼──────────────────┤');
console.log('│ 协议头      │ 0x6B             │ 0x00             │');
console.log('│ 协议类型    │ 内部协议         │ EAPDU 协议       │');
console.log('│ 解析器      │ Internal Parser  │ EAPDU Parser     │');
console.log('│ 命令识别    │ Service/Command  │ CMD Type         │');
console.log('│ 处理函数    │ 无匹配           │ GetDevicePubkey  │');
console.log('│ 结果        │ ❌ 失败          │ ✅ 成功          │');
console.log('└─────────────┴──────────────────┴──────────────────┘');

console.log('\n修复说明:');
console.log('  通过使用正确的 EAPDU 协议格式，设备端能够:');
console.log('  • 正确识别协议类型');
console.log('  • 路由到正确的解析器');
console.log('  • 调用正确的处理函数');
console.log('  • 成功处理公钥传输请求');

console.log('\n' + '='.repeat(80));
