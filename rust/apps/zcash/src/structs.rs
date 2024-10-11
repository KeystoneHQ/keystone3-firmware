use alloc::vec::Vec;
use app_utils::impl_internal_struct;

impl_internal_struct!(EnhancedPcztTransaction {
    global: EnhancedPcztGlobal,
    transparent: Option<EnhancedPcztTransparent>,
    orchard: Option<EnhancedPcztOrchard>
});

impl_internal_struct!(EnhancedPcztGlobal {
    tx_version: u32,
    version_group_id: u32,
    lock_time: u32,
    expiry_height: u32,
    consensus_branch_id: u32,
    network_id: u32
});

impl_internal_struct!(EnhancedPcztTransparent {
    inputs: Vec<EnhancedPcztTransparentInput>,
    outputs: Vec<EnhancedPcztTransparentOutput>
});

impl_internal_struct!(EnhancedPcztTransparentInput {
    pubkey: Vec<u8>,
    coin: EnhancedPcztTransparentCoin,
    out_point: EnhancedPcztTransparentOutput,
    path: EnhancedPcztDerivationPath
});

impl_internal_struct!(EnhancedPcztTransparentOutput {});

impl_internal_struct!(EnhancedPcztTransparentCoin {});

impl_internal_struct!(EnhancedPcztDerivationPath {});

impl_internal_struct!(EnhancedPathComponent {});

impl_internal_struct!(EnhancedPcztTransparentOutpoint {});

impl_internal_struct!(EnhancedPcztOrchard {});
