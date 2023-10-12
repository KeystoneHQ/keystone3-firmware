#ifndef _GUI_ANALYZE_H
#define _GUI_ANALYZE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "gui_chain.h"
#include "gui.h"

typedef void (*GetLabelDataFunc)(void *indata, void *param);
typedef int (*GetLabelDataLenFunc)(void *param);
typedef bool (*GetObjStateFunc)(void *indata, void *param);
typedef void (*GetContSizeFunc)(uint16_t *width, uint16_t *height, void *param);
typedef void *(*GetChainDataFunc)(void);
typedef void (*FreeChainDataFunc)(void);
typedef void *(*GetTableDataFunc)(uint8_t *row, uint8_t *col, void *param);
typedef void (*GetListLenFunc)(uint8_t *len, void *param);
typedef void (*GetListItemKeyFunc)(void *indata, void *param);
typedef void *(*GetCustomContainerFunc)(lv_obj_t *parent, void *g_totalData);

void *GuiTemplateReload(lv_obj_t *parent, uint8_t index);
void GuiTemplateClosePage(void);
GuiRemapViewType ViewTypeReMap(uint8_t viewType);

#ifdef __cplusplus
}
#endif

#endif
