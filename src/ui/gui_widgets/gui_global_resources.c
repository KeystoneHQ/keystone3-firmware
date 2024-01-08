#include "gui_global_resources.h"
#include "user_memory.h"
#include "lv_i18n.h"

static char * *g_ethDerivationPathDesc = NULL;
static char * *g_solDerivationPathDesc = NULL;
static char * *g_btcDerivationPathDesc = NULL;
static char * *g_nearDerivationPathDesc = NULL;

void DerivationPathDescsInit(void)
{
    if (g_ethDerivationPathDesc == NULL) {
        g_ethDerivationPathDesc = SRAM_MALLOC(3 * 128);
        g_ethDerivationPathDesc[ETH_STANDARD] = _("derivation_path_eth_standard_desc");
        g_ethDerivationPathDesc[ETH_LEDGER_LIVE] = _("derivation_path_eth_ledger_live_desc");
        g_ethDerivationPathDesc[ETH_LEDGER_LEGACY] = _("derivation_path_eth_ledger_legacy_desc");
    }
    if (g_solDerivationPathDesc == NULL) {
        g_solDerivationPathDesc = SRAM_MALLOC(3 * 128);
        g_solDerivationPathDesc[SOL_SOLFLARE] = _("derivation_path_sol_1_desc");
        g_solDerivationPathDesc[SOL_SOLLET] = _("derivation_path_sol_2_desc");
        g_solDerivationPathDesc[SOL_PHANTOM] = _("derivation_path_sol_3_desc");
    }
    if (g_btcDerivationPathDesc == NULL) {
        g_btcDerivationPathDesc = SRAM_MALLOC(3 * 128);
        g_btcDerivationPathDesc[BTC_NATIVE_SEGWIT] = _("derivation_path_btc_1_desc");
        g_btcDerivationPathDesc[BTC_NESTED_SEGWIT] = _("derivation_path_btc_2_desc");
        g_btcDerivationPathDesc[BTC_LEGACY] = _("derivation_path_btc_3_desc");
    }
    if (g_nearDerivationPathDesc == NULL) {
        g_nearDerivationPathDesc = SRAM_MALLOC(2 * 128);
        g_nearDerivationPathDesc[NEAR_STANDARD] = _("derivation_path_near_standard_desc");
        g_nearDerivationPathDesc[NEAR_LEDGER_LIVE] = _("derivation_path_near_ledger_live_desc");
    }
}

char **GetDerivationPathDescs(uint8_t index)
{
    if (index == ETH_DERIVATION_PATH_DESC) {
        return g_ethDerivationPathDesc;
    }
    if (index == SOL_DERIVATION_PATH_DESC) {
        return g_solDerivationPathDesc;
    }
    if (index == BTC_DERIVATION_PATH_DESC) {
        return g_btcDerivationPathDesc;
    }
    if (index == NEAR_DERIVATION_PATH_DESC) {
        return g_nearDerivationPathDesc;
    }

    return NULL;
}

void GlobalResourcesInit(void)
{
    DerivationPathDescsInit();
}
