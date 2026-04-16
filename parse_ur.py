#!/usr/bin/env python3
"""
解析 UR 编码的 PSBT
"""
import sys

# 检查是否有 ur 库
try:
    import ur
    from ur.ur_decoder import URDecoder
except ImportError:
    print("❌ 需要安装 ur-py 库")
    print("请运行: pip install ur-py")
    sys.exit(1)

UR_STRING = "UR:CRYPTO-PSBT/HKADDSJOJKIDJYZMADAEGOAOAEAEAEADBYSNZCMKMYBKCXKKCAFZTLJPPKBDJOSTLTGELDKOEOEOTDSRSERHLAGTREIORYGSADAEAEAEAEZCZMZMZMADMKAXAEAEAEAEAEAECFKOPTBBWPGTSBKGPRGOBBJOWMCSGSPMVEKPJOSSNEBEFHJZLOPSVTIMBAAEGWADAALOPRCKAXTSMOTYDLLAAEAEAEHPLKURNBNLOLLGHSJKTNPMJYLPIOLAHLFMKBGOWPVYBZMDKPTACEINWSVLIHBNETAORPTTYAAAAOWYQZKTIHLUOXNDBENDJPIOLFBTUEFDOLPLYKGOMHGRRHMTMESOKSWKBEDPBDTNRFGHAEAELAAEAEAELAAEAEAELAAEADADCTVSAXAEAEAEAEAEAECMAEBBSKGSIMFPLOPFZOJKZSIALRTIPSVYFNDIHNSBOYHNADAXAAADAEAEAECPAMAXSRJOVAFNTOTEZMDKRTGHMTTEADDEWFIACSWKCLCTVYCKHLJYGYISHYGSPDZOWKYTCSDPBDTNRFGHAEAELAAEAEAELAAEAEAELAAEAEAEAEAEAEAEAEAEAEWPMHATRO"

print("=" * 70)
print("UR PSBT 解析")
print("=" * 70)

try:
    # 解析 UR
    _, ur_type, cbor_data = ur.ur.decode(UR_STRING)
    
    print(f"\nUR 类型: {ur_type}")
    print(f"CBOR 数据长度: {len(cbor_data)} 字节")
    
    # 解析 CBOR (crypto-psbt 格式)
    import cbor2
    
    cbor_obj = cbor2.loads(cbor_data)
    print(f"CBOR 对象: {type(cbor_obj)}")
    
    # crypto-psbt 的 CBOR 结构是一个 map，key 1 是 PSBT 数据
    if isinstance(cbor_obj, dict) and 1 in cbor_obj:
        psbt_bytes = cbor_obj[1]
        print(f"PSBT 数据长度: {len(psbt_bytes)} 字节")
        
        # 检查 PSBT 魔数
        if psbt_bytes[:5] == b'psbt\xff':
            print("✅ PSBT 魔数正确\n")
            
            # 解析 PSBT
            def read_vi(d, pos):
                b = d[pos]
                if b < 0xfd:
                    return b, pos + 1
                if b == 0xfd:
                    return int.from_bytes(d[pos+1:pos+3], 'little'), pos + 3
                if b == 0xfe:
                    return int.from_bytes(d[pos+1:pos+5], 'little'), pos + 5
                return int.from_bytes(d[pos+1:pos+9], 'little'), pos + 9
            
            # Skip global map
            pos = 5
            while pos < len(psbt_bytes) and psbt_bytes[pos] != 0x00:
                klen, pos = read_vi(psbt_bytes, pos)
                pos += klen
                vlen, pos = read_vi(psbt_bytes, pos)
                pos += vlen
            pos += 1
            
            print("Input #0 分析:\n")
            
            # Parse input map
            has_witness_utxo = False
            has_non_witness_utxo = False
            witness_value = None
            
            while pos < len(psbt_bytes) and psbt_bytes[pos] != 0x00:
                klen, pos = read_vi(psbt_bytes, pos)
                if klen == 0:
                    break
                
                key_type = psbt_bytes[pos]
                pos += klen
                
                vlen, pos = read_vi(psbt_bytes, pos)
                val = psbt_bytes[pos:pos+vlen]
                pos += vlen
                
                if key_type == 0x00:  # non_witness_utxo
                    has_non_witness_utxo = True
                    print(f"✓ non_witness_utxo: {vlen} 字节")
                elif key_type == 0x01:  # witness_utxo
                    has_witness_utxo = True
                    if vlen >= 8:
                        witness_value = int.from_bytes(val[:8], 'little')
                        script = val[8:]
                        print(f"✓ witness_utxo:")
                        print(f"  金额: {witness_value} sats")
                        print(f"  脚本: {script.hex()}")
            
            print(f"\n{'='*70}")
            print("检测结果")
            print(f"{'='*70}\n")
            
            print(f"witness_utxo: {has_witness_utxo}")
            print(f"non_witness_utxo: {has_non_witness_utxo}")
            
            has_witness_only = has_witness_utxo and not has_non_witness_utxo
            
            print(f"\n→ has_witness_only_inputs = {has_witness_only}\n")
            
            if has_witness_only:
                print("✅ 这是 witness-only PSBT")
                print("\n设备应显示:")
                print("• \"Check input value\" 提示卡")
                print("• Input Reference QR 链接")
                if witness_value:
                    print(f"\nwitness_utxo 金额: {witness_value} sats")
            else:
                print("❌ 不是 witness-only PSBT")
                if has_non_witness_utxo:
                    print("原因: 包含 non_witness_utxo，设备可以独立验证")
        else:
            print("❌ 不是有效的 PSBT")
    else:
        print("❌ CBOR 格式不正确")
        
except Exception as e:
    print(f"❌ 解析失败: {e}")
    import traceback
    traceback.print_exc()

print(f"\n{'='*70}\n")
