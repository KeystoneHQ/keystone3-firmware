#ifndef BTC_ONLY
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

#ifdef COMPILE_SIMULATOR
#include "simulator_mock_define.h"
#endif

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
    {999, "Wanchain Testnet", "WAN"},
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
    {43114, "Avalanche Mainnet", "AVAX"},
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
#ifndef COMPILE_SIMULATOR
    UREncodeResult *encodeResult;
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        uint8_t seed[64];
        int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();
        GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
        if (isUnlimited) {
            encodeResult = eth_sign_tx_unlimited(data, seed, len);
        } else {
            encodeResult = eth_sign_tx(data, seed, len);
        }
        ClearSecretCache();
        CHECK_CHAIN_BREAK(encodeResult);
    } while (0);
    SetLockScreen(enable);
    return encodeResult;
#else
    UREncodeResult *encodeResult = SRAM_MALLOC(sizeof(UREncodeResult));
    encodeResult->is_multi_part = 0;
    encodeResult->data = "xpub6CZZYZBJ857yVCZXzqMBwuFMogBoDkrWzhsFiUd1SF7RUGaGryBRtpqJU6AGuYGpyabpnKf5SSMeSw9E9DSA8ZLov53FDnofx9wZLCpLNft";
    encodeResult->encoder = NULL;
    encodeResult->error_code = 0;
    encodeResult->error_message = NULL;
    return encodeResult;
#endif
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

    EvmNetwork_t network = _FindNetwork(eth->chain_id);
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

void *GuiGetEthTypeData(void)
{
#ifndef COMPILE_SIMULATOR
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
        CHECK_CHAIN_BREAK(parseResult);
        g_parseResult = (void *)parseResult;
    } while (0);
    free_TransactionCheckResult(result);
    free_ptr_string(rootPath);
    return g_parseResult;
#else
    return NULL;
#endif
}

void GetEthTypedDataDomianName(void *indata, void *param, uint32_t maxLen)
{
    DisplayETHTypedData *message = (DisplayETHTypedData *)param;
    strcpy_s((char *)indata, maxLen, message->name);
}

void GetEthTypedDataDomianVersion(void *indata, void *param, uint32_t maxLen)
{
    DisplayETHTypedData *message = (DisplayETHTypedData *)param;
    strcpy_s((char *)indata, maxLen, message->version);
}

void GetEthTypedDataDomianChainId(void *indata, void *param, uint32_t maxLen)
{
    DisplayETHTypedData *message = (DisplayETHTypedData *)param;
    strcpy_s((char *)indata, maxLen, message->chain_id);
}

void GetEthTypedDataDomianVerifyContract(void *indata, void *param, uint32_t maxLen)
{
    DisplayETHTypedData *message = (DisplayETHTypedData *)param;
    strcpy_s((char *)indata, maxLen, message->verifying_contract);
}

void GetEthTypedDataDomianSalt(void *indata, void *param, uint32_t maxLen)
{
    DisplayETHTypedData *message = (DisplayETHTypedData *)param;
    strcpy_s((char *)indata, maxLen, message->salt);
}

void GetEthTypedDataPrimayType(void *indata, void *param, uint32_t maxLen)
{
    DisplayETHTypedData *message = (DisplayETHTypedData *)param;
    strcpy_s((char *)indata, maxLen, message->primary_type);
}

void GetEthTypedDataMessage(void *indata, void *param, uint32_t maxLen)
{
    DisplayETHTypedData *message = (DisplayETHTypedData *)param;
    strcpy_s((char *)indata, maxLen, message->message);
}

int GetEthTypedDataMessageLen(void *param)
{
    DisplayETHTypedData *message = (DisplayETHTypedData *)param;
    return strlen(message->message);
}

void GetEthTypedDataFrom(void *indata, void *param, uint32_t maxLen)
{
    DisplayETHTypedData *message = (DisplayETHTypedData *)param;
    strcpy_s((char *)indata, maxLen, message->from);
}

void *GuiGetEthPersonalMessage(void)
{
#ifndef COMPILE_SIMULATOR
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
#else
    PtrT_TransactionParseResult_DisplayETHPersonalMessage parseResult = SRAM_MALLOC(sizeof(TransactionParseResult_DisplayETHPersonalMessage));
    parseResult->data = SRAM_MALLOC(sizeof(DisplayETHPersonalMessage));
    parseResult->data->from = "0x6c2ecd7f1a7c4c2a4a4f4d6b8f3c1d7f9f2a4f4d";
    parseResult->data->utf8_message = "hello world";
    parseResult->data->raw_message = "0x68656c6c6f20776f726c64";
    g_parseResult = (void *)parseResult;
#endif
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
    char *rootPath = eth_get_root_path(data);
    char *ethXpub = GetCurrentAccountPublicKey(GetEthPublickeyIndex(rootPath));
    GetMasterFingerPrint(mfp);
    do {
        PtrT_TransactionParseResult_DisplayETH parseResult = eth_parse(data, ethXpub);
        CHECK_CHAIN_BREAK(parseResult);
        g_parseResult = (void *)parseResult;
        g_fromEnsExist = GetEnsName((const char *)parseResult->data->overview->from, g_fromEthEnsName);
        g_toEnsExist = GetEnsName((const char *)parseResult->data->overview->to, g_toEthEnsName);
        printf("decode start\n");
        decodeEthContractData(parseResult);
        printf("decode finish\n");
    } while (0);
    free_ptr_string(rootPath);

    return g_parseResult;
}

PtrT_TransactionCheckResult GuiGetEthCheckResult(void)
{
    uint8_t mfp[4];
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    GetMasterFingerPrint(mfp);
    return eth_check(data, mfp, sizeof(mfp));
}

void GetEthTransType(void *indata, void *param, uint32_t maxLen)
{
    DisplayETH *eth = (DisplayETH *)param;
    strcpy_s((char *)indata, maxLen, eth->tx_type);
}

void GetEthTxFee(void *indata, void *param, uint32_t maxLen)
{
    DisplayETH *eth = (DisplayETH *)param;
    snprintf_s((char *)indata,  maxLen, "%s %s", eth->overview->max_txn_fee, _FindNetwork(eth->chain_id));
}

EvmNetwork_t _FindNetwork(uint64_t chainId)
{
    for (size_t i = 0; i < NUMBER_OF_ARRAYS(NETWORKS); i++) {
        EvmNetwork_t network = NETWORKS[i];
        if (chainId == network.chainId) {
            return network;
        }
    }
    return NETWORKS[0];
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
    EvmNetwork_t network = _FindNetwork(eth->chain_id);
    if (network.chainId == 0) {
        snprintf_s((char *)indata,  maxLen, "ID: %lu", eth->chain_id);
        return;
    }
    strcpy_s((char *)indata, maxLen, network.name);
}

void GetEthMaxFee(void *indata, void *param, uint32_t maxLen)
{
    DisplayETH *eth = (DisplayETH *)param;
    snprintf_s((char *)indata,  maxLen, "%s %s", eth->detail->max_fee, _FindNetwork(eth->chain_id));
}

void GetEthMaxPriority(void *indata, void *param, uint32_t maxLen)
{
    DisplayETH *eth = (DisplayETH *)param;
    snprintf_s((char *)indata,  maxLen, "%s %s", eth->detail->max_priority, _FindNetwork(eth->chain_id));
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

void GetEthToFromSize(uint16_t *width, uint16_t *height, void *param)
{
    *width = 408;
    *height = 244 + (g_fromEnsExist + g_toEnsExist) * (GAP + TEXT_LINE_HEIGHT) + g_contractDataExist * (GAP + TEXT_LINE_HEIGHT);
}

void GetEthToLabelPos(uint16_t *x, uint16_t *y, void *param)
{
    *x = 24;
    *y = 130 + g_fromEnsExist * 38;
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
        indata[i] = SRAM_MALLOC(sizeof(char *) * *row);
        for (j = 0; j < *row; j++) {
            int index = j / 2;
            DisplayContractParam param = contractData->data->params->data[index];
            if (!(j % 2)) {
                uint32_t len = strnlen_s(param.name, BUFFER_SIZE_128) + 10;
                indata[i][j] = SRAM_MALLOC(len);
                snprintf_s(indata[i][j], len, "#919191 %s#", param.name);
            } else {
                uint32_t len = strnlen_s(param.value, BUFFER_SIZE_128) + 1;
                indata[i][j] = SRAM_MALLOC(len);
                strcpy_s(indata[i][j], len, param.value);
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
#ifndef COMPILE_SIMULATOR
        free_Response_DisplayContractData(contractData);
#endif
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
    g_erc20Name = "Erc20";
    FixRecipientAndValueWhenErc20Contract(result->data->detail->input, 18);
    return true;
}

bool GetEthContractFromInternal(char *address, char *inputData)
{
    for (size_t i = 0; i < NUMBER_OF_ARRAYS(ethereum_abi_map); i++) {
        struct ABIItem item = ethereum_abi_map[i];
        if (strcasecmp(item.address, address) == 0) {
            Response_DisplayContractData *contractData = eth_parse_contract_data(inputData, (char *)item.json);
            if (contractData->error_code == 0) {
                g_contractDataExist = true;
                g_contractData = contractData;
            } else {
#ifndef COMPILE_SIMULATOR
                free_Response_DisplayContractData(contractData);
#endif
                return false;
            }
            return true;
        }
    }
    return false;
}

void EthContractLearnMore(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        GuiQRCodeHintBoxOpen(_("tx_details_eth_decoding_qr_link"), _("tx_details_eth_decoding_qr_title"), _("tx_details_eth_decoding_qr_link"));
    }
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
#ifndef COMPILE_SIMULATOR
            free_Response_DisplayContractData(contractData);
#endif
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
#ifndef COMPILE_SIMULATOR
    if (g_contractData != NULL) {
        free_Response_DisplayContractData(g_contractData);
        g_contractData = NULL;
    }
    if (g_erc20ContractData != NULL) {
        free_TransactionParseResult_EthParsedErc20Transaction(g_erc20ContractData);
        g_erc20ContractData = NULL;
    }
#endif
}

void FreeEthMemory(void)
{
#ifndef COMPILE_SIMULATOR
    CHECK_FREE_UR_RESULT(g_urResult, false);
    CHECK_FREE_UR_RESULT(g_urMultiResult, true);
    CHECK_FREE_PARSE_RESULT(g_parseResult);
    FreeContractData();
#endif
}
#endif