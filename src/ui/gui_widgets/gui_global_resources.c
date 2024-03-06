#include "gui_global_resources.h"
#include "user_memory.h"
#include "lv_i18n.h"

static const char *g_ethDerivationPathDesc[3];
static const char *g_solDerivationPathDesc[3];
static const char *g_btcDerivationPathDesc[4];
#ifdef BTC_ONLY
static const char *g_btcTestNetDerivationPathDesc[4];
#endif

void DerivationPathDescsInit(void)
{
    g_ethDerivationPathDesc[ETH_STANDARD] = (char *)_("derivation_path_eth_standard_desc");
    g_ethDerivationPathDesc[ETH_LEDGER_LIVE] = (char *)_("derivation_path_eth_ledger_live_desc");
    g_ethDerivationPathDesc[ETH_LEDGER_LEGACY] = (char *)_("derivation_path_eth_ledger_legacy_desc");

    g_solDerivationPathDesc[SOL_SOLFLARE] = (char *)_("derivation_path_sol_1_desc");
    g_solDerivationPathDesc[SOL_SOLLET] = (char *)_("derivation_path_sol_2_desc");
    g_solDerivationPathDesc[SOL_PHANTOM] = (char *)_("derivation_path_sol_3_desc");

    g_btcDerivationPathDesc[BTC_TAPROOT] = (char *)_("derivation_path_btc_4_desc");
    g_btcDerivationPathDesc[BTC_NATIVE_SEGWIT] = (char *)_("derivation_path_btc_1_desc");
    g_btcDerivationPathDesc[BTC_NESTED_SEGWIT] = (char *)_("derivation_path_btc_2_desc");
    g_btcDerivationPathDesc[BTC_LEGACY] = (char *)_("derivation_path_btc_3_desc");
#ifdef BTC_ONLY
    g_btcTestNetDerivationPathDesc[BTC_TAPROOT] = (char *)_("derivation_path_btc_test_net_4_desc");
    g_btcTestNetDerivationPathDesc[BTC_NATIVE_SEGWIT] = (char *)_("derivation_path_btc_test_net_1_desc");
    g_btcTestNetDerivationPathDesc[BTC_NESTED_SEGWIT] = (char *)_("derivation_path_btc_test_net_2_desc");
    g_btcTestNetDerivationPathDesc[BTC_LEGACY] = (char *)_("derivation_path_btc_test_net_3_desc");
#endif
}

char **GetDerivationPathDescs(uint8_t index)
{
    if (index == ETH_DERIVATION_PATH_DESC) {
        return (char **)g_ethDerivationPathDesc;
    }
    if (index == SOL_DERIVATION_PATH_DESC) {
        return (char **)g_solDerivationPathDesc;
    }
    if (index == BTC_DERIVATION_PATH_DESC) {
        return (char **)g_btcDerivationPathDesc;
    }
#ifdef BTC_ONLY
    if (index == BTC_TEST_NET_DERIVATION_PATH_DESC) {
        return (char **)g_btcTestNetDerivationPathDesc;
    }
#endif
    return NULL;
}

void GlobalResourcesInit(void)
{
    DerivationPathDescsInit();
}