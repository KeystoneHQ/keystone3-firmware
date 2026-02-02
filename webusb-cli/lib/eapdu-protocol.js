/**
 * EAPDU Protocol Implementation
 * Based on src/webusb_protocol/general/eapdu_protocol_parser.c
 */

// Command Types
export const CMD_TYPE = {
  ECHO_TEST: 0x00000001,
  RESOLVE_UR: 0x00000002,
  CHECK_LOCK_STATUS: 0x00000003,
  EXPORT_ADDRESS: 0x00000004,
  GET_DEVICE_INFO: 0x00000005,
  GET_DEVICE_USB_PUBKEY: 0x00000006,
};

// Status Codes
export const STATUS = {
  SUCCESS: 0x00000000,
  FAILURE: 0x00000001,
  INVALID_TOTAL_PACKETS: 0x00000002,
  INVALID_INDEX: 0x00000003,
};

// Protocol Constants
export const EAPDU = {
  HEADER: 0x00,           // CLA field
  MAX_PACKET_LENGTH: 64,  // Maximum packet size
  OFFSET_CLA: 0,
  OFFSET_INS: 1,          // Command type (2 bytes)
  OFFSET_P1: 3,           // Total packets (2 bytes)
  OFFSET_P2: 5,           // Packet index (2 bytes)
  OFFSET_LC: 7,           // Request ID (2 bytes)
  OFFSET_CDATA: 9,        // Data starts here
  RESPONSE_STATUS_LENGTH: 2,
};

/**
 * Insert 16-bit value into buffer (big-endian)
 */
function insert16BitValue(buffer, offset, value) {
  buffer.writeUInt16BE(value, offset);
}

/**
 * Extract 16-bit value from buffer (big-endian)
 */
function extract16BitValue(buffer, offset) {
  return buffer.readUInt16BE(offset);
}

/**
 * Build EAPDU packet(s) for sending data
 * @param {number} commandType - Command type from CMD_TYPE
 * @param {Buffer} data - Data to send
 * @param {number} requestId - Request ID (default: 0)
 * @returns {Array<Buffer>} Array of packets to send
 */
export function buildEAPDUPackets(commandType, data, requestId = 0) {
  const maxDataPerPacket = EAPDU.MAX_PACKET_LENGTH - EAPDU.OFFSET_CDATA;
  const dataLen = data ? data.length : 0;
  const totalPackets = Math.ceil(dataLen / maxDataPerPacket) || 1;
  
  const packets = [];
  let offset = 0;
  
  for (let packetIndex = 0; packetIndex < totalPackets; packetIndex++) {
    const packet = Buffer.alloc(EAPDU.MAX_PACKET_LENGTH);
    const remainingData = dataLen - offset;
    const packetDataSize = Math.min(remainingData, maxDataPerPacket);
    
    // Build EAPDU frame
    packet[EAPDU.OFFSET_CLA] = EAPDU.HEADER;                    // CLA = 0x00
    insert16BitValue(packet, EAPDU.OFFSET_INS, commandType);    // INS = command type
    insert16BitValue(packet, EAPDU.OFFSET_P1, totalPackets);    // P1 = total packets
    insert16BitValue(packet, EAPDU.OFFSET_P2, packetIndex);     // P2 = packet index
    insert16BitValue(packet, EAPDU.OFFSET_LC, requestId);       // LC = request ID
    
    // Copy data if available
    if (data && packetDataSize > 0) {
      data.copy(packet, EAPDU.OFFSET_CDATA, offset, offset + packetDataSize);
      offset += packetDataSize;
    }
    
    // Trim packet to actual size
    const actualSize = EAPDU.OFFSET_CDATA + packetDataSize;
    packets.push(packet.slice(0, actualSize));
  }
  
  return packets;
}

/**
 * Parse EAPDU response packet
 * @param {Buffer} data - Response data
 * @returns {Object} Parsed response
 */
export function parseEAPDUResponse(data) {
  if (!data || data.length < EAPDU.OFFSET_CDATA + EAPDU.RESPONSE_STATUS_LENGTH) {
    throw new Error('Invalid EAPDU response: data too short');
  }
  
  const cla = data[EAPDU.OFFSET_CLA];
  const commandType = extract16BitValue(data, EAPDU.OFFSET_INS);
  const totalPackets = extract16BitValue(data, EAPDU.OFFSET_P1);
  const packetIndex = extract16BitValue(data, EAPDU.OFFSET_P2);
  const requestId = extract16BitValue(data, EAPDU.OFFSET_LC);
  
  // Extract payload data (everything except the last 2 bytes which are status)
  const payloadLength = data.length - EAPDU.OFFSET_CDATA - EAPDU.RESPONSE_STATUS_LENGTH;
  const payload = data.slice(EAPDU.OFFSET_CDATA, EAPDU.OFFSET_CDATA + payloadLength);
  
  // Extract status from last 2 bytes
  const status = extract16BitValue(data, data.length - EAPDU.RESPONSE_STATUS_LENGTH);
  
  // Check if status indicates success (include SET_PUBKEY success codes)
  const isSuccess = status === STATUS.SUCCESS || 
                    status === 0x16 ||  // PRS_SET_PUBKEY_VERIFY_SUCCESS
                    status === 0x17;   // PRS_SET_PUBKEY_SET_SUCCESS
  
  return {
    cla,
    commandType,
    totalPackets,
    packetIndex,
    requestId,
    payload,
    status,
    success: isSuccess,
    statusMessage: getStatusMessage(status),
  };
}

/**
 * Parse multiple EAPDU response packets and combine them
 * @param {Array<Buffer>} packets - Array of response packets
 * @returns {Object} Combined response
 */
export function parseEAPDUMultiPacketResponse(packets) {
  if (!packets || packets.length === 0) {
    throw new Error('No packets to parse');
  }
  
  // Parse first packet to get metadata
  const firstPacket = parseEAPDUResponse(packets[0]);
  
  if (packets.length !== firstPacket.totalPackets) {
    throw new Error(`Expected ${firstPacket.totalPackets} packets, got ${packets.length}`);
  }
  
  // Combine all payload data
  const combinedPayload = Buffer.concat(packets.map(packet => parseEAPDUResponse(packet).payload));
  
  return {
    ...firstPacket,
    payload: combinedPayload,
    totalPackets: firstPacket.totalPackets,
  };
}

/**
 * Get status message for status code
 * @param {number} status - Status code
 * @returns {string} Status message
 */
export function getStatusMessage(status) {
  // Status code enum from eapdu_protocol_parser.h
  const statusMap = {
    0x00000000: 'Success',
    0x00000001: 'Failure',
    0x00000002: 'Invalid total packets',
    0x00000003: 'Invalid packet index',
    0x00000004: 'Parsing rejected',
    0x00000005: 'Parsing error',
    0x00000006: 'Parsing disallowed',
    0x00000007: 'Parsing unmatched',
    0x00000008: 'Parsing mismatched wallet',
    0x00000009: 'Parsing verify password error',
    0x0000000A: 'Export address unsupported chain',
    0x0000000B: 'Export address invalid params',
    0x0000000C: 'Export address error',
    0x0000000D: 'Export address disallowed',
    0x0000000E: 'Export address rejected',
    0x0000000F: 'Export address busy',
    0x00000010: 'Hardware call success',
    // PRS_SET_PUBKEY_* codes (17-23)
    0x00000011: 'Set pubkey error',
    0x00000012: 'Set pubkey rejected',
    0x00000013: 'Set pubkey busy',
    0x00000014: 'Set pubkey invalid params',
    0x00000015: 'Set pubkey verify failed',
    0x00000016: 'Set pubkey verify success',
    0x00000017: 'Set pubkey set success',
  };
  
  return statusMap[status] || `Unknown status: 0x${status.toString(16)}`;
}

/**
 * Build public key transfer request with signature
 * @param {string} publicKey - Public key in hex format (33 or 65 bytes)
 * @param {string} signature - Signature in hex format (64 bytes)
 * @returns {Array<Buffer>} Array of EAPDU packets
 */
export function buildPublicKeyRequest(publicKey, signature) {
  // Clean the inputs
  const cleanKey = publicKey.replace(/^0x/, '').replace(/\s/g, '');
  const cleanSig = signature.replace(/^0x/, '').replace(/\s/g, '');
  
  const keyBuffer = Buffer.from(cleanKey, 'hex');
  const sigBuffer = Buffer.from(cleanSig, 'hex');
  
  if (keyBuffer.length !== 33 && keyBuffer.length !== 65) {
    throw new Error('Public key must be 33 (compressed) or 65 (uncompressed) bytes');
  }
  
  if (sigBuffer.length !== 64) {
    throw new Error('Signature must be 64 bytes');
  }
  
  // Combine public key and signature
  // Format: [pubkey_length(1byte)] + [pubkey] + [signature(64bytes)]
  const dataLen = 1 + keyBuffer.length + sigBuffer.length;
  const combinedData = Buffer.alloc(dataLen);
  let offset = 0;
  
  // Write public key length
  combinedData.writeUInt8(keyBuffer.length, offset);
  offset += 1;
  
  // Write public key
  keyBuffer.copy(combinedData, offset);
  offset += keyBuffer.length;
  
  // Write signature
  sigBuffer.copy(combinedData, offset);
  
  // Build EAPDU packets with CMD_GET_DEVICE_USB_PUBKEY command
  return buildEAPDUPackets(CMD_TYPE.GET_DEVICE_USB_PUBKEY, combinedData);
}
