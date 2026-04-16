# PSBT Witness-Only 测试结果

## 测试 PSBT 信息

**Base64:**
```
cHNidP8BAHECAAAAAbHe89yvTAyW2l5lwTYShPFfFRAipdpkIh9zGDuxyaVOAQAAAAD9////AugDAAAAAAAAAFgAUxUxqQYiw+3P6Y4TQrOE8J2DLoWCLLwIAAAAAABYAFEIBXnY/cYgPYxeN11qkYX/whKHTz2oOAE8BBIiyHgPXktQvgAAAAFuM36CZpo1hc9qtdIVngF0+flXs4RWVddkcae/jZQw4ArbR+AQC7rR3ZYukmxCbcmeCDd5Ipq71VZBLuZaRyXj0EC0L2rxUAACAAAAAgAAAAIAAAQEf/zMCAAAAAAAWABTPAKLos6lGO6C+brxdUNQcb7D2IQEDBAEAAAAiBgN5Xt07J+yExyy05wvTeBLoWzBF4CmElv0E2Hqaw4VwcxgtC9q8VAAAgAAAAIAAAACAAQAAACUAAAAAACICAn5IVw+xr5qxYdVYeXeigWO3o3V7/yoLN2eBLftpb3N6GC0L2rxUAACAAAAAgAAAAIABAAAAJgAAAAA=
```

## 关键检测项

### 固件逻辑（parsed_psbt.rs line 33-35）

```rust
let has_witness_only_inputs = self
    .psbt
    .inputs
    .iter()
    .any(|input| input.witness_utxo.is_some() && input.non_witness_utxo.is_none());
```

### 预期结果

**根据代码逻辑：**

- `has_witness_only_inputs` = **TRUE**
- Input #0:
  - `witness_utxo.is_some()` = TRUE（金额：150252 聪）
  - `non_witness_utxo.is_none()` = TRUE
  - Outpoint: `4ea5c9b13b18731f2224da52225114f15f841216c15e96da968c4ccfdef3deb1:1`

## UI 显示预期

### ✅ 应该显示以下内容：

#### 1. 交易总览页（Overview）
- **顶部橙色提示卡**
  - 英文: "Check input value"
  - 中文: "请检查输入值"
  - i18n key: `btc_check_input_value_hint`
  
#### 2. 交易详情页（Detail - From 部分）
-  **Input Reference 链接**
  - 每个输入下方显示可点击的文字链接
  - 点击后弹出 QR HintBox
  - QR 内容：
    - Text: `4ea5c9b13b18731f2224da52225114f15f841216c15e96da968c4ccfdef3deb1:1`
    - URL: `https://mempool.space/tx/4ea5c9b13b18731f2224da52225114f15f841216c15e96da968c4ccfdef3deb1#vout=1`
  - i18n key: `btc_input_reference`

## 安全意义

### 为什么需要提示？

这笔 PSBT 只提供了 `witness_utxo`（仅包含一个 TxOut 结构：金额 + ScriptPubKey），
没有提供 `non_witness_utxo`（完整的前序交易）。

**风险：**
1. 设备无法独立验证这个输入的真实金额是否为 150252 聪
2. 恶意软件钱包可能提供错误的金额，导致：
   - 用户支付的手续费比预期高（Input 实际只有 10000 聪，但软件说有 150252 聪）
   - 用户不知不觉中签署了消耗更多资金的交易

**缓解措施：**
1. 提示用户"设备无法独立验证输入金额"
2. 提供 Input Reference QR，让用户可以在区块浏览器中交叉验证
3. 用户扫描 QR → 在 mempool.space 查看这笔 UTXO → 确认金额确实是 150252 聪

## 测试步骤

### 1. 单元测试（白盒）
```bash
cd rust
cargo test -p app_bitcoin test_user_provided_psbt_witness_only_check --  --show-output
```

**预期输出：**
```
✅ PSBT 分析结果:
• 输入数量: 1
  • 输出数量: 2
  • witness_utxo 存在: true
  • non_witness_utxo 存在: false
  • witness_utxo 金额: 150252 聪
  • has_witness_only_inputs: true

📱 设备 UI 预期行为:
  ✓ 应显示 "Check input value" 橙色提示卡
  ✓ 应显示输入的 QR 引用链接
  ✓ Outpoint: 4ea5c9b13b18731f2224da52225114f15f841216c15e96da968c4ccfdef3deb1:1
```

### 2. 设备测试（集成）

1. **导入 PSBT**
   - 在软件钱包中生成上述 PSBT
   - 通过 QR 或 USB 导入到设备

2. **检查总览页**
   - ✓ 顶部是否有橙色 "Check input value" 提示卡
   - ✓ 金额、网络、from/to 地址是否正确显示

3. **检查详情页**
   - ✓ From 部分的输入下方是否有 "Input Reference" 链接
   - ✓ 点击链接是否弹出 QR 码
   - ✓ QR 码内容是否包含正确的 outpoint 和 mempool 链接

4. **交叉验证**
   - 扫描 QR 码，在浏览器中打开
   - 确认该 UTXO 的金额确实为 150252 聪
   - 确认地址、脚本类型等信息匹配

## 相关代码文件

- `rust/apps/bitcoin/src/transactions/psbt/parsed_psbt.rs` (line 33-35)
  - `has_witness_only_inputs` 计算逻辑
  
- `rust/apps/bitcoin/src/transactions/parsed_tx.rs`
  - `OverviewTx` 结构定义，包含 `has_witness_only_inputs` 字段
  
- `rust/rust_c/src/bitcoin/structs.rs`
  - C FFI 结构 `DisplayTxOverview`，传递 `has_witness_only_inputs` 给 UI
  
- `src/ui/gui_chain/gui_btc.c`
  - `NeedShowCheckInputValueHint()` - 判断是否显示提示
  - `CreateCheckInputValueHintView()` - 创建提示卡 UI
  - `CreateInputRefView()` - 创建输入引用 QR 链接
  - `OpenInputRefQrCode()` - 打开 QR HintBox
  
- `src/ui/lv_i18n/data.csv`
  - `btc_check_input_value_hint` - 提示文案
  - `btc_input_reference` - 引用链接文案

## 参考

- BIP 174: Partially Signed Bitcoin Transaction Format
- BIP 32/44/49/84: HD Wallet derivation paths
- SegWit (BIP 141/143): Witness transaction format
