import usb from 'usb';
import { SERVICE_ID, DEVICE_INFO_CMD, TLV_TYPE, buildPacket, parseResponse, validatePublicKey, PROTOCOL } from './protocol.js';
import { buildPublicKeyRequest, parseEAPDUResponse } from './eapdu-protocol.js';
import { DEFAULT_VENDOR_ID, DEFAULT_PRODUCT_ID } from './constants.js';
import { sha256, signData, getPublicKey } from './crypto.js';

/**
 * Keystone USB Communication Library
 * 用于与 Keystone 硬件钱包通过 USB 通信
 */
export class KeystoneUSB {
  constructor() {
    this.device = null;
    this.interface = null;
    this.endpointIn = null;
    this.endpointOut = null;
    this.connected = false;
  }

  /**
   * 列出所有可用的 USB 设备
   */
  async listDevices() {
    const devices = usb.getDeviceList();
    return devices.map(device => {
      let manufacturer = 'Unknown';
      let product = 'Unknown';
      
      try {
        device.open();
        try {
          if (device.deviceDescriptor.iManufacturer) {
            manufacturer = device.getStringDescriptor(device.deviceDescriptor.iManufacturer);
          }
        } catch (e) {
          // 忽略获取制造商名称失败
        }
        try {
          if (device.deviceDescriptor.iProduct) {
            product = device.getStringDescriptor(device.deviceDescriptor.iProduct);
          }
        } catch (e) {
          // 忽略获取产品名称失败
        }
        device.close();
      } catch (error) {
        // 无法打开设备，使用默认值
      }
      
      return {
        vendorId: device.deviceDescriptor.idVendor,
        productId: device.deviceDescriptor.idProduct,
        manufacturer,
        product
      };
    });
  }

  /**
   * 连接到 Keystone 设备
   * @param {number} vendorId - USB Vendor ID
   * @param {number} productId - USB Product ID
   */
  async connect(vendorId = DEFAULT_VENDOR_ID, productId = DEFAULT_PRODUCT_ID) {
    // 查找设备
    this.device = usb.findByIds(vendorId, productId);
    
    if (!this.device) {
      throw new Error(`Device not found (VID: 0x${vendorId.toString(16)}, PID: 0x${productId.toString(16)})`);
    }

    try {
      // 打开设备
      this.device.open();

      // 获取第一个接口
      this.interface = this.device.interface(0);

      // 如果接口被内核驱动占用，detach 它（仅在非 Windows 系统）
      // Windows 不支持 detachKernelDriver，使用 WinUSB 时不需要
      if (process.platform !== 'win32') {
        try {
          if (this.interface.isKernelDriverActive()) {
            this.interface.detachKernelDriver();
          }
        } catch (error) {
          // 忽略不支持的操作
          console.warn('Kernel driver detach not supported on this platform');
        }
      }

      // 声明接口
      this.interface.claim();

      // 查找端点
      const endpoints = this.interface.endpoints;
      
      // 找到 IN 和 OUT 端点
      this.endpointIn = endpoints.find(ep => ep.direction === 'in');
      this.endpointOut = endpoints.find(ep => ep.direction === 'out');

      if (!this.endpointIn || !this.endpointOut) {
        throw new Error('Could not find required endpoints');
      }

      return true;
    } catch (error) {
      throw new Error(`Failed to connect to device: ${error.message}`);
    }
  }

  /**
   * 获取设备信息
   */
  getDeviceInfo() {
    if (!this.device) {
      throw new Error('Device not connected');
    }

    const descriptor = this.device.deviceDescriptor;
    
    return {
      vendorId: descriptor.idVendor,
      productId: descriptor.idProduct,
      manufacturer: this.device.getStringDescriptor(descriptor.iManufacturer) || 'Unknown',
      product: this.device.getStringDescriptor(descriptor.iProduct) || 'Unknown',
      serialNumber: descriptor.iSerialNumber 
        ? this.device.getStringDescriptor(descriptor.iSerialNumber) 
        : 'Unknown'
    };
  }

  /**
   * 发送 K1 公钥到设备（使用 EAPDU 协议）
   * @param {string|Buffer} publicKey - 公钥的十六进制字符串或 Buffer
   * @param {string|Buffer} privateKey - 私钥用于签名（十六进制字符串或 Buffer）
   */
  async sendPublicKey(publicKey, privateKey) {
    if (!this.device || !this.endpointOut) {
      throw new Error('Device not connected');
    }

    // 处理公钥
    let pubKeyBuffer;
    if (typeof publicKey === 'string') {
      const cleanKey = publicKey.replace(/^0x/, '').replace(/\s/g, '');
      const validation = validatePublicKey(cleanKey);
      if (!validation.valid) {
        throw new Error(validation.error);
      }
      pubKeyBuffer = Buffer.from(cleanKey, 'hex');
    } else {
      pubKeyBuffer = publicKey;
    }

    // 处理私钥
    let privKeyBuffer;
    if (typeof privateKey === 'string') {
      const cleanPriv = privateKey.replace(/^0x/, '').replace(/\s/g, '');
      if (cleanPriv.length !== 64) {
        throw new Error('Private key must be 32 bytes (64 hex characters)');
      }
      privKeyBuffer = Buffer.from(cleanPriv, 'hex');
    } else {
      privKeyBuffer = privateKey;
    }

    // 验证公钥和私钥匹配
    const derivedPubKey = getPublicKey(privKeyBuffer, pubKeyBuffer.length === 33);
    if (!derivedPubKey.equals(pubKeyBuffer)) {
      throw new Error('Public key does not match private key');
    }

    // 对公钥进行签名（签名公钥的 SHA256 哈希）
    const pubKeyHash = sha256(pubKeyBuffer);
    const signature = signData(pubKeyHash, privKeyBuffer);

    console.log(`\nPreparing to send public key with signature...`);
    console.log(`  Public key: ${pubKeyBuffer.toString('hex')}`);
    console.log(`  Signature: ${signature.toString('hex')}`);

    // 构建 EAPDU 协议包
    const packets = buildPublicKeyRequest(pubKeyBuffer.toString('hex'), signature.toString('hex'));
    
    console.log(`\nSending ${packets.length} EAPDU packet(s)...`);
    
    // 发送所有数据包
    for (let i = 0; i < packets.length; i++) {
      console.log(`  Packet ${i + 1}/${packets.length}: ${packets[i].length} bytes`);
      console.log(`  Packet data: ${packets[i].slice(0, 20).toString('hex')}...`);
      await this.sendRaw(packets[i]);
      
      // 在包之间添加小延迟
      if (i < packets.length - 1) {
        await new Promise(resolve => setTimeout(resolve, 50)); // 增加延迟到50ms
      }
    }
    
    console.log(`\nAll packets sent, waiting for device response...`);
    
    // 读取响应（可能是多包）
    return await this.readEAPDUResponse(10000); // 增加超时到10秒
  }

  /**
   * 读取 EAPDU 响应
   */
  async readEAPDUResponse(timeout = PROTOCOL.DEFAULT_TIMEOUT) {
    if (!this.endpointIn) {
      throw new Error('Device not connected');
    }

    console.log(`\nReading device response (timeout: ${timeout}ms)...`);
    
    // 读取第一个包以确定总包数
    const firstPacket = await this.readRaw(64, timeout);
    console.log(`  First packet received: ${firstPacket.length} bytes`);
    console.log(`  First packet data: ${firstPacket.slice(0, 20).toString('hex')}...`);
    
    const firstResponse = parseEAPDUResponse(firstPacket);
    
    console.log(`\nReceived EAPDU response:`);
    console.log(`  Total packets: ${firstResponse.totalPackets}`);
    console.log(`  Status: ${firstResponse.statusMessage} (0x${firstResponse.status.toString(16)})`);
    
    // 如果只有一个包，直接返回
    if (firstResponse.totalPackets === 1) {
      return {
        success: firstResponse.success,
        status: firstResponse.status,
        statusMessage: firstResponse.statusMessage,
        commandType: firstResponse.commandType,
        requestId: firstResponse.requestId,
        payload: firstResponse.payload,
        payloadHex: firstResponse.payload.toString('hex'),
      };
    }
    
    // 读取剩余的包
    const allPackets = [firstPacket];
    for (let i = 1; i < firstResponse.totalPackets; i++) {
      const packet = await this.readRaw(64, timeout);
      allPackets.push(packet);
      await new Promise(resolve => setTimeout(resolve, 10));
    }
    
    // 合并所有包的payload
    const allResponses = allPackets.map(packet => parseEAPDUResponse(packet));
    const combinedPayload = Buffer.concat(allResponses.map(r => r.payload));
    
    return {
      success: firstResponse.success,
      status: firstResponse.status,
      statusMessage: firstResponse.statusMessage,
      commandType: firstResponse.commandType,
      requestId: firstResponse.requestId,
      payload: combinedPayload,
      payloadHex: combinedPayload.toString('hex'),
      totalPackets: firstResponse.totalPackets,
    };
  }

  /**
   * 获取设备版本
   */
  async getVersion() {
    if (!this.device || !this.endpointOut) {
      throw new Error('Device not connected');
    }

    const packet = buildPacket(SERVICE_ID.DEVICE_INFO, DEVICE_INFO_CMD.BASIC, []);
    await this.sendRaw(packet);
    const response = await this.readResponse();
    
    if (response.success && response.tlvArray) {
      // 查找固件版本 TLV
      const versionTlv = response.tlvArray.find(tlv => tlv.type === TLV_TYPE.DEVICE_FIRMWARE_VERSION);
      if (versionTlv && versionTlv.value) {
        const versionStr = versionTlv.value.toString('utf8').replace(/\0/g, '');
        const parts = versionStr.split('.');
        if (parts.length >= 3) {
          return {
            major: parseInt(parts[0]) || 0,
            minor: parseInt(parts[1]) || 0,
            patch: parseInt(parts[2]) || 0,
            version: versionStr
          };
        }
      }
    }
    
    return null;
  }

  /**
   * 获取设备状态
   */
  async getStatus() {
    if (!this.device || !this.endpointOut) {
      throw new Error('Device not connected');
    }

    const packet = buildPacket(SERVICE_ID.DEVICE_INFO, DEVICE_INFO_CMD.BASIC, []);
    await this.sendRaw(packet);
    return await this.readResponse();
  }

  /**
   * 重置设备
   */
  async reset() {
    if (!this.device || !this.endpointOut) {
      throw new Error('Device not connected');
    }

    // 重置命令可能需要特定的服务和命令 ID
    // 这里先使用占位实现
    const packet = buildPacket(SERVICE_ID.DEVICE_INFO, 0xFF, []);
    await this.sendRaw(packet);
    return await this.readResponse();
  }

  /**
   * 读取设备响应
   */
  async readResponse(timeout = PROTOCOL.DEFAULT_TIMEOUT) {
    if (!this.endpointIn) {
      throw new Error('Device not connected');
    }

    return new Promise((resolve, reject) => {
      const timeoutId = setTimeout(() => {
        reject(new Error('Read timeout'));
      }, timeout);

      this.endpointIn.transfer(PROTOCOL.MAX_PACKET_SIZE, (error, data) => {
        clearTimeout(timeoutId);
        
        if (error) {
          reject(new Error(`Failed to read response: ${error.message}`));
          return;
        }

        try {
          // 使用协议模块解析响应
          const response = parseResponse(data);
          resolve(response);
        } catch (parseError) {
          reject(new Error(`Failed to parse response: ${parseError.message}`));
        }
      });
    });
  }

  /**
   * 发送原始数据
   * @param {Buffer} data - 要发送的数据
   */
  async sendRaw(data) {
    if (!this.device || !this.endpointOut) {
      throw new Error('Device not connected');
    }

    return new Promise((resolve, reject) => {
      this.endpointOut.transfer(data, (error) => {
        if (error) {
          reject(new Error(`Failed to send data: ${error.message}`));
          return;
        }
        resolve(true);
      });
    });
  }

  /**
   * 读取原始数据
   * @param {number} length - 要读取的字节数
   * @param {number} timeout - 超时时间（毫秒）
   */
  async readRaw(length = 64, timeout = 5000) {
    if (!this.endpointIn) {
      throw new Error('Device not connected');
    }

    return new Promise((resolve, reject) => {
      const timeoutId = setTimeout(() => {
        reject(new Error('Read timeout'));
      }, timeout);

      this.endpointIn.transfer(length, (error, data) => {
        clearTimeout(timeoutId);
        
        if (error) {
          reject(new Error(`Failed to read data: ${error.message}`));
          return;
        }

        resolve(data);
      });
    });
  }

  /**
   * 断开设备连接
   */
  async disconnect() {
    if (this.interface) {
      try {
        // 等待 interface release 完成后再关闭设备
        await new Promise((resolve) => {
          this.interface.release(true, (error) => {
            if (error) {
              console.error('Error releasing interface:', error);
            }
            resolve();
          });
        });
      } catch (error) {
        console.error('Error during disconnect:', error);
      }
    }

    if (this.device) {
      try {
        this.device.close();
      } catch (error) {
        console.error('Error closing device:', error);
      }
    }

    this.device = null;
    this.interface = null;
    this.endpointIn = null;
    this.endpointOut = null;
  }
}
