#!/usr/bin/env python3
"""
解析 PSBT 并分析 fee 计算和 sighash 类型
"""
import base64

B64_PSBT = "cHNidP8BAH4CAAAAAhO8hw9667PUR6oBgkBTIUfu1M4meBrOELK0zZ8dS7QVAQAAAAD9////Ec39mI8KIHkdQNVyqgtwx4dKiXYzM9LDwbmATbVnvUwAAAAAAP3///8B9zECAAAAAAAZdqkUvZ7ApUgugFlIRtMzUrodP53HGGqIrOdqDgBPAQSIsh4D15LUL4AAAABbjN+gmaaNYXParXSFZ4BdPn5V7OEVlXXZHGnv42UMOAK20fgEAu60d2WLpJsQm3Jngg3eSKau9VWQS7mWkcl49BAtC9q8VAAAgAAAAIAAAACAAAEBHyADAAAAAAAAFgAURcDnucYfxz6o84tJ/JIryNUL7YMBAwQCAAAAIgYC/waP8Qc3CLN26iHYHP86KKe/agvSybsr94OOcLfH5BQYLQvavFQAAIAAAACAAAAAgAAAAAAFAAAAAAEBH4svAgAAAAAAFgAUQgFedj9xiA9jF43XWqRhf/CEodMBAwQCAAAAIgYCfkhXD7GvmrFh1Vh5d6KBY7ejdXv/Kgs3Z4Et+2lvc3oYLQvavFQAAIAAAACAAAAAgAEAAAAmAAAAAAA="

raw = base64.b64decode(B64_PSBT)

print("=" * 70)
print("PSBT Fee 计算分析")
print("=" * 70)

def read_vi(d, pos):
    b = d[pos]
    if b < 0xfd:
        return b, pos + 1
    if b == 0xfd:
        return int.from_bytes(d[pos+1:pos+3], 'little'), pos + 3
    if b == 0xfe:
        return int.from_bytes(d[pos+1:pos+5], 'little'), pos + 5
    return int.from_bytes(d[pos+1:pos+9], 'little'), pos + 9

# 提取 unsigned transaction
pos = 5
unsigned_tx = None

while raw[pos] != 0x00:
    klen, pos = read_vi(raw, pos)
    key_type = raw[pos] if klen > 0 else None
    pos += klen
    vlen, pos = read_vi(raw, pos)
    
    if key_type == 0x00:  # PSBT_GLOBAL_UNSIGNED_TX
        unsigned_tx = raw[pos:pos+vlen]
    
    pos += vlen

pos += 1  # 跳过分隔符

print(f"\n{'='*70}")
print("解析 Unsigned Transaction")
print(f"{'='*70}\n")

if unsigned_tx:
    tx_pos = 4  # skip version
    input_count, tx_pos = read_vi(unsigned_tx, tx_pos)
    
    print(f"输入数量: {input_count}")
    
    # 跳过所有输入
    for i in range(input_count):
        tx_pos += 32  # prev txid
        tx_pos += 4   # prev vout
        scriptsig_len, tx_pos = read_vi(unsigned_tx, tx_pos)
        tx_pos += scriptsig_len
        tx_pos += 4   # sequence
    
    # 解析输出
    output_count, tx_pos = read_vi(unsigned_tx, tx_pos)
    print(f"输出数量: {output_count}\n")
    
    total_output = 0
    for i in range(output_count):
        value = int.from_bytes(unsigned_tx[tx_pos:tx_pos+8], 'little')
        tx_pos += 8
        
        script_len, tx_pos = read_vi(unsigned_tx, tx_pos)
        script = unsigned_tx[tx_pos:tx_pos+script_len]
        tx_pos += script_len
        
        total_output += value
        print(f"Output #{i}: {value} sats")
    
    print(f"\n✓ 总输出: {total_output} sats ({total_output/100000000:.8f} BTC)")

print(f"\n{'='*70}")
print("解析 Input Maps")
print(f"{'='*70}\n")

input_values = []
input_sighashes = []

for input_idx in range(input_count if unsigned_tx else 0):
    print(f"--- Input #{input_idx} ---")
    
    has_witness = False
    has_non_witness = False
    witness_value = None
    sighash_type = None
    
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
            has_non_witness = True
        elif key_type == 0x01:  # witness_utxo
            has_witness = True
            if vlen >= 8:
                witness_value = int.from_bytes(val[:8], 'little')
                print(f"  witness_utxo: {witness_value} sats")
        elif key_type == 0x03:  # PSBT_IN_SIGHASH_TYPE
            # sighash type 是 4 字节的 u32 (little endian)
            if vlen == 4:
                sighash_type = int.from_bytes(val, 'little') & 0xFF
                print(f"  sighash_type: 0x{sighash_type:02x}", end="")
                
                # 解码 sighash
                base = sighash_type & 0x1f
                has_acp = sighash_type & 0x80
                
                sighash_name = {0x01: "ALL", 0x02: "NONE", 0x03: "SINGLE"}.get(base, "UNKNOWN")
                if has_acp:
                    sighash_name += "|ANYONE_CAN_PAY"
                print(f" ({sighash_name})")
            else:
                print(f"  sighash field (unexpected length: {vlen})")
    
    if witness_value:
        input_values.append(witness_value)
    if sighash_type:
        input_sighashes.append(sighash_type)
    
    pos += 1  # 下一个 input map 的分隔符
    print()

print(f"{'='*70}")
print("Fee 计算")
print(f"{'='*70}\n")

total_input = sum(input_values)
print(f"总输入: {total_input} sats")
print(f"总输出: {total_output} sats")
base_fee = total_input - total_output
print(f"基础手续费: {base_fee} sats ({base_fee/total_input*100:.2f}%)\n")

# 检查特殊 sighash 类型
has_acp = any(sh & 0x80 for sh in input_sighashes)
has_none = any((sh & 0x1f) == 0x02 for sh in input_sighashes)
has_single = any((sh & 0x1f) == 0x03 for sh in input_sighashes)

print(f"{'='*70}")
print("Sighash 类型与 Fee 计算")
print(f"{'='*70}\n")

if has_acp or has_none or has_single:
    if has_none:
        print("⚠️ 检测到 SIGHASH_NONE (0x02)")
        print("  含义: 签名不保护任何输出")
        print("  风险: 所有输出可被完全替换")
        print("  Fee 影响: 输出可变 → Fee 未知\n")
        print("  → UI 应显示: Fee unknown")
        print("  → UI 应显示: 红色 SIGHASH_NONE 警告\n")
    
    if has_single:
        print("⚠️ 检测到 SIGHASH_SINGLE (0x03)")
        print("  含义: 只保护对应索引的输出")
        print("  风险: 其他输出可被修改/删除")
        print("  Fee 影响: 部分输出可变")
        if has_acp:
            print("  → UI 应显示: Fee unknown\n")
        else:
            print("  → UI 应显示: >= {0} sats\n".format(base_fee))
        print("  → UI 应显示: 红色 SIGHASH_SINGLE 警告\n")
    
    if has_acp and not has_none and not has_single:
        print("⚠️ 检测到 ANYONE_CAN_PAY (0x80)")
        print("  含义: 只签署当前输入")
        print("  风险: 其他人可添加更多输入")
        print("  Fee 影响: 输入可增加 → Fee 是下限\n")
        print("  → UI 应显示: >= {0} sats (fee_is_lower_bound)\n".format(base_fee))
else:
    print("✅ 标准 SIGHASH_ALL (0x01)")
    print("  含义: 签署所有输入和所有输出")
    print("  安全: 交易完全被保护")
    print("  Fee 计算: 精确\n")
    print("  → UI 应显示: {0} sats\n".format(base_fee))

print(f"{'='*70}\n")
