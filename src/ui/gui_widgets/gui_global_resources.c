#include "lv_i18n.h"
#include "user_memory.h"
#include "gui_global_resources.h"

#define ETH_DERIVATION_PATH_MAX_LEN                     (128)

static char **g_ethDerivationPathDesc = NULL;
static char **g_avaxDerivationPathDesc = NULL;
static char **g_solDerivationPathDesc = NULL;
static char **g_adaDerivationPathDesc = NULL;
static char **g_btcDerivationPathDesc = NULL;
static char **g_ltcDerivationPathDesc = NULL;
#ifdef BTC_ONLY
static char **g_btcTestNetDerivationPathDesc = NULL;
#endif

void DerivationPathDescsInit(void)
{
    if (g_ethDerivationPathDesc == NULL) {
        g_ethDerivationPathDesc = SRAM_MALLOC(3 * ETH_DERIVATION_PATH_MAX_LEN);
    }
    g_ethDerivationPathDesc[ETH_STANDARD] = (char *)_("derivation_path_eth_standard_desc");
    g_ethDerivationPathDesc[ETH_LEDGER_LIVE] = (char *)_("derivation_path_eth_ledger_live_desc");
    g_ethDerivationPathDesc[ETH_LEDGER_LEGACY] = (char *)_("derivation_path_eth_ledger_legacy_desc");

    if (g_solDerivationPathDesc == NULL) {
        g_solDerivationPathDesc = SRAM_MALLOC(3 * ETH_DERIVATION_PATH_MAX_LEN);
    }
    g_solDerivationPathDesc[SOL_SOLFLARE] = "";
    g_solDerivationPathDesc[SOL_SOLLET] = "";
    g_solDerivationPathDesc[SOL_PHANTOM] = "";

    if (g_adaDerivationPathDesc == NULL) {
        g_adaDerivationPathDesc = SRAM_MALLOC(2 * ETH_DERIVATION_PATH_MAX_LEN);
    }
    g_adaDerivationPathDesc[ADA_STANDARD] = (char *)_("derivation_path_ada_standard_desc");
    g_adaDerivationPathDesc[ADA_LEDGER] = (char *)_("derivation_path_ada_ledger_desc");

    if (g_btcDerivationPathDesc == NULL) {
        g_btcDerivationPathDesc = SRAM_MALLOC(4 * ETH_DERIVATION_PATH_MAX_LEN);
    }
    g_btcDerivationPathDesc[BTC_TAPROOT] = (char *)_("derivation_path_btc_4_desc");
    g_btcDerivationPathDesc[BTC_NATIVE_SEGWIT] = (char *)_("derivation_path_btc_1_desc");
    g_btcDerivationPathDesc[BTC_NESTED_SEGWIT] = (char *)_("derivation_path_btc_2_desc");
    g_btcDerivationPathDesc[BTC_LEGACY] = (char *)_("derivation_path_btc_3_desc");
#ifdef BTC_ONLY
    if (g_btcTestNetDerivationPathDesc == NULL) {
        g_btcTestNetDerivationPathDesc = SRAM_MALLOC(4 * ETH_DERIVATION_PATH_MAX_LEN);
    }
    g_btcTestNetDerivationPathDesc[BTC_NATIVE_SEGWIT] = (char *)_("derivation_path_btc_test_net_1_desc");
    g_btcTestNetDerivationPathDesc[BTC_TAPROOT] = (char *)_("derivation_path_btc_test_net_4_desc");
    g_btcTestNetDerivationPathDesc[BTC_NESTED_SEGWIT] = (char *)_("derivation_path_btc_test_net_2_desc");
    g_btcTestNetDerivationPathDesc[BTC_LEGACY] = (char *)_("derivation_path_btc_test_net_3_desc");
#endif

    if (g_avaxDerivationPathDesc == NULL) {
        g_avaxDerivationPathDesc = SRAM_MALLOC(2 * ETH_DERIVATION_PATH_MAX_LEN);
    }
    g_avaxDerivationPathDesc[AVAX_C_CHAIN] = (char *)_("chain_path_avax_c_desc");
    g_avaxDerivationPathDesc[AVAX_X_P_CHAIN] = (char *)_("chain_path_avax_x_p_desc");

    if (g_ltcDerivationPathDesc == NULL) {
        g_ltcDerivationPathDesc = SRAM_MALLOC(4 * ETH_DERIVATION_PATH_MAX_LEN);
    }
    g_ltcDerivationPathDesc[LTC_NESTED_SEGWIT] = (char *)_("derivation_path_ltc_nested_desc");
    g_ltcDerivationPathDesc[LTC_NATIVE_SEGWIT] = (char *)_("derivation_path_ltc_native_desc");
}

char **GetDerivationPathDescs(uint8_t index)
{
    DerivationPathDescsInit();
    switch (index) {
    case ETH_DERIVATION_PATH_DESC:
        return (char **)g_ethDerivationPathDesc;
    case AVAX_DERIVATION_PATH_DESC:
        return (char **)g_avaxDerivationPathDesc;
    case SOL_DERIVATION_PATH_DESC:
        return (char **)g_solDerivationPathDesc;
    case BTC_DERIVATION_PATH_DESC:
        return (char **)g_btcDerivationPathDesc;
    case ADA_DERIVATION_PATH_DESC:
        return (char **)g_adaDerivationPathDesc;
    case LTC_DERIVATION_PATH_DESC:
        return (char **)g_ltcDerivationPathDesc;
#ifdef BTC_ONLY
    case BTC_TEST_NET_DERIVATION_PATH_DESC:
        return (char **)g_btcTestNetDerivationPathDesc;
#endif
    default:
        break;
    }
    return NULL;
}

void GlobalResourcesInit(void)
{
    DerivationPathDescsInit();
}