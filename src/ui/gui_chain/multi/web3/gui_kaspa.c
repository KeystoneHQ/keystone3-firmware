#include <stdio.h>
#include <string.h>
#include "gui_analyze.h"
#include "user_memory.h"
#include "rust.h"
#include "account_public_info.h"
#include "keystore.h"
#include "version.h"
#include "secret_cache.h"
#include "screen_manager.h"
#include "account_manager.h"
#include "gui_chain_components.h"
#include "gui_home_widgets.h"
#include "gui_transaction_detail_widgets.h"
#include "err_code.h"
#include "gui_kaspa.h"
#include "cJSON.h"

// Global variables for storing parsed data
static bool g_isMulti = false;
static URParseResult *g_urResult = NULL;
static URParseMultiResult *g_urMultiResult = NULL;
static TransactionParseResult_DisplayKaspaTx *g_parseResult = NULL;

// Set Kaspa data parsed from UR
void GuiSetKaspaUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi)
{
    g_urResult = urResult;
    g_urMultiResult = urMultiResult;
    g_isMulti = multi;
}

// Parse Kaspa transaction (PSKT format)
// void *GuiGetParsedKaspaTx(void)
// {
//     // Free previous parse result
//     if (g_parseResult != NULL) {
//         free_TransactionParseResult_DisplayKaspaTx(g_parseResult);
//         g_parseResult = NULL;
//     }

//     // Get UR data
//     void *data = NULL;
//     if (g_isMulti) {
//         data = g_urMultiResult->data;
//     } else {
//         data = g_urResult->data;
//     }

//     if (data == NULL) {
//         printf("Error: Kaspa UR data is NULL\n");
//         return NULL;
//     }

//     // Get Master Fingerprint
//     uint8_t mfp[4] = {0};
//     GetMasterFingerPrint(mfp);


//     uint8_t *pskt_bytes = (uint8_t *)data;
//     size_t pskt_len = g_isMulti ? g_urMultiResult->data_len : g_urResult->data_len;
//     char *pskt_hex = SRAM_MALLOC(pskt_len * 2 + 1);
//     if (pskt_hex == NULL) {
//         printf("Error: Out of memory for PSKT hex\n");
//         return NULL;
//     }
//     for (size_t i = 0; i < pskt_len; i++) {
//         sprintf(pskt_hex + i * 2, "%02x", pskt_bytes[i]);
//     }
//     pskt_hex[pskt_len * 2] = '\0';

//     // Call Rust FFI to parse PSKT
//     g_parseResult = kaspa_parse_pskt(pskt_hex, mfp, sizeof(mfp));
//     SRAM_FREE(pskt_hex);
    
//     if (g_parseResult == NULL) {
//         printf("Error: Failed to parse Kaspa PSKT\n");
//         return NULL;
//     }

//     if (g_parseResult->error_code != 0) {
//         printf("Error: Kaspa PSKT parse error: %s\n", g_parseResult->error_message);
//         return NULL;
//     }

//     return g_parseResult;
// }

void *GuiGetParsedKaspaTx(void)
{
    if (g_parseResult != NULL) {
        free_TransactionParseResult_DisplayKaspaTx(g_parseResult);
        g_parseResult = NULL;
    }
    void *ur_ptr = g_isMulti ? (void*)g_urMultiResult : (void*)g_urResult;

    uint8_t mfp[4];
    GetMasterFingerPrint(mfp);

    g_parseResult = kaspa_parse_pskt(ur_ptr, mfp, 4);

    if (g_parseResult != NULL && g_parseResult->error_code != 0) {
        printf("Kaspa Parse Failed: %s\n", g_parseResult->error_message);
    }

    return g_parseResult;
}

// Get signed Kaspa QR code data
UREncodeResult *GuiGetKaspaSignQrCodeData(void)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);

    void *ur_ptr = g_isMulti ? (void*)g_urMultiResult : (void*)g_urResult;
    if (ur_ptr == NULL) {
        SetLockScreen(enable);
        return NULL;
    }

    // Get seed and Master Fingerprint
    uint8_t seed[64];
    int len = GetCurrentAccountSeedLen();
    int ret = GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
    if (ret != SUCCESS_CODE) {
        printf("Error: Failed to get seed, error code: %d\n", ret);
        SetLockScreen(enable);
        return NULL;
    }

    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);

    // Call Rust FFI to sign PSKT
   UREncodeResult *encodeResult = kaspa_sign_pskt(ur_ptr, seed, len, mfp, 4);
    // Clear sensitive data
    memset_s(seed, sizeof(seed), 0, sizeof(seed));
    ClearSecretCache();
    SetLockScreen(enable);

    if (encodeResult == NULL) {
        printf("Error: Failed to sign Kaspa PSKT\n");
        return NULL;
    }

    if (encodeResult->error_code != 0) {
        printf("Error: Kaspa PSKT sign error: %s\n", encodeResult->error_message);
    }

    return encodeResult;
}

// Check Kaspa transaction result
TransactionCheckResult *GuiGetKaspaCheckResult(void)
{
    // Parse transaction
    void *ur_ptr = g_isMulti ? (void*)g_urMultiResult : (void*)g_urResult;
    if (ur_ptr == NULL) return NULL;

    uint8_t mfp[4];
    GetMasterFingerPrint(mfp);
    return kaspa_check_pskt(ur_ptr, mfp, 4);
}

// Clear Kaspa related data
void GuiClearKaspaData(void)
{
    if (g_parseResult != NULL) {
        free_TransactionParseResult_DisplayKaspaTx(g_parseResult);
        g_parseResult = NULL;
    }
    g_urResult = NULL;
    g_urMultiResult = NULL;
    g_isMulti = false;
}

// ============ UI Component Creation Functions ============

// Create Kaspa input item
static void CreateKaspaInputItem(lv_obj_t *parent, DisplayKaspaInput *input, int index)
{
    char title[32];
    snprintf(title, sizeof(title), "Input #%d", index + 1);
    
    // Create input container
    lv_obj_t *container = GuiCreateContainerWithParent(parent, 408, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(container, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_radius(container, 24, LV_PART_MAIN);
    lv_obj_set_style_pad_all(container, 24, LV_PART_MAIN);
    
    // Title
    lv_obj_t *label = GuiCreateNoticeLabel(container, title);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 0);
    
    // Address
    label = GuiCreateNoticeLabel(container, "Address:");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 36);
    
    label = GuiCreateIllustrateLabel(container, input->address);
    lv_obj_set_width(label, 360);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 64);
    
    // Amount
    label = GuiCreateNoticeLabel(container, "Amount:");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 128);
    
    label = GuiCreateIllustrateLabel(container, input->amount);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 156);
}

// Create Kaspa inputs list
static void GuiCreateKaspaInputsList(lv_obj_t *parent, DisplayKaspaTx *tx)
{
    lv_obj_t *container = GuiCreateContainerWithParent(parent, 408, LV_SIZE_CONTENT);
    
    // Title
    lv_obj_t *label = GuiCreateTitleLabel(container, "Inputs");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 0);
    
    // Total count
    char countStr[64];
    snprintf(countStr, sizeof(countStr), "Total: %d input(s)", tx->inputs.size);
    label = GuiCreateNoticeLabel(container, countStr);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 40);
    
    // Create each input item
    int yOffset = 80;
    for (int i = 0; i < tx->inputs.size; i++) {
        lv_obj_t *inputItem = lv_obj_create(container);
        lv_obj_set_pos(inputItem, 0, yOffset);
        CreateKaspaInputItem(inputItem, &tx->inputs.data[i], i);
        yOffset += 220; // Adjust based on actual height
    }
}

// Create Kaspa output item
static void CreateKaspaOutputItem(lv_obj_t *parent, DisplayKaspaOutput *output, int index)
{
    char title[64];
    if (output->is_external) {
        snprintf(title, sizeof(title), "Output #%d (Change)", index + 1);
    } else {
        snprintf(title, sizeof(title), "Output #%d", index + 1);
    }
    
    // Create output container
    lv_obj_t *container = GuiCreateContainerWithParent(parent, 408, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(container, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_radius(container, 24, LV_PART_MAIN);
    lv_obj_set_style_pad_all(container, 24, LV_PART_MAIN);
    
    // Title (mark if it's change output)
    lv_obj_t *label = GuiCreateNoticeLabel(container, title);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 0);
    if (output->is_external) {
        lv_obj_set_style_text_color(label, lv_color_hex(0x1BE0C6), LV_PART_MAIN); // Cyan color indicates change
    }
    
    // Address
    label = GuiCreateNoticeLabel(container, "Address:");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 36);
    
    label = GuiCreateIllustrateLabel(container, output->address);
    lv_obj_set_width(label, 360);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 64);
    
    // Amount
    label = GuiCreateNoticeLabel(container, "Amount:");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 128);
    
    label = GuiCreateIllustrateLabel(container, output->amount);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 156);
}

// Create Kaspa outputs list
static void GuiCreateKaspaOutputsList(lv_obj_t *parent, DisplayKaspaTx *tx)
{
    lv_obj_t *container = GuiCreateContainerWithParent(parent, 408, LV_SIZE_CONTENT);
    
    // Title
    lv_obj_t *label = GuiCreateTitleLabel(container, "Outputs");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 0);
    
    // Total count
    char countStr[64];
    snprintf(countStr, sizeof(countStr), "Total: %d output(s)", tx->outputs.size);
    label = GuiCreateNoticeLabel(container, countStr);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 40);
    
    // Create each output item
    int yOffset = 80;
    for (int i = 0; i < tx->outputs.size; i++) {
        lv_obj_t *outputItem = lv_obj_create(container);
        lv_obj_set_pos(outputItem, 0, yOffset);
        CreateKaspaOutputItem(outputItem, &tx->outputs.data[i], i);
        yOffset += 220; // Adjust based on actual height
    }
}

// Create Kaspa transaction summary
static void GuiCreateKaspaTransactionSummary(lv_obj_t *parent, DisplayKaspaTx *tx)
{
    lv_obj_t *container = GuiCreateContainerWithParent(parent, 408, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(container, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_radius(container, 24, LV_PART_MAIN);
    lv_obj_set_style_pad_all(container, 24, LV_PART_MAIN);
    
    // Title
    lv_obj_t *label = GuiCreateTitleLabel(container, "Transaction Summary");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 0);
    
    int yOffset = 50;
    
    // Network
    label = GuiCreateNoticeLabel(container, "Network:");
    lv_obj_set_pos(label, 0, yOffset);
    
    label = GuiCreateIllustrateLabel(container, tx->network);
    lv_obj_set_pos(label, 0, yOffset + 28);
    
    yOffset += 80;
    
    // Total spend
    label = GuiCreateNoticeLabel(container, "Total Spend:");
    lv_obj_set_pos(label, 0, yOffset);
    
    char totalSpendStr[128];
    snprintf(totalSpendStr, sizeof(totalSpendStr), "%s KAS", tx->total_spend);
    label = GuiCreateIllustrateLabel(container, totalSpendStr);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFF9500), LV_PART_MAIN); // Orange color
    lv_obj_set_pos(label, 0, yOffset + 28);
    
    yOffset += 80;
    
    // Network fee
    label = GuiCreateNoticeLabel(container, "Network Fee:");
    lv_obj_set_pos(label, 0, yOffset);
    
    char feeStr[128];
    snprintf(feeStr, sizeof(feeStr), "%s KAS", tx->fee);
    label = GuiCreateIllustrateLabel(container, feeStr);
    lv_obj_set_pos(label, 0, yOffset + 28);
}

// Create Kaspa transaction details view
static void GuiCreateKaspaTransactionView(lv_obj_t *parent, DisplayKaspaTx *tx)
{
    // Create summary (at top, most important)
    GuiCreateKaspaTransactionSummary(parent, tx);
    
    // Create outputs list
    GuiCreateKaspaOutputsList(parent, tx);
    
    // Create inputs list
    GuiCreateKaspaInputsList(parent, tx);
}

// Create Kaspa transaction template (main entry point)
void GuiCreateKaspaTemplate(lv_obj_t *parent, void *data)
{
    (void)data;
    // Get parsed transaction data
    if (g_parseResult == NULL) {
        printf("Error: No parsed Kaspa transaction data\n");
        return;
    }
    
    if (g_parseResult->error_code != 0) {
        printf("Error: Kaspa transaction parse error: %s\n", g_parseResult->error_message);
        GuiTransactionParseFailed();
        return;
    }
    
    DisplayKaspaTx *tx = g_parseResult->data;
    if (tx == NULL) {
        printf("Error: Kaspa transaction data is NULL\n");
        GuiTransactionParseFailed();
        return;
    }
    
    // Create Kaspa transaction view
    GuiCreateKaspaTransactionView(parent, tx);
}

