#ifndef _GUI_PENDING_HINTBOX_H
#define _GUI_PENDING_HINTBOX_H

void GuiPendingHintBoxRemove();
void GuiPendingHintBoxOpen(const char *title, const char *subtitle);
void GuiUpdatePendingHintBoxSubtitle(const char *subtitle);
void GuiPendingHintBoxMoveToTargetParent(lv_obj_t *parent);

#endif