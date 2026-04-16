#!/usr/bin/env python3
"""
PSBT witness-only 检测工具
用于白盒测试：解析用户提供的 PSBT，判断是否触发 witness-only 提示
"""
import base64

# 你提供的 PSBT b64 
B64_PSBT = "cHNidP8BAHECAAAAAbHe89yvTAyW2l5lwTYShPFfFRAipdpkIh9zGDuxyaVOAQAAAAD9////AugDAAAAAAAAAFgAUxUxqQYiw+3P6Y4TQrOE8J2DLoWCLLwIAAAAAABYAFEIBXnY/cYgPYxeN11qkYX/whKHTz2oOAE8BBIiyHgPXktQvgAAAAFuM36CZpo1hc9qtdIVngF0+flXs4RWVddkcae/jZQw4ArbR+AQC7rR3ZYukmxCbcmeCDd5Ipq71VZBLuZaRyXj0EC0L2rxUAACAAAAAgAAAAIAAAQEf/zMCAAAAAAAWABTPAKLos6lGO6C+brxdUNQcb7D2IQEDBAEAAAAiBgN5Xt07J+yExyy05wvTeBLoWzBF4CmElv0E2Hqaw4VwcxgtC9q8VAAAgAAAAIAAAACAAQAAACUAAAAAACICAn5IVw+xr5qxYdVYeXeigWO3o3V7/yoLN2eBLftpb3N6GC0L2rxUAACAAAAAgAAAAIABAAAAJgAAAAA="

raw = base64.b64decode(B64_PSBT)

def read_vi(d, pos):
    """读取 varint"""
    b = d[pos]
    if b < 0xfd:
        return b, pos + 1
    if b == 0xfd:
        return int.from_bytes(d[pos+1:pos+3], 'little'), pos + 3
    if b == 0xfe:
        return int.from_bytes(d[pos+1:pos+5], 'little'), pos + 5
    return int.from_bytes(d[pos+1:pos+9], 'little'), pos + 9

print("=" * 70)
print("PSBT Witness-Only 检测（白盒测试）")
print("=" * 70)

# 检查魔数
if raw[:5] != b'psbt\xff':
    print("❌ 非法 PSBT")
    exit(1)

print(f"\n✓ PS BT 有效 (长度 {len(raw)} 字节)\n")

# 跳过 global map
pos = 5
while raw[pos] != 0x00:  # 读到分隔符为止
    klen, pos = read_vi(raw, pos)
    pos += klen  # skip key
    vlen, pos = read_vi(raw, pos)
    pos += vlen  # skip value

pos += 1  # 跳过分隔符 0x00

print(f"Input #0 map 从 pos {pos} 开始\n")

# 解析 input map
has_wit = False
has_non_wit = False
wit_val = None

while pos < len(raw) and raw[pos] != 0x00:
    klen, pos = read_vi(raw, pos)
    if klen == 0:
        break
    
    key_type = raw[pos]
    pos += klen
    
    vlen, pos = read_vi(raw, pos)
    val = raw[pos:pos+vlen]
    pos += vlen
    
    if key_type == 0x00:  # non_witness_utxo
        has_non_wit = True
        print(f"✓ non_witness_utxo: {vlen} 字节")
    elif key_type == 0x01:  # witness_utxo
        has_wit = True
        wit_val = int.from_bytes(val[:8], 'little')
        print(f"✓ witness_utxo: {wit_val} 聪\n")

print("=" * 70)
print("结果")
print("=" * 70)

# 固件逻辑 (parsed_psbt.rs line 33-35)
witness_only = has_wit and not has_non_wit

print(f"\nwitness_utxo: {has_wit}")
print(f"non_witness_utxo: {has_non_wit}")
print(f"\n→ has_witness_only_inputs = {witness_only}\n")

if witness_only:
    print("✅ 设备应显示:")
    print("   • 顶部橙色 \"Check input value\" 提示卡")
    print("   • 输入详情下方的 \"Input Reference\" QR 链接\n")
    print(f"   witness_utxo 金额: {wit_val} 聪\n")
    print("原因: 软件钱包只提供了 witness_utxo，设备无法独立验证金额正确性")
else:
    print("❌ 设备不应显示 witness-only 提示\n")

print("=" * 70)
