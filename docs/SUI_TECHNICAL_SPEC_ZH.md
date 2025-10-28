# Sui 技术规范文档

## 摘要

Sui 是一个 Layer 1 协议区块链。简单来说，这意味着 Sui 在其网络上使用自己的原生代币（SUI）执行自己的共识和验证交易区块（活动）。以太坊（ETH）和比特币（BTC）是其他 Layer 1 区块链的例子。

相比之下，Layer 2 区块链利用 Layer 1 网络的基础设施，依赖 Layer 1 区块链来最终确定交易区块。Polygon（MATIC）是扩展以太坊的 Layer 2 区块链的一个例子。

## 签名算法

Sui 支持多种签名算法，提供了灵活的密码学选择。

### 支持的签名方案

| 方案 | 路径 | 说明 |
| --- | --- | --- |
| Ed25519 | `m/44'/784'/{account}'/{change}'/{address}'` | 每个层级都是强化派生（hardened）|
| ECDSA Secp256k1 | `m/54'/784'/{account}'/{change}/{address}` | 前三个层级是强化派生 |
| ECDSA Secp256r1 | `m/74'/784'/{account}'/{change}/{address}` | 前三个层级是强化派生 |

**主流方案**: Ed25519

**注意**: 
- Sui coin type = 784（根据 SLIP-0044 标准）
- Ed25519 使用完全强化派生路径（所有层级带 '）
- Secp256k1/r1 仅前三个层级强化派生

## 公钥生成

### Ed25519 公钥生成流程

```c
// 从 seed 生成 Ed25519 公钥
get_ed25519_pubkey_by_seed(seed, seed_len, path)
    ↓
slip10_ed25519::get_public_key_by_seed(seed, path)
    ↓
[32 字节 Ed25519 公钥]
```

### Keystone3 设备预存路径

设备上预先保存了 10 个标准路径对应的公钥：

```c
{XPUB_TYPE_SUI_0,  "sui_0", "M/44'/784'/0'/0'/0'"},
{XPUB_TYPE_SUI_1,  "sui_1", "M/44'/784'/1'/0'/0'"},
{XPUB_TYPE_SUI_2,  "sui_2", "M/44'/784'/2'/0'/0'"},
{XPUB_TYPE_SUI_3,  "sui_3", "M/44'/784'/3'/0'/0'"},
{XPUB_TYPE_SUI_4,  "sui_4", "M/44'/784'/4'/0'/0'"},
{XPUB_TYPE_SUI_5,  "sui_5", "M/44'/784'/5'/0'/0'"},
{XPUB_TYPE_SUI_6,  "sui_6", "M/44'/784'/6'/0'/0'"},
{XPUB_TYPE_SUI_7,  "sui_7", "M/44'/784'/7'/0'/0'"},
{XPUB_TYPE_SUI_8,  "sui_8", "M/44'/784'/8'/0'/0'"},
{XPUB_TYPE_SUI_9,  "sui_9", "M/44'/784'/9'/0'/0'"},
```

## 地址生成

### 地址格式

参考: https://docs.sui.io/concepts/cryptography/transaction-auth/keys-addresses#address-format

```
对于派生 32 字节的 Sui 地址，Sui 使用 BLAKE2b（256 位输出）哈希函数
对签名方案标志 1 字节与公钥字节连接后进行哈希。

Sui 地址当前支持：
- 纯 Ed25519（标志字节 0x00）
- Secp256k1（标志字节 0x01）
- Secp256r1（标志字节 0x02）
- MultiSig（标志字节 0x03）
```

### 生成算法

```rust
pub fn generate_address(pub_key: &str) -> Result<String> {
    // 1. 解码公钥（32 字节）
    let mut buf: Vec<u8> = hex::decode(pub_key)?;
    if buf.len() != 32 {
        return Err(InvalidData);
    }
    
    // 2. 插入签名方案标志字节
    // Ed25519 = 0x00, Secp256k1 = 0x01, Secp256r1 = 0x02, MultiSig = 0x03
    buf.insert(0, 0);  // Ed25519
    
    // 3. BLAKE2b-256 哈希
    let mut hasher = Blake2bVar::new(32)?;
    hasher.update(&buf);
    let mut addr = [0u8; 32];
    hasher.finalize_variable(&mut addr)?;
    
    // 4. 返回十六进制格式（带 0x 前缀）
    Ok(format!("0x{}", hex::encode(addr)))
}
```

### 地址生成示例

```rust
// Ed25519 示例
公钥 (32 字节): bfa73107effa14b21ff1b9ae2e6b2e770232b7c29018abbf76475b25395369c0
标志字节: 0x00
连接后: 00bfa73107effa14b21ff1b9ae2e6b2e770232b7c29018abbf76475b25395369c0
BLAKE2b-256: [32 字节哈希]
地址: 0x[64 位十六进制字符]
```

## 签名机制

### 签名流程概述

Sui 支持两种签名模式：

1. **Intent 签名**11：签名完整的交易数据（推荐）
2. **Hash 签名**：直接签名预计算的哈希值

### 1. Intent 签名（交易签名）

#### Intent 消息结构

```rust
Intent Message = [intent_scope, intent_version, intent_app_id, bcs_data]

struct Intent {
    scope: IntentScope,      // u8: 0x00 = TransactionData
    version: IntentVersion,  // u8: 0x00
    app_id: AppId,          // u8: 0x00
}

IntentMessage {
    intent: Intent,
    value: T  // BCS 编码的数据
}
```

#### 签名过程

```rust
fn sign_intent(seed: &[u8], path: &str, intent_data: &[u8]) -> [u8; 64] {
    // 1. 解析 Intent 消息（BCS 编码）
    let intent_message = parse_intent(intent_data);
    
    // 2. 计算 BLAKE2b-256 哈希
    let hash = blake2b_256(&intent_data);
    
    // 3. Ed25519 签名
    let signature = ed25519::sign(seed, path, &hash);
    
    return signature;  // 64 字节 [R(32) || S(32)]
}
```

### 2. Hash 签名

直接签名 32 字节的哈希值：

```rust
fn sign_hash(seed: &[u8], path: &str, message_hash: &[u8]) -> [u8; 64] {
    // 1. 验证哈希长度
    assert!(message_hash.len() == 32);
    
    // 2. Ed25519 签名
    let signature = ed25519::sign(seed, path, message_hash);
    
    return signature;  // 64 字节
}
```

## 各签名算法详细规范

### Ed25519 签名

- **标准**: [RFC 8032](https://www.rfc-editor.org/rfc/rfc8032.html#section-5.1.6)
- **内部哈希**: SHA-512
- **签名长度**: 64 字节
- **验证**: 必须通过 [ZIP215](https://github.com/zcash/zips/blob/main/zip-0215.rst) 验证

#### 签名结构
```
Signature = [R || S]
- R: 32 字节（曲线点）
- S: 32 字节（标量）
```

#### 实现
```rust
pub fn sign_message_by_seed(
    seed: &[u8], 
    path: &String, 
    message: &[u8]
) -> Result<[u8; 64]> {
    // 1. 派生密钥对
    let keypair = derive_keypair_from_seed(seed, path);
    
    // 2. Ed25519 签名（RFC 8032）
    // - 确定性签名
    // - 内部使用 SHA-512
    let signature = cryptoxide::ed25519::signature(message, &keypair);
    
    Ok(signature)
}
```

### ECDSA Secp256k1 和 Secp256r1

#### 签名过程

1. **消息哈希**: SHA256(signData)
2. **签名输出**: [r, s] = 32 + 32 字节 = 64 字节
3. **r 值范围**: 0x1 到 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364140
4. **s 值规范化**: s 值必须在曲线的下半部分
   - 如果 s 过高，应用 BIP-62 转换: `s = order - s`
   
   | 方案 | Order（曲线阶） |
   | --- | --- |
   | Secp256k1 | 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141 |
   | Secp256r1 | 0xFFFFFFFF00000000FFFFFFFFFFFFFFFFBCE6FAADA7179E84F3B9CAC2FC632551 |

5. **确定性 Nonce**: 遵循 [RFC6979](https://www.rfc-editor.org/rfc/rfc6979)

#### S 值规范化示例

```rust
fn normalize_s_value(s: &[u8], curve_order: &[u8]) -> Vec<u8> {
    let s_value = BigUint::from_bytes_be(s);
    let order = BigUint::from_bytes_be(curve_order);
    let half_order = &order / 2u32;
    
    if s_value > half_order {
        // s 在上半部分，需要转换
        let normalized_s = order - s_value;
        normalized_s.to_bytes_be()
    } else {
        // s 已经在下半部分
        s.to_vec()
    }
}
```

## 哈希算法

### BLAKE2b-256
- **用途**: 地址生成、交易签名哈希
- **输出**: 32 字节
- **特点**: 比 SHA-256 更快，安全性相当

```rust
use blake2::{Blake2bVar, digest::{Update, VariableOutput}};

let mut hasher = Blake2bVar::new(32)?;  // 256 位 = 32 字节
hasher.update(data);
let mut result = [0u8; 32];
hasher.finalize_variable(&mut result)?;
```

### SHA-256
- **用途**: ECDSA 签名的消息哈希
- **输出**: 32 字节

### SHA-512
- **用途**: Ed25519 签名内部使用
- **输出**: 64 字节
- **标准**: RFC 8032

## 安全性考虑

### 密钥派生安全
- ✅ Ed25519 使用 SLIP-0010 标准
- ✅ ECDSA 使用 BIP32 标准
- ✅ 所有关键路径都使用强化派生
- ✅ Seed 存储在安全元件（SE）中

### 签名安全
- ✅ Ed25519 使用确定性签名（RFC 8032）
- ✅ ECDSA 使用确定性 nonce（RFC6979）
- ✅ 签名后立即清零 seed（zeroize）
- ✅ S 值规范化防止签名可塑性

### Keystone3 实现的安全措施

```rust
unsafe fn sui_sign_internal<F>(
    seed: &mut [u8],
    path: &str,
    sign_fn: F,
) -> Result<([u8; 64], Vec<u8>), UREncodeResult>
{
    // 1. 执行签名
    let signature = sign_fn(seed, path).map_err(|e| {
        seed.zeroize();  // ❌ 失败时清零 seed
        UREncodeResult::from(e)
    })?;

    // 2. 获取公钥
    let pub_key = get_public_key(seed, &path.to_string()).map_err(|e| {
        seed.zeroize();  // ❌ 失败时清零 seed
        UREncodeResult::from(e)
    })?;

    Ok((signature, pub_key))
}

// 使用后清零
let result = sui_sign_internal(seed, path, sign_fn)?;
seed.zeroize();  // ✅ 成功后也清零
```

### Master Fingerprint 验证

```rust
pub unsafe extern "C" fn sui_check_request(
    ptr: PtrUR,
    master_fingerprint: PtrBytes,
    length: u32,
) -> PtrT<TransactionCheckResult> {
    // 验证 MFP 长度
    if length != 4 {
        return InvalidMasterFingerprint;
    }
    
    // 验证路径非空
    let paths = sign_request.get_derivation_paths();
    if paths.is_empty() {
        return InvalidHDPath;
    }
    
    // 验证 MFP 匹配
    let ur_mfp = paths[0].get_source_fingerprint();
    if mfp != ur_mfp {
        return MasterFingerprintMismatch;
    }
    
    Ok(())
}
```

## 代码示例

### 生成地址（Ed25519）

```rust
use app_sui::generate_address;

let pubkey_hex = "bfa73107effa14b21ff1b9ae2e6b2e770232b7c29018abbf76475b25395369c0";
let address = generate_address(pubkey_hex)?;

println!("地址: {}", address);
// 输出: 地址: 0x[64位十六进制]
```

### 签名 Intent（C FFI）

```c
// 签名交易
UREncodeResult* result = sui_sign_intent(
    request_ptr,     // SuiSignRequest UR
    seed,            // seed 字节数组
    seed_len         // seed 长度
);

// 返回格式
SuiSignature {
    request_id: Option<Vec<u8>>,
    signature: Vec<u8>,      // 64 字节
    public_key: Vec<u8>,     // 32 字节
}
```

### 签名 Hash（C FFI）

```c
// 签名哈希
UREncodeResult* result = sui_sign_hash(
    request_ptr,     // SuiSignHashRequest UR
    seed,            // seed 字节数组
    seed_len         // seed 长度
);
```

### Rust 完整示例

```rust
use keystore::algorithms::ed25519::slip10_ed25519;

// 1. 从 seed 获取公钥
let path = "m/44'/784'/0'/0'/0'";
let public_key = slip10_ed25519::get_public_key_by_seed(seed, &path.to_string())?;

// 2. 生成地址
let address = app_sui::generate_address(&hex::encode(public_key))?;

// 3. 签名交易
let intent_data = [/* BCS 编码的交易数据 */];
let signature = app_sui::sign_intent(seed, &path.to_string(), &intent_data)?;

// 4. 清零敏感数据
seed.zeroize();
```

## Intent 类型

Sui 支持多种 Intent 类型：

### IntentScope 枚举

```rust
pub enum IntentScope {
    TransactionData = 0,        // 0x00: 交易数据
    TransactionEffects = 1,     // 0x01: 交易效果
    CheckpointSummary = 2,      // 0x02: 检查点摘要
    PersonalMessage = 3,        // 0x03: 个人消息
}
```

### 个人消息签名

```rust
// 个人消息 Intent
let msg = PersonalMessage {
    message: b"Hello, Sui!".to_vec()
};

let intent = IntentMessage {
    intent: Intent {
        scope: IntentScope::PersonalMessage,
        version: IntentVersion::V0,
        app_id: AppId::Sui,
    },
    value: msg,
};

// BCS 编码
let intent_bytes = bcs::to_bytes(&intent)?;

// 签名
let signature = sign_intent(seed, path, &intent_bytes)?;
```

## 参考资料

### 官方文档
- [Sui 文档](https://docs.sui.io/)
- [Sui 密码学](https://docs.sui.io/concepts/cryptography)
- [密钥和地址](https://docs.sui.io/concepts/cryptography/transaction-auth/keys-addresses)

### 标准规范
- [RFC 8032 - Ed25519](https://www.rfc-editor.org/rfc/rfc8032.html)
- [RFC 6979 - 确定性 ECDSA](https://www.rfc-editor.org/rfc/rfc6979)
- [SLIP-0010 - Ed25519 密钥派生](https://github.com/satoshilabs/slips/blob/master/slip-0010.md)
- [SLIP-0044 - Coin Types](https://github.com/satoshilabs/slips/blob/master/slip-0044.md)
- [BIP-32 - HD 钱包](https://github.com/bitcoin/bips/blob/master/bip-0032.mediawiki)
- [BIP-62 - 签名规范化](https://github.com/bitcoin/bips/blob/master/bip-0062.mediawiki)
- [ZIP215 - Ed25519 验证](https://github.com/zcash/zips/blob/main/zip-0215.rst)

### 库和工具
- [BLAKE2b 官方网站](https://www.blake2.net/)
- [BCS 序列化](https://github.com/diem/bcs)

## 附录

### 派生路径示例

| 用途 | 算法 | 路径 |
|------|------|------|
| 第一个账户 | Ed25519 | m/44'/784'/0'/0'/0' |
| 第二个账户 | Ed25519 | m/44'/784'/1'/0'/0' |
| 找零地址 | Ed25519 | m/44'/784'/0'/1'/0' |
| Secp256k1 账户 | Secp256k1 | m/54'/784'/0'/0/0 |
| Secp256r1 账户 | Secp256r1 | m/74'/784'/0'/0/0 |

### 错误码

| 错误码 | 说明 |
|--------|------|
| InvalidData | 数据格式无效 |
| InvalidHDPath | 派生路径无效 |
| SignFailure | 签名失败 |
| InvalidHex | 十六进制解码失败 |
| InvalidMasterFingerprint | Master fingerprint 无效 |
| MasterFingerprintMismatch | Master fingerprint 不匹配 |

### 测试向量

```
# Ed25519
派生路径: m/44'/784'/0'/0'/0'
种子: [32 字节测试种子]
私钥: [32 字节]
公钥: [32 字节]
地址: 0x[64 位十六进制]

# 签名测试
消息: "Hello, Sui!"
消息哈希 (BLAKE2b-256): [32 字节]
签名: [64 字节]
```

---

**文档版本**: 1.0  
**最后更新**: 2025-10-27  
**适用于**: Keystone3 固件 v1.0+  
**作者**: Keystone Team
