#include "gui_chain.h"
#include "gui_analyze.h"

GetCustomContainerFunc GetOtherChainCustomFunc(char *funcName)
{
    if (!strcmp(funcName, "GuiZcashOverview")) {
        return GuiZcashOverview;
    }

    return NULL;
}
