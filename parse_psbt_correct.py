#!/usr/bin/env python3
"""
正确解析 PSBT 的 witness_utxo 金额
"""
import base64

B64_PSBT = "cHNidP8BAHECAAAAAbHe89yvTAyW2l5lwTYShPFfFRAipdpkIh9zGDuxyaVOAQAAAAD9////AugDAAAAAAAAAFgAUxUxqQYiw+3P6Y4TQrOE8J2DLoWCLLwIAAAAAABYAFEIBXnY/cYgPYxeN11qkYX/whKHTz2oOAE8BBIiyHgPXktQvgAAAAFuM36CZpo1hc9qtdIVngF0+flXs4RWVddkcae/jZQw4ArbR+AQC7rR3ZYukmxCbcmeCDd5Ipq71VZBLuZaRyXj0EC0L2rxUAACAAAAAgAAAAIAAAQEf/zMCAAAAAAAWABTPAKLos6lGO6C+brxdUNQcb7D2IQEDBAEAAAAiBgN5Xt07J+yExyy05wvTeBLoWzBF4CmElv0E2Hqaw4VwcxgtC9q8VAAAgAAAAIAAAACAAQAAACUAAAAAACICAn5IVw+xr5qxYdVYeXeigWO3o3V7/yoLN2eBLftpb3N6GC0L2rxUAACAAAAAgAAAAIABAAAAJgAAAAA="

raw = base64.b64decode(B64_PSBT)

def read_vi(d, pos):
    b = d[pos]
    if b < 0xfd:
        return b, pos + 1
    if b == 0xfd:
        return int.from_bytes(d[pos+1:pos+3], 'little'), pos + 3
    if b == 0xfe:
        return int.from_bytes(d[pos+1:pos+5], 'little'), pos + 5
    return int.from_bytes(d[pos+1:pos+9], 'little'), pos + 9

print("=" * 70)
print("重新解析 PSBT - 正确提取 witness_utxo")
print("=" * 70)

# Skip magic
pos = 5

# Skip global map
while raw[pos] != 0x00:
    klen, pos = read_vi(raw, pos)
    pos += klen
    vlen, pos = read_vi(raw, pos)
    pos += vlen
pos += 1

print(f"\nInput map 从位置 {pos} 开始\n")

# Parse input map 正确解析每个字段
witness_utxo_found = False
witness_utxo_value = None

while pos < len(raw) and raw[pos] != 0x00:
    klen, pos = read_vi(raw, pos)
    if klen == 0:
        break
    
    key_start = pos
    key_type = raw[pos]
    pos += klen
    
    vlen, pos = read_vi(raw, pos)
    val_start = pos
    val = raw[pos:pos+vlen]
    pos += vlen
    
    print(f"Key type: 0x{key_type:02x}, key_len: {klen}, val_len: {vlen}")
    
    if key_type == 0x01:  # PSBT_IN_WITNESS_UTXO
        witness_utxo_found = True
        # witness_utxo = TxOut: amount (8 bytes) + script
        if vlen >= 8:
            witness_utxo_value = int.from_bytes(val[:8], 'little')
            script = val[8:]
            print(f"  ✓ witness_utxo:")
            print(f"    Amount: {witness_utxo_value} sats")
            print(f"    Script ({len(script)} bytes): {script.hex()}")
    elif key_type == 0x00:
        print(f"  ✓ non_witness_utxo: {vlen} bytes")
    else:
        print(f"  (other field)")

print(f"\n{'='*70}")
print("结果")
print(f"{'='*70}\n")

if witness_utxo_value:
    print(f"✅ witness_utxo 金额: {witness_utxo_value} sats")
    
    # 与签名交易对比
    SIGNED_TX_TOTAL_OUTPUT = 144243
    fee = witness_utxo_value - SIGNED_TX_TOTAL_OUTPUT
    
    print(f"\n与已签名交易对比:")
    print(f"  Input (witness_utxo): {witness_utxo_value} sats")
    print(f"  Output (已签名交易): {SIGNED_TX_TOTAL_OUTPUT} sats")
    print(f"  Fee: {fee} sats")
    print(f"  Fee rate: ~{fee/223:.2f} sats/vbyte")
    
    if witness_utxo_value == 144383:
        print(f"\n✅ 确认：witness_utxo 金额 = 144383 sats（与用户数据一致）")
else:
    print("❌ 未找到 witness_utxo")

print(f"\n{'='*70}\n")
