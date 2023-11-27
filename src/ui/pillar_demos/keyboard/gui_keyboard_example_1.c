#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "gui.h"
#include "gui_keyboard.h"
#include "gui_mnemonic_input.h"
#include "gui_letter_tree.h"

static KeyBoard_t *g_keyBoard;
static MnemonicKeyBoard_t *g_mkb;
static uint8_t g_inputWordsCnt = 0;
static int g_currentInputId = 0;
extern TrieSTPtr rootTree;

void gui_keyboard_example_1(void)
{
    lv_obj_t *cont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()));
    g_inputWordsCnt = 12;
    g_mkb = GuiCreateMnemonicKeyBoard(cont, GuiMnemonicInputHandler, KEY_STONE_MNEMONIC_12, NULL);
    g_mkb->currentId = 0;
    lv_obj_align(g_mkb->cont, LV_ALIGN_TOP_MID, 0, 0);
    g_keyBoard = GuiCreateLetterKeyBoard(cont, NULL, true, g_mkb);
    g_mkb->letterKb = g_keyBoard;
    int wordcnt = searchTrie(rootTree, "comm");
    printf("g_mkb = %p\n", g_mkb);
    printf("g_mkb->btnm = %p\n", g_mkb->btnm);
    printf("g_mkb->letterKb = %p\n", g_mkb->letterKb);
//    GuiDeleteKeyBoard(g_keyBoard);
}

