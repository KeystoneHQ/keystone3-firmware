#ifndef _GUI_PENDING_HINTBOX_H
#define _GUI_PENDING_HINTBOX_H

void GuiPendingHintBoxRemove();
void GuiPendingHintBoxOpen(char* title, char* subtitle);
void GuiPendingHintBoxMoveToTargetParent(lv_obj_t *parent);

#endif