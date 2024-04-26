#include "gui_pop_message_box.h"
#include "gui.h"
#include "gui_hintbox.h"
#include "usb_task.h"
#include "assert.h"

static GuiMsgBox_t *g_currentMsgBox = NULL;

void OpenMsgBox(const GuiMsgBox_t *msgBox)
{
    ASSERT(msgBox);
    ASSERT(msgBox->init);
    if (g_currentMsgBox) {
        if (msgBox->pagePriority <= g_currentMsgBox->pagePriority) {
            printf("priority is not high enough,%d,%d\n", msgBox->pagePriority, g_currentMsgBox->pagePriority);
            return;
        }
        CloseMsgBox(g_currentMsgBox);
    }
    msgBox->init();
    g_currentMsgBox = (GuiMsgBox_t *)msgBox;
}

void CloseMsgBox(const GuiMsgBox_t *msgBox)
{
    ASSERT(msgBox);
    ASSERT(msgBox->deinit);
    msgBox->deinit();
    if (msgBox == g_currentMsgBox) {
        g_currentMsgBox = NULL;
    }
}

void CloseCurrentMsgBox(void)
{
    if (g_currentMsgBox) {
        CloseMsgBox(g_currentMsgBox);
    }
}
