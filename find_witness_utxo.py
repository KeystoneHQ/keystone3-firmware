#!/usr/bin/env python3
"""
直接从 PSBT 十六进制中提取 witness_utxo
"""
import base64

B64_PSBT = "cHNidP8BAHECAAAAAbHe89yvTAyW2l5lwTYShPFfFRAipdpkIh9zGDuxyaVOAQAAAAD9////AugDAAAAAAAAAFgAUxUxqQYiw+3P6Y4TQrOE8J2DLoWCLLwIAAAAAABYAFEIBXnY/cYgPYxeN11qkYX/whKHTz2oOAE8BBIiyHgPXktQvgAAAAFuM36CZpo1hc9qtdIVngF0+flXs4RWVddkcae/jZQw4ArbR+AQC7rR3ZYukmxCbcmeCDd5Ipq71VZBLuZaRyXj0EC0L2rxUAACAAAAAgAAAAIAAAQEf/zMCAAAAAAAWABTPAKLos6lGO6C+brxdUNQcb7D2IQEDBAEAAAAiBgN5Xt07J+yExyy05wvTeBLoWzBF4CmElv0E2Hqaw4VwcxgtC9q8VAAAgAAAAIAAAACAAQAAACUAAAAAACICAn5IVw+xr5qxYdVYeXeigWO3o3V7/yoLN2eBLftpb3N6GC0L2rxUAACAAAAAgAAAAIABAAAAJgAAAAA="

raw = base64.b64decode(B64_PSBT)

print("PSBT 十六进制分析\n")
print(f"总长度: {len(raw)} 字节\n")

# 打印完整十六进制
hex_str = raw.hex()
for i in range(0, len(hex_str), 80):
    offset = i // 2
    chunk = hex_str[i:i+80]
    print(f"{offset:04x}: {chunk}")

print("\n" + "="*70)

# 搜索 PSBT_IN_WITNESS_UTXO marker (0x01)
# witness_utxo 格式：01 <key_data> <varint_len> <8_bytes_amount> <script>

# 在 PSBT 中查找 "01" 后面跟着金额的位置
print("\n查找 witness_utxo (key type 0x01)...\n")

# 从 PSBT 中直接搜索可能的 witness_utxo 字段
# 格式: 01 (key_type) + xx (val_len) + 8字节金额

# 手动定位：查找 0x0101 (key_len=1, key_type=0x01)
marker = b'\x01\x01'
idx = raw.find(marker)

if idx != -1:
    print(f"找到可能的 witness_utxo 在位置: {idx} (0x{idx:04x})")
    print(f"上下文: {raw[idx-5:idx+30].hex()}")
    
    # 解析
    pos = idx
    key_len = raw[pos]
    pos += 1
    key_type = raw[pos]
    pos += 1
    
    val_len = raw[pos]
    pos += 1
    
    print(f"\nkey_len: {key_len}")
    print(f"key_type: 0x{key_type:02x}")
    print(f"val_len: {val_len}")
    
    if val_len >= 8:
        amount_bytes = raw[pos:pos+8]
        amount = int.from_bytes(amount_bytes, 'little')
        script = raw[pos+8:pos+val_len]
        
        print(f"\n✅ witness_utxo 解析:")
        print(f"   金额: {amount} sats")
        print(f"   金额 (hex): {amount_bytes.hex()}")
        print(f"   脚本 ({len(script)} bytes): {script.hex()}")
        
        # 验证
        SIGNED_TX_OUTPUT = 144243
        fee = amount - SIGNED_TX_OUTPUT
        
        print(f"\n与已签名交易验证:")
        print(f"   Input: {amount} sats")
        print(f"   Output: {SIGNED_TX_OUTPUT} sats")
        print(f"   Fee: {fee} sats (~{fee/223:.1f} sat/vB)")

# 同时查找其他标记来确认结构
print(f"\n" + "="*70)
print("PSBT 字段标记查找:")

for marker_name, marker_bytes in [
    ("GLOBAL_UNSIGNED_TX (0x00)", b'\x01\x00'),
    ("IN_WITNESS_UTXO (0x01)", b'\x01\x01'),
    ("IN_PARTIAL_SIG (0x02)", b'\x01\x02'),
]:
    idx = raw.find(marker_bytes)
    if idx != -1:
        print(f"  {marker_name}: 位置 {idx} (0x{idx:04x})")
