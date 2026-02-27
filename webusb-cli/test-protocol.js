#!/usr/bin/env node

import { SERVICE_ID, DEVICE_INFO_CMD, TLV_TYPE, buildPacket } from './lib/protocol.js';

console.log('Testing protocol packet building...\n');

// 测试 1: 构建获取设备信息的包
console.log('Test 1: Build device info request');
const packet1 = buildPacket(SERVICE_ID.DEVICE_INFO, DEVICE_INFO_CMD.BASIC, []);
console.log('Packet hex:', packet1.toString('hex'));
console.log('Packet length:', packet1.length);
console.log('Frame breakdown:');
console.log('  Header:', packet1.slice(0, 1).toString('hex'), '(should be 6b)');
console.log('  Version:', packet1.slice(1, 2).toString('hex'), '(should be 00)');
console.log('  PacketIndex:', packet1.slice(2, 4).toString('hex'));
console.log('  ServiceId:', packet1.slice(4, 5).toString('hex'), '(should be 01)');
console.log('  CommandId:', packet1.slice(5, 6).toString('hex'), '(should be 01)');
console.log('  Flag:', packet1.slice(6, 8).toString('hex'), '(should be 0200)');
console.log('  Length:', packet1.slice(8, 10).toString('hex'));
console.log('  CRC32:', packet1.slice(-4).toString('hex'));
console.log();

// 测试 2: 构建带 TLV 数据的包（模拟发送公钥）
console.log('Test 2: Build packet with TLV data (mock public key)');
const mockPubKey = Buffer.from('04' + '00'.repeat(64), 'hex'); // 65 字节未压缩公钥
const packet2 = buildPacket(SERVICE_ID.DEVICE_INFO, DEVICE_INFO_CMD.BASIC, [
  {
    type: TLV_TYPE.UPDATE_PUB_KEY,
    value: mockPubKey
  }
]);
console.log('Packet hex:', packet2.toString('hex'));
console.log('Packet length:', packet2.length);
console.log('Frame breakdown:');
console.log('  Header:', packet2.slice(0, 1).toString('hex'));
console.log('  Version:', packet2.slice(1, 2).toString('hex'));
console.log('  PacketIndex:', packet2.slice(2, 4).toString('hex'));
console.log('  ServiceId:', packet2.slice(4, 5).toString('hex'));
console.log('  CommandId:', packet2.slice(5, 6).toString('hex'));
console.log('  Flag:', packet2.slice(6, 8).toString('hex'));
console.log('  Length:', packet2.slice(8, 10).toString('hex'));
console.log('  TLV Type:', packet2.slice(10, 11).toString('hex'), '(should be 10)');
console.log('  TLV Length:', packet2.slice(11, 12).toString('hex'), '(should be 41 = 65 bytes)');
console.log('  TLV Value (first 10 bytes):', packet2.slice(12, 22).toString('hex'));
console.log('  CRC32:', packet2.slice(-4).toString('hex'));
console.log();

// 测试 3: 验证 CRC32 计算
console.log('Test 3: Verify CRC32 calculation');
import { crc32 } from 'crc';
const dataWithoutCrc = packet1.slice(0, -4);
const calculatedCrc = crc32(dataWithoutCrc);
const packetCrc = packet1.readUInt32LE(packet1.length - 4);
console.log('Calculated CRC32:', calculatedCrc.toString(16));
console.log('Packet CRC32:', packetCrc.toString(16));
console.log('Match:', calculatedCrc === packetCrc ? '✓' : '✗');
console.log();

console.log('Protocol test completed!');
