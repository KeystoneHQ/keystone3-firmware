/**
 * Keystone USB 协议定义
 * 基于 src/webusb_protocol/protocol_codec.h 的实现
 */

import { crc32 } from 'crc';

// 服务 ID
export const SERVICE_ID = {
  DEVICE_INFO: 1,
  FILE_TRANS: 2,
  NFT_FILE_TRANS: 3,
};

// 设备信息命令 ID
export const DEVICE_INFO_CMD = {
  BASIC: 1,      // 获取基本信息
  RUNNING: 2,    // 获取运行状态
};

// TLV 类型定义
export const TLV_TYPE = {
  DEVICE_MODEL: 1,
  DEVICE_SERIAL_NUMBER: 2,
  DEVICE_HARDWARE_VERSION: 3,
  DEVICE_FIRMWARE_VERSION: 4,
  DEVICE_BOOT_VERSION: 5,
  GENERAL_RESULT_ACK: 0xFF,
  // 公钥相关（需要根据实际固件确定）
  UPDATE_PUB_KEY: 0x10,
};

// 状态码
export const STATUS = {
  SUCCESS: 0x00,
  ERROR_INVALID_CMD: 0x01,
  ERROR_INVALID_LENGTH: 0x02,
  ERROR_INVALID_DATA: 0x03,
  ERROR_DEVICE_BUSY: 0x04,
  ERROR_TIMEOUT: 0x05,
  ERROR_UNKNOWN: 0xFF,
};

// 协议常量
export const PROTOCOL = {
  HEADER: 0x6B,           // 协议头标识
  VERSION: 0,             // 协议版本
  MAX_PACKET_SIZE: 4500,  // 最大包大小
  FRAME_HEAD_SIZE: 10,    // FrameHead 大小
  DEFAULT_TIMEOUT: 5000,  // 默认超时时间（毫秒）
};

/**
 * 编码 TLV 数据
 * @param {number} type - TLV 类型
 * @param {Buffer} value - TLV 值
 * @returns {Buffer} 编码后的 TLV
 */
function encodeTLV(type, value) {
  const length = value ? value.length : 0;
  let tlvBuffer;
  
  if (length > 127) {
    // 长度大于 127，使用两字节编码
    tlvBuffer = Buffer.alloc(3 + length);
    tlvBuffer.writeUInt8(type, 0);
    tlvBuffer.writeUInt8(0x80 | (length >> 8), 1);
    tlvBuffer.writeUInt8(length & 0xFF, 2);
    if (value) {
      value.copy(tlvBuffer, 3);
    }
  } else {
    // 长度小于等于 127，使用单字节编码
    tlvBuffer = Buffer.alloc(2 + length);
    tlvBuffer.writeUInt8(type, 0);
    tlvBuffer.writeUInt8(length, 1);
    if (value) {
      value.copy(tlvBuffer, 2);
    }
  }
  
  return tlvBuffer;
}

/**
 * 构建协议帧
 * @param {number} serviceId - 服务 ID
 * @param {number} commandId - 命令 ID
 * @param {Array<{type: number, value: Buffer}>} tlvArray - TLV 数组
 * @param {number} packetIndex - 包索引
 * @returns {Buffer} 完整的协议帧
 */
export function buildPacket(serviceId, commandId, tlvArray = [], packetIndex = 0) {
  // 编码所有 TLV
  const tlvBuffers = tlvArray.map(tlv => encodeTLV(tlv.type, tlv.value));
  const tlvData = Buffer.concat(tlvBuffers);
  
  // 计算帧头
  const frameHead = Buffer.alloc(PROTOCOL.FRAME_HEAD_SIZE);
  frameHead.writeUInt8(PROTOCOL.HEADER, 0);           // head = 0x6B
  frameHead.writeUInt8(PROTOCOL.VERSION, 1);          // protocolVersion = 0
  frameHead.writeUInt16LE(packetIndex, 2);            // packetIndex
  frameHead.writeUInt8(serviceId, 4);                 // serviceId
  frameHead.writeUInt8(commandId, 5);                 // commandId
  frameHead.writeUInt16LE(0x0002, 6);                 // flag: isHost=1, ack=0
  frameHead.writeUInt16LE(tlvData.length, 8);         // length
  
  // 组装帧（不包含 CRC32）
  const frameWithoutCrc = Buffer.concat([frameHead, tlvData]);
  
  // 计算 CRC32（使用 IEEE 802.3 标准）
  const crc32Value = crc32(frameWithoutCrc);
  const crc32Buffer = Buffer.alloc(4);
  crc32Buffer.writeUInt32LE(crc32Value, 0);
  
  // 完整的帧
  const completeFrame = Buffer.concat([frameWithoutCrc, crc32Buffer]);
  
  return completeFrame;
}

/**
 * 解析 TLV 数据
 * @param {Buffer} data - TLV 数据
 * @returns {Array<{type: number, length: number, value: Buffer}>} TLV 数组
 */
function parseTLV(data) {
  const tlvArray = [];
  let index = 0;
  
  while (index < data.length) {
    if (index + 2 > data.length) break;
    
    const type = data.readUInt8(index++);
    let length;
    
    if (data[index] > 127) {
      // 长度使用两字节编码
      if (index + 2 > data.length) break;
      length = ((data[index] & 0x7F) << 8) | data[index + 1];
      index += 2;
    } else {
      // 长度使用单字节编码
      length = data.readUInt8(index++);
    }
    
    if (index + length > data.length) break;
    
    const value = data.slice(index, index + length);
    index += length;
    
    tlvArray.push({ type, length, value });
  }
  
  return tlvArray;
}

/**
 * 解析响应包
 * @param {Buffer} data - 响应数据
 * @returns {Object} 解析后的响应
 */
export function parseResponse(data) {
  if (!data || data.length < PROTOCOL.FRAME_HEAD_SIZE) {
    throw new Error('Invalid response: data too short');
  }
  
  // 解析帧头
  const head = data.readUInt8(0);
  const protocolVersion = data.readUInt8(1);
  const packetIndex = data.readUInt16LE(2);
  const serviceId = data.readUInt8(4);
  const commandId = data.readUInt8(5);
  const flag = data.readUInt16LE(6);
  const length = data.readUInt16LE(8);
  
  // 验证协议头
  if (head !== PROTOCOL.HEADER) {
    throw new Error(`Invalid protocol header: 0x${head.toString(16)}`);
  }
  
  // 提取 TLV 数据
  const tlvData = data.slice(PROTOCOL.FRAME_HEAD_SIZE, PROTOCOL.FRAME_HEAD_SIZE + length);
  
  // 解析 TLV
  const tlvArray = parseTLV(tlvData);
  
  // 查找通用结果 ACK
  const ackTlv = tlvArray.find(tlv => tlv.type === TLV_TYPE.GENERAL_RESULT_ACK);
  const success = ackTlv ? ackTlv.value.readUInt8(0) === STATUS.SUCCESS : true;
  
  return {
    success,
    head,
    protocolVersion,
    packetIndex,
    serviceId,
    commandId,
    flag: {
      ack: (flag & 0x01) !== 0,
      isHost: (flag & 0x02) !== 0,
    },
    length,
    tlvArray,
    statusMessage: success ? 'Success' : 'Failed',
    dataHex: tlvData.toString('hex')
  };
}

/**
 * 获取状态消息
 * @param {number} status - 状态码
 * @returns {string} 状态消息
 */
export function getStatusMessage(status) {
  switch (status) {
    case STATUS.SUCCESS:
      return 'Success';
    case STATUS.ERROR_INVALID_CMD:
      return 'Invalid command';
    case STATUS.ERROR_INVALID_LENGTH:
      return 'Invalid length';
    case STATUS.ERROR_INVALID_DATA:
      return 'Invalid data';
    case STATUS.ERROR_DEVICE_BUSY:
      return 'Device busy';
    case STATUS.ERROR_TIMEOUT:
      return 'Timeout';
    case STATUS.ERROR_UNKNOWN:
      return 'Unknown error';
    default:
      return `Unknown status code: 0x${status.toString(16)}`;
  }
}

/**
 * 验证公钥格式
 * @param {string} publicKey - 公钥十六进制字符串
 * @returns {Object} 验证结果
 */
export function validatePublicKey(publicKey) {
  const cleaned = publicKey.replace(/^0x/, '').replace(/\s/g, '');
  
  // 检查是否为有效的十六进制字符串
  if (!/^[0-9a-fA-F]+$/.test(cleaned)) {
    return {
      valid: false,
      error: 'Public key must be a valid hexadecimal string'
    };
  }
  
  // 检查长度
  if (cleaned.length === 66) {
    // 压缩格式 (33 bytes)
    const prefix = cleaned.substring(0, 2);
    if (prefix !== '02' && prefix !== '03') {
      return {
        valid: false,
        error: 'Compressed public key must start with 02 or 03'
      };
    }
    return {
      valid: true,
      compressed: true,
      length: 33
    };
  } else if (cleaned.length === 130) {
    // 非压缩格式 (65 bytes)
    const prefix = cleaned.substring(0, 2);
    if (prefix !== '04') {
      return {
        valid: false,
        error: 'Uncompressed public key must start with 04'
      };
    }
    return {
      valid: true,
      compressed: false,
      length: 65
    };
  } else {
    return {
      valid: false,
      error: `Invalid length: ${cleaned.length / 2} bytes. Expected 33 (compressed) or 65 (uncompressed) bytes`
    };
  }
}
