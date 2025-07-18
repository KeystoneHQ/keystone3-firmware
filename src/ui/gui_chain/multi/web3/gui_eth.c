#include "gui_analyze.h"
#include "user_memory.h"
#include "secret_cache.h"
#include "account_public_info.h"
#include "keystore.h"
#include "gui_model.h"
#include "gui_qr_hintbox.h"
#include "screen_manager.h"
#include "user_sqlite3.h"
#include "account_manager.h"
#include "math.h"
#include "stdio.h"
#include "string.h"
#include "drv_mpu.h"
#include "device_setting.h"
#include "cjson/cJSON.h"
#include "gui_hintbox.h"
#include "gui_views.h"

static void decodeEthContractData(void *parseResult);
static bool GetEthErc20ContractData(void *parseResult);

static bool g_isMulti = false;
static URParseResult *g_urResult = NULL;
static URParseMultiResult *g_urMultiResult = NULL;
static void *g_parseResult = NULL;
static void *g_erc20ContractData = NULL;
static char *g_erc20Name = NULL;
static void *g_contractData = NULL;
static bool g_contractDataExist = false;
static char g_fromEthEnsName[64];
static char g_toEthEnsName[64];
static bool g_fromEnsExist = false;
static bool g_toEnsExist = false;
static bool g_isPermit = false;
static bool g_isPermitSingle = false;
static bool g_isOperation = false;
static ViewType g_viewType = ViewTypeUnKnown;

const static EvmNetwork_t NETWORKS[] = {
    {0, "Unknown Network", "ETH"},
    {1, "Ethereum Mainnet", "ETH"},
    {2, "Expanse Network", "EXP"},
    {3, "Ethereum Testnet Ropsten", "ETH"},
    {4, "Ethereum Testnet Rinkeby", "ETH"},
    {5, "Ethereum Testnet Goerli", "ETH"},
    {6, "Ethereum Classic Testnet Kotti", "KOT"},
    {7, "ThaiChain", "TCH"},
    {8, "Ubiq", "UBQ"},
    {9, "Ubiq Network Testnet", "TUBQ"},
    {10, "Optimistic Ethereum", "OETH"},
    {11, "Metadium Mainnet", "META"},
    {12, "Metadium Testnet", "KAL"},
    {13, "Diode Testnet Staging", "sDIODE"},
    {14, "Flare Mainnet", "FLR"},
    {15, "Diode Prenet", "DIODE"},
    {16, "Flare Testnet Coston", "CFLR"},
    {17, "ThaiChain 2.0 ThaiFi", "TFI"},
    {18, "ThunderCore Testnet", "TST"},
    {19, "Songbird Canary-Network", "SGB"},
    {20, "ELA-ETH-Sidechain Mainnet", "ELA"},
    {21, "ELA-ETH-Sidechain Testnet", "tELA"},
    {22, "ELA-DID-Sidechain Mainnet", "ELA"},
    {23, "ELA-DID-Sidechain Testnet", "tELA"},
    {30, "RSK Mainnet", "RBTC"},
    {31, "RSK Testnet", "tRBTC"},
    {32, "GoodData Testnet", "GooD"},
    {33, "GoodData Mainnet", "GooD"},
    {35, "TBWG Chain", "TBG"},
    {38, "Valorbit", "VAL"},
    {40, "Telos EVM Mainnet", "TLOS"},
    {41, "Telos EVM Testnet", "TLOS"},
    {42, "Ethereum Testnet Kovan", "ETH"},
    {43, "Darwinia Pangolin Testnet", "PRING"},
    {44, "Darwinia Crab Network", "CRING"},
    {50, "XinFin Network Mainnet", "XDC"},
    {51, "XinFin Apothem Testnet", "TXDC"},
    {52, "CoinEx Smart Chain Mainnet", "cet"},
    {53, "CoinEx Smart Chain Testnet", "cett"},
    {56, "Binance Smart Chain Mainnet", "BNB"},
    {58, "Ontology Mainnet", "ONG"},
    {59, "EOS Mainnet", "EOS"},
    {60, "GoChain", "GO"},
    {61, "Ethereum Classic Mainnet", "ETC"},
    {62, "Ethereum Classic Testnet Morden", "TETC"},
    {63, "Ethereum Classic Testnet Mordor", "METC"},
    {64, "Ellaism", "ELLA"},
    {65, "OKExChain Testnet", "OKT"},
    {66, "OKXChain Mainnet", "OKT"},
    {67, "DBChain Testnet", "DBM"},
    {68, "SoterOne Mainnet", "SOTER"},
    {69, "Optimistic Ethereum Testnet Kovan", "KOR"},
    {76, "Mix", "MIX"},
    {77, "POA Network Sokol", "POA"},
    {78, "PrimusChain mainnet", "PETH"},
    {80, "GeneChain", "RNA"},
    {82, "Meter Mainnet", "MTR"},
    {85, "GateChain Testnet", "GT"},
    {86, "GateChain Mainnet", "GT"},
    {88, "TomoChain", "TOMO"},
    {95, "CryptoKylin Testnet", "EOS"},
    {97, "Binance Smart Chain Testnet", "tBNB"},
    {99, "POA Network Core", "SKL"},
    {100, "Gnosis Chain", "xDAI"},
    {101, "EtherInc", "ETI"},
    {102, "Web3Games Testnet", "W3G"},
    {106, "Velas EVM Mainnet", "VLX"},
    {108, "ThunderCore Mainnet", "TT"},
    {110, "Proton Testnet", "XPR"},
    {111, "EtherLite Chain", "ETL"},
    {122, "Fuse Mainnet", "FUSE"},
    {124, "Decentralized Web Mainnet", "DWU"},
    {127, "Factory 127 Mainnet", "FETH"},
    {128, "Huobi ECO Chain Mainnet", "HT"},
    {137, "Polygon Mainnet", "MATIC"},
    {142, "DAX CHAIN", "DAX"},
    {162, "Lightstreams Testnet", "PHT"},
    {163, "Lightstreams Mainnet", "PHT"},
    {170, "HOO Smart Chain Testnet", "HOO"},
    {172, "Latam-Blockchain Resil Testnet", "usd"},
    {200, "Arbitrum on xDai", "xDAI"},
    {211, "Freight Trust Network", "0xF"},
    {222, "Permission", "ASK"},
    {246, "Energy Web Chain", "EWT"},
    {250, "Fantom Opera", "FTM"},
    {256, "Huobi ECO Chain Testnet", "htt"},
    {262, "SUR Blockchain Network", "SRN"},
    {269, "High Performance Blockchain", "HPB"},
    {321, "KCC Mainnet", "KCS"},
    {322, "KCC Testnet", "tKCS"},
    {336, "Shiden", "SDN"},
    {338, "Cronos Testnet", "TCRO"},
    {361, "Theta Mainnet", "TFUEL"},
    {363, "Theta Sapphire Testnet", "TFUEL"},
    {364, "Theta Amber Testnet", "TFUEL"},
    {365, "Theta Testnet", "TFUEL"},
    {369, "PulseChain Mainnet", "PLS"},
    {385, "Lisinski", "LISINSKI"},
    {420, "Optimistic Ethereum Testnet Goerli", "GOR"},
    {499, "Rupaya", "RUPX"},
    {558, "Tao Network", "TAO"},
    {595, "Acala Mandala Testnet", "mACA"},
    {686, "Karura Network", "KAR"},
    {721, "Factory 127 Testnet", "FETH"},
    {777, "cheapETH", "cTH"},
    {787, "Acala Network", "ACA"},
    {803, "Haic", "HAIC"},
    {820, "Callisto Mainnet", "CLO"},
    {821, "Callisto Testnet", "TCLO"},
    {888, "Wanchain", "WAN"},
    {940, "PulseChain Testnet", "tPLS"},
    {977, "Nepal Blockchain Network", "YETI"},
    {998, "Hyperliquid Testnet", "HYPE"},
    {999, "Hyperliquid", "HYPE"},
    {1001, "Klaytn Testnet Baobab", "KLAY"},
    {1007, "Newton Testnet", "NEW"},
    {1010, "Evrice Network", "EVC"},
    {1012, "Newton", "NEW"},
    {1022, "Sakura", "SKU"},
    {1023, "Clover Testnet", "CLV"},
    {1024, "Clover Mainnet", "CLV"},
    {1139, "MathChain", "MATH"},
    {1140, "MathChain Testnet", "MATH"},
    {1284, "Moonbeam", "GLMR"},
    {1285, "Moonriver", "MOVR"},
    {1286, "Moonrock", "ROC"},
    {1287, "Moonbase Alpha", "DEV"},
    {1288, "Moonshadow", "MSHD"},
    {1618, "Catecoin Chain Mainnet", "CATE"},
    {1620, "Atheios", "ATH"},
    {1657, "Btachain", "BTA"},
    {1856, "Teslafunds", "TSF"},
    {1987, "EtherGem", "EGEM"},
    {2020, "420coin", "420"},
    {2021, "Edgeware Mainnet", "EDG"},
    {2022, "Beresheet Testnet", "tEDG"},
    {2100, "Ecoball Mainnet", "ECO"},
    {2101, "Ecoball Testnet Espuma", "ECO"},
    {2559, "Kortho Mainnet", "KTO"},
    {4002, "Fantom Testnet", "FTM"},
    {4689, "IoTeX Network Mainnet", "IOTX"},
    {4690, "IoTeX Network Testnet", "IOTX"},
    {5197, "EraSwap Mainnet", "ES"},
    {5700, "Syscoin Tanenbaum Testnet", "tSYS"},
    {5851, "Ontology Testnet", "ONG"},
    {5869, "Wegochain Rubidium Mainnet", "RBD"},
    {8029, "MDGL Testnet", "MDGLT"},
    {8080, "GeneChain Adenine Testnet", "tRNA"},
    {8217, "Klaytn Mainnet Cypress", "KLAY"},
    {8285, "KorthoTest", "KTO"},
    {8723, "TOOL Global Mainnet", "OLO"},
    {8724, "TOOL Global Testnet", "OLO"},
    {8995, "bloxberg", "U+25B3"},
    {9000, "Evmos Testnet", "PHOTON"},
    {9001, "Evmos", "EVMOS"},
    {10000, "Smart Bitcoin Cash", "BCH"},
    {10001, "Smart Bitcoin Cash Testnet", "BCHT"},
    {10101, "Blockchain Genesis Mainnet", "GEN"},
    {16000, "MetaDot Mainnet", "MTT"},
    {16001, "MetaDot Testnet", "MTT-test"},
    {24484, "Webchain", "WEB"},
    {24734, "MintMe.com Coin", "MINTME"},
    {31102, "Ethersocial Network", "ESN"},
    {31337, "GoChain Testnet", "GO"},
    {32659, "Fusion Mainnet", "FSN"},
    {39797, "Energi Mainnet", "NRG"},
    {42069, "pegglecoin", "peggle"},
    {42161, "Arbitrum One", "AETH"},
    {42220, "Celo Mainnet", "CELO"},
    {43110, "Athereum", "ATH"},
    {43113, "Avalanche Fuji Testnet", "AVAX"},
    {43114, "Avalanche C-Chain", "AVAX"},
    {44787, "Celo Alfajores Testnet", "CELO"},
    {49797, "Energi Testnet", "NRG"},
    {62320, "Celo Baklava Testnet", "CELO"},
    {71393, "Polyjuice Testnet", "CKB"},
    {73799, "Energy Web Volta Testnet", "VT"},
    {78110, "Firenze test network", "FIN"},
    {80001, "Polygon Testnet Mumbai", "MATIC"},
    {100000, "QuarkChain Mainnet Root", "QKC"},
    {100001, "QuarkChain Mainnet Shard 0", "QKC"},
    {100002, "QuarkChain Mainnet Shard 1", "QKC"},
    {100003, "QuarkChain Mainnet Shard 2", "QKC"},
    {100004, "QuarkChain Mainnet Shard 3", "QKC"},
    {100005, "QuarkChain Mainnet Shard 4", "QKC"},
    {100006, "QuarkChain Mainnet Shard 5", "QKC"},
    {100007, "QuarkChain Mainnet Shard 6", "QKC"},
    {100008, "QuarkChain Mainnet Shard 7", "QKC"},
    {110000, "QuarkChain Devnet Root", "QKC"},
    {110001, "QuarkChain Devnet Shard 0", "QKC"},
    {110002, "QuarkChain Devnet Shard 1", "QKC"},
    {110003, "QuarkChain Devnet Shard 2", "QKC"},
    {110004, "QuarkChain Devnet Shard 3", "QKC"},
    {110005, "QuarkChain Devnet Shard 4", "QKC"},
    {110006, "QuarkChain Devnet Shard 5", "QKC"},
    {110007, "QuarkChain Devnet Shard 6", "QKC"},
    {110008, "QuarkChain Devnet Shard 7", "QKC"},
    {200625, "Akroma", "AKA"},
    {246529, "ARTIS sigma1", "ATS"},
    {246785, "ARTIS Testnet tau1", "tATS"},
    {333888, "Polis Testnet", "tPOLIS"},
    {333999, "Polis Mainnet", "POLIS"},
    {421611, "Arbitrum Testnet Rinkeby", "ARETH"},
    {955305, "Eluvio Content Fabric", "ELV"},
    {1313114, "Etho Protocol", "ETHO"},
    {1313500, "Xerom", "XERO"},
    {7762959, "Musicoin", "MUSIC"},
    {13371337, "PepChain Churchill", "TPEP"},
    {18289463, "IOLite", "ILT"},
    {20181205, "quarkblockchain", "QKI"},
    {28945486, "Auxilium Network Mainnet", "AUX"},
    {35855456, "Joys Digital Mainnet", "JOYS"},
    {61717561, "Aquachain", "AQUA"},
    {99415706, "Joys Digital TestNet", "TOYS"},
    {245022926, "Neon EVM DevNet", "NEON"},
    {245022934, "Neon EVM MainNet", "NEON"},
    {245022940, "Neon EVM TestNet", "NEON"},
    {311752642, "OneLedger Mainnet", "OLT"},
    {1122334455, "IPOS Network", "IPOS"},
    {1313161554, "Aurora MainNet", "aETH"},
    {1313161555, "Aurora TestNet", "aETH"},
    {1313161556, "Aurora BetaNet", "aETH"},
    {1666600000, "Harmony Mainnet Shard 0", "ONE"},
    {1666600001, "Harmony Mainnet Shard 1", "ONE"},
    {1666600002, "Harmony Mainnet Shard 2", "ONE"},
    {1666600003, "Harmony Mainnet Shard 3", "ONE"},
    {1666700000, "Harmony Testnet Shard 0", "ONE"},
    {1666700001, "Harmony Testnet Shard 1", "ONE"},
    {1666700002, "Harmony Testnet Shard 2", "ONE"},
    {1666700003, "Harmony Testnet Shard 3", "ONE"},
    {3125659152, "Pirl", "PIRL"},
    {4216137055, "OneLedger Testnet Frankenstein", "OLT"},
    {11297108109, "Palm Mainnet", "PALM"},
    {11297108099, "Palm Testnet", "PALM"},
    {210425, "PlatON Mainnet", "LAT"},
    {2206132, "PlatON Testnet", "LAT"},
    {324, "zkSync Mainnet", "ETH"},
};

const static Erc20Contract_t ERC20_CONTRACTS[] = {
    // ETH
    {"USDC", "0xa0b86991c6218b36c1d19d4a2e9eb0ce3606eb48", 6},
    {"USDT", "0xdac17f958d2ee523a2206206994597c13d831ec7", 6},
    {"DAI", "0x6b175474e89094c44da98b954eedeac495271d0f", 18},
    {"MKR", "0x9f8f72aa9304c8b593d555f12ef6589cc3a579a2", 18},
    {"BNB", "0xB8c77482e45F1F44dE1745F52C74426C631bDD52", 18},
    {"LINK", "0x514910771af9ca656af840dff83e8264ecf986ca", 18},
    {"UNI", "0x1f9840a85d5af5bf1d1762f925bdaddc4201f984", 18},
    {"COMP", "0xc00e94cb662c3520282e6f5717214004a7f26888", 18},
    {"AAVE", "0x7fc66500c84a76ad7e9c93437bfc5ac33e2ddae9", 18},
    {"stETH", "0xae7ab96520de3a18e5e111b5eaab095312d7fe84", 18},
    {"TRX", "0x50327c6c5a14dcade707abad2e27eb517df87ab5", 6},
    {"TONCOIN", "0x582d872a1b094fc48f5de31d3b73f2d9be47def1", 9},
    {"WBTC", "0x2260fac5e5542a773aa44fbcfedf7c193bc2c599", 8},
    {"MATIC", "0x7d1afa7b718fb893db30a3abc0cfc608aacfebb0", 18},
    {"SHIB", "0x95aD61b0a150d79219dCF64E1E6Cc01f0B64C4cE", 18},
    {"LEO", "0x2af5d2ad76741191d15dfe7bf6ac92d4bd912ca3", 18},
    {"OKB", "0x75231f58b43240c9718dd58b4967c5114342a86c", 18},
    {"INJ", "0xe28b3B32B6c345A34Ff64674606124Dd5Aceca30", 18},
    {"IMX", "0xf57e7e7c23978c3caec3c3548e3d615c346e79ff", 18},
    {"CRO", "0xa0b73e1ff0b80914ab6fe0444e65848c4c34450b", 8},
    {"TUSD", "0x0000000000085d4780B73119b644AE5ecd22b376", 18},
    {"VEN", "0xd850942ef8811f2a866692a623011bde52a462c1", 18},
    {"NEAR", "0x85f17cf997934a597031b2e18a9ab6ebd4b9f6a4", 24},
    {"LDO", "0x5a98fcbea516cf06857215779fd812ca3bef1b32", 18},
    {"MNT", "0x3c3a81e81dc49a522a592e7622a7e711c06bf354", 18},
    {"RNDR", "0x6de037ef9ad2725eb40118bb1702ebb27e4aeb24", 18},
    {"QNT", "0x4a220e6096b25eadb88358cb44068a3248254675", 18},
    {"stkAAVE", "0x4da27a545c0c5b758a6ba100e3a049001de870f5", 18},
    {"GRT", "0xc944e90c64b2c07662a292be6244bdf05cda44a7", 18},
    {"ARB", "0xB50721BCf8d664c30412Cfbc6cf7a15145234ad1", 18},
    {"rETH", "0xae78736cd615f374d3085123a210448e74fc6393", 18},
    {"SNX", "0xd0dA9cBeA9C3852C5d63A95F9ABCC4f6eA0F9032", 18},
    {"SNX", "0xc011a73ee8576fb46f5e1c5751ca3b9fe0af2a6f", 18},
    {"BUSD", "0x4fabb145d64652a948d72533023f6e7a623c7c53", 18},
    {"BTT", "0xc669928185dbce49d2230cc9b0979be6dc797957", 18},
    {"FTM", "0x4e15361fd6b4bb609fa63c81a2be19d873717870", 18},
    {"KCS", "0xf34960d9d60be18cc1d5afc1a6f012a723a28811", 6},
    {"SAND", "0x3845badAde8e6dFF049820680d1F14bD3903a5d0", 18},
    {"THETA", "0x3883f5e181fccaf8410fa61e12b59bad963fb645", 18},
    {"BEAM", "0x62D0A8458eD7719FDAF978fe5929C6D342B0bFcE", 18},
    {"HEX", "0x2b591e99afe9f32eaa6214f7b7629768c40eeb39", 8},
    {"MANA", "0x0f5d2fb29fb7d3cfee444a200298f468908cc942", 18},
    {"wMANA", "0xfd09cf7cfffa9932e33668311c4777cb9db3c9be", 18},
    {"WOO", "0x4691937a7508860f876c9c0a2a617e7d9e945d4b", 18},
    {"DYDX", "0x92d6c1e31e14520e676a687f0a93788b716beff5", 18},
    {"FET", "0xaea46A60368A7bD060eec7DF8CBa43b7EF41Ad85", 18},
    {"USDD", "0x0C10bF8FcB7Bf5412187A595ab97a3609160b5c6", 18},
    {"frxETH", "0x5e8422345238f34275888049021821e8e08caa1f", 18},
    {"FXS", "0x3432b6a60d23ca0dfca7761b7ab56459d9c964d0", 18},
    {"FRAX", "0x853d955acef822db058eb8505911ed77f175b99e", 18},
    {"CHZ", "0x3506424f91fd33084466f402d5d97f05f8e3b4af", 18},
    {"APE", "0x4d224452801aced8b2f0aebe155379bb5d594381", 18},
    {"Cake", "0x152649eA73beAb28c5b49B26eb48f7EAD6d4c898", 18},
    {"GNO", "0x6810e776880c02933d47db1b9fc05908e5386b96", 18},
    {"ILV", "0x767fe9edc9e0df98e07454847909b5e959d7ca0e", 18},
    {"PEPE", "0x6982508145454ce325ddbe47a25d4ec3d2311933", 18},
    {"BLUR", "0x5283d291dbcf85356a21ba090e6db59121208b44", 18},
    {"ELF", "0xbf2179859fc6d5bee9bf9158632dc51678a4100e", 18},
    {"RPL", "0xd33526068d116ce69f19a9ee46f0bd304f21a51f", 18},
    {"AXL", "0x467719ad09025fcc6cf6f8311755809d45a5e5f3", 6},
    {"cETH", "0x4ddc2d193948926d02f9b1fe9e1daa0718270ed5", 8},
    {"RLB", "0x046eee2cc3188071c02bfc1745a6b17c656e3f3d", 18},
    {"sfrxETH", "0xac3e018457b222d93114458476f3e3416abbe38f", 18},
    {"XAUt", "0x68749665ff8d2d112fa859aa293f07a622782f38", 6},
    {"IOTX", "0x6fb3e0a217407efff7ca062d46c26e5d60a14d69", 18},
    {"NEXO", "0xb62132e35a6c13ee1ee0f84dc5d40bad8d815206", 18},
    {"cbETH", "0xBe9895146f7AF43049ca1c1AE358B0541Ea49704", 18},
    {"PAXG", "0x45804880De22913dAFE09f4980848ECE6EcbAf78", 18},
    {"NFT", "0x198d14f2ad9ce69e76ea330b374de4957c3f850a", 6},
    {"HT", "0x6f259637dcd74c767781e37bc6133cd6a68aa161", 18},
    {"WLD", "0x163f8c2467924be0ae7b5347228cabf260318753", 18},
    {"HBTC", "0x0316EB71485b0Ab14103307bf65a021042c6d380", 18},
    {"1INCH", "0x111111111117dc0aa78b770fa6a738034120c302", 18},
    {"AGIX", "0x5b7533812759b45c2b44c19e320ba2cd2681b542", 8},
    {"ZIL", "0x05f4a42e251f2d52b8ed15e9fedaacfcef1fad27", 12},
    {"GMT", "0xe3c408BD53c31C085a1746AF401A4042954ff740", 8},
    {"HOT", "0x6c6ee5e31d828de241282b9606c8e98ea48526e2", 18},
    {"USDP", "0x8e870d67f660d95d5be530380d0ec0bd388289e1", 18},
    {"FLOKI", "0xcf0c122c6b73ff809c693db761e7baebe62b6a2e", 9},
    {"BAT", "0x0d8775f648430679a709e98d2b0cb6250d2887ef", 18},
    {"CVX", "0x4e3fbd56cd56c3e72c1403e103b45db9da5b9d2b", 18},
    {"ENJ", "0xf629cbd94d3791c9250152bd8dfbdf380e2a3b9c", 18},
    {"LRC", "0xbbbbca6a901c926f240b89eacb641d8aec7aeafd", 18},
    {"TRB", "0x88df592f8eb5d7bd38bfef7deb0fbc02cf3778a0", 18},
    {"WQTUM", "0x3103df8f05c4d8af16fd22ae63e406b97fec6938", 18},
    {"CELO", "0x3294395e62f4eb6af3f1fcf89f5602d90fb3ef69", 18},
    {"wCELO", "0xe452e6ea2ddeb012e20db73bf5d3863a3ac8d77a", 18},
    {"ZRX", "0xe41d2489571d322189246dafa5ebde1f4699f498", 18},
    {"SFP", "0x12e2b8033420270db2f3b328e32370cb5b2ca134", 18},
    {"OCEAN", "0x967da4048cD07aB37855c090aAF366e4ce1b9F48", 18},
    {"MX", "0x11eef04c884e24d9b7b4760e7476d06ddf797f36", 18},
    {"PRIME", "0xb23d80f5fefcddaa212212f028021b41ded428cf", 18},
    {"RBN", "0x6123b0049f904d730db3c36a31167d9d4121fa6b", 18},
    {"YFI", "0x0bc529c00C6401aEF6D220BE8C6Ea1667F6Ad93e", 18},
    {"MASK", "0x69af81e73a73b40adf4f3d4223cd9b1ece623074", 18},
    {"MEME", "0xb131f4A55907B10d1F0A50d8ab8FA09EC342cd74", 18},
    {"ENS", "0xc18360217d8f7ab5e7c516566761ea12ce7f9d72", 18},
    {"Auction", "0xa9b1eb5908cfc3cdf91f9b8b3a74108598009096", 18},
    {"NXM", "0xd7c49cee7e9188cca6ad8ff264c1da2e69d4cf3b", 18},
    {"GLM", "0x7DD9c5Cba05E151C895FDe1CF355C9A1D5DA6429", 18},
    {"BAND", "0xba11d00c5f74255f56a5e366f4f77f5a186d7f55", 18},
    {"TGT", "0x108a850856Db3f85d0269a2693D896B394C80325", 18},
    // BSC
    {"CAKE", "0x0E09FaBB73Bd3Ade0a17ECC321fD13a19e81cE82", 18},
    {"BUSD", "0xe9e7CEA3DedcA5984780Bafc599bD69ADd087D56", 18},
    {"SAFEMOON", "0x42981d0bfbAf196529376EE702F2a9Eb9092fcB5", 9},
    {"ETH", "0x2170ed0880ac9a755fd29b2688956bd959f933f8", 18},
    {"BSC-USD", "0x55d398326f99059ff775485246999027b3197955", 18},
    {"WBNB", "0xbb4CdB9CBd36B01bD1cBaEBF2De08d9173bc095c", 18},
    {"XRP", "0x1d2f0da169ceb9fc7b3144628db156f3f6c60dbe", 18},
    {"anyUSDC", "0x8965349fb649a33a30cbfda057d8ec2c48abe2a2", 18},
    {"USDC", "0x8ac76a51cc950d9822d68b83fe1ad97b32cd580d", 18},
    {"ADA", "0x3ee2200efb3400fabb9aacf31297cbdd1d435d47", 18},
    {"AVAX", "0x1ce0c2827e2ef14d5c4f29a091d735a204794041", 18},
    {"DOGE", "0xba2ae424d960c26247dd6c32edc70b295c744c43", 8},
    {"TRX", "0xce7de646e7208a4ef112cb6ed5038fa6cc6b12e3", 6},
    {"DOT", "0x7083609fce4d1d8dc0c979aab8c869ea2c873402", 18},
    {"LINK", "0xf8a0bf9cf54bb92f17374d9e9a321e6a111a51bd", 18},
    {"MATIC", "0xcc42724c6683b7e57334c4e856f4c9965ed682bd", 18},
    {"TONCOIN", "0x76A797A59Ba2C17726896976B7B3747BfD1d220f", 9},
    {"DAI", "0x1af3f329e8be154074d8769d1ffa4ee058b1dbc3", 18},
    {"LTC", "0x4338665cbb7b2485a8855a139b75d5e34ab0db94", 18},
    {"SHIB", "0x2859e4544c4bb03966803b044a93563bd2d0dd4d", 18},
    {"BCH", "0x8ff795a6f4d97e7887c79bea79aba5cc76444adf", 18},
    {"ATOM", "0x0eb3a705fc54725037cc9e008bdede697f62f335", 18},
    {"UNI", "0xbf5140a22578168fd562dccf235e5d43a02ce9b1", 18},
    {"XLM", "0x43c934a845205f0b514417d757d7235b8f53f1b9", 18},
    {"ETC", "0x3d6545b08693dae087e957cb1180ee38b9e3c25e", 18},
    {"FIL", "0x0d8ce2a99bb6e3b7db580ed848240e4a0f9ae153", 18},
    {"BTT", "0x8595f9da7b868b1822194faed312235e43007b49", 18},
    {"TUSD", "0x40af3827F39D0EAcBF4A168f8D4ee67c121D11c9", 18},
    {"VET", "0x6fdcdfef7c496407ccb0cec90f9c5aaa1cc8d888", 18},
    {"NEAR", "0x1fa4a73a3f0133f0025378af00236f3abdee5d63", 18},
    {"BTCB", "0x7130d2a12b9bcbfae4f2634d864a1ee1ce3ead9c", 18},
    {"Arbitrum", "0xa050ffb3eeb8200eeb7f61ce34ff644420fd3522", 18},
    {"MKR", "0x5f0da599bb2cccfcf6fdfd7d81743b6020864350", 18},
    {"FLOW", "0xc943c5320b9c18c153d1e2d12cc3074bebfb31a2", 18},
    {"BTT", "0x352Cb5E19b12FC216548a2677bD0fce83BaE434B", 18},
    {"FTM", "0xad29abb318791d579433d831ed122afeaf29dcfe", 18},
    {"EGLD", "0xbf7c81fff98bbe61b40ed186e4afd6ddd01337fe", 18},
    {"AXS", "0x715d400f88c167884bbcc41c5fea407ed4d2f8a0", 18},
    {"PAX", "0xb7f8cd00c5a06c0537e2abff0b58033d02e5e094", 18},
    {"MANA", "0x26433c8127d9b4e9b71eaa15111df99ea2eeb2f8", 18},
    {"EOS", "0x56b6fb708fc5732dec1afc8d8556423a2edccbd6", 18},
    {"XTZ", "0x16939ef78684453bfdfb47825f8a5f714f12623a", 18},
    {"IOTA", "0xd944f1d1e9d5f9bb90b62f9d45e447d989580782", 6},
    {"WOO", "0x4691937a7508860f876c9c0a2a617e7d9e945d4b", 18},
    {"USDD", "0xd17479997F34dd9156Deef8F95A52D81D265be9c", 18},
    {"CHEEL", "0x1f1c90aeb2fd13ea972f0a71e35c0753848e3db0", 18},
    {"SNX", "0x9ac983826058b8a9c7aa1c9171441191232e8404", 18},
    {"frxETH", "0x64048a7eecf3a2f1ba9e144aac3d7db6e58f555e", 18},
    {"FXS", "0xe48a3d7d0bc88d552f730b62c006bc925eadb9ee", 18},
    {"bCFX", "0x045c4324039dA91c52C55DF5D785385Aab073DcF", 18},
    {"FRAX", "0x29ced01c447166958605519f10dcf8b0255fb379", 18},
    {"FRAX", "0x90c97f71e18723b0cf0dfa30ee176ab653e89f40", 18},
    // Polygon
    {"USDT", "0xc2132d05d31c914a87c6611c10748aeb04b58e8f", 6},
    {"USDC", "0x3c499c542cEF5E3811e1192ce70d8cC03d5c3359", 6},
    {"BUSD", "0xdab529f40e671a1d4bf91361c21bf9f0c9712ab7", 18},
    {"USDC.e", "0x2791bca1f2de4661ed88a30c99a7a9449aa84174", 6},
    {"AVAX", "0x2C89bbc92BD86F8075d1DEcc58C7F4E0107f286b", 18},
    {"BNB", "0x3BA4c387f786bFEE076A58914F5Bd38d668B42c3", 18},
    {"LINK", "0xb0897686c545045afc77cf20ec7a532e3120e0f1", 18},
    {"INJ", "0x4e8dc2149eac3f3def36b1c281ea466338249371", 18},
    {"MATIC", "0x0000000000000000000000000000000000001010", 18},
    {"LINK", "0x53e0bca35ec356bd5dddfebbd1fc0fd03fabad39", 18},
    {"WETH", "0x7ceb23fd6bc0add59e62ac25578270cff1b9f619", 18},
    {"FET", "0x7583feddbcefa813dc18259940f76a02710a8905", 18},
    {"SHIB", "0x6f8a06447ff6fcf75d803135a7de15ce88c1d4ec", 18},
    {"AAVE", "0xd6df932a45c0f255f85145f286ea0b292b21c90b", 18},
    {"WBTC", "0x1bfd67037b42cf73acf2047067bd4f2c47d9bfd6", 8},
    {"TUSD", "0x2e1ad108ff1d8c782fcbbb89aad783ac49586756", 18},
    {"DAI", "0x8f3cf7ad23cd3cadbd9735aff958023239c6a063", 18},
    {"IOTX", "0xf6372cdb9c1d3674e83842e3800f2a62ac9f3c66", 18},
    {"UNI", "0xb33eaad8d922b1083446dc23f610c2567fb5180f", 18},
    {"FTM", "0xc9c1c1c20b3658f8787cc2fd702267791f224ce1", 18},
    {"WOO", "0x1b815d120b3ef02039ee11dc2d33de7aa4a8c603", 18},
    {"SXP", "0x6abb753c1893194de4a83c6e8b4eadfc105fd5f5", 18},
    {"POLY", "0xcb059c5573646047d6d88dddb87b745c18161d3b", 18},
    {"RNDR", "0x61299774020da444af134c82fa83e3810b309991", 18},
    {"CHZ", "0xf1938ce12400f9a761084e7a80d37e732a4da056", 18},
    {"CRV", "0x172370d5cd63279efa6d502dab29171933a610af", 18},
    {"TRB", "0xe3322702bedaaed36cddab233360b939775ae5f1", 18},
    {"MANA", "0xa1c57f48f0deb89f569dfbe6e2b7f46d33606fd4", 18},
    {"SNX", "0x50b728d8d964fd00c2d0aad81718b71311fef68a", 18},
    {"LRC", "0x84e1670f61347cdaed56dcc736fb990fbb47ddc1", 18},
    {"APE", "0xB7b31a6BC18e48888545CE79e83E06003bE70930", 18},
    {"SAND", "0xBbba073C31bF03b8ACf7c28EF0738DeCF3695683", 18},
    {"GRT", "0x5fe2b58c013d7601147dcdd68c143a77499f5531", 18},
    {"MKR", "0x6f7C932e7684666C9fd1d44527765433e01fF61d", 18},
    {"WMATIC", "0x0d500b1d8e8ef31e21c99d1db9a6444d3adf1270", 18},
    {"GMT", "0x714db550b574b3e927af3d93e26127d15721d4c2", 8},
    {"MASK", "0x2b9e7ccdf0f4e5b24757c1e1a80e311e34cb10c7", 18},
    {"AGIX", "0x190eb8a183d22a4bdf278c6791b152228857c033", 8},
    {"LDO", "0xc3c7d422809852031b44ab29eec9f1eff2a58756", 18},
    {"BLZ", "0x438b28c5aa5f00a817b7def7ce2fb3d5d1970974", 18},
    {"COMP", "0x8505b9d2254a7ae468c0e9dd10ccea3a837aef5c", 18},
    {"SUSHI", "0x0b3f868e0be5597d5db7feb59e1cadbb0fdda50a", 18},
    {"OCEAN", "0x282d8efce846a88b159800bd4130ad77443fa1a1", 18},
    {"LPT", "0x3962f4a0a0051dcce0be73a7e09cef5756736712", 18},
    {"REQ", "0xb25e20de2f2ebb4cffd4d16a55c7b395e8a94762", 18},
    {"OMG", "0x62414d03084eeb269e18c970a21f45d2967f0170", 18},
    {"FXS", "0x1a3acf6d19267e2d3e7f898f42803e90c9219062", 18},
    {"AXL", "0x6e4e624106cb12e168e6533f8ec7c82263358940", 6},
    {"1INCH", "0x9c2c5fd7b07e95ee044ddeba0e97a665f142394f", 18},
    {"STG", "0x2f6f07cdcf3588944bf4c42ac74ff24bf56e7590", 18},
    //StarkNet, we do not support StarkNet currently.
    // {"ETH", "0x049d36570d4e46f48e99674bd3fcc84644ddd6b96f7c741b1562b82f9e004dc7", 18},
    // {"zETH", "0x01b5bd713e72fdc5d63ffd83762f81297f6175a5e0a4771cdadbc1dd5fe72cb1", 18},
    // {"USDC", "0x053c91253bc9682c04929ca02ed00b3e423f6710d2ee7e0d5ebb06f3ecf368a8", 6},
    // {"STRK", "0x04718f5a0fc34cc1af16a1cdee98ffb20c31f5cd61d6ab07201858f4287c938d", 18},
    // {"USDT", "0x068f5c6a61780768455de69077e07e89787839bf8166decfbf92b645209c0fb8", 6},
    // {"DAI", "0x00da114221cb83fa859dbdb4c44beeaa0bb37c7537ad5ae66fe5e0efd20e6eb3", 18},
    // {"zUSDC", "0x047ad51726d891f972e74e4ad858a261b43869f7126ce7436ee0b2529a98f486", 6},
    // {"GOL", "0x06a05844a03bb9e744479e3298f54705a35966ab04140d3d8dd797c1f6dc49d0", 0},
    // {"LPT", "0x000023c72abdf49dffc85ae3ede714f2168ad384cc67d08524732acea90df325", 18},
    // {"JEDI-P", "0x04d0390b777b424e43839cd1e744799f3de6c176c7e32c1812a41dbd9c19db6a", 18},
    // {"MYLP", "0x022b05f9396d2c48183f6deaf138a57522bcc8b35b67dee919f76403d1783136", 12},
    // {"iETH-c", "0x057146f6409deb4c9fa12866915dd952aa07c1eb2752e451d7f3b042086bdeb8", 18},
    // {"zUSDT", "0x00811d8da5dc8a2206ea7fd0b28627c2d77280a515126e62baa4d78e22714c4a", 6},
    // {"STRKR", "0x0030c42f4c0a094ea1eda7e3086056a225a464c43dd7da48bd2083fc3114a4db", 18},
    // {"WBTC", "0x03fe2b97c1fd336e750087d68b9b867997fd64a2661ff3ca5a7c771641e8e7ac", 8},
    // {"EKUBO", "0x075afe6402ad5a5c20dd25e10ec3b3986acaa647b77e4ae24b0cbc9a54a27a87", 18},
    // {"vSTRK", "0x0782f0ddca11d9950bc3220e35ac82cf868778edb67a5e58b39838544bc4cd0f", 18},
    // {"LORDS", "0x0124aeb495b947201f5fac96fd1138e326ad86195b98df6dec9009158a533b49", 18},
    // {"zDAI", "0x062fa7afe1ca2992f8d8015385a279f49fad36299754fb1e9866f4f052289376", 18},
    // {"iSTRK-c", "0x07c2e1e733f28daa23e78be3a4f6c724c0ab06af65f6a95b5e0545215f1abc1b", 18},
    // {"wstETH", "0x042b8f0484674ca266ac5d08e4ac6a3fe65bd3129795def2dca5c34ecc5f96d2", 18},
    // {"SPIST", "0x06182278e63816ff4080ed07d668f991df6773fd13db0ea10971096033411b11", 18},
    // {"zSTRK", "0x06d8fa671ef84f791b7f601fa79fea8f6ceb70b5fa84189e3159d532162efc21", 18},
    // {"iUSDC-c", "0x029959a546dda754dc823a7b8aa65862c5825faeaaf7938741d8ca6bfdc69e4e", 6},
    // {"iETH-c", "0x070f8a4fcd75190661ca09a7300b7c93fab93971b67ea712c664d7948a8a54c6", 18},
    // {"JEDI-P", "0x045e7131d776dddc137e30bdd490b431c7144677e97bf9369f629ed8d3fb7dd6", 18},
    // {"LPT", "0x05900cfa2b50d53b097cb305d54e249e31f24f881885aae5639b0cd6af4ed298", 18},
    // {"iUSDC-c", "0x05dcd26c25d9d8fd9fc860038dcb6e4d835e524eb8a85213a8cda5b7fff845f6", 6},
    // {"MYLP", "0x041f9a1e9a4d924273f5a5c0c138d52d66d2e6a8bee17412c6b0f48fe059ae04", 12},
    // {"ssVLP-ETH/USDC", "0x030615bec9c1506bfac97d9dbd3c546307987d467a7f95d5533c2e861eb81f3f", 18},
    // {"JEDI-P", "0x07e2a13b40fc1119ec55e0bcf9428eedaa581ab3c924561ad4e955f95da63138", 18},
    // {"C-ETHUSDC-P", "0x018a6abca394bd5f822cfa5f88783c01b13e593d1603e7b41b00d31d2ea4827a", 18},
    // {"WARS", "0x015905d4ce047ed501b498964e74d82750c22826db5a38449080e9bf4e4796ca", 18},
    // {"LPT", "0x017e9e62c04b50800d7c59454754fe31a2193c9c3c6c92c093f2ab0faadf8c87", 18},
    // {"MYLP", "0x07c662b10f409d7a0a69c8da79b397fd91187ca5f6230ed30effef2dceddc5b3", 18},
    // {"AKU", "0x0137dfca7d96cdd526d13a63176454f35c691f55837497448fad352643cfe4d4", 18},
    // {"iUSDT-c", "0x055ba2baf189b98c59f6951a584a3a7d7d6ff2c4ef88639794e739557e1876f0", 6},
    // {"C-ETHUSDC-C", "0x07aba50fdb4e024c1ba63e2c60565d0fd32566ff4b18aa5818fc80c30e749024", 18},
    // {"SPIST", "0x060cf64cf9edfc1b16ec903cee31a2c21680ee02fc778225dacee578c303806a", 18},
    // {"ssSLP-USDC/USDT", "0x0601f72228f73704e827de5bcd8dadaad52c652bb1e42bf492d90bbe22df2cec", 18},
    // {"LPT", "0x041a708cf109737a50baa6cbeb9adf0bf8d97112dc6cc80c7a458cbad35328b0", 18},
    // {"DRAB", "0x02a3c4db12911f4acc6480f45928c2858e6517ec77d15a9709bc727af5f26e1b", 18},
    // {"SOCKS", "0x023ed2ba4fb5709302c5dfd739fa7613359042f143286c115b6c7f7dc2601015", 18},
    // {"LPT", "0x069d369e52fd3f8ce32ab6073a4296f4ce8c832b125284e8f0d653e02e93541d", 18},
    // {"zWBTC", "0x02b9ea3acdb23da566cee8e8beae3125a1458e720dea68c4a9a7a2d8eb5bbb4a", 8},
    // {"iDAI", "0x00b9b1a4373de5b1458e598df53195ea3204aa926f46198b50b32ed843ce508b", 18},
    // {"instSTRK-c", "0x067a34ff63ec38d0ccb2817c6d3f01e8b0c4792c77845feb43571092dcf5ebb5", 18},
    // {"ZEND", "0x00585c32b625999e6e5e78645ff8df7a9001cf5cf3eb6b80ccdd16cb64bd3a34", 18},
    // {"SPEPE", "0x01e0eee22c684fdf32babdd65e6bcca62a8ce2c23c8d5e68f3989595d26e1b4a", 18},
    // {"SPEPE", "0x06f15ec4b6ff0b7f7a216c4b2ccdefc96cbf114d6242292ca82971592f62273b", 18}
    // Arb
    {"USDT", "0xfd086bc7cd5c481dcc9c85ebe478a1c0b69fcbb9", 6},
    {"USDC.e", "0xff970a61a04b1ca14834a43f5de4533ebddb5cc8", 6},
    {"USDC", "0xaf88d065e77c8cc2239327c5edb3a432268e5831", 6},
    {"WBTC", "0x2f2a2543b76a4166549f7aab2e75bef0aefc5b0f", 8},
    {"LINK", "0xf97f4df75117a78c1a5a0dbb814af92458539fb4", 18},
    {"UNI", "0xfa7f8980b0f1e64a2062791cc3b0871572f1f7f0", 18},
    {"PEPE", "0x25d887ce7a35172c62febfd67a1856f20faebb00", 18},
    {"DAI", "0xda10009cbd5d07dd0cecc66161fc93d7c9000da1", 18},
    {"GRT", "0x9623063377ad1b27544c965ccd7342f7ea7e88c7", 18},
    {"ARB", "0x912ce59144191c1204e64559fe8253a0e49e6548", 18},
    {"LDO", "0x13ad51ed4f1b7e9dc168d8a00cb3f4ddd85efa60", 18},
    {"PYTH", "0xE4D5c6aE46ADFAF04313081e8C0052A30b6Dd724", 6},
    {"W", "0xb0ffa8000886e57f86dd5264b9582b2ad87b2b91", 18},
    {"PENDLE", "0x0c880f6761f1af8d9aa9c466984b80dab9a8c9e8", 18},
    {"GNO", "0xa0b862f60edef4452f25b4160f177db44deb6cf1", 18},
    {"frxETH", "0x178412e79c25968a32e89b11f63b33f733770c2a", 18},
    {"USDD", "0x680447595e8b7b3aa1b43beb9f6098c79ac2ab3f", 18},
    {"LPT", "0x289ba1701c2f088cf0faf8b3705246331cb8a839", 18},
    {"AXL", "0x23ee2343b892b1bb63503a4fabc840e0e2c6810f", 6},
    {"FRAX", "0x17fc002b466eec40dae837fc4be5c67993ddbd6f", 18},
    {"LUNC", "0x1A4dA80967373fd929961e976b4b53ceeC063a15", 6},
    {"WOO", "0xcafcd85d8ca7ad1e1c6f82f651fa15e33aefd07b", 18},
    {"CRV", "0x11cdb42b0eb46d95f990bedd4695a6e3fa034978", 18},
    {"TUSD", "0x4d15a3a2286d883af0aa1b3f21367843fac63e07", 18},
    {"CELO", "0x4e51ac49bc5e2d87e0ef713e9e5ab2d71ef4f336", 18},
    {"COMP", "0x354a6da3fcde098f8389cad84b0182725c6c91de", 18},
    {"LRC", "0x46d0ce7de6247b0a95f67b43b589b4041bae7fbe", 18},
    {"FXS", "0x9d2f299715d94d8a7e6f5eaa8e654e8c74a988a7", 18},
    {"GMX", "0xfc5a1a6eb076a2c7ad06ed22c90d7e710e35ad0a", 18},
    {"UMA", "0xd693ec944a85eeca4247ec1c3b130dca9b0c3b22", 18},
    {"TRB", "0xd58d345fd9c82262e087d2d0607624b410d88242", 18},
    {"tBTC", "0x6c84a8f1c29108F47a79964b5Fe888D4f4D0dE40", 18},
    {"YFI", "0x82e3a8f066a6989666b031d916c43672085b1582", 18},
    {"SUSHI", "0xd4d42f0b6def4ce0383636770ef773390d85c61a", 18},
    {"COTI", "0x6fe14d3cc2f7bddffba5cdb3bbe7467dd81ea101", 18},
    {"BAL", "0x040d1edc9569d4bab2d15287dc5a4f10f56a56b8", 18},
    {"OHM", "0x6e6a3d8f1affac703b1aef1f43b8d2321be40043", 9},
    {"POND", "0xda0a57b710768ae17941a9fa33f8b720c8bd9ddd", 18},
    {"JOE", "0x371c7ec6d8039ff7933a2aa28eb827ffe1f52f07", 18},
    {"SYN", "0x080f6aed32fc474dd5717105dba5ea57268f46eb", 18},
    {"CTSI", "0x319f865b287fcc10b30d8ce6144e8b6d1b476999", 18},
    {"CELR", "0x3a8b787f78d775aecfeea15706d4221b40f345ab", 18},
    {"LADYS", "0x3b60FF35D3f7F62d636b067dD0dC0dFdAd670E4E", 18},
    {"USTC", "0x13780E6d5696DD91454F6d3BbC2616687fEa43d0", 6},
    {"ORBS", "0xf3c091ed43de9c270593445163a41a876a0bb3dd", 18},
    {"SDEX", "0xabd587f2607542723b17f14d00d99b987c29b074", 18},
    {"ACX", "0x53691596d1bce8cea565b84d4915e69e03d9c99d", 18},
    {"DODO", "0x69eb4fa4a2fbd498c257c57ea8b7655a2559a581", 18},
    {"LON", "0x55678cd083fcdc2947a0df635c93c838c89454a3", 18},
    {"GNS", "0x18c11FD286C5EC11c3b683Caa813B77f5163A122", 18},
    {"TGT", "0x429fEd88f10285E61b12BDF00848315fbDfCC341", 18},
    //Optimism
    {"USDT", "0x94b008aa00579c1307b0ef2c499ad98a8ce58e58", 6},
    {"USDC", "0x0b2c639c533813f4aa9d7837caf62653d097ff85", 6},
    {"USDC.e", "0x7f5c764cbc14f9669b88837ca1490cca17c31607", 6},
    {"WBTC", "0x68f180fcce6836688e9084f035309e29bf0a2095", 8},
    {"LINK", "0x350a791bfc2c21f9ed5d10980dad2e2638ffa7f6", 18},
    {"DAI", "0xda10009cbd5d07dd0cecc66161fc93d7c9000da1", 18},
    {"OP", "0x4200000000000000000000000000000000000042", 18},
    {"LDO", "0xfdb794692724153d1488ccdbe0c56c252596735f", 18},
    {"PYTH", "0x99C59ACeBFEF3BBFB7129DC90D1a11DB0E91187f", 6},
    {"WLD", "0xdc6ff44d5d932cbd77b52e5612ba0529dc6226f1", 18},
    {"W", "0xb0ffa8000886e57f86dd5264b9582b2ad87b2b91", 18},
    {"PENDLE", "0xbc7b1ff1c6989f006a1185318ed4e7b5796e66e1", 18},
    //zkSync Era
    {"USDC", "0x1d17CBcF0D6D143135aE902365D2E5e2A16538D4", 6},
    {"USDC.e", "0x3355df6D4c9C3035724Fd0e3914dE96A5a83aaf4", 6},
    {"USDT", "0x493257fD37EDB34451f62EDf8D2a0C418852bA4C", 6},
    {"WBTC", "0xBBeB516fb02a01611cBBE0453Fe3c580D7281011", 8},
    {"BNB", "0x96e4069B746bD88Db76eE126acfDA537DdcEe6FF", 18},
    {"TON", "0x4E14EC08875c88f9B0Cf2A075F481EDa0143d1f0", 9},
    {"DAI", "0x4B9eb6c0b6ea15176BBF62841C6B2A8a398cb656", 18},
    //AVAX C-chain
    {"USDT", "0x9702230A8Ea53601f5cD2dc00fDBc13d4dF4A8c7", 6},
    {"USDT.e", "0xc7198437980c041c805A1EDcbA50c1Ce5db95118", 6},
    {"USDC", "0xB97EF9Ef8734C71904D8002F8b6Bc66Dd9c48a6E", 6},
    {"USDC.e", "0xA7D7079b0FEaD91F3e65f86E8915Cb59c1a4C664", 6},
    {"BTC.b", "0x152b9d0FdC40C096757F570A51E494bd4b943E50", 18},
    {"PNG", "0x60781C2586D68229fde47564546784ab3fACA982", 18},
    {"WAVAX", "0xB31f66AA3C1e785363F0875A1B74E27b85FD66c7", 18},
    {"JOE", "0x6e84a6216eA6dACC71eE8E6b0a5B7322EEbC0fDd", 18},
    {"WETH.e", "0x49D5c2BdFfac6CE2BFdB6640F4F80f226bc10bAB", 18},
};
#include "abi_ethereum.h"
#include "gui_constants.h"
extern const ABIItem_t ethereum_abi_map[];

static uint8_t GetEthPublickeyIndex(char* rootPath);

void GuiSetEthUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi)
{
    g_urResult = urResult;
    g_urMultiResult = urMultiResult;
    g_isMulti = multi;
    g_viewType = g_isMulti ? g_urMultiResult->t : g_urResult->t;
    g_isPermitSingle = false;
    g_isPermit = false;
    g_isOperation = false;
}

#define CHECK_FREE_PARSE_RESULT(result)                                                                                           \
    if (result != NULL)                                                                                                           \
    {                                                                                                                             \
        switch (g_viewType)                                                                                                       \
        {                                                                                                                         \
        case EthTx:                                                                                                               \
            free_TransactionParseResult_DisplayETH((PtrT_TransactionParseResult_DisplayETH)result);                               \
            break;                                                                                                                \
        case EthPersonalMessage:                                                                                                  \
            free_TransactionParseResult_DisplayETHPersonalMessage((PtrT_TransactionParseResult_DisplayETHPersonalMessage)result); \
            break;                                                                                                                \
        case EthTypedData:                                                                                                        \
            free_TransactionParseResult_DisplayETHTypedData((PtrT_TransactionParseResult_DisplayETHTypedData)result);             \
            break;                                                                                                                \
        default:                                                                                                                  \
            break;                                                                                                                \
        }                                                                                                                         \
        result = NULL;                                                                                                            \
    }

// The results here are released in the close qr timer species
static UREncodeResult *GetEthSignDataDynamic(bool isUnlimited)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    UREncodeResult *encodeResult;
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    // get the urType
    enum QRCodeType urType = URTypeUnKnown;
    if (g_isMulti) {
        urType = g_urMultiResult->ur_type;
    } else {
        urType = g_urResult->ur_type;
    }
    do {
        uint8_t seed[64];
        int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();
        GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
        if (isUnlimited) {
            if (urType == Bytes) {
                uint8_t mfp[4] = {0};
                GetMasterFingerPrint(mfp);
                // sign the bytes from keystone hot wallet
                encodeResult = eth_sign_tx_bytes(data, seed, len, mfp, sizeof(mfp));
            } else {
                encodeResult = eth_sign_tx_unlimited(data, seed, len);
            }
        } else {
            if (urType == Bytes) {
                uint8_t mfp[4] = {0};
                GetMasterFingerPrint(mfp);
                encodeResult = eth_sign_tx_bytes(data, seed, len, mfp, sizeof(mfp));
            } else {
                encodeResult = eth_sign_tx(data, seed, len);
            }
        }
        ClearSecretCache();
        CHECK_CHAIN_BREAK(encodeResult);
    } while (0);
    SetLockScreen(enable);
    return encodeResult;
}

static bool isErc20Transfer(void *param)
{
    DisplayETH *eth = (DisplayETH *)param;
    char *input = eth->detail->input;
    if (strnlen_s(input, 9) <= 8) {
        return false;
    }
    // FIXME: 0xa9059cbb is the method of erc20 transfer
    const char *erc20Method = "a9059cbb";
    if (strncmp(input, erc20Method, 8) == 0) {
        return true;
    }

    return false;
}

static char *CalcSymbol(void *param)
{
    DisplayETH *eth = (DisplayETH *)param;

    //eth->detail->to: the actual contract address;
    if (isErc20Transfer(eth) && eth->detail->to != NULL) {
        for (size_t i = 0; i < NUMBER_OF_ARRAYS(ERC20_CONTRACTS); i++) {
            Erc20Contract_t contract = ERC20_CONTRACTS[i];
            if (strcasecmp(contract.contract_address, eth->detail->to) == 0) {
                return contract.symbol;
            }
        }
    }

    if (isErc20Transfer(eth)) {
        return "Token";
    }

    EvmNetwork_t network = FindEvmNetwork(eth->chain_id);
    return network.symbol;
}

UREncodeResult *GuiGetEthSignQrCodeData(void)
{
    return GetEthSignDataDynamic(false);
}

UREncodeResult *GuiGetEthSignUrDataUnlimited(void)
{
    return GetEthSignDataDynamic(true);
}

static void UpdatePermitFlag(const char *primaryType)
{
    printf("primaryType: %s\n", primaryType);
    if (!strcmp("Permit", primaryType) || !strcmp("PermitSingle", primaryType) || !strcmp("PermitBatch", primaryType)) {
        g_isPermit = true;
        if (!strcmp("Permit", primaryType)) {
            g_isPermitSingle = true;
        } else {
            g_isPermitSingle = false;
        }
    } else {
        g_isPermit = false;
        g_isPermitSingle = false;
    }
}

void *GuiGetEthTypeData(void)
{
    CHECK_FREE_PARSE_RESULT(g_parseResult);
    uint8_t mfp[4];
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    char *rootPath = eth_get_root_path(data);
    char *ethXpub = GetCurrentAccountPublicKey(GetEthPublickeyIndex(rootPath));
    GetMasterFingerPrint(mfp);
    TransactionCheckResult *result = NULL;
    do {
        result = eth_check(data, mfp, sizeof(mfp));
        CHECK_CHAIN_BREAK(result);
        PtrT_TransactionParseResult_DisplayETHTypedData parseResult = eth_parse_typed_data(data, ethXpub);
        cJSON *json = cJSON_Parse(parseResult->data->message);
        cJSON *operation = cJSON_GetObjectItem(json, "operation");
        if (operation) {
            uint32_t operationValue;
            sscanf(operation->valuestring, "%d", &operationValue);
            g_isOperation = operationValue == 1;
        }
        cJSON_Delete(json);
        CHECK_CHAIN_BREAK(parseResult);
        g_parseResult = (void *)parseResult;
        UpdatePermitFlag(parseResult->data->primary_type);
    } while (0);
    free_TransactionCheckResult(result);
    free_ptr_string(rootPath);
    return g_parseResult;
}

void GetEthTypedDataDomianName(void *indata, void *param, uint32_t maxLen)
{
    DisplayETHTypedData *message = (DisplayETHTypedData *)param;
    if (message->name != NULL) {
        strcpy_s((char *)indata, maxLen, message->name);
    } else {
        strcpy_s((char *)indata, maxLen, "");
    }
}

void _colorfulHash(char *hash, char *indata, uint32_t maxLen)
{
    size_t len = strlen(hash);
    if (len >= 19) {
        char prefix[9] = {0};
        char suffix[9] = {0};
        char middle[233] = {0};
        strncpy(prefix, hash, 8);
        strncpy(suffix, hash + (len - 8), 8);
        strncpy(middle, hash + 8, len - 16);

        snprintf_s((char *)indata, maxLen, "#F5870A %s#%s#F5870A %s#", prefix, middle, suffix);
    } else {
        strcpy_s((char *)indata, maxLen, hash);
    }

}

void GetEthTypedDataDomainHash(void *indata, void *param, uint32_t maxLen)
{
    DisplayETHTypedData *message = (DisplayETHTypedData *)param;
    if (message->domain_hash != NULL) {
        // first 8 char and last 8 char color #F5870A
        char *hash = message->domain_hash;
        // if not start with 0x, add 0x
        if (hash[0] != '0' || (hash[1] != 'x' && hash[1] != 'X')) {
            strcpy_s((char *)indata, maxLen, "0x");
            strcat_s((char *)indata, maxLen, hash);
            _colorfulHash((char *)indata, (char *)indata, maxLen);
        } else {
            _colorfulHash(hash, (char *)indata, maxLen);
        }
    } else {
        strcpy_s((char *)indata, maxLen, "");
    }
}

bool GetEthTypeDataHashExist(void *indata, void *param)
{
    DisplayETHTypedData *message = (DisplayETHTypedData *)param;
    if (strncmp(message->primary_type, "SafeTx", 6) == 0) {
        return true;
    }
    return false;
}

bool GetEthTypeDataChainExist(void *indata, void *param)
{
    DisplayETHTypedData *message = (DisplayETHTypedData *)param;
    return message->chain_id != NULL;
}

bool GetEthTypeDataVersionExist(void *indata, void *param)
{
    DisplayETHTypedData *message = (DisplayETHTypedData *)param;
    return message->version != NULL;
}

void GetEthTypedDataMessageHash(void *indata, void *param, uint32_t maxLen)
{
    DisplayETHTypedData *message = (DisplayETHTypedData *)param;
    if (message->message_hash != NULL) {
        // first 8 char and last 8 char color #F5870A
        char *hash = message->message_hash;
        // if not start with 0x, add 0x
        if (hash[0] != '0' || (hash[1] != 'x' && hash[1] != 'X')) {
            strcpy_s((char *)indata, maxLen, "0x");
            strcat_s((char *)indata, maxLen, hash);
            _colorfulHash((char *)indata, (char *)indata, maxLen);
        } else {
            _colorfulHash(hash, (char *)indata, maxLen);
        }
    } else {
        strcpy_s((char *)indata, maxLen, "");
    }
}

void GetEthTypedDataSafeTxHash(void *indata, void *param, uint32_t maxLen)
{
    DisplayETHTypedData *message = (DisplayETHTypedData *)param;
    if (message->safe_tx_hash != NULL) {
        char *hash = message->safe_tx_hash;
        _colorfulHash(hash, (char *)indata, maxLen);
    } else {
        strcpy_s((char *)indata, maxLen, "");
    }
}

void GetEthTypedDataDomianVersion(void *indata, void *param, uint32_t maxLen)
{
    DisplayETHTypedData *message = (DisplayETHTypedData *)param;
    if (message->version != NULL) {
        snprintf_s((char *)indata, maxLen, "v%s", message->version);
    } else {
        strcpy_s((char *)indata, maxLen, "");
    }
}

void GetEthTypedDataDomianChainId(void *indata, void *param, uint32_t maxLen)
{
    DisplayETHTypedData *message = (DisplayETHTypedData *)param;
    if (message->chain_id != NULL) {
        snprintf_s((char *)indata, maxLen, "%s (%s)", message->chain_id, FindEvmNetwork(atoi(message->chain_id)).name);
    } else {
        strcpy_s((char *)indata, maxLen, "");
    }
}

void GetEthTypedDataDomianVerifyContract(void *indata, void *param, uint32_t maxLen)
{
    DisplayETHTypedData *message = (DisplayETHTypedData *)param;
    if (message->verifying_contract != NULL) {
        strcpy_s((char *)indata, maxLen, message->verifying_contract);
    } else {
        strcpy_s((char *)indata, maxLen, "");
    }
}

void GetEthTypedDataDomianSalt(void *indata, void *param, uint32_t maxLen)
{
    DisplayETHTypedData *message = (DisplayETHTypedData *)param;
    if (message->salt != NULL) {
        strcpy_s((char *)indata, maxLen, message->salt);
    } else {
        strcpy_s((char *)indata, maxLen, "");
    }
}

void GetEthTypedDataPrimayType(void *indata, void *param, uint32_t maxLen)
{
    DisplayETHTypedData *message = (DisplayETHTypedData *)param;
    if (message->primary_type != NULL) {
        strcpy_s((char *)indata, maxLen, message->primary_type);
    } else {
        strcpy_s((char *)indata, maxLen, "");
    }
}

void GetEthTypedDataMessage(void *indata, void *param, uint32_t maxLen)
{
    DisplayETHTypedData *message = (DisplayETHTypedData *)param;
    if (message->message != NULL) {
        snprintf((char *)indata, maxLen, "%s", message->message);
    } else {
        strcpy_s((char *)indata, maxLen, "");
    }
}

int GetEthTypedDataMessageLen(void *param)
{
    DisplayETHTypedData *message = (DisplayETHTypedData *)param;
    return strlen(message->message) + 3;
}

void GetEthTypedDataFrom(void *indata, void *param, uint32_t maxLen)
{
    DisplayETHTypedData *message = (DisplayETHTypedData *)param;
    if (message->from != NULL) {
        strcpy_s((char *)indata, maxLen, message->from);
    } else {
        strcpy_s((char *)indata, maxLen, "");
    }
}

void *GuiGetEthPersonalMessage(void)
{
    CHECK_FREE_PARSE_RESULT(g_parseResult);
    uint8_t mfp[4];
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    char *rootPath = eth_get_root_path(data);
    char *ethXpub = GetCurrentAccountPublicKey(GetEthPublickeyIndex(rootPath));
    GetMasterFingerPrint(mfp);
    TransactionCheckResult *result = NULL;
    do {
        result = eth_check(data, mfp, sizeof(mfp));
        CHECK_CHAIN_BREAK(result);
        PtrT_TransactionParseResult_DisplayETHPersonalMessage parseResult = eth_parse_personal_message(data, ethXpub);
        CHECK_CHAIN_BREAK(parseResult);
        g_parseResult = (void *)parseResult;
    } while (0);
    free_TransactionCheckResult(result);
    free_ptr_string(rootPath);
    return g_parseResult;
}

void GetEthPersonalMessageType(void *indata, void *param, uint32_t maxLen)
{
    DisplayETHPersonalMessage *message = (DisplayETHPersonalMessage *)param;
    if (message->utf8_message) {
        strcpy_s((char *)indata, maxLen, "utf8_message");
    } else {
        strcpy_s((char *)indata, maxLen, "raw_message");
    }
}

void GetMessageFrom(void *indata, void *param, uint32_t maxLen)
{
    DisplayETHPersonalMessage *message = (DisplayETHPersonalMessage *)param;
    if (strlen(message->from) >= maxLen) {
        snprintf((char *)indata, maxLen - 3, "%s", message->from);
        strcat((char *)indata, "...");
    } else {
        strcpy_s((char *)indata, maxLen, message->from);
    }
}
void GetMessageUtf8(void *indata, void *param, uint32_t maxLen)
{
    DisplayETHPersonalMessage *message = (DisplayETHPersonalMessage *)param;
    if (strlen(message->utf8_message) >= maxLen) {
        snprintf((char *)indata, maxLen - 3, "%s", message->utf8_message);
        strcat((char *)indata, "...");
    } else {
        snprintf((char *)indata, maxLen, "%s", message->utf8_message);
    }
}

void GetMessageRaw(void *indata, void *param, uint32_t maxLen)
{
    int len = strlen("\n#F5C131 The data is not parseable. Please#\n#F5C131 refer to the software wallet interface#\n#F5C131 for viewing.#");
    DisplayETHPersonalMessage *message = (DisplayETHPersonalMessage *)param;
    if (strlen(message->raw_message) >= maxLen - len) {
        snprintf((char *)indata, maxLen - 3 - len, "%s", message->raw_message);
        strcat((char *)indata, "...");
    } else {
        snprintf((char *)indata, maxLen, "%s%s", message->raw_message, "\n#F5C131 The data is not parseable. Please#\n#F5C131 refer to the software wallet interface#\n#F5C131 for viewing.#");
    }
}

static uint8_t GetEthPublickeyIndex(char* rootPath)
{
    if (strcmp(rootPath, "44'/60'/0'") == 0) return XPUB_TYPE_ETH_BIP44_STANDARD;
    if (strcmp(rootPath, "44'/60'/1'") == 0) return XPUB_TYPE_ETH_LEDGER_LIVE_1;
    if (strcmp(rootPath, "44'/60'/2'") == 0) return XPUB_TYPE_ETH_LEDGER_LIVE_2;
    if (strcmp(rootPath, "44'/60'/3'") == 0) return XPUB_TYPE_ETH_LEDGER_LIVE_3;
    if (strcmp(rootPath, "44'/60'/4'") == 0) return XPUB_TYPE_ETH_LEDGER_LIVE_4;
    if (strcmp(rootPath, "44'/60'/5'") == 0) return XPUB_TYPE_ETH_LEDGER_LIVE_5;
    if (strcmp(rootPath, "44'/60'/6'") == 0) return XPUB_TYPE_ETH_LEDGER_LIVE_6;
    if (strcmp(rootPath, "44'/60'/7'") == 0) return XPUB_TYPE_ETH_LEDGER_LIVE_7;
    if (strcmp(rootPath, "44'/60'/8'") == 0) return XPUB_TYPE_ETH_LEDGER_LIVE_8;
    if (strcmp(rootPath, "44'/60'/9'") == 0) return XPUB_TYPE_ETH_LEDGER_LIVE_9;

    return -1;
}

// pase result
void *GuiGetEthData(void)
{
    memset_s(g_fromEthEnsName, sizeof(g_fromEthEnsName), 0, sizeof(g_fromEthEnsName));
    memset_s(g_toEthEnsName, sizeof(g_toEthEnsName), 0, sizeof(g_toEthEnsName));
    g_contractDataExist = false;
    g_erc20Name = NULL;
    CHECK_FREE_PARSE_RESULT(g_parseResult);
    uint8_t mfp[4];
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;

    enum ViewType viewType = ViewTypeUnKnown;
    enum QRCodeType urType = URTypeUnKnown;
    void *crypto = NULL;
    if (g_isMulti) {
        crypto = g_urMultiResult->data;
        urType = g_urMultiResult->ur_type;
        viewType = g_urMultiResult->t;
    } else {
        crypto = g_urResult->data;
        urType = g_urResult->ur_type;
    }
    char *rootPath = NULL;
    if (urType == Bytes) {
        rootPath  = eth_get_root_path_bytes(data);
    } else {
        rootPath = eth_get_root_path(data);
    }
    char *ethXpub = GetCurrentAccountPublicKey(GetEthPublickeyIndex(rootPath));
    GetMasterFingerPrint(mfp);
    PtrT_TransactionParseResult_DisplayETH parseResult = NULL;
    do {
        if (urType == Bytes) {
            parseResult = eth_parse_bytes_data(data, ethXpub);
        } else {
            parseResult = eth_parse(data, ethXpub);
        }
        CHECK_CHAIN_BREAK(parseResult);
        g_parseResult = (void *)parseResult;
        g_fromEnsExist = GetEnsName((const char *)parseResult->data->overview->from, g_fromEthEnsName);
        g_toEnsExist = GetEnsName((const char *)parseResult->data->overview->to, g_toEthEnsName);
        decodeEthContractData(parseResult);
    } while (0);
    free_ptr_string(rootPath);
    return g_parseResult;
}

PtrT_TransactionCheckResult GuiGetEthCheckResult(void)
{
    uint8_t mfp[4];
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    enum ViewType viewType = ViewTypeUnKnown;
    enum QRCodeType urType = URTypeUnKnown;
    void *crypto = NULL;
    if (g_isMulti) {
        crypto = g_urMultiResult->data;
        urType = g_urMultiResult->ur_type;
        viewType = g_urMultiResult->t;
    } else {
        crypto = g_urResult->data;
        urType = g_urResult->ur_type;
    }
    GetMasterFingerPrint(mfp);
    // get the urType
    if (urType == Bytes) {
        return eth_check_ur_bytes(data, mfp, sizeof(mfp), urType);
    } else {
        return eth_check(data, mfp, sizeof(mfp));
    }
}

void GetEthTransType(void *indata, void *param, uint32_t maxLen)
{
    DisplayETH *eth = (DisplayETH *)param;
    strcpy_s((char *)indata, maxLen, eth->tx_type);
}

void GetEthTxFee(void *indata, void *param, uint32_t maxLen)
{
    DisplayETH *eth = (DisplayETH *)param;
    if (eth->overview->max_txn_fee != NULL) {
        snprintf_s((char *)indata,  maxLen, "%s %s", eth->overview->max_txn_fee, FindEvmNetwork(eth->chain_id).symbol);
    } else {
        snprintf_s((char *)indata,  maxLen, "0 %s", FindEvmNetwork(eth->chain_id).symbol);
    }
}

EvmNetwork_t FindEvmNetwork(uint64_t chainId)
{
    for (size_t i = 0; i < NUMBER_OF_ARRAYS(NETWORKS); i++) {
        EvmNetwork_t network = NETWORKS[i];
        if (chainId == network.chainId) {
            return network;
        }
    }
    return NETWORKS[0];
}

void *FindErc20Contract(char *contract_address)
{
    for (size_t i = 0; i < NUMBER_OF_ARRAYS(ERC20_CONTRACTS); i++) {
        Erc20Contract_t contract = ERC20_CONTRACTS[i];
        if (strcasecmp(contract.contract_address, contract_address) == 0) {
            Erc20Contract_t *result = malloc(sizeof(Erc20Contract_t));
            result->contract_address = contract.contract_address;
            result->decimals = contract.decimals;
            result->symbol = contract.symbol;
            return result;
        }
    }
    return NULL;
}

void GetEthValue(void *indata, void *param, uint32_t maxLen)
{
    DisplayETH *eth = (DisplayETH *)param;
    if (isErc20Transfer(eth)) {
        TransactionParseResult_EthParsedErc20Transaction *contract = (TransactionParseResult_EthParsedErc20Transaction *)g_erc20ContractData;
        snprintf_s((char *)indata,  maxLen, "%s %s", contract->data->value, CalcSymbol(param));
    } else {
        snprintf_s((char *)indata,  maxLen, "%s %s", eth->overview->value, CalcSymbol(param));
    }
}

void GetEthGasPrice(void *indata, void *param, uint32_t maxLen)
{
    DisplayETH *eth = (DisplayETH *)param;
    strcpy_s((char *)indata, maxLen, eth->overview->gas_price);
}

void GetEthGasLimit(void *indata, void *param, uint32_t maxLen)
{
    DisplayETH *eth = (DisplayETH *)param;
    strcpy_s((char *)indata, maxLen, eth->overview->gas_limit);
}

void GetEthNetWork(void *indata, void *param, uint32_t maxLen)
{
    DisplayETH *eth = (DisplayETH *)param;
    EvmNetwork_t network = FindEvmNetwork(eth->chain_id);
    if (network.chainId == 0) {
        snprintf_s((char *)indata,  maxLen, "ID: %lu", eth->chain_id);
        return;
    }
    strcpy_s((char *)indata, maxLen, network.name);
}

void GetEthMaxFee(void *indata, void *param, uint32_t maxLen)
{
    DisplayETH *eth = (DisplayETH *)param;
    if (eth->detail->max_fee != NULL) {
        snprintf_s((char *)indata,  maxLen, "%s %s", eth->detail->max_fee, FindEvmNetwork(eth->chain_id).symbol);
    } else {
        snprintf_s((char *)indata,  maxLen, "0 %s", FindEvmNetwork(eth->chain_id).symbol);
    }
}

void GetEthMaxPriority(void *indata, void *param, uint32_t maxLen)
{
    DisplayETH *eth = (DisplayETH *)param;
    if (eth->detail->max_priority != NULL) {
        snprintf_s((char *)indata,  maxLen, "%s %s", eth->detail->max_priority, FindEvmNetwork(eth->chain_id).symbol);
    } else {
        snprintf_s((char *)indata,  maxLen, "0 %s", FindEvmNetwork(eth->chain_id).symbol);
    }
}

void GetEthMaxFeePrice(void *indata, void *param, uint32_t maxLen)
{
    DisplayETH *eth = (DisplayETH *)param;
    strcpy_s((char *)indata, maxLen, eth->detail->max_fee_price);
}

void GetEthMaxPriorityFeePrice(void *indata, void *param, uint32_t maxLen)
{
    DisplayETH *eth = (DisplayETH *)param;
    strcpy_s((char *)indata, maxLen, eth->detail->max_priority_price);
}

void GetEthGetFromAddress(void *indata, void *param, uint32_t maxLen)
{
    DisplayETH *eth = (DisplayETH *)param;
    strcpy_s((char *)indata, maxLen, eth->overview->from);
}

void GetEthGetSignerAddress(void *indata, void *param, uint32_t maxLen)
{
    DisplayETHTypedData *message = (DisplayETHTypedData *)param;
    strcpy_s((char *)indata, maxLen, message->from);
}

void GetEthGetToAddress(void *indata, void *param, uint32_t maxLen)
{
    DisplayETH *eth = (DisplayETH *)param;
    if (isErc20Transfer(eth)) {
        TransactionParseResult_EthParsedErc20Transaction *contract = (TransactionParseResult_EthParsedErc20Transaction *)g_erc20ContractData;
        strcpy_s((char *)indata, maxLen, contract->data->to);
    } else {
        strcpy_s((char *)indata, maxLen, eth->overview->to);
    }
}

void GetEthGetDetailPageToAddress(void *indata, void *param, uint32_t maxLen)
{
    DisplayETH *eth = (DisplayETH *)param;
    if (isErc20Transfer(eth)) {
        strcpy_s((char *)indata, maxLen, eth->detail->to);
    } else {
        strcpy_s((char *)indata, maxLen, eth->overview->to);
    }
}


void GetTxnFeeDesc(void *indata, void *param, uint32_t maxLen)
{
    strcpy_s((char *)indata, maxLen, "  \xE2\x80\xA2  Max Txn Fee = Gas Price * Gas Limit");
}

void GetEthEnsName(void *indata, void *param, uint32_t maxLen)
{
    strcpy_s((char *)indata, maxLen, g_fromEthEnsName);
}

void GetToEthEnsName(void *indata, void *param, uint32_t maxLen)
{
    strcpy_s((char *)indata, maxLen, g_toEthEnsName);
}

void GetEthNonce(void *indata, void *param, uint32_t maxLen)
{
    DisplayETH *eth = (DisplayETH *)param;
    strcpy_s((char *)indata, maxLen, eth->detail->nonce);
}

void GetEthInputData(void *indata, void *param, uint32_t maxLen)
{
    DisplayETH *eth = (DisplayETH *)param;
    snprintf((char *)indata, maxLen, "0x%s", eth->detail->input);
}

int GetEthInputDataLen(void *param)
{
    DisplayETH *eth = (DisplayETH *)param;
    return strlen(eth->detail->input) + 3;
}

bool GetEthEnsExist(void *indata, void *param)
{
    return g_fromEnsExist;
}

bool GetToEthEnsExist(void *indata, void *param)
{
    return g_toEnsExist;
}

bool GetEthInputDataExist(void *indata, void *param)
{
    DisplayETH *eth = (DisplayETH *)param;
    return strlen(eth->detail->input) > 0;
}

bool EthInputExistContractNot(void *indata, void *param)
{
    DisplayETH *eth = (DisplayETH *)param;
    return !g_contractDataExist && strlen(eth->detail->input) > 0;
}

void GetEthToFromSize(uint16_t *width, uint16_t *height, void *param)
{
    *width = 408;
    *height = 244 + (g_fromEnsExist + g_toEnsExist) * (GAP + TEXT_LINE_HEIGHT) + g_contractDataExist * (GAP + TEXT_LINE_HEIGHT);
}

void GetEthTypeDomainSize(uint16_t *width, uint16_t *height, void *param)
{
    *width = 408;
    *height = 298 + (98 + 16) * GetEthTypeDataHashExist(NULL, param) +
              GetEthTypeDataChainExist(NULL, param) * 90 +
              GetEthTypeDataVersionExist(NULL, param) * 90;
}

void GetEthToLabelPos(uint16_t *x, uint16_t *y, void *param)
{
    *x = 24;
    *y = 130 + g_fromEnsExist * 38;
}

void GetEthTypeDomainPos(uint16_t *x, uint16_t *y, void *param)
{
    *x = 36;
    if (g_isOperation) {
        *y = 228;
    } else {
        *y = (152 + 16) * g_isPermit + 26;
    }
}

bool GetEthContractDataExist(void *indata, void *param)
{
    return g_contractDataExist;
}

bool GetEthContractDataNotExist(void *indata, void *param)
{
    return !g_contractDataExist;
}

void GetEthTransactionData(void *indata, void *param, uint32_t maxLen)
{
    DisplayETH *eth = (DisplayETH *)param;
    if (strlen(eth->detail->input) > 51) {
        char data[49];
        strncpy(data, eth->detail->input, 48);
        data[48] = '\0';
        snprintf_s((char *)indata,  maxLen, "0x%s...", data);
    } else {
        snprintf_s((char *)indata,  maxLen, "0x%s", eth->detail->input);
    }
}

void GetEthMethodName(void *indata, void *param, uint32_t maxLen)
{
    Response_DisplayContractData *contractData = (Response_DisplayContractData *)g_contractData;
    strcpy_s((char *)indata, maxLen, contractData->data->method_name);
}

void GetEthContractName(void *indata, void *param, uint32_t maxLen)
{
    Response_DisplayContractData *contractData = (Response_DisplayContractData *)g_contractData;
    if (g_erc20Name != NULL && strlen(g_erc20Name) > 0) {
        strcpy_s((char *)indata, maxLen, g_erc20Name);
        return;
    }
    // add contract address string
    if (strlen(contractData->data->contract_name) > 0) {
        strcpy_s((char *)indata, maxLen, contractData->data->contract_name);
    } else {
        snprintf_s((char *)indata,  maxLen, "Unknown Contract Name");
    }
}

void GetEthContractDataSize(uint16_t *width, uint16_t *height, void *param)
{
    *width = 408;
    Response_DisplayContractData *contractData = (Response_DisplayContractData *)g_contractData;
    *height = PADDING;
    if (g_contractDataExist) {
        *height += TEXT_LINE_HEIGHT; // "Method"
        *height += GAP;
        *height += TEXT_LINE_HEIGHT; // method name
        *height += GAP;

        for (size_t i = 0; i < contractData->data->params->size; i++) {
            *height += TEXT_LINE_HEIGHT; // param name
            *height += GAP;
            *height += TEXT_LINE_HEIGHT; // param value
            *height += GAP;
        }
    } else {
        *height += CONTRACT_MIN_HEIGHT; // input data
        *height += GAP;
        *height += CONTRACT_LEARN_MORE_HEIGHT;
    }
    *height += PADDING;
}

void *GetEthContractData(uint8_t *row, uint8_t *col, void *param)
{
    Response_DisplayContractData *contractData = (Response_DisplayContractData *)g_contractData;
    *col = 1;
    *row = contractData->data->params->size * 2; // name + value
    int i = 0, j = 0;
    char ***indata = (char ***)SRAM_MALLOC(sizeof(char **) * *col);
    for (i = 0; i < *col; i++) {
        if (*row == 0) {
            indata[i] = NULL;
            continue;
        }
        indata[i] = SRAM_MALLOC(sizeof(char *) * *row);
        for (j = 0; j < *row; j++) {
            int index = j / 2;
            DisplayContractParam param = contractData->data->params->data[index];
            if (!(j % 2)) {
                uint32_t len = strnlen_s(param.name, BUFFER_SIZE_128) + 10;
                indata[i][j] = SRAM_MALLOC(len);
                snprintf_s(indata[i][j], len, "#919191 %s#", param.name);
            } else {
                // if param.value length > 512, we only show first 512-3 char
                if (strlen(param.value) > BUFFER_SIZE_512) {
                    uint32_t len = strlen(param.value) + 1;
                    char *suffix = "...";
                    indata[i][j] = SRAM_MALLOC(len);
                    strncpy(indata[i][j], param.value, 509);
                    strncat_s(indata[i][j], sizeof(char) * (strlen(param.value) + 1), suffix, 3);
                } else {
                    uint32_t len = strnlen_s(param.value, BUFFER_SIZE_512) + 1;
                    indata[i][j] = SRAM_MALLOC(len);
                    strcpy_s(indata[i][j], len, param.value);
                }
            }
        }
    }
    return (void *)indata;
}

static void decodeEthContractData(void *parseResult)
{
    TransactionParseResult_DisplayETH *result = (TransactionParseResult_DisplayETH *)parseResult;
    char *contractAddress = result->data->detail->to;
    if (strlen(result->data->detail->input) <= 8) {
        // not a valid contract data;
        return;
    }
    if (GetEthErc20ContractData(parseResult)) {
        return;
    }

    if (!GetEthContractFromInternal(contractAddress, result->data->detail->input)) {
        char selectorId[9] = {0};
        strncpy(selectorId, result->data->detail->input, 8);
        GetEthContractFromExternal(contractAddress, selectorId, result->data->chain_id, result->data->detail->input);
    }
}

static void FixRecipientAndValueWhenErc20Contract(const char *inputdata, uint8_t decimals)
{
    TransactionParseResult_DisplayETH *result = (TransactionParseResult_DisplayETH *)g_parseResult;
    if (!isErc20Transfer(result->data)) {
        return;
    }
    PtrT_TransactionParseResult_EthParsedErc20Transaction contractData = eth_parse_erc20((PtrString)inputdata, decimals);
    g_erc20ContractData = contractData;
    // result->data->detail->to = contractData->data->to;
    // result->data->overview->to = contractData->data->to;
    // result->data->detail->value = contractData->data->value;
    // result->data->overview->value = contractData->data->value;
}

static bool GetEthErc20ContractData(void *parseResult)
{
    g_erc20Name = NULL;
    TransactionParseResult_DisplayETH *result = (TransactionParseResult_DisplayETH *)parseResult;
    Response_DisplayContractData *contractData = eth_parse_contract_data(result->data->detail->input, (char *)ethereum_erc20_json);
    if (contractData->error_code == 0) {
        g_contractDataExist = true;
        g_contractData = contractData;
    } else {
        free_Response_DisplayContractData(contractData);
        return false;
    }
    char *to = result->data->detail->to;
    for (size_t i = 0; i < NUMBER_OF_ARRAYS(ERC20_CONTRACTS); i++) {
        Erc20Contract_t contract = ERC20_CONTRACTS[i];
        if (strcasecmp(contract.contract_address, to) == 0) {
            g_erc20Name = contract.symbol;
            FixRecipientAndValueWhenErc20Contract(result->data->detail->input, contract.decimals);
            return true;
        }
    }
    g_erc20Name = "Erc20 Contract Address";
    FixRecipientAndValueWhenErc20Contract(result->data->detail->input, 0);
    return true;
}

static bool GetSafeContractData(char* inputData)
{
    char selectorId[9] = {0};
    strncpy(selectorId, inputData, 8);
    if (strcasecmp(selectorId, "6a761202") == 0) {
        Response_DisplayContractData *contractData = eth_parse_contract_data(inputData, (char *)safe_json);
        if (contractData->error_code == 0) {
            g_contractDataExist = true;
            g_contractData = contractData;
        } else {
            printf("safe contract error: %s\n", contractData->error_message);
            free_Response_DisplayContractData(contractData);
            return false;
        }
        return true;
    }
    return false;
}

bool GetEthPermitWarningExist(void *indata, void *param)
{
    return g_isPermit;
}

bool GetEthOperationWarningExist(void *indata, void *param)
{
    return g_isOperation;
}

bool GetEthPermitCantSign(void *indata, void *param)
{
    return (g_isPermitSingle && !GetPermitSign());
}

bool GetEthContractFromInternal(char *address, char *inputData)
{
    if (GetSafeContractData(inputData)) {
        return true;
    }
    // address_key = address + "_" + functionSelector
    char* address_key = (char*)SRAM_MALLOC(strlen(address) + 10);
    snprintf_s(address_key, strlen(address) + 10, "%s_%.8s", address, inputData);
    for (size_t i = 0; i < GetEthereumABIMapSize(); i++) {
        struct ABIItem item = ethereum_abi_map[i];
        if (strcasecmp(item.address, address_key) == 0) {
            Response_DisplayContractData *contractData = eth_parse_contract_data(inputData, (char *)item.json);
            if (contractData->error_code == 0) {
                g_contractDataExist = true;
                g_contractData = contractData;
            } else {
                free_Response_DisplayContractData(contractData);
                SRAM_FREE(address_key);
                return false;
            }
            SRAM_FREE(address_key);
            return true;
        }
    }
    SRAM_FREE(address_key);
    return false;
}

void EthContractLearnMore(lv_event_t *e)
{
    GuiQRCodeHintBoxOpen(_("tx_details_eth_decoding_qr_link"), _("tx_details_eth_decoding_qr_title"), _("tx_details_eth_decoding_qr_link"));
}

lv_obj_t *g_contractRawDataHintbox = NULL;
lv_obj_t *GuiCreateContractRawDataHintbox(const char *titleText, const char *descText)
{
    uint16_t descHeight = 290;
    lv_obj_t *title = NULL, *desc = NULL, *rightBtn = NULL;
    lv_obj_t *cont = GuiCreateHintBox(800);

    if (strlen(descText) > 512) {
        desc = GuiCreateContainerWithParent(lv_obj_get_child(cont, lv_obj_get_child_cnt(cont) - 1), 360, 300);
        lv_obj_set_style_bg_color(desc, DARK_BG_COLOR, LV_PART_MAIN);
        lv_obj_add_flag(desc, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(desc, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_align(desc, LV_ALIGN_BOTTOM_LEFT, 36 + 20, -110);
        char *text = descText;
        int textLen = strlen(text); 
        int segmentSize = 1000;
        int numSegments = (textLen + segmentSize - 1) / segmentSize;

        for (int i = 0; i < numSegments; i++) {
            int offset = i * segmentSize;
            lv_obj_t *label = GuiCreateIllustrateLabel(desc, text + offset);
            lv_obj_set_width(label, 360);
            lv_label_set_recolor(label, true);
            if (i == 0) {
                lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 0);
            } else {
                GuiAlignToPrevObj(label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
            }

            if (i < numSegments - 1 && textLen > offset + segmentSize) {
                char savedChar = text[offset + segmentSize];
                text[offset + segmentSize] = '\0';
                lv_label_set_text(label, text + offset);
                text[offset + segmentSize] = savedChar;
            }
        }
    } else {
        desc = GuiCreateIllustrateLabel(cont, descText);
        lv_obj_set_width(desc, 360);
        lv_obj_align(desc, LV_ALIGN_BOTTOM_LEFT, 36 + 20, -110);
        lv_obj_refr_size(desc);
        descHeight = lv_obj_get_self_height(desc);
    }
    title = GuiCreateLittleTitleLabel(cont, titleText);
    lv_obj_align_to(title, desc, LV_ALIGN_OUT_TOP_LEFT, 0, -16);

    rightBtn = GuiCreateTextBtn(cont, _("Ok"));
    lv_obj_align(rightBtn, LV_ALIGN_BOTTOM_RIGHT, -36, -20);
    lv_obj_set_size(rightBtn, lv_obj_get_self_width(lv_obj_get_child(rightBtn, 0)) + 40, 66);
    lv_obj_set_style_bg_color(rightBtn, WHITE_COLOR_OPA12, LV_PART_MAIN);

    uint32_t height =
        24 + lv_obj_get_self_height(title) + 12 + descHeight + 16 + 114;
    GuiHintBoxResize(cont, height);

    return cont;
}

void EthContractCheckRawData(lv_event_t *e)
{
    GuiNoPendingHintBoxOpen(_("Loading"));
    GuiModelParseTransactionRawData();
}

void EthContractCheckRawDataCallback(void)
{
    char *rawData = ((TransactionParseResult_DisplayETH *)g_parseResult)->data->detail->input;
    g_contractRawDataHintbox = GuiCreateContractRawDataHintbox("Raw Data", rawData);
    lv_obj_t *rightBtn = GuiGetHintBoxRightBtn(g_contractRawDataHintbox);
    lv_obj_add_event_cb(rightBtn, CloseHintBoxHandler, LV_EVENT_CLICKED, &g_contractRawDataHintbox);
    GuiModelTransactionParseRawDataDelay();
}

bool GetEthContractFromExternal(char *address, char *selectorId, uint64_t chainId, char *inputData)
{
    char *contractMethodJson = SRAM_MALLOC(SQL_ABI_BUFF_MAX_SIZE);
    memset_s(contractMethodJson, SQL_ABI_BUFF_MAX_SIZE, 0, SQL_ABI_BUFF_MAX_SIZE);
    char contractName[64] = {0};
    if (GetDBContract(address, selectorId, chainId, contractMethodJson, contractName)) {
        Response_DisplayContractData *contractData = eth_parse_contract_data_by_method(inputData, contractName, contractMethodJson);
        if (contractData->error_code == 0) {
            g_contractDataExist = true;
            g_contractData = contractData;
        } else {
            SRAM_FREE(contractMethodJson);
            free_Response_DisplayContractData(contractData);
            return false;
        }
        SRAM_FREE(contractMethodJson);
        return true;
    }
    SRAM_FREE(contractMethodJson);
    return false;
}

void FreeContractData(void)
{
    if (g_contractData != NULL) {
        free_Response_DisplayContractData(g_contractData);
        g_contractData = NULL;
    }
    if (g_erc20ContractData != NULL) {
        free_TransactionParseResult_EthParsedErc20Transaction(g_erc20ContractData);
        g_erc20ContractData = NULL;
    }
}

void FreeEthMemory(void)
{
    CHECK_FREE_UR_RESULT(g_urResult, false);
    CHECK_FREE_UR_RESULT(g_urMultiResult, true);
    CHECK_FREE_PARSE_RESULT(g_parseResult);
    FreeContractData();
    GUI_DEL_OBJ(g_contractRawDataHintbox);
}