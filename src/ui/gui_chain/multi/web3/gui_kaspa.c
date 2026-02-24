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
    void *ur_ptr = g_isMulti ? g_urMultiResult->data : g_urResult->data;

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
    void *ur_ptr = g_isMulti ? g_urMultiResult->data : g_urResult->data;
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
static void SetContainerDefaultStyle(lv_obj_t *container)
{
    lv_obj_set_style_radius(container, 24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(container, WHITE_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(container, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
}

static void SetTitleLabelStyle(lv_obj_t *label)
{
    lv_obj_set_style_text_font(label, g_defIllustrateFont, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_text_opa(label, 144, LV_PART_MAIN | LV_STATE_DEFAULT);
}

static void SetContentLabelStyle(lv_obj_t *label)
{
    lv_obj_set_style_text_font(label, g_defIllustrateFont, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, WHITE_COLOR, LV_PART_MAIN);
}

static lv_obj_t *CreateKaspaDetailAmountView(lv_obj_t *parent, DisplayKaspaTx *tx, lv_obj_t *lastView)
{
    lv_obj_t *amountContainer = GuiCreateContainerWithParent(parent, 408, 138);
    lv_obj_align_to(amountContainer, lastView, LV_ALIGN_OUT_BOTTOM_MID, 0, 16);
    SetContainerDefaultStyle(amountContainer);

    // --- Input Value ---
    lv_obj_t *label = lv_label_create(amountContainer);
    lv_label_set_text(label, "Input Value");
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 16);
    SetTitleLabelStyle(label);

    lv_obj_t *inputValue = lv_label_create(amountContainer);
    lv_label_set_text(inputValue, tx->total_spend); 
    lv_obj_align_to(inputValue, label, LV_ALIGN_OUT_RIGHT_MID, 12, 0);
    SetContentLabelStyle(inputValue);

    // --- Output Value ---
    lastView = label;
    label = lv_label_create(amountContainer);
    lv_label_set_text(label, "Output Value");
    lv_obj_align_to(label, lastView, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    SetTitleLabelStyle(label);

    lv_obj_t *outputValue = lv_label_create(amountContainer);
    // Output Value = Total - Fee
    lv_label_set_text(outputValue, tx->total_spend); 
    lv_obj_align_to(outputValue, label, LV_ALIGN_OUT_RIGHT_MID, 12, 0);
    SetContentLabelStyle(outputValue);

    // --- Fee ---
    lastView = label;
    label = lv_label_create(amountContainer);
    lv_label_set_text(label, "Fee");
    lv_obj_align_to(label, lastView, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    SetTitleLabelStyle(label);

    lv_obj_t *feeValue = lv_label_create(amountContainer);
    char fee_buf[64];
    snprintf(fee_buf, sizeof(fee_buf), "%llu KAS", (unsigned long long)tx->fee);
    lv_label_set_text(feeValue, fee_buf);
    lv_obj_align_to(feeValue, label, LV_ALIGN_OUT_RIGHT_MID, 12, 0);
    SetContentLabelStyle(feeValue);

    return amountContainer;
}

static lv_obj_t *CreateKaspaOverviewAmountView(lv_obj_t *parent, DisplayKaspaTx *tx, lv_obj_t *lastView)
{
    lv_obj_t *amountContainer = GuiCreateContainerWithParent(parent, 408, 144);
    if (lastView == NULL) {
        lv_obj_align(amountContainer, LV_ALIGN_TOP_MID, 0, 0);
    } else {
        lv_obj_align_to(amountContainer, lastView, LV_ALIGN_OUT_BOTTOM_MID, 0, 16);
    }
    SetContainerDefaultStyle(amountContainer);

    lv_obj_t *label = lv_label_create(amountContainer);
    lv_label_set_text(label, "Amount");
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 16);
    SetTitleLabelStyle(label);

    lv_obj_t *amountValue = lv_label_create(amountContainer);
    lv_label_set_text(amountValue, tx->total_spend);
    lv_obj_align_to(amountValue, label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);
    lv_obj_set_style_text_font(amountValue, &openSansEnLittleTitle, LV_PART_MAIN);
    lv_obj_set_style_text_color(amountValue, lv_color_hex(0xF5870A), LV_PART_MAIN);

    lv_obj_t *feeLabel = lv_label_create(amountContainer);
    lv_label_set_text(feeLabel, "Fee");
    lv_obj_align_to(feeLabel, amountValue, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    SetTitleLabelStyle(feeLabel);

    lv_obj_t *feeValue = lv_label_create(amountContainer);
    char fee_buf[64];
    snprintf(fee_buf, sizeof(fee_buf), "%llu KAS", (unsigned long long)tx->fee);
    lv_label_set_text(feeValue, fee_buf);
    lv_obj_align_to(feeValue, feeLabel, LV_ALIGN_OUT_RIGHT_MID, 12, 0);
    SetContentLabelStyle(feeValue);

    return amountContainer;
}

static lv_obj_t *CreateKaspaDetailListView(lv_obj_t *parent, DisplayKaspaTx *tx, bool isInput, lv_obj_t *lastView)
{
    lv_obj_t *mainContainer = GuiCreateContainerWithParent(parent, 408, 0);
    SetContainerDefaultStyle(mainContainer);
    lv_obj_align_to(mainContainer, lastView, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);

    lv_obj_t *titleLabel = lv_label_create(mainContainer);
    lv_label_set_text(titleLabel, isInput ? "From" : "To");
    lv_obj_align(titleLabel, LV_ALIGN_DEFAULT, 24, 16);
    SetTitleLabelStyle(titleLabel);

    lv_obj_t *currLastView = titleLabel;
    int totalHeight = 56;
    int count = isInput ? tx->inputs.size : tx->outputs.size;

    for (int i = 0; i < count; i++) {
        lv_obj_t *itemCont = GuiCreateContainerWithParent(mainContainer, 360, 0);
        lv_obj_set_style_bg_opa(itemCont, 0, LV_PART_MAIN);

        lv_obj_t *order = lv_label_create(itemCont);
        lv_label_set_text_fmt(order, "%d", i + 1);
        lv_obj_align(order, LV_ALIGN_DEFAULT, 0, 0);
        SetTitleLabelStyle(order);

        lv_obj_t *amount = lv_label_create(itemCont);
        lv_label_set_text(amount, isInput ? tx->inputs.data[i].amount : tx->outputs.data[i].amount);
        lv_obj_set_style_text_color(amount, lv_color_hex(0xf5870a), LV_PART_MAIN);
        lv_obj_align_to(amount, order, LV_ALIGN_OUT_RIGHT_TOP, 16, 0);

        lv_obj_t *addr = lv_label_create(itemCont);
        lv_obj_set_width(addr, 332);
        lv_label_set_long_mode(addr, LV_LABEL_LONG_WRAP);
        lv_label_set_text(addr, isInput ? tx->inputs.data[i].address : tx->outputs.data[i].address);
        SetContentLabelStyle(addr);
        lv_obj_align_to(addr, order, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);
        lv_obj_update_layout(addr);

        int itemHeight = lv_obj_get_y2(addr);
        lv_obj_set_height(itemCont, itemHeight);
        lv_obj_align_to(itemCont, currLastView, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

        totalHeight += (itemHeight + 8);
        currLastView = itemCont;
    }

    lv_obj_set_height(mainContainer, totalHeight + 16);
    return mainContainer;
}

static lv_obj_t *CreateNetworkView(lv_obj_t *parent, char *network, lv_obj_t *lastView)
{
    lv_obj_t *networkContainer = GuiCreateContainerWithParent(parent, 408, 62);
    if (lastView == NULL) {
        lv_obj_align(networkContainer, LV_ALIGN_DEFAULT, 0, 0);
    } else {
        lv_obj_align_to(networkContainer, lastView, LV_ALIGN_OUT_BOTTOM_MID, 0, 16);
    }
    SetContainerDefaultStyle(networkContainer);

    lv_obj_t *label = lv_label_create(networkContainer);
    lv_label_set_text(label, "Network");
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 16);
    SetTitleLabelStyle(label);

    lv_obj_t *networkValue = lv_label_create(networkContainer);
    lv_label_set_text(networkValue, network);
    lv_obj_align_to(networkValue, label, LV_ALIGN_OUT_RIGHT_MID, 16, 0);
    SetContentLabelStyle(networkValue);
    return networkContainer;
}

void GuiKaspaTxOverview(lv_obj_t *parent, void *data)
{   
    if (g_parseResult == NULL || g_parseResult->data == NULL) return;
    DisplayKaspaTx *tx = g_parseResult->data;

    lv_obj_set_size(parent, 408, 444);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *last_view = NULL;

    // --- 1. Amount Card ---
    last_view = CreateKaspaOverviewAmountView(parent, tx, last_view);

    // --- 2. Network Card ---
    last_view = CreateNetworkView(parent, tx->network, last_view);

    // --- 3. Address Card ---
    lv_obj_t *addr_card = GuiCreateContainerWithParent(parent, 408, LV_SIZE_CONTENT);
    SetContainerDefaultStyle(addr_card);
    lv_obj_align_to(addr_card, last_view, LV_ALIGN_OUT_BOTTOM_MID, 0, 16);

    // From
    lv_obj_t *label = lv_label_create(addr_card);
    lv_label_set_text(label, "From");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 16);
    SetTitleLabelStyle(label);

    lv_obj_t *from_addr = lv_label_create(addr_card);
    lv_label_set_text(from_addr, tx->inputs.data[0].address);
    lv_obj_set_width(from_addr, 360);
    lv_label_set_long_mode(from_addr, LV_LABEL_LONG_WRAP);
    SetContentLabelStyle(from_addr);
    lv_obj_align_to(from_addr, label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);

    // To
    label = lv_label_create(addr_card);
    lv_label_set_text(label, "To");
    lv_obj_align_to(label, from_addr, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    SetTitleLabelStyle(label);

    const char* to_addr = "Unknown";
    for(int i=0; i<tx->outputs.size; i++) {
        if(!tx->outputs.data[i].is_external) {
            to_addr = tx->outputs.data[i].address;
            break;
        }
    }
    lv_obj_t *dest_addr = lv_label_create(addr_card);
    lv_label_set_text(dest_addr, to_addr);
    lv_obj_set_width(dest_addr, 360);
    lv_label_set_long_mode(dest_addr, LV_LABEL_LONG_WRAP);
    SetContentLabelStyle(dest_addr);
    lv_obj_align_to(dest_addr, label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);

    // Dynamically adjust address card height
    lv_obj_update_layout(dest_addr);
    lv_obj_set_height(addr_card, lv_obj_get_y2(dest_addr) + 16);
}

void GuiKaspaTxDetail(lv_obj_t *parent, void *data)
{
    if (g_parseResult == NULL || g_parseResult->data == NULL) return;
    DisplayKaspaTx *tx = g_parseResult->data;

    lv_obj_set_size(parent, 408, 444);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *lastView = NULL;

    lastView = CreateNetworkView(parent, tx->network, NULL);

    lastView = CreateKaspaDetailAmountView(parent, tx, lastView);

    lastView = CreateKaspaDetailListView(parent, tx, true, lastView);
    CreateKaspaDetailListView(parent, tx, false, lastView);
}

