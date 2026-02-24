# Kaspa 集成
**概览**
- 范围：Rust 应用层（`rust/apps/kaspa`），Rust→C FFI（`rust/rust_c/src/kaspa`），LVGL C UI（`src/ui/...`），UR 编码/解码与 registry 支持。
- 主要功能：xPub 导出、地址派生、PSKT 解析/签名（Schnorr），UR 编码（ur:kaspa-pskt），多帧 QR 动画。

**一、核心职责与接口**
- 派生并导出扩展公钥（xPub）：标准 BIP44 路径 m/44'/111111'/account'（coin_type=111111）。
- 通过 FFI 暴露：xPub（Base58/78 字节）、主指纹（4 字节）、地址派生、PSKT 解析、PSKT 签名与签名结果 UR。

主要 FFI（表面接口）
- `kaspa_get_account_xpub(seed, seed_len, account_index)` → xPub
- `kaspa_get_extended_pubkey(seed, seed_len, path)` → xPub
- `kaspa_get_extended_pubkey_bytes(seed, seed_len, path)` → 78 字节 xpub（hex）
- `kaspa_get_master_fingerprint(seed, seed_len)` → 4 字节 xfp（hex）
- `kaspa_get_address(xpub, path)` → Kaspa 地址（kaspa: 前缀）
- `kaspa_get_derivation_path(account, change, index)` → 派生路径字符串
- `kaspa_parse_pskt(pskt_hex, mfp, len)` → TransactionParseResult<DisplayKaspaTx>
- `kaspa_sign_pskt(pskt_hex, seed, seed_len, mfp, mfp_len)` → UREncodeResult（ur:kaspa-pskt）
- `kaspa_check_pskt(pskt_hex, mfp, len)` → TransactionCheckResult

**二、UR 设计与建议**
请求（软件 → 硬件）：`ur:kaspa-pskt`（tag 建议 8601），携带 PSKT（原始二进制或 CBOR 封装的 `KaspaPskt`）。已在 `ur-registry/src/kaspa/kaspa_pskt.rs` 中实现。

返回（硬件 → 软件）：`ur:kaspa-pskt`，包含签名后的 PSKT（建议使用 CBOR 的 KaspaSignature 结构以携带可选元数据）。
- 编码原则：传输二进制 PSKT（`pskt.to_bytes()`）而非 UTF-8 字符串，或将签名包装为 CBOR 结构以附带元数据。
- 支持多帧编码（Fountain code 分片）以兼容大 payload 的动画 QR。

**三、UI（C 层）集成要点**
- Connect Software Wallet 菜单中添加 `Kaspium` 项，选择后调用 `GuiGetKaspiumData()` 生成 xPub/xfp JSON 或 UR，用于对方扫码导入。
- Home 钱包卡添加 `HOME_WALLET_CARD_KASPA`，并将 Kaspa 图标（coinKaspa/walletKaspium）加入资源数组。
- 接收（Receive）页面：优先使用通用 UTXO 接收路径 `src/ui/gui_widgets/gui_utxo_receive_widgets.c` 中的 Kaspa 分支（调用 `kaspa_get_address`）；
- 交易签名视图：`gui_kaspa` 组件显示 `DisplayKaspaTx`（inputs/outputs/fee/total），确认后调用 `kaspa_sign_pskt()`，播放多帧 UR QR 码。

## "Connect Software Wallet" 菜单集成 ✅—————》xpub导出
1.  **添加 WALLET_LIST_KASPIUM 枚举** (gui_connect_wallet_widgets.h.  ~60)
2.  **添加 Kaspium 币种图标数组** (gui_connect_wallet_widgets.c. line ~ 179)
3.  **添加 Kaspium 到钱包列表** (gui_connect_wallet_widgets.c, line ~219)
4.  **实现数据生成函数** (gui_wallet.c. line ~748)————》Kaspium 只导出 account 0 的 xpub，符合生态标准
5.  **添加函数声明** (gui_wallet.h   line ~33)
6.  **添加 switch case 处理** (gui_connect_wallet_widgets.c, line ~1266)

## UR 格式支持
1. 使用标准 ur:crypto-hdkey CBOR 编码？还是UR:CRYPTO-MULTI-ACCOUNTS？
入口（C UI）: GuiGetKaspiumData — gui_wallet.c:761
	构造 ExtendedPublicKey 列表并调用 get_connect_kaspa_ur(...)。
FFI 封装（Rust→C）: get_connect_kaspa_ur — kaspium.rs:1
	校验 mfp、规范化 xpubs，然后调用 wallets 层的生成函数并通过 UREncodeResult::encode() 编码成 UR。
UR 内容生成: generate_crypto_multi_accounts — kaspium.rs:1
	把每个 xpub 转为 CryptoHDKey，组装成 CryptoMultiAccounts（ur-registry 的类型），返回给 FFI 层供 UR 编码。
简短流程：UI → GuiGetKaspiumData → get_connect_kaspa_ur（FFI）→ generate_crypto_multi_accounts（构建 CryptoMultiAccounts）→ UR 编码并返回。


## Home 页面ui集成 
1. 添加 HOME_WALLET_CARD_KASPA 枚举 (gui_general_home_widgets.h)
2. 添加 Kaspa 卡片状态 (gui_general_home_widgets.h)
3. 添加 Kaspa 图标配置 (gui_general_home_widgets.h)
4. 添加点击处理 (gui_general_home_widgets.c)———————》 line~346
	复用g_utxoReceiveView

## 地址显示界面
1. src/ui/gui_widgets/gui_utxo_receive_widgets.c - 新增Kaspa 接收地址界面
	加入kaspa的case处理， 然后注意ModelGetUtxoAddress，kaspa不要复用之前的utxo_get_address----bitcoin的address生成方法
2. 从 xPub 派生地址（kaspa_get_address()）
	——》/rust/apps/kaspa/src/addresses.rs
3. 地址索引切换（Address-0, Address-1...）
地址索引切换的全部逻辑都在文件 gui_utxo_receive_widgets.c. 关键函数/handler（以及作用）：
* 读取/写入索引: GetCurrentSelectIndex / SetCurrentSelectIndex — 负责从持久层读取/写入当前地址索引（见 gui_utxo_receive_widgets.c:610-660）。
* 快速切换（+1）: ChangeAddressHandler — 点击“Generate new address” 增加索引并更新显示（见 gui_utxo_receive_widgets.c:1300-1335）。
* 打开地址选择页: OpenSwitchAddressHandler — 切到切换地址的 tile 并刷新列表（见 gui_utxo_receive_widgets.c:1320-1335）。
* 在 5 个候选中切换: SwitchAddressHandler — 点击某一行（5 个候选之一）将 g_selectIndex = g_showIndex + i 并更新确认按钮状态（见 gui_utxo_receive_widgets.c:980-1040）。
* 跳转到指定索引: GotoAddressHandler / GotoAddressKeyboardHandler — 弹出数字键盘输入指定 Address 索引并跳转（见 gui_utxo_receive_widgets.c:1330-1380）。
* 确认并写回: ConfirmAddrIndexHandler — 点击确认后调用 SetCurrentSelectIndex(g_selectIndex) 把选择写回持久层（见 gui_utxo_receive_widgets.c:720-760）。
* 显示更新: RefreshSwitchAccount / RefreshQrCode / RefreshDefaultAddress — 根据 g_selectIndex/g_showIndex 调用 ModelGetUtxoAddress(index, ...) 来抓取并显示对应地址。
简短结论：地址索引切换发生在上述几个 handler 里，最终读写索引的接口是 GetAccountReceiveIndex/SetAccountReceiveIndex（由 GetCurrentSelectIndex/SetCurrentSelectIndex 封装）。
4. 二维码展示/派生路径显示
GuiCreateQrCodeWidget 的作用：在接收页创建并初始化二维码显示区及其相关 UI 和事件绑定。
* 功能概述：创建二维码控件并支持全屏预览（通过 CreateUTXOReceiveQRCode、GuiFullscreenModeInit / GuiFullscreenModeCreateObject）。
* 布局/控件：设置 g_utxoReceiveWidgets.qrCode、g_utxoReceiveWidgets.addressLabel、g_utxoReceiveWidgets.addressCountLabel、g_utxoReceiveWidgets.pathLabel 等显示元素。
* 交互：添加地址切换按钮（addressButton，绑定 OpenSwitchAddressHandler）和“生成新地址”按钮（changeButton/changeImg/changeLabel，绑定 ChangeAddressHandler）。
* 初始化：设置样式/对齐、二维码初始为空（由 RefreshQrCode 后续填充），并在首次接收时弹出注意提示（GetFirstReceive / SetFirstReceive + attentionCont）。
一句话：负责搭建二维码 + 地址展示 + 切换/生成地址的 UI 和事件流程。
RefreshQrCode 等函数通过 ModelGetUtxoAddress(GetCurrentSelectIndex(), &item) 读取并展示二维码/地址文本


**四、重要文件（高层）**
- Rust 应用：`rust/apps/kaspa/`（xpub.rs, addresses.rs, pskt/*）
- Rust FFI：`rust/rust_c/src/kaspa/`（xpub.rs, address.rs, pskt.rs, mod.rs）
- C UI：`src/ui/gui_chain/multi/web3/gui_kaspa.*`、`src/ui/gui_widgets/*`（connect-wallet、home、utxo receive）
- UR/Registry 建议：`ur-registry` 中新增 `kaspa_pskt` 模块，采用 UR 类型字符串 `kaspa-pskt`（对应常量名建议 `KASPA_PSKT`）。

**五、数据流（概述）**
1) 软件钱包构建 KaspaSignRequest（PSKT），编码为 UR（`kaspa-pskt`），显示动画 QR。
2) 硬件钱包扫描，`parse_ur()` / `receive()` 接收并解码多帧，分发到 Kaspa handler `GuiSetKaspaUrData()`。
3) UI 调用 `kaspa_parse_pskt()` 将 PSKT 解析为 `DisplayKaspaTx` 并展示给用户。
4) 用户确认后，C 层获取种子并调用 `kaspa_sign_pskt()`；Rust 签名后返回 UR 编码（`kaspa-pskt`，或 CBOR 包装的签名结果），硬件播放多帧签名 QR。
5) 软件钱包扫描签名 UR，恢复签名后的 PSKT，验证并广播交易。

**六、实现与安全要点**
- 签名算法：Schnorr（BIP-340，x-only pubkey，64 字节签名）。
- 哈希算法：Blake2b-256（Kaspa 特有，非 SHA-256）。
- 地址格式：CashAddr-like，前缀 `kaspa:`，基于 Blake2b-256 摘要。
- 防盲签：在签名前必须验证 script_public_key/派生路径/xfp 匹配，显示完整交易信息。
- 敏感数据处理：调用后立即清零种子/私钥（`memset_s` / `ClearSecretCache()`），正确释放 Rust 分配的内存。

**七、构建与工具链注意事项**
- 构建命令（仿真器）：`python3 build.py -o simulator`（包含资源生成、CMake、cargo）。
- Rust toolchain：部分第三方 crate（如 `zmij`）可能使用不稳定特性，需要 nightly 或对该 crate 打补丁；可用 `patch.crates-io` 指向修复分支或在本地 registry 中临时修改 crate 源（短期解决）。

**八、UR Registry 与编码细节建议**
UR/Registry 状态：`KaspaPskt` 已实现于 `ur-registry/src/kaspa/kaspa_pskt.rs`，UR 类型为 `kaspa-pskt`（常量 `KASPA_PSKT`，tag 建议 8601）。

**九、测试与下一步**
- 建议新增单元测试：xpub 导出、地址派生向量、PSKT 解析/签名用例、UR encode/decode 测试。
- 处理依赖问题：如果 cargo build 报错引用不稳定特性，优先采用 `patch.crates-io` 指向修复分支或短期在本地 registry 修改 crate（添加 `#![feature(...)]`），长期建议向 upstream 提交 PR。
- UI 验证：构建并运行模拟器，执行完整流程：Connect→导出 xPub→Kaspium 导入→构建 PSKT→扫描签名并验证。

