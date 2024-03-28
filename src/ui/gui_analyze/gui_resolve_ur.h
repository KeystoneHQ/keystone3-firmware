#ifndef _GUI_RESOLVE_UR_H_
#define _GUI_RESOLVE_UR_H_

#include "ui_display_task.h"
#include "librust_c.h"
#include "qrdecode_task.h"

void handleURResult(URParseResult *urResult, URParseMultiResult *urMultiResult, UrViewType_t urViewType, bool is_multi);

#endif