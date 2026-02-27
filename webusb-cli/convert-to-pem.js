#!/usr/bin/env node

/**
 * 将原始十六进制密钥转换为 PEM 格式
 * 使用 SEC1 格式用于 secp256k1 密钥
 */

import fs from 'fs';
import crypto from 'crypto';

function hexToPem(hexKey, type) {
  const keyBuffer = Buffer.from(hexKey, 'hex');
  
  if (type === 'private') {
    // SEC1 格式的 EC 私钥
    // 包含 version(1), privateKey(32), parameters(OID), publicKey(optional)
    const oid = Buffer.from('06052b8104000a', 'hex'); // secp256k1 OID
    
    // 构建 ASN.1 DER 编码
    const version = Buffer.from('020101', 'hex'); // version = 1
    const privateKeyTag = Buffer.concat([
      Buffer.from('0420', 'hex'), // OCTET STRING tag + length
      keyBuffer
    ]);
    const paramsTag = Buffer.concat([
      Buffer.from('a007', 'hex'), // CONTEXT SPECIFIC [0]
      oid
    ]);
    
    // 计算公钥（如果需要）
    // const publicKeyTag = ...
    
    const sequence = Buffer.concat([version, privateKeyTag, paramsTag]);
    const derLength = sequence.length;
    const der = Buffer.concat([
      Buffer.from('3074', 'hex'), // SEQUENCE tag + length(116)
      sequence
    ]);
    
    const base64 = der.toString('base64');
    const pem = `-----BEGIN EC PRIVATE KEY-----\n${base64.match(/.{1,64}/g).join('\n')}\n-----END EC PRIVATE KEY-----\n`;
    return pem;
    
  } else if (type === 'public') {
    // SubjectPublicKeyInfo 格式
    const oid = Buffer.from('06052b8104000a', 'hex'); // secp256k1 OID
    const ecPublicKeyOid = Buffer.from('06072a8648ce3d0201', 'hex'); // ecPublicKey OID
    
    const algorithmId = Buffer.concat([
      Buffer.from('3010', 'hex'), // SEQUENCE
      ecPublicKeyOid,
      oid
    ]);
    
    const publicKeyBits = Buffer.concat([
      Buffer.from('0342', 'hex'), // BIT STRING tag + length(66)
      Buffer.from('00', 'hex'), // unused bits = 0
      keyBuffer
    ]);
    
    const der = Buffer.concat([
      Buffer.from('3056', 'hex'), // SEQUENCE tag + length(86)
      algorithmId,
      publicKeyBits
    ]);
    
    const base64 = der.toString('base64');
    const pem = `-----BEGIN PUBLIC KEY-----\n${base64.match(/.{1,64}/g).join('\n')}\n-----END PUBLIC KEY-----\n`;
    return pem;
  }
}

// 读取文件
const privateKeyHex = fs.readFileSync('private_key.txt', 'utf8')
  .split('\n')
  .find(line => !line.startsWith('#') && line.trim().length > 0)
  .trim();

const publicKeyHex = fs.readFileSync('public_key.txt', 'utf8')
  .split('\n')
  .find(line => !line.startsWith('#') && line.trim().length > 0)
  .trim();

console.log('Converting keys to PEM format...\n');

// 转换并保存
const privatePem = hexToPem(privateKeyHex, 'private');
const publicPem = hexToPem(publicKeyHex, 'public');

fs.writeFileSync('private_key.pem', privatePem);
fs.writeFileSync('public_key.pem', publicPem);

console.log('✓ Created: private_key.pem');
console.log('✓ Created: public_key.pem\n');

console.log('Private Key PEM:');
console.log(privatePem);
console.log('Public Key PEM:');
console.log(publicPem);

console.log('\n⚠️  Remember: Keep private_key.pem secure and never share it!');
