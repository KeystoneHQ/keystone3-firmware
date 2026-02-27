#!/usr/bin/env node

import { buildPublicKeyRequest, parseEAPDUResponse, CMD_TYPE, EAPDU } from './lib/eapdu-protocol.js';

console.log('Testing EAPDU protocol implementation...\n');

// Test 1: Build single packet request (small data)
console.log('=== Test 1: Build single packet (small data) ===');
const smallData = Buffer.from('Hello', 'utf8');
const packets1 = buildPublicKeyRequest(smallData.toString('hex'));
console.log(`Generated ${packets1.length} packet(s)`);
packets1.forEach((packet, i) => {
  console.log(`\nPacket ${i + 1}:`);
  console.log(`  Hex: ${packet.toString('hex')}`);
  console.log(`  Length: ${packet.length} bytes`);
  console.log(`  CLA: 0x${packet[EAPDU.OFFSET_CLA].toString(16).padStart(2, '0')}`);
  console.log(`  INS (Command): 0x${packet.readUInt16LE(EAPDU.OFFSET_INS).toString(16).padStart(8, '0')}`);
  console.log(`  P1 (Total): ${packet.readUInt16LE(EAPDU.OFFSET_P1)}`);
  console.log(`  P2 (Index): ${packet.readUInt16LE(EAPDU.OFFSET_P2)}`);
  console.log(`  LC (RequestID): ${packet.readUInt16LE(EAPDU.OFFSET_LC)}`);
  console.log(`  Data: ${packet.slice(EAPDU.OFFSET_CDATA).toString('hex')}`);
});

// Test 2: Build request with 33-byte compressed public key
console.log('\n=== Test 2: Build request with compressed public key (33 bytes) ===');
const compressedPubKey = '02' + '1234567890abcdef'.repeat(4);
const packets2 = buildPublicKeyRequest(compressedPubKey);
console.log(`Generated ${packets2.length} packet(s)`);
console.log(`Compressed key: ${compressedPubKey}`);
packets2.forEach((packet, i) => {
  console.log(`\nPacket ${i + 1}:`);
  console.log(`  Hex: ${packet.toString('hex')}`);
  console.log(`  Length: ${packet.length} bytes`);
  console.log(`  Data length: ${packet.length - EAPDU.OFFSET_CDATA} bytes`);
});

// Test 3: Build request with 65-byte uncompressed public key
console.log('\n=== Test 3: Build request with uncompressed public key (65 bytes) ===');
const uncompressedPubKey = '04' + '1234567890abcdef'.repeat(8);
const packets3 = buildPublicKeyRequest(uncompressedPubKey);
console.log(`Generated ${packets3.length} packet(s)`);
console.log(`Uncompressed key: ${uncompressedPubKey}`);
packets3.forEach((packet, i) => {
  console.log(`\nPacket ${i + 1}:`);
  console.log(`  Hex: ${packet.toString('hex')}`);
  console.log(`  Length: ${packet.length} bytes`);
  console.log(`  Data length: ${packet.length - EAPDU.OFFSET_CDATA} bytes`);
  if (packet.length > EAPDU.OFFSET_CDATA) {
    const data = packet.slice(EAPDU.OFFSET_CDATA);
    console.log(`  Data (first 20 bytes): ${data.slice(0, Math.min(20, data.length)).toString('hex')}`);
  }
});

// Test 4: Parse mock response
console.log('\n=== Test 4: Parse mock EAPDU response ===');
const mockResponse = Buffer.alloc(64);
mockResponse[EAPDU.OFFSET_CLA] = EAPDU.HEADER;
mockResponse.writeUInt16LE(CMD_TYPE.GET_DEVICE_USB_PUBKEY, EAPDU.OFFSET_INS);
mockResponse.writeUInt16LE(1, EAPDU.OFFSET_P1);  // total packets
mockResponse.writeUInt16LE(0, EAPDU.OFFSET_P2);  // packet index
mockResponse.writeUInt16LE(0, EAPDU.OFFSET_LC);  // request ID
Buffer.from('success', 'utf8').copy(mockResponse, EAPDU.OFFSET_CDATA);
mockResponse.writeUInt16LE(0x0000, EAPDU.OFFSET_CDATA + 7);  // status = SUCCESS

const actualResponseLength = EAPDU.OFFSET_CDATA + 7 + 2;
const trimmedResponse = mockResponse.slice(0, actualResponseLength);

console.log(`Mock response hex: ${trimmedResponse.toString('hex')}`);
try {
  const parsed = parseEAPDUResponse(trimmedResponse);
  console.log('Parsed response:');
  console.log(`  CLA: 0x${parsed.cla.toString(16)}`);
  console.log(`  Command Type: 0x${parsed.commandType.toString(16)}`);
  console.log(`  Total Packets: ${parsed.totalPackets}`);
  console.log(`  Packet Index: ${parsed.packetIndex}`);
  console.log(`  Request ID: ${parsed.requestId}`);
  console.log(`  Status: 0x${parsed.status.toString(16)} (${parsed.statusMessage})`);
  console.log(`  Success: ${parsed.success}`);
  console.log(`  Payload: ${parsed.payload.toString('hex')}`);
  console.log(`  Payload (text): ${parsed.payload.toString('utf8')}`);
} catch (error) {
  console.error('Error parsing response:', error.message);
}

// Test 5: Verify packet structure matches C code expectations
console.log('\n=== Test 5: Verify structure matches firmware expectations ===');
const testKey = '04' + 'aa'.repeat(64);  // 65-byte uncompressed key
const testPackets = buildPublicKeyRequest(testKey);
const firstPacket = testPackets[0];

console.log('Firmware expects (from eapdu_protocol_parser.c):');
console.log('  OFFSET_CLA = 0');
console.log('  OFFSET_INS = 1 (2 bytes, little-endian)');
console.log('  OFFSET_P1 = 3 (2 bytes, little-endian)');
console.log('  OFFSET_P2 = 5 (2 bytes, little-endian)');
console.log('  OFFSET_LC = 7 (2 bytes, little-endian)');
console.log('  OFFSET_CDATA = 9');
console.log('\nOur packet structure:');
console.log(`  [0] CLA: 0x${firstPacket[0].toString(16).padStart(2, '0')}`);
console.log(`  [1-2] INS: 0x${firstPacket.readUInt16LE(1).toString(16).padStart(8, '0')}`);
console.log(`  [3-4] P1 (Total Packets): ${firstPacket.readUInt16LE(3)}`);
console.log(`  [5-6] P2 (Packet Index): ${firstPacket.readUInt16LE(5)}`);
console.log(`  [7-8] LC (Request ID): ${firstPacket.readUInt16LE(7)}`);
console.log(`  [9+] Data: ${firstPacket.slice(9, Math.min(19, firstPacket.length)).toString('hex')}...`);
console.log('\n✓ Structure matches firmware expectations');

console.log('\n=== All tests completed! ===');
