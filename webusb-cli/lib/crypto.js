/**
 * Crypto utilities for signing and verifying
 */
import secp256k1 from 'secp256k1';
import crypto from 'crypto';

/**
 * Sign data with private key using secp256k1
 * @param {Buffer} data - Data to sign (typically a hash)
 * @param {Buffer} privateKey - 32-byte private key
 * @returns {Buffer} Signature (64 bytes in compact format)
 */
export function signData(data, privateKey) {
  if (data.length !== 32) {
    throw new Error('Data must be 32 bytes (hash)');
  }
  if (privateKey.length !== 32) {
    throw new Error('Private key must be 32 bytes');
  }
  
  const signature = secp256k1.ecdsaSign(data, privateKey);
  // Return signature in compact format (64 bytes: r + s)
  return Buffer.from(signature.signature);
}

/**
 * Verify signature
 * @param {Buffer} signature - 64-byte signature
 * @param {Buffer} data - 32-byte hash
 * @param {Buffer} publicKey - 33 or 65 byte public key
 * @returns {boolean} True if signature is valid
 */
export function verifySignature(signature, data, publicKey) {
  if (signature.length !== 64) {
    throw new Error('Signature must be 64 bytes');
  }
  if (data.length !== 32) {
    throw new Error('Data must be 32 bytes (hash)');
  }
  
  return secp256k1.ecdsaVerify(signature, data, publicKey);
}

/**
 * Generate public key from private key
 * @param {Buffer} privateKey - 32-byte private key
 * @param {boolean} compressed - Return compressed format (33 bytes) or uncompressed (65 bytes)
 * @returns {Buffer} Public key
 */
export function getPublicKey(privateKey, compressed = true) {
  if (privateKey.length !== 32) {
    throw new Error('Private key must be 32 bytes');
  }
  
  return Buffer.from(secp256k1.publicKeyCreate(privateKey, compressed));
}

/**
 * SHA256 hash
 * @param {Buffer} data - Data to hash
 * @returns {Buffer} 32-byte hash
 */
export function sha256(data) {
  return crypto.createHash('sha256').update(data).digest();
}

/**
 * Generate random private key
 * @returns {Buffer} 32-byte private key
 */
export function generatePrivateKey() {
  let privateKey;
  do {
    privateKey = crypto.randomBytes(32);
  } while (!secp256k1.privateKeyVerify(privateKey));
  
  return privateKey;
}
