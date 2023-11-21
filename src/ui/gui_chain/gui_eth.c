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

static void decodeEthContractData(void *parseResult);
static bool GetEthErc20ContractData(void *parseResult);

static bool g_isMulti = false;
static void *g_urResult = NULL;
static void *g_parseResult = NULL;
static char *g_erc20ContractAddress = NULL;
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
    {"USDC", "0xa0b86991c6218b36c1d19d4a2e9eb0ce3606eb48", 6},
    {"USDT", "0xdac17f958d2ee523a2206206994597c13d831ec7", 6},
    {"DAI", "0x6b175474e89094c44da98b954eedeac495271d0f", 18},
    {"MKR", "0x9f8f72aa9304c8b593d555f12ef6589cc3a579a2", 18},
    {"BNB", "0xB8c77482e45F1F44dE1745F52C74426C631bDD52", 18},
    {"LINK", "0x514910771af9ca656af840dff83e8264ecf986ca", 18},
    {"UNI", "0x1f9840a85d5af5bf1d1762f925bdaddc4201f984", 18},
    {"COMP", "0xc00e94cb662c3520282e6f5717214004a7f26888", 18},
    {"AAVE", "0x7fc66500c84a76ad7e9c93437bfc5ac33e2ddae9", 18},
};
#include "abi_ethereum.h"
#include "gui_constants.h"
extern const ABIItem_t ethereum_abi_map[];

static uint8_t GetEthPublickeyIndex(char* rootPath);

void GuiSetEthUrData(void *data, bool multi)
{
#ifndef COMPILE_SIMULATOR
    g_urResult = data;
    g_isMulti = multi;
    g_viewType = g_isMulti ? ((URParseMultiResult *)g_urResult)->t : ((URParseResult *)g_urResult)->t;
#endif
}

#define CHECK_FREE_PARSE_RESULT(result)                                                                                           \
    if (result != NULL)                                                                                                           \
    {                                                                                                                             \
        switch (g_viewType)                                                                                                       \
        {                                                                                                                         \
        case EthTx:                                                                                                               \
            free_transaction_parse_result_display_eth((PtrT_TransactionParseResult_DisplayETH)result);                            \
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
    void *data = g_isMulti ? ((URParseMultiResult *)g_urResult)->data : ((URParseResult *)g_urResult)->data;
    do {
        uint8_t seed[64];
        int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();
        GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
        if (isUnlimited)
        {
            encodeResult = eth_sign_tx_unlimited(data, seed, len);
        }
        else
        {
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
    if (strlen(input) <= 8)
    {
        g_erc20ContractAddress = NULL;
        return false;
    }
    // FIXME: 0xa9059cbb is the method of erc20 transfer
    char *erc20Method = "a9059cbb";
    for (int i = 0; i < 8; i++)
    {
        if (input[i] != erc20Method[i])
        {
            g_erc20ContractAddress = NULL;
            return false;
        }
    }
    return true;
}

static char *CalcSymbol(void *param)
{
    DisplayETH *eth = (DisplayETH *)param;

    TransactionParseResult_DisplayETH *result = (TransactionParseResult_DisplayETH *)g_parseResult;

    if (isErc20Transfer(result->data) && g_erc20ContractAddress != NULL)
    {
        for (size_t i = 0; i < NUMBER_OF_ARRAYS(ERC20_CONTRACTS); i++)
        {
            Erc20Contract_t contract = ERC20_CONTRACTS[i];
            if (strcasecmp(contract.contract_address, g_erc20ContractAddress) == 0)
            {
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
    void *data = g_isMulti ? ((URParseMultiResult *)g_urResult)->data : ((URParseResult *)g_urResult)->data;
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
    return g_parseResult;
#else
    return NULL;
#endif
}

void GetEthTypedDataDomianName(void *indata, void *param)
{
    DisplayETHTypedData *message = (DisplayETHTypedData *)param;
    strcpy((char *)indata, message->name);
}

void GetEthTypedDataDomianVersion(void *indata, void *param)
{
    DisplayETHTypedData *message = (DisplayETHTypedData *)param;
    strcpy((char *)indata, message->version);
}

void GetEthTypedDataDomianChainId(void *indata, void *param)
{
    DisplayETHTypedData *message = (DisplayETHTypedData *)param;
    strcpy((char *)indata, message->chain_id);
}

void GetEthTypedDataDomianVerifyContract(void *indata, void *param)
{
    DisplayETHTypedData *message = (DisplayETHTypedData *)param;
    strcpy((char *)indata, message->verifying_contract);
}

void GetEthTypedDataDomianSalt(void *indata, void *param)
{
    DisplayETHTypedData *message = (DisplayETHTypedData *)param;
    strcpy((char *)indata, message->salt);
}

void GetEthTypedDataPrimayType(void *indata, void *param)
{
    DisplayETHTypedData *message = (DisplayETHTypedData *)param;
    strcpy((char *)indata, message->primary_type);
}

void GetEthTypedDataMessage(void *indata, void *param)
{
    DisplayETHTypedData *message = (DisplayETHTypedData *)param;
    strcpy((char *)indata, message->message);
}

void GetEthTypedDataFrom(void *indata, void *param)
{
    DisplayETHTypedData *message = (DisplayETHTypedData *)param;
    strcpy((char *)indata, message->from);
}

void *GuiGetEthPersonalMessage(void)
{
#ifndef COMPILE_SIMULATOR
    CHECK_FREE_PARSE_RESULT(g_parseResult);
    uint8_t mfp[4];
    void *data = g_isMulti ? ((URParseMultiResult *)g_urResult)->data : ((URParseResult *)g_urResult)->data;
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

void GetEthPersonalMessageType(void *indata, void *param)
{
    DisplayETHPersonalMessage *message = (DisplayETHPersonalMessage *)param;
    if (message->utf8_message) {
        strcpy((char *)indata, "utf8_message");
    } else {
        strcpy((char *)indata, "raw_message");
    }
}

void GetMessageFrom(void *indata, void *param)
{
    DisplayETHPersonalMessage *message = (DisplayETHPersonalMessage *)param;
    if (strlen(message->from) >= LABEL_MAX_BUFF_LEN) {
        snprintf((char *)indata, LABEL_MAX_BUFF_LEN - 3, "%s", message->from);
        strcat((char *)indata, "...");
    } else {
        snprintf((char *)indata, LABEL_MAX_BUFF_LEN, "%s", message->from);
    }
    // strcpy((char *)indata, message->from);
}
void GetMessageUtf8(void *indata, void *param)
{
    DisplayETHPersonalMessage *message = (DisplayETHPersonalMessage *)param;
    if (strlen(message->utf8_message) >= LABEL_MAX_BUFF_LEN) {
        snprintf((char *)indata, LABEL_MAX_BUFF_LEN - 3, "%s", message->utf8_message);
        strcat((char *)indata, "...");
    } else {
        snprintf((char *)indata, LABEL_MAX_BUFF_LEN, "%s", message->utf8_message);
    }
    // strcpy((char *)indata, message->utf8_message);
}

void GetMessageRaw(void *indata, void *param)
{
    int len = strlen("\n#F5C131 The data is not parseable. Please#\n#F5C131 refer to the software wallet interface#\n#F5C131 for viewing.#");
    DisplayETHPersonalMessage *message = (DisplayETHPersonalMessage *)param;
    if (strlen(message->raw_message) >= LABEL_MAX_BUFF_LEN - len) {
        snprintf((char *)indata, LABEL_MAX_BUFF_LEN - 3 - len, "%s", message->raw_message);
        strcat((char *)indata, "...");
    } else {
        snprintf((char *)indata, LABEL_MAX_BUFF_LEN, "%s%s", message->raw_message, "\n#F5C131 The data is not parseable. Please#\n#F5C131 refer to the software wallet interface#\n#F5C131 for viewing.#");
    }
    // sprintf((char *)indata, "%s%s", message->raw_message, "\n#F5C131 The data is not parseable. Please#\n#F5C131 refer to the software wallet interface#\n#F5C131 for viewing.#");
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
}

// pase result
void *GuiGetEthData(void)
{
    memset(g_fromEthEnsName, 0, sizeof(g_fromEthEnsName));
    memset(g_toEthEnsName, 0, sizeof(g_toEthEnsName));
    g_contractDataExist = false;
#ifndef COMPILE_SIMULATOR
    CHECK_FREE_PARSE_RESULT(g_parseResult);
    uint8_t mfp[4];
    void *data = g_isMulti ? ((URParseMultiResult *)g_urResult)->data : ((URParseResult *)g_urResult)->data;
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
#else
    TransactionParseResult_DisplayETH *g_parseResult = malloc(sizeof(TransactionParseResult_DisplayETH));
    DisplayETH *eth = malloc(sizeof(DisplayETH));
    g_parseResult->data = eth;
    g_parseResult->error_code = 0;
    // eth->tx_type = "legacy";
    eth->tx_type = "FeeMarket";
    eth->overview = malloc(sizeof(DisplayETHOverview));
    eth->overview->from = malloc(sizeof(PtrT_VecFFI_DisplayTxOverviewInput));
    eth->overview->value = "0.024819276 ETH";

    eth->overview->gas_price = "0.000000001 gwei";
    eth->overview->gas_limit = "21000";
    // eth->overview->network = "ETH Mainnet";
    eth->overview->max_txn_fee = "0.00000021 ETH";
    eth->overview->from = "0x1q3qqt6mthrlgshy542tpf408lcfa7je92scxtz8";
    eth->overview->to = "0x12Z82nWhvUfFC4tn6iKb1jzuoCnnmsgN";

    eth->detail = malloc(sizeof(DisplayETHDetail));
    eth->detail->max_fee = "49.8089 Gwei";
    eth->detail->max_priority = "49.8089 Gwei";
    eth->detail->max_fee_price = "49.8089 Gwei";
    eth->detail->max_priority_price = "49.8089 Gwei";
    eth->detail->input = "123";
    g_contractDataExist = false;
    struct Response_DisplayContractData *contractData = malloc(sizeof(Response_DisplayContractData));
    contractData->data = malloc(sizeof(DisplayContractData));
    contractData->data->contract_name = "contract name";
    contractData->data->method_name = "method_name name";
    contractData->data->params = SRAM_MALLOC(sizeof(VecFFI_DisplayContractParam));
    contractData->data->params[0].size = 1;
    contractData->data->params[0].cap = 1;
    contractData->data->params[0].data = SRAM_MALLOC(sizeof(DisplayContractParam));
    contractData->data->params[0].data->name = "name1";
    contractData->data->params[0].data->value = "value";
    g_contractData = contractData;
    g_fromEnsExist = false;
    g_toEnsExist = true;
    strcpy(g_fromEthEnsName, "kantfish.eth");
    strcpy(g_toEthEnsName, "xiaoliu.eth");
#endif
    return g_parseResult;
}

PtrT_TransactionCheckResult GuiGetEthCheckResult(void)
{
#ifndef COMPILE_SIMULATOR
    uint8_t mfp[4];
    void *data = g_isMulti ? ((URParseMultiResult *)g_urResult)->data : ((URParseResult *)g_urResult)->data;
    GetMasterFingerPrint(mfp);
    return eth_check(data, mfp, sizeof(mfp));
#else
    return NULL;
#endif
}

void GetEthTransType(void *indata, void *param)
{
    DisplayETH *eth = (DisplayETH *)param;
    strcpy((char *)indata, eth->tx_type);
}

void GetEthTxFee(void *indata, void *param)
{
    DisplayETH *eth = (DisplayETH *)param;
    sprintf((char *)indata, "%s %s", eth->overview->max_txn_fee, _FindNetwork(eth->chain_id));
}

void GetEthTxFrom(void *indata, void *param)
{
    DisplayETH *eth = (DisplayETH *)param;
    strcpy((char *)indata, eth->overview->from);
}

void GetEthTxTo(void *indata, void *param)
{
    DisplayETH *eth = (DisplayETH *)param;
    strcpy((char *)indata, eth->overview->to);
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

void GetEthValue(void *indata, void *param)
{
    DisplayETH *eth = (DisplayETH *)param;
    sprintf((char *)indata, "%s %s", eth->overview->value, CalcSymbol(param));
}

void GetEthGasPrice(void *indata, void *param)
{
    DisplayETH *eth = (DisplayETH *)param;
    strcpy((char *)indata, eth->overview->gas_price);
}

void GetEthGasLimit(void *indata, void *param)
{
    DisplayETH *eth = (DisplayETH *)param;
    strcpy((char *)indata, eth->overview->gas_limit);
}

void GetEthNetWork(void *indata, void *param)
{
    DisplayETH *eth = (DisplayETH *)param;
    EvmNetwork_t network = _FindNetwork(eth->chain_id);
    if (network.chainId == 0) {
        sprintf((char *)indata, "ID: %lu", eth->chain_id);
        return;
    }
    strcpy((char *)indata, network.name);
}

void GetEthMaxFee(void *indata, void *param)
{
    DisplayETH *eth = (DisplayETH *)param;
    sprintf((char *)indata, "%s %s", eth->detail->max_fee, _FindNetwork(eth->chain_id));
}

void GetEthMaxPriority(void *indata, void *param)
{
    DisplayETH *eth = (DisplayETH *)param;
    sprintf((char *)indata, "%s %s", eth->detail->max_priority, _FindNetwork(eth->chain_id));
}

void GetEthMaxFeePrice(void *indata, void *param)
{
    DisplayETH *eth = (DisplayETH *)param;
    strcpy((char *)indata, eth->detail->max_fee_price);
}

void GetEthMaxPriorityFeePrice(void *indata, void *param)
{
    DisplayETH *eth = (DisplayETH *)param;
    strcpy((char *)indata, eth->detail->max_priority_price);
}

void GetEthGetFromAddress(void *indata, void *param)
{
    DisplayETH *eth = (DisplayETH *)param;
    strcpy((char *)indata, eth->overview->from);
}

void GetEthGetToAddress(void *indata, void *param)
{
    DisplayETH *eth = (DisplayETH *)param;
    strcpy((char *)indata, eth->overview->to);
}

void GetTxnFeeDesc(void *indata, void *param)
{
    strcpy((char *)indata, "  \xE2\x80\xA2  Max Txn Fee = Gas Price * Gas Limit");
}

void GetEthEnsName(void *indata, void *param)
{
    strcpy((char *)indata, g_fromEthEnsName);
}

void GetToEthEnsName(void *indata, void *param)
{
    strcpy((char *)indata, g_toEthEnsName);
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

void GetEthTransactionData(void *indata, void *param)
{
    DisplayETH *eth = (DisplayETH *)param;
    if (strlen(eth->detail->input) > 51) {
        char data[49];
        strncpy(data, eth->detail->input, 48);
        data[48] = '\0';
        sprintf((char *)indata, "0x%s...", data);
    } else {
        sprintf((char *)indata, "0x%s", eth->detail->input);
    }
}

void GetEthMethodName(void *indata, void *param)
{
    Response_DisplayContractData *contractData = (Response_DisplayContractData *)g_contractData;
    strcpy((char *)indata, contractData->data->method_name);
}

void GetEthContractName(void *indata, void *param)
{
    Response_DisplayContractData *contractData = (Response_DisplayContractData *)g_contractData;
    if(strlen(g_erc20Name) > 0) {
        strcpy((char *)indata, g_erc20Name);
        return;
    }
    if (strlen(contractData->data->contract_name) > 0) {
        strcpy((char *)indata, contractData->data->contract_name);
    } else {
        sprintf((char *)indata, "Unknown Contract Name");
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
                indata[i][j] = SRAM_MALLOC(strlen(param.name) + 9);
                sprintf(indata[i][j], "#919191 %s#", param.name);
            } else {
                indata[i][j] = SRAM_MALLOC(strlen(param.value));
                strcpy(indata[i][j], param.value);
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

static void hex_to_dec(char *hex, uint8_t decimals, uint8_t *dec)
{
    uint64_t num = 0;
    uint32_t decimalsNum = pow(10, decimals);
    sscanf(hex, "%llx", &num);
    uint64_t integer = num / decimalsNum;
    uint64_t fractional = num % decimalsNum;
    sprintf(dec, "%llu.%llu", integer, fractional);
}

static void FixRecipientAndValueWhenErc20Contract(const char *inputdata, uint8_t decimals)
{
    TransactionParseResult_DisplayETH *result = (TransactionParseResult_DisplayETH *)g_parseResult;
    if (!isErc20Transfer(result->data)) {
        return;
    }
    PtrT_TransactionParseResult_EthParsedErc20Transaction contractData = eth_parse_erc20(inputdata, decimals);
    g_erc20ContractAddress = result->data->detail->to;
    result->data->detail->to = contractData->data->to;
    result->data->overview->to = contractData->data->to;
    result->data->detail->value = contractData->data->value;
    result->data->overview->value = contractData->data->value;
}

static bool GetEthErc20ContractData(void *parseResult)
{
    g_erc20Name = NULL;
    TransactionParseResult_DisplayETH *result = (TransactionParseResult_DisplayETH *)parseResult;
    Response_DisplayContractData *contractData = eth_parse_contract_data(result->data->detail->input, (char *)ethereum_erc20_json);
    if (contractData->error_code == 0)
    {
        g_contractDataExist = true;
        g_contractData = contractData;
    }
    char *to = result->data->detail->to;
    for (size_t i = 0; i < NUMBER_OF_ARRAYS(ERC20_CONTRACTS); i++)
    {
        Erc20Contract_t contract = ERC20_CONTRACTS[i];
        if (strcasecmp(contract.contract_address, to) == 0)
        {
            g_erc20Name = contract.symbol;
            FixRecipientAndValueWhenErc20Contract(result->data->detail->input, contract.decimals);
            return true;
        }
    }
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
    char *contractMethodJson = SRAM_MALLOC(2000);
    memset(contractMethodJson, 0, 2000);
    char contractName[64] = {0};
    if (GetDBContract(address, selectorId, chainId, contractMethodJson, contractName)) {
        Response_DisplayContractData *contractData = eth_parse_contract_data_by_method(inputData, contractName, contractMethodJson);
        if (contractData->error_code == 0) {
            g_contractDataExist = true;
            g_contractData = contractData;
        } else {
            SRAM_FREE(contractMethodJson);
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
    }
    g_contractData = NULL;
#endif
}

void FreeEthMemory(void)
{
#ifndef COMPILE_SIMULATOR
    CHECK_FREE_UR_RESULT(g_urResult, g_isMulti);
    CHECK_FREE_PARSE_RESULT(g_parseResult);
    FreeContractData();
#endif
}