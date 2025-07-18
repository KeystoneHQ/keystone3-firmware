#include <stdio.h>
#include <stdlib.h>
#include "cjson/cJSON.h"
#include "lvgl.h"
#include "gui_analyze.h"
#include "gui_chain.h"
#include "gui_model.h"

#ifndef COMPILE_SIMULATOR
#include "safe_mem_lib.h"
#else
#include "simulator_model.h"
#endif

#define PC_SIMULATOR_PATH "C:/assets"
#define OBJ_VECTOR_MAX_LEN 5

typedef void (*SetLvglFlagFunc)(lv_obj_t *obj, lv_obj_flag_t);

#include "gui_resource.h"
#include "gui.h"
#include "librust_c.h"
#include "user_memory.h"
typedef struct {
    GuiRemapViewType index;
    const char *config;
    GetChainDataFunc func;
    GetLabelDataFunc typeFunc;
    FreeChainDataFunc freeFunc;
} GuiAnalyze_t;

#define GUI_ANALYZE_TABVIEW_CNT 3
typedef struct {
    lv_obj_t *obj[GUI_ANALYZE_TABVIEW_CNT];
    uint8_t tabviewIndex;
} GuiAnalyzeTabview_t;
static GuiAnalyzeTabview_t g_analyzeTabview;

typedef struct {
    void *totalPtr;
    uint8_t col;
    uint8_t row;
} GuiAnalyzeTable_t;

const static GuiAnalyze_t g_analyzeArray[] = {
    {
        REMAPVIEW_BTC,
#ifndef COMPILE_SIMULATOR
        "{\"name\":\"btc_page\",\"type\":\"tabview\",\"pos\":[36,0],\"size\":[408,774],\"bg_color\":0,\"border_width\":0,\"children\":[{\"type\":\"tabview_child\",\"index\":1,\"tab_name\":\"Overview\",\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"custom_container\",\"bg_color\":0,\"bg_opa\":0,\"pos\":[0,12],\"custom_show_func\":\"GuiBtcTxOverview\"}]},{\"type\":\"tabview_child\",\"index\":2,\"tab_name\":\"Details\",\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"custom_container\",\"bg_color\":0,\"bg_opa\":0,\"pos\":[0,12],\"custom_show_func\":\"GuiBtcTxDetail\"}]}]}",
#else
        PC_SIMULATOR_PATH "/page_btc.json",
#endif
        GuiGetParsedQrData,
        NULL,
        FreePsbtUxtoMemory,
    },
    {
        REMAPVIEW_BTC_MESSAGE,
#ifndef COMPILE_SIMULATOR
        "{\"type\":\"container\",\"pos\":[36,0],\"size\":[408,526],\"bg_opa\":0,\"children\":[{\"type\":\"custom_container\",\"bg_color\":0,\"bg_opa\":0,\"pos\":[0,12],\"custom_show_func\":\"GuiBtcMsg\"}]}",
#else
        PC_SIMULATOR_PATH "/page_btc_msg.json",
#endif
        GuiGetParsedQrData,
        NULL,
        FreeBtcMsgMemory,
    },
    GUI_ANALYZE_OBJ_SURPLUS
};

void *GuiTemplateReload(lv_obj_t *parent, uint8_t index);
void GuiTemplateClosePage(void);
static lv_obj_t *GuiWidgetFactoryCreate(lv_obj_t *parent, cJSON *json);

lv_obj_t *g_templateContainer = NULL;
lv_obj_t *g_tableView = NULL;
void *g_totalData;
GuiRemapViewType g_reMapIndex = REMAPVIEW_BUTT;
uint8_t g_viewTypeIndex = ViewTypeUnKnown;
lv_obj_t *g_defaultVector[OBJ_VECTOR_MAX_LEN];
lv_obj_t *g_hiddenVector[OBJ_VECTOR_MAX_LEN];
GuiAnalyzeTable_t g_tableData[8];
uint8_t g_tableDataAmount = 0;
static char g_tableName[GUI_ANALYZE_TABVIEW_CNT][16];
void GuiAnalyzeFreeTable(uint8_t row, uint8_t col, void *param)
{
    if (param == NULL) {
        return;
    }
    int i = 0, j = 0;
    char ***indata = (char ***)param;
    for (i = 0; i < col; i++) {
        for (j = 0; j < row; j++) {
            if (indata[i][j] != NULL) SRAM_FREE(indata[i][j]);
        }
        if (indata[i] != NULL) SRAM_FREE(indata[i]);
    }
    if (indata != NULL) SRAM_FREE(indata);
}

const lv_font_t *GetLvglTextFont(char *fontStr)
{
    if (!strcmp(fontStr, "openSansEnTitle")) {
        return &openSansEnTitle;
    } else if (!strcmp(fontStr, "openSansEnLittleTitle")) {
        return &openSansEnLittleTitle;
    } else if (!strcmp(fontStr, "openSansEnText")) {
        return &openSansEnText;
    } else if (!strcmp(fontStr, "openSansEnIllustrate")) {
        return &openSansEnIllustrate;
    } else if (!strcmp(fontStr, "openSansDesc")) {
        return &openSansDesc;
    } else if (!strcmp(fontStr, "illustrate")) {
        return g_defIllustrateFont;
    }

    return &openSansEnIllustrate;
}

GetContSizeFunc GetPsbtContainerSize(char *type)
{
    if (!strcmp(type, "GetPsbtOverviewSize")) {
        return GetPsbtOverviewSize;
    } else if (!strcmp(type, "GetPsbtDetailSize")) {
        return GetPsbtDetailSize;
    }
    return NULL;
}

__attribute__((weak)) GetContSizeFunc GetOtherChainContainerSize(char *type, GuiRemapViewType remapIndex)
{
    return NULL;
}

GetContSizeFunc GuiTemplateSizeFuncGet(char *type)
{
    if (g_reMapIndex == REMAPVIEW_BTC) {
        return GetPsbtContainerSize(type);
    }

    return GetOtherChainContainerSize(type, g_reMapIndex);
}

__attribute__((weak)) GetListLenFunc GetOtherChainListLenFuncGet(char *type, GuiRemapViewType remapIndex)
{
    return NULL;
}

GetListLenFunc GuiTemplateListLenFuncGet(char *type)
{
    return GetOtherChainListLenFuncGet(type, g_reMapIndex);
}

__attribute__((weak)) GetListItemKeyFunc GetOtherChainListItemKeyFuncGet(char *type, GuiRemapViewType remapIndex)
{
    return NULL;
}

GetListItemKeyFunc GuiTemplateListItemKeyFuncGet(char *type)
{
    return GetOtherChainListItemKeyFuncGet(type, g_reMapIndex);
}

__attribute__((weak)) GetContSizeFunc GetOtherChainPos(char *type, GuiRemapViewType remapIndex)
{
    return NULL;
}

GetContSizeFunc GuiTemplatePosFuncGet(char *type)
{
    return GetOtherChainPos(type, g_reMapIndex);
}

GetLabelDataFunc GuiBtcTextFuncGet(char *type)
{
    if (!strcmp(type, "GetPsbtTotalOutAmount")) {
        return GetPsbtTotalOutAmount;
    } else if (!strcmp(type, "GetPsbtFeeAmount")) {
        return GetPsbtFeeAmount;
    } else if (!strcmp(type, "GetPsbtTotalOutSat")) {
        return GetPsbtTotalOutSat;
    } else if (!strcmp(type, "GetPsbtFeeSat")) {
        return GetPsbtFeeSat;
    } else if (!strcmp(type, "GetPsbtNetWork")) {
        return GetPsbtNetWork;
    } else if (!strcmp(type, "GetPsbtDetailOutputValue")) {
        return GetPsbtDetailOutputValue;
    } else if (!strcmp(type, "GetPsbtDetailInputValue")) {
        return GetPsbtDetailInputValue;
    } else if (!strcmp(type, "GetPsbtDetailFee")) {
        return GetPsbtDetailFee;
    }
    return NULL;
}

GetTableDataFunc GuiBtcTableFuncGet(char *type)
{
    if (!strcmp(type, "GetPsbtInputData")) {
        return GetPsbtInputData;
    } else if (!strcmp(type, "GetPsbtOutputData")) {
        return GetPsbtOutputData;
    } else if (!strcmp(type, "GetPsbtInputDetailData")) {
        return GetPsbtInputDetailData;
    } else if (!strcmp(type, "GetPsbtOutputDetailData")) {
        return GetPsbtOutputDetailData;
    }
    return NULL;
}

__attribute__((weak)) GetLabelDataLenFunc GuiOtherChainTextLenFuncGet(char *type, GuiRemapViewType remapIndex)
{
    return NULL;
}

GetLabelDataLenFunc GuiTemplateTextLenFuncGet(char *type)
{
    return GuiOtherChainTextLenFuncGet(type, g_reMapIndex);
}

__attribute__((weak)) GetLabelDataFunc GuiOtherChainTextFuncGet(char *type, GuiRemapViewType remapIndex)
{
    return NULL;
}

GetLabelDataFunc GuiTemplateTextFuncGet(char *type)
{
    if (g_reMapIndex == REMAPVIEW_BTC || g_reMapIndex == REMAPVIEW_BTC_MESSAGE) {
        return GuiBtcTextFuncGet(type);
    }

    return GuiOtherChainTextFuncGet(type, g_reMapIndex);
}

__attribute__((weak)) GetTableDataFunc GuiOtherChainTableFuncGet(char *type, GuiRemapViewType remapIndex)
{
    return NULL;
}

GetTableDataFunc GuiTemplateTableFuncGet(char *type)
{
    if (g_reMapIndex == REMAPVIEW_BTC) {
        return GuiBtcTableFuncGet(type);
    }

    return GuiOtherChainTableFuncGet(type, g_reMapIndex);
}

const void *GetImgSrc(char *type)
{
    if (!strcmp(type, "imgEns")) {
        return &imgEns;
    }
    if (!strcmp(type, "imgWarningRed")) {
        return &imgWarningRed;
    }
    if (!strcmp(type, "imgContract")) {
        return &imgContract;
    }
    if (!strcmp(type, "imgQrcodeTurquoise")) {
        return &imgQrcodeTurquoise;
    }
    if (!strcmp(type, "imgConversion")) {
        return &imgConversion;
    }
    return &imgSwitch;
}

__attribute__((weak)) GetObjStateFunc GuiOtherChainStateFuncGet(char *type)
{
    return NULL;
}

GetObjStateFunc GuiTemplateStateFuncGet(char *type)
{
    return GuiOtherChainStateFuncGet(type);
}

static void SwitchHidden(lv_event_t *e)
{
    SetLvglFlagFunc clearHidden = lv_obj_add_flag;
    SetLvglFlagFunc addHidden = lv_obj_clear_flag;
    if (lv_obj_has_flag(g_defaultVector[0], LV_OBJ_FLAG_HIDDEN)) {
        clearHidden = lv_obj_clear_flag;
        addHidden = lv_obj_add_flag;
    } else {
        clearHidden = lv_obj_add_flag;
        addHidden = lv_obj_clear_flag;
    }

    for (int i = 0; i < OBJ_VECTOR_MAX_LEN && g_defaultVector[i] != NULL; i++) {
        clearHidden(g_defaultVector[i], LV_OBJ_FLAG_HIDDEN);
    }

    for (int i = 0; i < OBJ_VECTOR_MAX_LEN && g_hiddenVector[i] != NULL; i++) {
        addHidden(g_hiddenVector[i], LV_OBJ_FLAG_HIDDEN);
    }
}

__attribute__((weak)) lv_event_cb_t GuiOtherChainEventCbGet(char *funcName)
{
    return NULL;
}

lv_event_cb_t GuiTemplateEventCbGet(char *type)
{
    if (!strcmp(type, "SwitchHidden")) {
        return SwitchHidden;
    }

    return GuiOtherChainEventCbGet(type);
}

void GuiWidgetBaseInit(lv_obj_t *obj, cJSON *json)
{
    GetContSizeFunc func = NULL;
    uint16_t xpos, ypos;
    int xsize, ysize;
    lv_align_t align;
    // virtual void create_lvgl_widget(cJSON *json) {};
    cJSON *item = cJSON_GetObjectItem(json, "pos_func");
    if (item != NULL) {
        func = GuiTemplatePosFuncGet(item->valuestring);
        func(&xpos, &ypos, g_totalData);
        lv_obj_align(obj, LV_ALIGN_DEFAULT, xpos, ypos);
    } else {
        item = cJSON_GetObjectItem(json, "pos");
        if (item != NULL) {
            xpos = item->child->valueint;
            ypos = item->child->next->valueint;
            item = cJSON_GetObjectItem(json, "align");
            if (item != NULL) {
                align = item->valueint;
                item = cJSON_GetObjectItem(json, "align_to");
                if (item != NULL) {
                    lv_obj_t *parent = lv_obj_get_parent(obj);
                    lv_obj_t *alignChild = lv_obj_get_child(parent, lv_obj_get_child_cnt(parent) + item->valueint);
                    lv_obj_align_to(obj, alignChild, align, xpos, ypos);
                } else {
                    lv_obj_align(obj, align, xpos, ypos);
                }
            } else {
                lv_obj_align(obj, LV_ALIGN_DEFAULT, xpos, ypos);
            }
        }
    }
    item = cJSON_GetObjectItem(json, "size");
    if (item != NULL) {
        xsize = item->child->valueint;
        ysize = item->child->next->valueint;
        lv_obj_set_size(obj, xsize, ysize);
    } else {
        item = cJSON_GetObjectItem(json, "width");
        if (item != NULL) {
            lv_obj_set_width(obj, item->valueint);
        }
    }

    item = cJSON_GetObjectItem(json, "bg_color");
    if (item != NULL) {
        lv_obj_set_style_bg_color(obj, lv_color_hex(item->valueint), LV_PART_MAIN | LV_STATE_DEFAULT);
    } else {
        lv_obj_set_style_bg_color(obj, WHITE_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    item = cJSON_GetObjectItem(json, "bg_opa");
    if (item != NULL) {
        lv_obj_set_style_bg_opa(obj, item->valueint, LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    item = cJSON_GetObjectItem(json, "radius");
    if (item != NULL) {
        lv_obj_set_style_radius(obj, item->valueint, LV_PART_MAIN | LV_STATE_DEFAULT);
    } else {
        lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    item = cJSON_GetObjectItem(json, "border_width");
    if (item != NULL) {
        lv_obj_set_style_border_width(obj, item->valueint, LV_PART_MAIN | LV_STATE_DEFAULT);
    } else {
        lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    item = cJSON_GetObjectItem(json, "border_color");
    if (item != NULL) {
        lv_obj_set_style_border_color(obj, lv_color_hex(item->valueint), LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    item = cJSON_GetObjectItem(json, "pad_vertical");
    if (item != NULL) {
        lv_obj_set_style_pad_top(obj, item->valueint, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_bottom(obj, item->valueint, LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    item = cJSON_GetObjectItem(json, "pad_horizontal");
    if (item != NULL) {
        lv_obj_set_style_pad_left(obj, item->valueint, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_right(obj, item->valueint, LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    item = cJSON_GetObjectItem(json, "aflag");
    if (item != NULL) {
        lv_obj_add_flag(obj, item->valueint);
    }

    item = cJSON_GetObjectItem(json, "cflag");
    if (item != NULL) {
        lv_obj_clear_flag(obj, item->valueint);
    }

    item = cJSON_GetObjectItem(json, "is_show");
    if (item != NULL) {
        // ((1<<(g_viewTypeIndex - 1)) & 0x7fffffff);
        if ((item->valueint & (1 << g_viewTypeIndex)) == 0) {
            lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
        }
    }

    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);

    cJSON *childrenArray = cJSON_GetObjectItem(json, "children");
    if (childrenArray != NULL) {
        for (cJSON *child = childrenArray->child; child != NULL; child = child->next) {
            GuiWidgetFactoryCreate(obj, child);
        }
    }
}

static char *GetLabelText(lv_obj_t *parent, cJSON *json)
{
    GetLabelDataFunc pFunc = NULL;
    GetLabelDataLenFunc lenFunc = NULL;
    cJSON *item = cJSON_GetObjectItem(json, "text_len_func");
    int bufLen = BUFFER_SIZE_512;
    if (item != NULL) {
        lenFunc = GuiTemplateTextLenFuncGet(item->valuestring);
        if (lenFunc != NULL) {
            bufLen = lenFunc(g_totalData) + 1;
        }
    }
    char *text = EXT_MALLOC(bufLen);
    item = cJSON_GetObjectItem(json, "text");
    if (item != NULL) {
        strcpy_s(text, bufLen, _(item->valuestring));
        return text;
    }

    item = cJSON_GetObjectItem(json, "text_key");
    if (item != NULL) {
        strcpy_s(text, bufLen, item->valuestring);
    }

    item = cJSON_GetObjectItem(json, "text_func");
    if (item != NULL) {
        pFunc = GuiTemplateTextFuncGet(item->valuestring);
        if (pFunc != NULL) {
            pFunc(text, g_totalData, bufLen);
        }
    }
    return text;
}

lv_obj_t *GuiWidgetTextArea(lv_obj_t *parent, cJSON *json)
{
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t *cont = GuiCreateContainerWithParent(parent, 360, 330);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICKABLE);
    char *text = GetLabelText(parent, json);
    int textLen = strlen(text);
    int segmentSize = 1000;
    int numSegments = (textLen + segmentSize - 1) / segmentSize;

    for (int i = 0; i < numSegments; i++) {
        int offset = i * segmentSize;
        lv_obj_t *label = GuiCreateIllustrateLabel(cont, text + offset);
        lv_obj_set_width(label, 360);
        lv_label_set_recolor(label, true);
        if (i == 0) {
            lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 10);
        } else {
            GuiAlignToPrevObj(label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);
        }

        if (i < numSegments - 1 && textLen > offset + segmentSize) {
            char savedChar = text[offset + segmentSize];
            text[offset + segmentSize] = '\0';
            lv_label_set_text(label, text + offset);
            text[offset + segmentSize] = savedChar;
        }
    }

    EXT_FREE(text);
    return cont;
}

lv_obj_t *GuiCreateValueLabel(lv_obj_t *parent, const char *text, int indent, uint32_t *yOffset)
{
    lv_obj_t *label = GuiCreateIllustrateLabel(parent, text);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(label, 320);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, indent * 20, *yOffset);
    lv_obj_refr_size(label);
    *yOffset += lv_obj_get_self_height(label);
    return label;
}

static bool g_isJsonStringTooLong = false;
static void DisplayJsonRecursive(lv_obj_t *parent, cJSON *item, int indent, uint32_t *yOffset)
{
    lv_obj_t* label;
    char buf[BUFFER_SIZE_256];

    while (item != NULL) {
        if (item->string != NULL) {
            snprintf(buf, sizeof(buf), "%s:", item->string);
            label = GuiCreateValueLabel(parent, buf, indent, yOffset);
        }

        if (cJSON_IsObject(item)) {
            DisplayJsonRecursive(parent, item->child, indent + 1, yOffset);
        } else if (cJSON_IsArray(item)) {
            int size = cJSON_GetArraySize(item);
            for (int i = 0; i < 1; i++) {
                cJSON* subitem = cJSON_GetArrayItem(item, i);
                DisplayJsonRecursive(parent, subitem, indent, yOffset);
            }
        } else if (cJSON_IsString(item)) {
            if (strlen(item->valuestring) >= BUFFER_SIZE_256) {
                g_isJsonStringTooLong = true;
                snprintf_s(buf, sizeof(buf), "%.252s...", item->valuestring);
            } else {
                snprintf_s(buf, sizeof(buf), "%s", item->valuestring);
            }
            label = GuiCreateValueLabel(parent, buf, indent + 1, yOffset);
        } else if (cJSON_IsNumber(item)) {
            snprintf(buf, sizeof(buf), "%.0f", item->valuedouble);
            label = GuiCreateValueLabel(parent, buf, indent + 1, yOffset);
        } else if (cJSON_IsBool(item)) {
            snprintf(buf, sizeof(buf), "%*s%s", (indent + 1) * 2, "", item->valueint ? "true" : "false");
            label = GuiCreateValueLabel(parent, buf, indent, yOffset);
        } else if (cJSON_IsNull(item)) {
            snprintf(buf, sizeof(buf), "%*snull", (indent + 1) * 2, "");
            label = GuiCreateValueLabel(parent, buf, indent, yOffset);
        }

        item = item->next;
    }
}

lv_obj_t *GuiWidgetJsonLabel(lv_obj_t *parent, cJSON *json)
{
    g_isJsonStringTooLong = false;
    char *text = GetLabelText(parent, json);
    cJSON *root = cJSON_Parse(text);
    if (root == NULL) {
        printf("cJSON_Parse failed\n");
        return parent;
    }

    uint32_t yOffset = 16;
    DisplayJsonRecursive(parent, root, 0, &yOffset);
    cJSON_Delete(root);
    EXT_FREE(text);
    if (g_isJsonStringTooLong) {
        lv_obj_t *label = GuiCreateIllustrateLabel(parent, "Some data has been truncated due to display limitations. Please refer to the #F55831 Raw Data#  for full details.");
        lv_obj_set_style_text_color(label, ORANGE_COLOR, LV_PART_MAIN);
        lv_label_set_recolor(label, true);
        lv_obj_set_width(label, 360);
        GuiAlignToPrevObj(label, LV_ALIGN_OUT_BOTTOM_LEFT, -20, 4);
    }
    return parent;
}

lv_obj_t *GuiWidgetLabel(lv_obj_t *parent, cJSON *json)
{
    char *text = GetLabelText(parent, json);
    lv_obj_t *obj = lv_label_create(parent);
    lv_label_set_long_mode(obj, LV_LABEL_LONG_SCROLL);
    int textWidth = 0;
    cJSON *item = cJSON_GetObjectItem(json, "text_width");
    if (item != NULL) {
        textWidth = item->valueint;
    } else {
        textWidth = 400;
    }

    item = cJSON_GetObjectItem(json, "font");
    if (item != NULL) {
        const lv_font_t *font = GetLvglTextFont(item->valuestring);
        lv_obj_set_style_text_font(obj, font, LV_STATE_DEFAULT | LV_PART_MAIN);
    }

    item = cJSON_GetObjectItem(json, "text_color");
    if (item != NULL) {
        lv_obj_set_style_text_color(obj, lv_color_hex(item->valueint), LV_PART_MAIN | LV_STATE_DEFAULT);
    } else {
        lv_obj_set_style_text_color(obj, WHITE_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    item = cJSON_GetObjectItem(json, "text_opa");
    if (item != NULL) {
        lv_obj_set_style_text_opa(obj, item->valueint, LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    lv_label_set_recolor(obj, true);
    lv_obj_set_style_text_letter_space(obj, LV_STATE_DEFAULT | LV_PART_MAIN, 20);
    lv_label_set_text(obj, text);
    if (lv_obj_get_self_width(obj) >= textWidth) {
        item = cJSON_GetObjectItem(json, "one_line");
        if (item != NULL) {
            lv_label_set_long_mode(obj, LV_LABEL_LONG_SCROLL);
            lv_obj_set_width(obj, textWidth);
            lv_obj_set_style_anim_speed(obj, 100, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_anim_time(obj, 1000, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_anim_set_playback_delay(obj, 1000);
        } else {
            lv_label_set_long_mode(obj, LV_LABEL_LONG_WRAP);
            lv_obj_set_width(obj, textWidth);
        }
    }
    EXT_FREE(text);
    return obj;
}

lv_obj_t *GuiWidgetContainer(lv_obj_t *parent, cJSON *json)
{
    uint16_t contWidth = 0;
    uint16_t contHeight = 0;
    GetContSizeFunc func = NULL;
    lv_obj_t *obj = lv_obj_create(parent);
    lv_obj_set_style_outline_width(obj, 0, LV_STATE_DEFAULT | LV_PART_MAIN);
    lv_obj_set_style_border_width(obj, 0, LV_STATE_DEFAULT | LV_PART_MAIN);
    lv_obj_set_style_border_side(obj, 0, LV_STATE_DEFAULT | LV_PART_MAIN);
    lv_obj_set_style_shadow_width(obj, 0, LV_STATE_DEFAULT | LV_PART_MAIN);
    lv_obj_set_style_pad_all(obj, 0, LV_STATE_DEFAULT | LV_PART_MAIN);

    cJSON *item = cJSON_GetObjectItem(json, "size_func");
    if (item != NULL) {
        func = GuiTemplateSizeFuncGet(item->valuestring);
        func(&contWidth, &contHeight, g_totalData);
        lv_obj_set_size(obj, contWidth, contHeight);
    }

    item = cJSON_GetObjectItem(json, "cb");
    if (item != NULL) {
        lv_obj_add_event_cb(obj, GuiTemplateEventCbGet(item->valuestring), LV_EVENT_CLICKED, NULL);
    }
    return obj;
}

__attribute__((weak)) GetCustomContainerFunc GetOtherChainCustomFunc(char *funcName)
{
    return NULL;
}

GetCustomContainerFunc GuiTemplateCustomFunc(char *funcName)
{
    if (!strcmp(funcName, "GuiBtcTxOverview")) {
        return GuiBtcTxOverview;
    } else if (!strcmp(funcName, "GuiBtcTxDetail")) {
        return GuiBtcTxDetail;
    } else if (!strcmp(funcName, "GuiBtcMsg")) {
        return GuiBtcMsg;
    }

    return GetOtherChainCustomFunc(funcName);
}

lv_obj_t *GuiWidgetCustomContainer(lv_obj_t *parent, cJSON *json)
{
    lv_obj_t *obj = lv_obj_create(parent);
    lv_obj_set_style_outline_width(obj, 0, LV_STATE_DEFAULT | LV_PART_MAIN);
    lv_obj_set_style_border_width(obj, 0, LV_STATE_DEFAULT | LV_PART_MAIN);
    lv_obj_set_style_border_side(obj, 0, LV_STATE_DEFAULT | LV_PART_MAIN);
    lv_obj_set_style_shadow_width(obj, 0, LV_STATE_DEFAULT | LV_PART_MAIN);
    lv_obj_set_style_pad_all(obj, 0, LV_STATE_DEFAULT | LV_PART_MAIN);

    GetCustomContainerFunc func = NULL;
    cJSON *item = cJSON_GetObjectItem(json, "custom_show_func");
    if (item != NULL) {
        func = GuiTemplateCustomFunc(item->valuestring);
        if (func != NULL) {
            func(obj, g_totalData);
        }
    }
    return obj;
}

void GuiWidgetList(lv_obj_t *parent, cJSON *json)
{
    GetListLenFunc lenFunc = NULL;
    GetListItemKeyFunc keyFunc = NULL;
    cJSON *item = cJSON_GetObjectItem(json, "len_func");
    uint8_t len = 0;
    if (item != NULL) {
        lenFunc = GuiTemplateListLenFuncGet(item->valuestring);
        lenFunc(&len, g_totalData);
    } else {
        item = cJSON_GetObjectItem(json, "len");
        len = item->valueint;
    }
    if (len != 0) {
        cJSON *itemMap = cJSON_GetObjectItem(json, "item_map");
        cJSON *itemDefault = itemMap == NULL ? NULL : cJSON_GetObjectItem(itemMap, "default");
        cJSON *itemKeyFunc = cJSON_GetObjectItem(json, "item_key_func");
        cJSON *child = cJSON_GetObjectItem(json, "item");
        keyFunc = GuiTemplateListItemKeyFuncGet(itemKeyFunc->valuestring);
        char *key = SRAM_MALLOC(BUFFER_SIZE_64);
        for (uint8_t i = 0; i < len; i++) {
            if (itemKeyFunc != NULL) {
                keyFunc(key, g_totalData, BUFFER_SIZE_64);
            }
            if (itemMap != NULL) {
                child = cJSON_GetObjectItem(itemMap, key);
                if (child == NULL) {
                    if (itemDefault != NULL) {
                        child = itemDefault;
                    } else {
                        printf("key %s not found!\r\n", key);
                    }
                }
            }
            GuiWidgetFactoryCreate(parent, child);
        }
        SRAM_FREE(key);
    }
}

lv_obj_t *GuiWidgetTable(lv_obj_t *parent, cJSON *json)
{
    uint16_t tableWidth = 400;
    char ***tableData;
    uint8_t col;
    uint8_t row;
    uint16_t keyWidth = 50;
    GetTableDataFunc getDataFunc = NULL;
    lv_obj_t *obj = lv_table_create(parent);
    cJSON *item = cJSON_GetObjectItem(json, "width");
    if (item != NULL) {
        tableWidth = item->valueint;
        lv_obj_set_width(obj, tableWidth);
    }

    item = cJSON_GetObjectItem(json, "key_width");
    if (item != NULL) {
        keyWidth = item->valueint;
    }

    item = cJSON_GetObjectItem(json, "bg_color");
    if (item != NULL) {
        lv_obj_set_style_bg_color(obj, lv_color_hex(item->valueint), LV_PART_MAIN | LV_PART_ITEMS);
    }

    item = cJSON_GetObjectItem(json, "font");
    if (item != NULL) {
        const lv_font_t *font = GetLvglTextFont(item->valuestring);
        lv_obj_set_style_text_font(obj, font, LV_STATE_DEFAULT | LV_PART_MAIN);
    }

    item = cJSON_GetObjectItem(json, "table_func");
    if (item != NULL) {
        getDataFunc = GuiTemplateTableFuncGet(item->valuestring);
    }

    lv_obj_set_style_bg_color(obj, lv_color_hex(0X0), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(obj, LV_OPA_10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(obj, lv_color_hex(0xffffff), LV_PART_ITEMS);
    lv_obj_set_style_text_line_space(obj, -5, LV_PART_MAIN | LV_PART_ITEMS);
    lv_obj_set_style_outline_pad(obj, 0, LV_PART_MAIN | LV_PART_ITEMS);
    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_PART_ITEMS);
    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_PART_ITEMS);
    lv_obj_set_style_border_color(obj, lv_color_hex(0X202020), LV_PART_ITEMS);
    lv_obj_clear_state(obj, LV_STATE_FOCUS_KEY);

    tableData = (char ***)getDataFunc(&row, &col, g_totalData);

    if (col == 1) {
        lv_table_set_col_width(obj, 0, tableWidth);
    } else {
        lv_table_set_col_width(obj, 0, keyWidth);
        lv_table_set_col_width(obj, 1, tableWidth - keyWidth);
    }

    for (int i = 0; i < col; i++) {
        for (int j = 0; j < row; j++) {
            lv_table_set_cell_value(obj, j, i, (char *)tableData[i][j]);
        }
    }
    g_tableData[g_tableDataAmount].col = col;
    g_tableData[g_tableDataAmount].row = row;
    g_tableData[g_tableDataAmount].totalPtr = tableData;
    g_tableDataAmount++;

    return obj;
}

lv_obj_t *GuiWidgetImg(lv_obj_t *parent, cJSON *json)
{
    lv_obj_t *obj = lv_img_create(parent);
    cJSON *item = cJSON_GetObjectItem(json, "img_src");
    if (item != NULL) {
        lv_img_set_src(obj, GetImgSrc(item->valuestring));
    } else {
        lv_img_set_src(obj, &imgSwitch);
        lv_obj_align(obj, LV_ALIGN_RIGHT_MID, -24, 0);
    }

    item = cJSON_GetObjectItem(json, "cb");
    if (item != NULL) {
        lv_obj_add_event_cb(obj, GuiTemplateEventCbGet(item->valuestring), LV_EVENT_CLICKED, NULL);
    }

    cJSON *array = cJSON_GetObjectItem(json, "default");
    if (array != NULL) {
        int i = 0;
        for (cJSON *iter = array->child; iter != NULL; iter = iter->next) {
            lv_obj_t *parent = lv_obj_get_parent(obj);
            lv_obj_t *child = lv_obj_get_child(parent, lv_obj_get_child_cnt(parent) + iter->valueint);
            g_defaultVector[i++] = child;
        }
    }

    array = cJSON_GetObjectItem(json, "hidden");
    if (array != NULL) {
        int i = 0;
        for (cJSON *iter = array->child; iter != NULL; iter = iter->next) {
            lv_obj_t *parent = lv_obj_get_parent(obj);
            lv_obj_t *child = lv_obj_get_child(parent, lv_obj_get_child_cnt(parent) + iter->valueint);
            g_hiddenVector[i++] = child;
        }
    }

    return obj;
}

lv_obj_t *GuiWidgetBtn(lv_obj_t *parent, cJSON *json)
{
    lv_obj_t *obj = NULL;
    cJSON *item = cJSON_GetObjectItem(json, "text");
    if (item != NULL) {
        obj = lv_btn_create(parent);
        lv_obj_t *label = GuiCreateTextLabel(obj, item->valuestring);
        lv_obj_set_style_text_font(label, &openSansEnIllustrate, LV_PART_MAIN);
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 0, 0);
        lv_obj_set_style_outline_width(obj, 0, LV_PART_MAIN);
        lv_obj_set_style_shadow_width(obj, 0, LV_PART_MAIN);
        item = cJSON_GetObjectItem(json, "text_color");
        if (item != NULL) {
            lv_obj_set_style_text_color(label, lv_color_hex(item->valueint), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }

    item = cJSON_GetObjectItem(json, "cb");
    if (item != NULL) {
        lv_obj_add_event_cb(obj, GuiTemplateEventCbGet(item->valuestring), LV_EVENT_CLICKED, NULL);
        lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
    }

    return obj;
}

lv_obj_t *GuiWidgetTabView(lv_obj_t *parent, cJSON *json)
{
    lv_obj_t *obj = lv_tabview_create(parent, LV_DIR_TOP, 64);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x0), LV_PART_MAIN);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x0), LV_PART_ITEMS);
    lv_obj_set_style_border_width(obj, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(obj, lv_color_hex(0xffffff), LV_PART_ITEMS);

    lv_obj_set_style_bg_color(obj, lv_color_hex(0), LV_PART_ITEMS);
    lv_obj_set_style_text_color(obj, lv_color_hex(0xffffff), LV_PART_ITEMS);
    lv_obj_set_style_border_color(obj, lv_color_hex(0), LV_PART_ITEMS);

    g_tableView = obj;
    lv_obj_t *line;
    static lv_point_t points[2] = {{0, 0}, {408, 0}};
    line = (lv_obj_t *)GuiCreateLine(parent, points, 2);
    lv_obj_align(line, LV_ALIGN_TOP_LEFT, 36, 64);

    return obj;
}

lv_obj_t *GuiWidgetTabViewChild(lv_obj_t *parent, cJSON *json)
{
    cJSON *item = cJSON_GetObjectItem(json, "tab_name");
    if (item != NULL) {
        strcpy(g_tableName[g_analyzeTabview.tabviewIndex], item->valuestring);
    }
    lv_obj_t *obj = lv_tabview_add_tab(g_tableView, item->valuestring);

    lv_obj_t *tab_btns = lv_tabview_get_tab_btns(g_tableView);
    lv_obj_set_style_bg_color(tab_btns, BLACK_COLOR, LV_PART_MAIN);
    lv_obj_set_style_text_font(tab_btns, g_defIllustrateFont, LV_PART_ITEMS);
    lv_obj_set_style_text_color(tab_btns, WHITE_COLOR, LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(tab_btns, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_border_side(tab_btns, LV_BORDER_SIDE_BOTTOM, LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_width(tab_btns, 200);

    item = cJSON_GetObjectItem(json, "font");
    if (item != NULL) {
        const lv_font_t *font = GetLvglTextFont(item->valuestring);
        lv_obj_set_style_text_font(obj, font, LV_STATE_DEFAULT | LV_PART_MAIN);
    }

    item = cJSON_GetObjectItem(json, "text_color");
    if (item != NULL) {
        lv_obj_set_style_text_color(obj, lv_color_hex(item->valueint), LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    item = cJSON_GetObjectItem(json, "opa");
    if (item != NULL) {
        lv_obj_set_style_text_opa(obj, item->valueint, LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    g_analyzeTabview.obj[g_analyzeTabview.tabviewIndex] = obj;
    g_analyzeTabview.tabviewIndex++;

    return obj;
}

static lv_obj_t *GuiWidgetFactoryCreate(lv_obj_t *parent, cJSON *json)
{
    lv_obj_t *obj = NULL;
    cJSON *item = cJSON_GetObjectItem(json, "type");
    if (item == NULL) {
        item = cJSON_GetObjectItem(json, "table");
        if (item != NULL) {
            char typeBuf[16];
            // find the ui type
            for (int i = 0; i < NUMBER_OF_ARRAYS(g_analyzeArray); i++) {
                if (g_analyzeArray[i].index == g_reMapIndex) {
                    g_analyzeArray[i].typeFunc(typeBuf, g_totalData, sizeof(typeBuf));
                    break;
                }
            }
            item = cJSON_GetObjectItem(item, typeBuf);
            if (item != NULL) {
                return GuiWidgetFactoryCreate(parent, item);
            }
        } else {
            return NULL;
        }
    }
    const char *type = item->valuestring;
    if (!type) {
        return NULL;
    }
    item = cJSON_GetObjectItem(json, "exist_func");
    if (item != NULL) {
        GetObjStateFunc func = GuiTemplateStateFuncGet(item->valuestring);
        if (!func(NULL, g_totalData)) {
            return NULL;
        }
    }

    if (0 == strcmp(type, "list")) {
        GuiWidgetList(parent, json);
        return NULL;
    }
    if (0 == strcmp(type, "container")) {
        obj = GuiWidgetContainer(parent, json);
    } else if (0 == strcmp(type, "img")) {
        obj = GuiWidgetImg(parent, json);
    } else if (0 == strcmp(type, "label")) {
        obj = GuiWidgetLabel(parent, json);
    } else if (0 == strcmp(type, "table")) {
        obj = GuiWidgetTable(parent, json);
    } else if (0 == strcmp(type, "tabview")) {
        obj = GuiWidgetTabView(parent, json);
    } else if (0 == strcmp(type, "tabview_child")) {
        obj = GuiWidgetTabViewChild(parent, json);
    } else if (0 == strcmp(type, "custom_container")) {
        obj = GuiWidgetCustomContainer(parent, json);
    } else if (0 == strcmp(type, "textarea")) {
        obj = GuiWidgetTextArea(parent, json);
    } else if (0 == strcmp(type, "json_label")) {
        obj = GuiWidgetJsonLabel(parent, json);
    } else if (0 == strcmp(type, "btn")) {
        obj = GuiWidgetBtn(parent, json);
    } else {
        printf("json type is %s\n", type);
        return NULL;
    }
    GuiWidgetBaseInit(obj, json);
    return obj;
}

void SetParseTransactionResult(void* result)
{
    g_totalData = ((TransactionParseResult_DisplayTx *)result)->data;
}

static void* CreateTransactionDetailWidgets()
{
    int index = -1;
    for (int j = 0; j < NUMBER_OF_ARRAYS(g_analyzeArray); j++) {
        if (g_reMapIndex == g_analyzeArray[j].index) {
            index = j;
            break;
        }
    }
    if (index == -1) {
        return NULL;
    }
#ifdef COMPILE_SIMULATOR
    lv_fs_file_t fd;
    uint32_t size;
#define JSON_MAX_LEN (1024 * 100)
    char buf[JSON_MAX_LEN];
    if (LV_FS_RES_OK != lv_fs_open(&fd, g_analyzeArray[index].config, LV_FS_MODE_RD)) {
        printf("lv_fs_open failed %s\n", g_analyzeArray[index].config);
        return NULL;
    }

    lv_fs_read(&fd, buf, JSON_MAX_LEN, &size);
    buf[size] = '\0';
#endif

#ifdef COMPILE_SIMULATOR
    cJSON *paramJson = cJSON_Parse(buf);
#else
    cJSON *paramJson = cJSON_Parse(g_analyzeArray[index].config);
#endif
    if (paramJson == NULL) {
        printf("cJSON_Parse failed\n");
#ifdef COMPILE_SIMULATOR
        lv_fs_close(&fd);
#endif
        return NULL;
    }
    lv_obj_t *obj = GuiWidgetFactoryCreate(g_templateContainer, paramJson);
#ifdef COMPILE_SIMULATOR
    lv_fs_close(&fd);
#endif
    cJSON_Delete(paramJson);
    return obj;
}

void ParseTransaction(uint8_t index)
{
    g_reMapIndex = ViewTypeReMap(index);
    g_viewTypeIndex = index;

    for (int i = 0; i < NUMBER_OF_ARRAYS(g_analyzeArray); i++) {
        if (g_reMapIndex == g_analyzeArray[i].index) {
            GuiModelParseTransaction(g_analyzeArray[i].func);
            break;
        }
    }
}

static lv_obj_t *g_imgCont = NULL;
void GuiAnalyzeViewInit(lv_obj_t *parent)
{
    uint16_t width = 0;
    g_imgCont = (lv_obj_t *)GuiCreateContainerWithParent(parent, 440, 530);
    lv_obj_align(g_imgCont, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_scroll_dir(g_imgCont, LV_DIR_VER);
    lv_obj_add_flag(g_imgCont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(g_imgCont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_scrollbar_mode(g_imgCont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(g_imgCont, LV_OBJ_FLAG_SCROLL_ELASTIC);

    lv_obj_t *tabView = lv_tabview_create(g_imgCont, LV_DIR_TOP, 64);
    lv_obj_set_style_bg_color(tabView, lv_color_hex(0x0), LV_PART_MAIN);
    lv_obj_set_style_bg_color(tabView, lv_color_hex(0x0), LV_PART_ITEMS);
    lv_obj_set_style_border_width(tabView, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(tabView, lv_color_hex(0xffffff), LV_PART_ITEMS);
    lv_obj_clear_flag(tabView, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_set_scrollbar_mode(tabView, LV_SCROLLBAR_MODE_OFF);

    lv_obj_set_style_bg_color(tabView, lv_color_hex(0), LV_PART_ITEMS);
    lv_obj_set_style_text_color(tabView, lv_color_hex(0xffffff), LV_PART_ITEMS);
    lv_obj_set_style_border_color(tabView, lv_color_hex(0), LV_PART_ITEMS);

    static lv_point_t points[2] = {{0, 0}, {408, 0}};
    lv_obj_t *line = (lv_obj_t *)GuiCreateLine(g_imgCont, points, 2);
    lv_obj_align(line, LV_ALIGN_TOP_LEFT, 0, 64);
    uint16_t tabWidth = 0;
    for (int i = 0; i < GUI_ANALYZE_TABVIEW_CNT; i++) {
        if (g_analyzeTabview.obj[i] == NULL) {
            if (i <= 1) {
                tabWidth = 440;
            } else {
                tabWidth = 300;
            }
            break;
        }
    }

    for (int i = 0; i < GUI_ANALYZE_TABVIEW_CNT; i++) {
        if (g_analyzeTabview.obj[i] == NULL) {
            break;
        }
        lv_obj_t *tabChild;
        tabChild = lv_tabview_add_tab(tabView, g_tableName[i]);
        lv_obj_set_scrollbar_mode(tabChild, LV_SCROLLBAR_MODE_OFF);
        lv_obj_clear_flag(tabChild, LV_OBJ_FLAG_SCROLL_ELASTIC);
        lv_obj_set_style_pad_all(tabChild, 0, LV_PART_MAIN);
        lv_obj_set_style_border_width(tabChild, 0, LV_PART_MAIN);
        lv_obj_set_style_shadow_width(tabChild, 0, LV_PART_MAIN);
        lv_obj_t *tab_btns = lv_tabview_get_tab_btns(tabView);
        lv_obj_set_style_bg_color(tab_btns, BLACK_COLOR, LV_PART_MAIN);
        lv_obj_set_style_text_font(tab_btns, g_defIllustrateFont, LV_PART_ITEMS);
        lv_obj_set_style_text_color(tab_btns, WHITE_COLOR, LV_PART_ITEMS | LV_STATE_CHECKED);
        lv_obj_set_style_text_color(tab_btns, WHITE_COLOR, LV_PART_MAIN);
        lv_obj_set_style_border_side(tab_btns, LV_BORDER_SIDE_BOTTOM, LV_PART_ITEMS | LV_STATE_CHECKED);
        lv_obj_set_style_text_opa(tab_btns, 255, LV_PART_MAIN | LV_STATE_CHECKED);
        lv_obj_set_style_text_opa(tab_btns, 150, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_width(tab_btns, tabWidth);
        int childCnt = lv_obj_get_child_cnt(g_analyzeTabview.obj[i]);
        int yOffset = 12;
        for (int j = 0; j < childCnt; j++) {
            lv_obj_t *child = lv_obj_get_child(g_analyzeTabview.obj[i], j);
            if (lv_obj_get_child_cnt(child) == 0) {
                lv_obj_align(child, LV_ALIGN_DEFAULT, 0, yOffset);
            } else {
                lv_obj_align(child, LV_ALIGN_TOP_MID, 0, yOffset);
            }
            yOffset = yOffset + lv_obj_get_content_height(child) + 16;
        }
        lv_obj_set_parent(g_analyzeTabview.obj[i], tabChild);
        lv_obj_clear_flag(g_analyzeTabview.obj[i], LV_OBJ_FLAG_SCROLL_ELASTIC);
    }
}

void GuiRefreshOnePage(void)
{
    lv_obj_t *line = lv_obj_get_child(g_templateContainer, 1);
    lv_obj_del(line);
    lv_obj_t *firstChild = lv_obj_get_child(g_templateContainer, 0);
    uint8_t childCnt = lv_obj_get_child_cnt(firstChild);
    for (int i = 0; i < childCnt; i++) {
        lv_obj_t *child = lv_obj_get_child(firstChild, i);
        if (child == NULL) {
            continue;
        }
        if (i == 1) {
            lv_obj_set_parent(child, g_templateContainer);
        } else {
            lv_obj_del(child);
        }
    }
    lv_obj_del(firstChild);
    lv_obj_align(g_templateContainer, LV_ALIGN_TOP_MID, 12, 0);
}

void *GuiTemplateReload(lv_obj_t *parent, uint8_t index)
{
    g_tableView = NULL;
    for (int i = 0; i < GUI_ANALYZE_TABVIEW_CNT; i++) {
        g_analyzeTabview.obj[i] = NULL;
    }
    g_analyzeTabview.tabviewIndex = 0;
    for (uint32_t i = 0; i < GUI_ANALYZE_TABVIEW_CNT; i++) {
        g_analyzeTabview.obj[i] = NULL;
        memset_s(g_tableName[i], sizeof(g_tableName[i]), 0, sizeof(g_tableName[i]));
    }
    g_reMapIndex = ViewTypeReMap(index);
    if (g_reMapIndex == REMAPVIEW_BUTT) {
        return NULL;
    }
    g_viewTypeIndex = index;
    g_templateContainer = (lv_obj_t *)GuiCreateContainerWithParent(parent, 480, 542);
    lv_obj_align(g_templateContainer, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_scrollbar_mode(g_templateContainer, LV_SCROLLBAR_MODE_OFF);
    if (CreateTransactionDetailWidgets() == NULL) {
        lv_obj_del(g_templateContainer);
        return NULL;
    }

    if (g_tableView == NULL || g_analyzeTabview.obj[0] == NULL) {
        if (g_analyzeTabview.obj[0] == NULL) {
            GuiRefreshOnePage();
        }
        return g_templateContainer;
    }
    GuiAnalyzeViewInit(parent);
    lv_obj_del(g_templateContainer);
    return g_imgCont;
}

void GuiTemplateClosePage(void)
{
    for (uint32_t i = 0; i < NUMBER_OF_ARRAYS(g_analyzeArray); i++) {
        if (g_reMapIndex == g_analyzeArray[i].index) {
            g_analyzeArray[i].freeFunc();
            break;
        }
    }

    for (uint32_t i = 0; i < g_tableDataAmount; i++) {
        GuiAnalyzeFreeTable(g_tableData[i].row, g_tableData[i].col, g_tableData[i].totalPtr);
        memset_s(&g_tableData[i], sizeof(g_tableData[i]), 0, sizeof(g_tableData[i]));
    }
    g_tableDataAmount = 0;
}
