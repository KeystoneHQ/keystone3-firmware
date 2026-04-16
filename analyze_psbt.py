#!/usr/bin/env python3
import base64

b64 = "cHNidP8BAHECAAAAAbHe89yvTAyW2l5lwTYShPFfFRAipdpkIh9zGDuxyaVOAQAAAAD9////AugDAAAAAAAAAFgAUxUxqQYiw+3P6Y4TQrOE8J2DLoWCLLwIAAAAAABYAFEIBXnY/cYgPYxeN11qkYX/whKHTz2oOAE8BBIiyHgPXktQvgAAAAFuM36CZpo1hc9qtdIVngF0+flXs4RWVddkcae/jZQw4ArbR+AQC7rR3ZYukmxCbcmeCDd5Ipq71VZBLuZaRyXj0EC0L2rxUAACAAAAAgAAAAIAAAQEf/zMCAAAAAAAWABTPAKLos6lGO6C+brxdUNQcb7D2IQEDBAEAAAAiBgN5Xt07J+yExyy05wvTeBLoWzBF4CmElv0E2Hqaw4VwcxgtC9q8VAAAgAAAAIAAAACAAQAAACUAAAAAACICAn5IVw+xr5qxYdVYeXeigWO3o3V7/yoLN2eBLftpb3N6GC0L2rxUAACAAAAAgAAAAIABAAAAJgAAAAA="
raw = base64.b64decode(b64)

print("=" * 60)
print("PSBT 结构分析（白盒测试）")
print("=" * 60)

if raw[:5] != b"psbt\xff":
    print("❌ 不是有效的 PSBT")
    exit(1)

print(f"\n✓ PSBT 魔数正确: psbt\\xff")
print(f"✓ PSBT 总长度: {len(raw)} 字节")

def read_varint(data, pos):
    """读取 Bitcoin 可变长度整数"""
    first = data[pos]
    if first < 0xfd:
        return first, pos + 1
    elif first == 0xfd:
        return int.from_bytes(data[pos+1:pos+3], 'little'), pos + 3
    elif first == 0xfe:
        return int.from_bytes(data[pos+1:pos+5], 'little'), pos + 5
    else:
        return int.from_bytes(data[pos+1:pos+9], 'little'), pos + 9

def read_pairs(data, pos):
    """读取 PSBT key-value 对"""
    pairs = []
    while pos < len(data):
        key_len, pos = read_varint(data, pos)
        if key_len == 0:  # separator
            break
        key = data[pos:pos+key_len]
        pos += key_len
        val_len, pos = read_varint(data, pos)
        val = data[pos:pos+val_len]
        pos += val_len
        pairs.append((key, val))
    return pairs, pos

# 解析 Global map
print("\n[1] 解析 Global Map...")
global_pairs, pos = read_pairs(raw, 5)
print(f"    Global key-value 对数量: {len(global_pairs)}")
print(f"    Global map 结束位置: {pos}")
for i, (key, val) in enumerate(global_pairs):
    print(f"    Pair {i}: key_type=0x{key[0]:02x}, key_len={len(key)}, val_len={len(val)}")

# 提取 unsigned transaction
unsigned_tx = None
for key, val in global_pairs:
    if len(key) >= 1 and key[0] == 0x00:  # PSBT_GLOBAL_UNSIGNED_TX
        unsigned_tx = val
        break

if unsigned_tx:
    print(f"    ✓ Unsigned transaction 长度: {len(unsigned_tx)} 字节")
    
    # 解析 tx 基本结构
    tx_pos = 4  # skip version (4 bytes)
    input_count, tx_pos = read_varint(unsigned_tx, tx_pos)
    print(f"    ✓ 输入数量: {input_count}")
    
    # 读取第一个 input 的 outpoint
    if input_count > 0:
        prev_txid = unsigned_tx[tx_pos:tx_pos+32]
        prev_vout = int.from_bytes(unsigned_tx[tx_pos+32:tx_pos+36], 'little')
        txid_hex = prev_txid[::-1].hex()
        print(f"\n    Input #0 Outpoint:")
        print(f"      txid: {txid_hex}")
        print(f"      vout: {prev_vout}")

# 解析第一个 Input map
print(f"\n[2] 解析 Input #0 Map...")
print(f"    当前位置: {pos}, 接下来 20 字节: {raw[pos:pos+20].hex()}")
# pos 现在在 global map 结尾的分隔符 0x00 后，继续读取下一个 map
input_pairs, pos_after = read_pairs(raw, pos)
print(f"    Input #0 key-value 对数量: {len(input_pairs)}")
print(f"    解析后位置: {pos_after}")

has_witness_utxo = False
has_non_witness_utxo = False
witness_value = None

for key, val in input_pairs:
    if len(key) >= 1:
        key_type = key[0]
        
        # PSBT_IN_NON_WITNESS_UTXO = 0x00
        if key_type == 0x00:
            has_non_witness_utxo = True
            print(f"\n    ✓ 发现 non_witness_utxo (type=0x00)")
            print(f"      完整前序交易长度: {len(val)} 字节")
        
        # PSBT_IN_WITNESS_UTXO = 0x01
        elif key_type == 0x01:
            has_witness_utxo = True
            if len(val) >= 8:
                witness_value = int.from_bytes(val[:8], 'little')
                script_len = len(val[8:])
                print(f"\n    ✓ 发现 witness_utxo (type=0x01)")
                print(f"      金额: {witness_value} 聪")
                print(f"      ScriptPubKey 长度: {script_len} 字节")

print(f"\n{'='*60}")
print("[3] PSBT 结构总结")
print(f"{'='*60}\n")

print(f"witness_utxo 存在:     {has_witness_utxo}")
print(f"non_witness_utxo 存在: {has_non_witness_utxo}")
if witness_value:
    print(f"witness_utxo 金额:     {witness_value} 聪")

# 固件检测逻辑 (mirrors parsed_psbt.rs line 33-35)
has_witness_only_inputs = has_witness_utxo and not has_non_witness_utxo

print(f"\n{'='*60}")
print("[4] 固件检测结果")
print(f"{'='*60}\n")

print(f"has_witness_only_inputs = {has_witness_only_inputs}")
print()

if has_witness_only_inputs:
    print("✅ 设备 UI 应该显示以下内容:")
    print()
    print("   [交易总览页顶部]")
    print("   • 橙色提示卡: \"Check input value\"")
    print("     - 中文: \"请检查输入值\"")
    print("     - 提示用户此 PSBT 只有 witness_utxo")
    print()
    print("   [交易详情页 - From 部分]")
    print("   • 每个输入下方显示 \"Input Reference\" 链接")
    print("   • 点击后弹出 QR 码，内容为:")
    print("     - Outpoint: 4ea5c9b13b18731f2224da52225114f15f841216c15e96da968c4ccfdef3deb1:1")
    print("     - 浏览器链接: https://mempool.space/tx/4ea5c9b1...#vout=1")
    print()
    print("📌 触发原因:")
    print("   这笔 PSBT 只提供了 witness_utxo (150252 聪)，")
    print("   没有提供 non_witness_utxo (完整前序交易)。")
    print()
    print("   设备无法独立验证这个输入的真实金额是否为 150252 聪。")
    print("   软件钱包可能提供了错误的金额，导致用户支付过高的手续费。")
    print()
    print("🔍 测试验证步骤:")
    print("   1. 在设备上导入此 PSBT")
    print("   2. 检查总览页是否显示橙色 \"Check input value\" 提示")
    print("   3. 进入详情页，检查输入下方是否有 \"Input Reference\" 链接")
    print("   4. 点击链接，扫描 QR 码")
    print("   5. 在浏览器中验证 UTXO 金额是否确实为 150252 聪")
else:
    print("❌ 设备 UI 不应该显示 witness-only 提示")
    print()
    if has_non_witness_utxo:
        print("原因: 有完整的 non_witness_utxo，设备可以独立验证金额")
    else:
        print("原因: PSBT 格式异常")

print(f"\n{'='*60}\n")
