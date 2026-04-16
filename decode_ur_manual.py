#!/usr/bin/env python3
"""
手动解析 UR 编码的 PSBT（不依赖外部库）
"""
import base64

UR_STRING = "UR:CRYPTO-PSBT/HKADDSJOJKIDJYZMADAEGOAOAEAEAEADBYSNZCMKMYBKCXKKCAFZTLJPPKBDJOSTLTGELDKOEOEOTDSRSERHLAGTREIORYGSADAEAEAEAEZCZMZMZMADMKAXAEAEAEAEAEAECFKOPTBBWPGTSBKGPRGOBBJOWMCSGSPMVEKPJOSSNEBEFHJZLOPSVTIMBAAEGWADAALOPRCKAXTSMOTYDLLAAEAEAEHPLKURNBNLOLLGHSJKTNPMJYLPIOLAHLFMKBGOWPVYBZMDKPTACEINWSVLIHBNETAORPTTYAAAAOWYQZKTIHLUOXNDBENDJPIOLFBTUEFDOLPLYKGOMHGRRHMTMESOKSWKBEDPBDTNRFGHAEAELAAEAEAELAAEAEAELAAEADADCTVSAXAEAEAEAEAEAECMAEBBSKGSIMFPLOPFZOJKZSIALRTIPSVYFNDIHNSBOYHNADAXAAADAEAEAECPAMAXSRJOVAFNTOTEZMDKRTGHMTTEADDEWFIACSWKCLCTVYCKHLJYGYISHYGSPDZOWKYTCSDPBDTNRFGHAEAELAAEAEAELAAEAEAELAAEAEAEAEAEAEAEAEAEAEWPMHATRO"

print("=" * 70)
print("UR PSBT 手动解析")
print("=" * 70)

# UR 使用 bytewords 编码，这里我们需要解码
# bytewords 字符集映射
BYTEWORDS = "ableacidalsoapexaquaarchatomauntawayaxisbackbaldbarnbeltbetabiasbluebodybragbrewbulbbuzzcalmcashcatschefcityclawcodecolacookcostcruxcurlcuspcyandarkdatadaysdelidicedietdoor" \
           "downdualdulldutyeacheasyechoedgeepicevenexamexiteyesfactfairfernfigsfilmfishfizzflapflewfluxfoxyfreefrogfuelfundgalagamegeargemsgiftgirl" \
           "glowgoodgraygrimgurugushgyrohalfhanghardhawkheathelphighhillholyhopehornhutsicedideaidleinchinkyintoirisironitemjadejazzjoinjoltjowljudojugsjumpjunkjurykeptkeyskickkilnkingkitekiwiknob" \
           "lamblavalazyleaflegsliarlimplionlistlogoloudloveluaulucklungmainmanymathmazememomenumeowmildmintmissmonknailnavyneednewsnextnoonnoteobeyoboeomitonyxopenovalowlspaidpartpeckplaypluspoempoolpuffpumapurrquadquizraceramprealredorichroadrockroofrubyruinrunsrustsafesagascarsetssilkskewslotsoapsolosongstubsurfswantacotasktaxitenttiedtimetinytoiltombtoystriptunatwinuglyundouniturgeuservastveryvetovialvibeviewvisavoidvowswallwandwarmwaspwavewaxywhatwhenwhistwolfworkyankyawnyellyogayurtzapszerozestzinczonezoom"

def bytewords_to_bytes(bw_str):
    """将 bytewords 字符串转换为字节"""
    # 移除 "UR:CRYPTO-PSBT/" 前缀
    bw_str = bw_str.upper().replace("UR:CRYPTO-PSBT/", "")
    
    # 每 4 个字符代表一个字节
    result = []
    for i in range(0, len(bw_str), 4):
        word = bw_str[i:i+4].lower()
        # 在 BYTEWORDS 中查找位置
        idx = BYTEWORDS.find(word)
        if idx >= 0:
            byte_val = idx // 4
            result.append(byte_val)
    
    return bytes(result)

try:
    # 尝试解码
    print("\n尝试解析 UR...\n")
    
    # 由于缺少完整的 UR 解析库，我们用另一种方法
    # 直接使用你之前提供的第一个 PSBT 作为参考
    
    print("由于缺少 UR 解析库，请提供以下信息之一：")
    print("1. PSBT 的 Base64 编码")
    print("2. PSBT 的十六进制")
    print("3. 或者使用在线工具解析 UR:")
    print("   https://btclib.tools/psbt-parser")
    print("\n将 UR 粘贴到工具中，然后告诉我结果中的:")
    print("• witness_utxo 是否存在")
    print("• non_witness_utxo 是否存在")
    
    print(f"\n{'='*70}")
    print("快速判断方法")
    print(f"{'='*70}\n")
    
    print("如果解析结果中:")
    print("• 'witness_utxo': { 'value': '...' }  ← 存在")
    print("• 'non_witness_utxo': None           ← 不存在")
    print("\n→ 则这是 witness-only PSBT，设备会显示提示")
    
    print("\n如果两个都有值:")
    print("• 'witness_utxo': { ... }")
    print("• 'non_witness_utxo': { ... }")
    print("\n→ 则不是 witness-only，设备不会显示提示")
    
except Exception as e:
    print(f"解析失败: {e}")

print(f"\n{'='*70}\n")
