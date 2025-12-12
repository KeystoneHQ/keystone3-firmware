/**
 * Keystone USB 常量配置
 * 统一管理设备 ID 和其他配置参数
 */

// 默认设备 ID
export const DEFAULT_VENDOR_ID = 0x1209;
export const DEFAULT_PRODUCT_ID = 0x3001;

// 设备信息
export const DEVICE_INFO = {
  vendorId: DEFAULT_VENDOR_ID,
  productId: DEFAULT_PRODUCT_ID,
  manufacturer: 'Keystone',
  name: 'Keystone Hardware Wallet'
};

// 通信超时配置（毫秒）
export const TIMEOUTS = {
  connect: 5000,
  read: 5000,
  write: 3000,
  command: 10000
};

// 公钥格式
export const PUBKEY_FORMAT = {
  COMPRESSED_LENGTH: 66,      // 33 bytes * 2 (hex)
  UNCOMPRESSED_LENGTH: 130,   // 65 bytes * 2 (hex)
  COMPRESSED_PREFIX: ['02', '03'],
  UNCOMPRESSED_PREFIX: ['04']
};

// CLI 默认配置
export const CLI_DEFAULTS = {
  vendorId: '0x1209',
  productId: '0x3001'
};

export default {
  DEFAULT_VENDOR_ID,
  DEFAULT_PRODUCT_ID,
  DEVICE_INFO,
  TIMEOUTS,
  PUBKEY_FORMAT,
  CLI_DEFAULTS
};
