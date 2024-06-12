
#include "gui.h"
#include "gui_obj.h"
#include "gui_create_wallet_widgets.h"
#include "gui_keyboard.h"
#include "gui_hintbox.h"
#include "gui_views.h"
#include "gui_lock_widgets.h"
#include "gui_letter_tree.h"
#include "string.h"
#include "gui_mnemonic_input.h"
#include "slip39.h"
#include "gui_model.h"
#include "bip39.h"
#include "sha256.h"
#include "gui_setting_widgets.h"
#include "secret_cache.h"
#include "gui_forget_pass_widgets.h"
#include "gui_setting_widgets.h"
#include "keystore.h"
#include "account_manager.h"
#include "user_memory.h"

#ifndef COMPILE_MAC_SIMULATOR
#include "sha256.h"
#else
#include "simulator_model.h"
#include "simulator_mock_define.h"
#endif

#pragma GCC optimize ("O0")
extern TrieSTPtr rootTree;
extern char g_wordBuf[GUI_KEYBOARD_CANDIDATE_WORDS_CNT][GUI_KEYBOARD_CANDIDATE_WORDS_LEN];
static char g_sliceHeadWords[GUI_KEYBOARD_CANDIDATE_WORDS_LEN];                                           // slip39 head three words
static uint8_t g_sliceSha256[15][GUI_KEYBOARD_CANDIDATE_WORDS_LEN];                                       // slip39 words hash
static lv_obj_t *g_noticeHintBox = NULL;

char *GuiMnemonicGetTrueWord(const char *word, char *trueWord)
{
    char *temp = trueWord;
    for (int i = 0; i < strnlen_s(word, GUI_KEYBOARD_CANDIDATE_WORDS_LEN); i++) {
        if (word[i] >= 'a' && word[i] <= 'z') {
            *temp++ = word[i];
        }
        if (word[i] == '#') {
            break;
        }
    }
    return trueWord;
}

void ImportShareNextSlice(MnemonicKeyBoard_t *mkb, KeyBoard_t *letterKb)
{
    // todo slice==0 clear
    if (mkb->currentSlice == 0) {
        for (int i = 0; i < 15; i++) {
            memset_s(g_sliceSha256[i], 32, 0, 32);
        }
        memset_s(g_sliceHeadWords, sizeof(g_sliceHeadWords), 0, sizeof(g_sliceHeadWords));
    }
    mkb->currentId = 0;
    bool isSame = false;
    char *mnemonic = SRAM_MALLOC(10 * mkb->wordCnt + 1);
    memset_s(mnemonic, 10 * mkb->wordCnt + 1, 0, 10 * mkb->wordCnt + 1);

    for (int i = 0, j = 0; i < mkb->wordCnt; j++, i += 3) {
        for (int k = i; k < i + 3; k++) {
            char trueBuf[12] = {0};
            GuiMnemonicGetTrueWord(lv_btnmatrix_get_btn_text(mkb->btnm, k), trueBuf);
            strcat(mnemonic, trueBuf);
            strcat(mnemonic, " ");
        }
    }
    if (mkb->wordCnt == 20) {
        mnemonic[strlen(mnemonic) - 2] = '\0';
    } else {
        mnemonic[strlen(mnemonic) - 1] = '\0';
    }

    uint8_t threShold = 0;
    do {
        int ret;
        if (mkb->intputType == MNEMONIC_INPUT_SETTING_VIEW) {
            ret = Slip39OneSliceCheck(mnemonic, mkb->wordCnt, GetSlip39Id(), GetSlip39Ie(), &threShold);
        } else {
            ret = Slip39CheckFirstWordList(mnemonic, mkb->wordCnt, &threShold);
        }
        if (ret < 0) {
            if (ret == SLIP39_NOT_BELONG_THIS_WALLET && mkb->intputType == MNEMONIC_INPUT_SETTING_VIEW) { // recovery
                g_noticeHintBox = GuiCreateResultHintbox(356, &imgFailed, _("seed_check_verify_not_match_title"),
                                  _("error_box_mnemonic_not_match_wallet_desc"), NULL, DARK_GRAY_COLOR, _("OK"), DARK_GRAY_COLOR);
                lv_obj_t *btn = GuiGetHintBoxRightBtn(g_noticeHintBox);
                lv_obj_add_event_cb(btn, CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeHintBox);
            } else {
                g_noticeHintBox = GuiCreateResultHintbox(386, &imgFailed,
                                  _("error_box_invalid_seed_phrase"), _("import_wallet_invalid_phrase_desc"),
                                  NULL, DARK_GRAY_COLOR, _("OK"), DARK_GRAY_COLOR);
                lv_obj_t *btn = GuiGetHintBoxRightBtn(g_noticeHintBox);
                lv_obj_add_event_cb(btn, CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeHintBox);
            }
        } else {
            if (mkb->threShold == 0xff) {
                mkb->threShold = threShold;
                for (int i = 0; i < 3; i++) {
                    char trueBuf[12] = {0};
                    strcat(g_sliceHeadWords, GuiMnemonicGetTrueWord(lv_btnmatrix_get_btn_text(mkb->btnm, i), trueBuf));
                    if (i == 2) {
                        break;
                    }
                    strcat(g_sliceHeadWords, " ");
                }
            } else {
                uint8_t tempHash[32];
                sha256((struct sha256 *)tempHash, mnemonic, strlen(mnemonic));
                for (int i = 0; i < mkb->currentSlice; i++) {
                    if (!memcmp(tempHash, g_sliceSha256[i], 32)) {
                        g_noticeHintBox = GuiCreateResultHintbox(386, &imgFailed, _("import_wallet_ssb_incorrect_title"),
                                          _("import_wallet_ssb_repeat_desc"), NULL, DARK_GRAY_COLOR, _("OK"), DARK_GRAY_COLOR);
                        lv_obj_t *rightBtn = GuiGetHintBoxRightBtn(g_noticeHintBox);
                        lv_obj_add_event_cb(rightBtn, CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeHintBox);
                        isSame = true;
                        break;
                    }
                }
            }
            ClearMnemonicKeyboard(mkb, &mkb->currentId);
            if (!isSame) {
                sha256((struct sha256 *)g_sliceSha256[mkb->currentSlice], mnemonic, strlen(mnemonic));
                if (mkb->intputType != MNEMONIC_INPUT_SETTING_VIEW) {
                    SecretCacheSetSlip39Mnemonic(mnemonic, mkb->currentSlice);
                }

                if (mkb->intputType == MNEMONIC_INPUT_SETTING_VIEW) {
                    mkb->currentSlice++;
                    lv_label_set_text_fmt(mkb->titleLabel, _("import_wallet_ssb_title_fmt"), mkb->currentSlice + 1);
                    lv_label_set_text_fmt(mkb->descLabel, _("import_wallet_ssb_desc_fmt"), mkb->wordCnt, mkb->currentSlice + 1);
                    g_noticeHintBox = GuiCreateResultHintbox(386, &imgSuccess, _("shamir_phrase_verify_success_title"),
                                      _("import_wallet_share_success_desc"), _("Continue"), DARK_GRAY_COLOR, _("Done"), ORANGE_COLOR);
                    lv_obj_t *rightBtn = GuiGetHintBoxRightBtn(g_noticeHintBox);
                    lv_obj_add_event_cb(rightBtn, CloseToSubtopViewHandler, LV_EVENT_CLICKED, &g_noticeHintBox);
                    lv_obj_t *leftBtn = GuiGetHintBoxLeftBtn(g_noticeHintBox);
                    lv_obj_add_event_cb(leftBtn, CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeHintBox);
                    ClearMnemonicKeyboard(mkb, &mkb->currentId);
                } else {
                    lv_obj_clear_flag(mkb->stepLabel, LV_OBJ_FLAG_HIDDEN);
                    if (mkb->currentSlice + 1 == mkb->threShold) {
                        if (mkb->intputType == MNEMONIC_INPUT_FORGET_VIEW) {
                            GuiForgetAnimContDel(1);
                            lv_obj_add_flag(letterKb->cont, LV_OBJ_FLAG_HIDDEN);
                            Slip39Data_t slip39 = {
                                .threShold = mkb->threShold,
                                .wordCnt = mkb->wordCnt,
                            };
                            GuiModelSlip39ForgetPassword(slip39);
                        } else {
                            GuiEmitSignal(SIG_SETUP_VIEW_TILE_NEXT, NULL, 0);
                        }
                    } else {
                        mkb->currentSlice++;
                        if (mkb->stepLabel != NULL) {
                            if (lv_obj_has_flag(mkb->stepLabel, LV_OBJ_FLAG_HIDDEN)) {
                                lv_obj_clear_flag(mkb->stepLabel, LV_OBJ_FLAG_HIDDEN);
                            }
                            lv_label_set_text_fmt(mkb->stepLabel, _("import_wallet_ssb_step_fmt"), mkb->currentSlice + 1, mkb->threShold);
                        }
                        if (mkb->titleLabel != NULL) {
                            lv_label_set_text_fmt(mkb->titleLabel, _("import_wallet_ssb_title_fmt"), mkb->currentSlice + 1);
                        }
                        if (mkb->descLabel != NULL) {
                            lv_label_set_text_fmt(mkb->descLabel, _("import_wallet_ssb_desc_fmt"),
                                                  mkb->wordCnt, mkb->currentSlice + 1);
                        }
                    }
                }
            }
        }
    } while (0);
    GuiSetLetterBoardConfirm(letterKb, 0);
    memset_s(mnemonic, strlen(mnemonic), 0, strlen(mnemonic));
    SRAM_FREE(mnemonic);
}

void ImportSinglePhraseWords(MnemonicKeyBoard_t *mkb, KeyBoard_t *letterKb)
{
    char *mnemonic = SRAM_MALLOC(BIP39_MAX_WORD_LEN * mkb->wordCnt + 1);
    memset_s(mnemonic, BIP39_MAX_WORD_LEN * mkb->wordCnt + 1, 0, BIP39_MAX_WORD_LEN * mkb->wordCnt + 1);

    for (int i = 0, j = 0; i < mkb->wordCnt; j++, i += 3) {
        for (int k = i; k < i + 3; k++) {
            char trueBuf[12] = {0};
            GuiMnemonicGetTrueWord(lv_btnmatrix_get_btn_text(mkb->btnm, k), trueBuf);
            strcat(mnemonic, trueBuf);
            strcat(mnemonic, " ");
        }
    }
    mnemonic[strlen(mnemonic) - 1] = '\0';

    SecretCacheSetMnemonic(mnemonic);

    bool isTon = ton_verify_mnemonic(mnemonic);
    if (mkb->intputType == MNEMONIC_INPUT_IMPORT_VIEW) {
        if (isTon) {
            GuiEmitSignal(SIG_SETUP_SHOW_TON_MNEMONIC_HINT, NULL, 0);

        } else {
            GuiEmitSignal(SIG_SETUP_VIEW_TILE_NEXT, NULL, 0);
            Bip39Data_t bip39 = {
                .wordCnt = mkb->wordCnt,
                .forget = false,
            };
            GuiModelBip39CalWriteSe(bip39);
            GuiCreateCircleAroundAnimation(lv_scr_act(), -40);
        }
        // GuiSetLetterBoardConfirm(letterKb, 0);
    } else if (mkb->intputType == MNEMONIC_INPUT_SETTING_VIEW) {
        if (isTon) {
            GuiModelTonRecoveryCheck();
        } else {
            GuiModelBip39RecoveryCheck(mkb->wordCnt);
        }
        GuiSettingRecoveryCheck();
    } else if (mkb->intputType == MNEMONIC_INPUT_FORGET_VIEW) {
        GuiForgetAnimContDel(1);
        if (isTon) {
            GuiModelTonForgetPassword();
        } else {
            GuiModelBip39ForgetPassword(mkb->wordCnt);
        }
    }
    lv_obj_add_flag(letterKb->cont, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_height(mkb->cont, 400);

    memset_s(mnemonic, strlen(mnemonic), 0, strlen(mnemonic));
    SRAM_FREE(mnemonic);
}

bool GuiMnemonicInputCheck(MnemonicKeyBoard_t *mkb, KeyBoard_t *letterKb)
{
    char trueText[12] = {0};
    if (mkb->currentId != mkb->wordCnt) {
        return false;
    }

    for (int i = 0; i < mkb->wordCnt; i++) {
        memset_s(trueText, sizeof(trueText), 0, sizeof(trueText));
        const char *text = lv_btnmatrix_get_btn_text(mkb->btnm, i);
        GuiMnemonicGetTrueWord(text, trueText);
        if (strlen(trueText) > 0 && !GuiWordsWhole(trueText)) {
            return false;
        }
    }
    GuiSetLetterBoardConfirm(letterKb, 1);
    // lv_obj_add_flag(mkb->nextButton, LV_OBJ_FLAG_CLICKABLE);
    return true;
}

// The current button must be a full word in order to proceed, as determined by kbcb
// The next button is a space
// Next button is full word
// The next button is an incomplete word
static void GuiMnemonicUpdateNextBtn(MnemonicKeyBoard_t *mkb, KeyBoard_t *letterKb, lv_obj_t *obj, const char *word)
{
    bool needNext = false;
    uint32_t currentId = 0;
    char trueText[12] = {0};
    int wordcnt = searchTrie(rootTree, word);
    if (wordcnt == 1 || GuiWordsWhole(word)) {
        GuiInputMnemonicKeyBoard(mkb, g_wordBuf[0], mkb->currentId, 1);
        needNext = true;
    } else if (strlen(word) == 0) {
        currentId = lv_btnmatrix_get_selected_btn(obj);
        const char *currText = lv_btnmatrix_get_btn_text(obj, currentId);
        GuiMnemonicGetTrueWord(currText, trueText);
        if (strlen(trueText) > 0 && GuiWordsWhole(trueText)) {
            needNext = true;
        }
    }

    GuiClearKeyBoard(letterKb);
    if (needNext) {
        if (mkb->currentId < mkb->wordCnt) {
            mkb->currentId++;
        }
        lv_btnmatrix_set_selected_btn(mkb->btnm, mkb->currentId);
        memset_s(trueText, sizeof(trueText), 0, sizeof(trueText));
        currentId = lv_btnmatrix_get_selected_btn(obj);
        const char *nextText = lv_btnmatrix_get_btn_text(obj, currentId);
        GuiMnemonicGetTrueWord(nextText, trueText);
        if (searchTrie(rootTree, trueText) == 1) { // whole word
            GuiSetLetterBoardNext(letterKb);
        }
    }

    lv_obj_scroll_to_y(mkb->cont, (mkb->currentId / 3 - 1) * 72, LV_ANIM_ON);
    GuiMnemonicInputCheck(mkb, letterKb);
}

void GuiMnemonicInputHandler(lv_event_t *e)
{
    char tempBuf[BUFFER_SIZE_128 + 1] = {0};
    static uint8_t isClick = 0;
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    MnemonicKeyBoard_t *mkb = lv_event_get_user_data(e);
    KeyBoard_t *letterKb = mkb->letterKb;
    char trueText[12] = {0};
    if (code == LV_EVENT_DRAW_PART_BEGIN) {
        lv_obj_draw_part_dsc_t *dsc = lv_event_get_draw_part_dsc(e);
        if (dsc->class_p == &lv_btnmatrix_class && dsc->type == LV_BTNMATRIX_DRAW_PART_BTN) {
            uint32_t currentId = lv_btnmatrix_get_selected_btn(obj);
            if (currentId == dsc->id) {
                dsc->rect_dsc->border_color = ORANGE_COLOR;
            } else {
                dsc->rect_dsc->border_color = BLACK_COLOR;
            }
        }
    }

    if (code == LV_EVENT_CLICKED) {
        lv_obj_set_height(mkb->cont, 236);
        uint32_t currentId = lv_btnmatrix_get_selected_btn(obj);
        lv_obj_scroll_to_y(mkb->cont, (currentId / 3 - 1) * 72, LV_ANIM_ON);
        lv_obj_clear_flag(letterKb->cont, LV_OBJ_FLAG_HIDDEN);

        // 1.Determine if the current word is complete
        const char *currText = lv_btnmatrix_get_btn_text(obj, currentId);
        GuiMnemonicGetTrueWord(currText, trueText);
        GuiSetMnemonicCache(letterKb, trueText);
        if (strlen(trueText) > 0 && GuiWordsWhole(trueText)) {
            GuiSetLetterBoardNext(letterKb);
        }
        // GuiClearKeyBoard(letterKb);
        isClick = 2;

        // 2.first determine whether the previous word is complete or not
        for (int i = 0; i < mkb->wordCnt; i++) {
            if (i == currentId) {
                continue;
            }
            memset_s(trueText, sizeof(trueText), 0, sizeof(trueText));
            const char *lastText = lv_btnmatrix_get_btn_text(obj, i);
            // const char *lastText = lv_btnmatrix_get_btn_text(obj, mkb->currentId);
            GuiMnemonicGetTrueWord(lastText, trueText);
            if (strlen(trueText) > 0 && !GuiWordsWhole(trueText)) {
                char buf[BUFFER_SIZE_32] = { 0 };
                snprintf_s(buf, BUFFER_SIZE_32, "#FF0000 %s#", trueText);
                GuiInputMnemonicKeyBoard(mkb, buf, i, 1);
            }
        }

        mkb->currentId = currentId;
        GuiMnemonicInputCheck(mkb, letterKb);
    } else if (code == LV_EVENT_READY) {
        if (mkb->currentId == mkb->wordCnt) {
            GuiSetLetterBoardConfirm(letterKb, 1);
            if (mkb->wordCnt == 33 || mkb->wordCnt == 20) {
                ImportShareNextSlice(mkb, letterKb);
            } else {
                ImportSinglePhraseWords(mkb, letterKb);
            }
            return;
        }

        char *word = lv_event_get_param(e);
        GuiMnemonicUpdateNextBtn(mkb, letterKb, obj, word);

        if (mkb->currentId == mkb->wordCnt) {
            GuiSetLetterBoardConfirm(letterKb, 1);
            if (mkb->wordCnt == 33 || mkb->wordCnt == 20) {
                ImportShareNextSlice(mkb, letterKb);
            } else {
                ImportSinglePhraseWords(mkb, letterKb);
            }
            return;
        }

        if (mkb->wordCnt == 33 || mkb->wordCnt == 20) {
            if (mkb->currentId >= 3) {
                char tempBuf[32] = {0};
                for (int i = 0; i < 3; i++) {
                    char trueBuf[12] = {0};
                    GuiMnemonicGetTrueWord(lv_btnmatrix_get_btn_text(mkb->btnm, i), trueBuf);
                    strcat(tempBuf, trueBuf);
                    if (i == 2) {
                        break;
                    }
                    strcat(tempBuf, " ");
                }
                if (mkb->threShold != 0xff) {
                    if (strcmp(tempBuf, g_sliceHeadWords)) {
                        g_noticeHintBox = GuiCreateResultHintbox(416, &imgFailed, _("import_wallet_ssb_incorrect_title"),
                                          _("import_wallet_ssb_notbelong_desc"), NULL, DARK_GRAY_COLOR, _("OK"), DARK_GRAY_COLOR);
                        lv_obj_t *btn = GuiGetHintBoxRightBtn(g_noticeHintBox);
                        lv_obj_add_event_cb(btn, CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeHintBox);
                    }
                }
            }
        }
    } else if (code == KEY_STONE_KEYBOARD_VALUE_CHANGE || code == LV_EVENT_CANCEL) {
        for (int i = 0; i < 3; i++) {
            memset_s(g_wordBuf[i], sizeof(g_wordBuf[i]), 0, sizeof(g_wordBuf[i]));
            lv_label_set_text(letterKb->associateLabel[i], "");
        }
        uint32_t currentId = lv_btnmatrix_get_selected_btn(obj);
        lv_obj_scroll_to_y(mkb->cont, (currentId / 3 - 1) * 72, LV_ANIM_ON);

        char *word = lv_event_get_param(e);
        if ((strlen(word) == 0 && code == KEY_STONE_KEYBOARD_VALUE_CHANGE)) {
            // if (isClick || (strlen(word) == 0 && code == KEY_STONE_KEYBOARD_VALUE_CHANGE)) {
            if (isClick > 0) {
                isClick--;
            }
            return;
        }

        if (strlen(word) > 0 && GuiSingleWordsWhole(word)) {
            GuiInputMnemonicKeyBoard(mkb, word, mkb->currentId, 1);
            if (mkb->currentId < mkb->wordCnt) {
                mkb->currentId++;
            }
            lv_btnmatrix_set_selected_btn(mkb->btnm, mkb->currentId);
            GuiSetMnemonicCache(letterKb, word);
        } else if (strlen(word) >= 3) {
            int wordcnt = searchTrie(rootTree, word);
            if (wordcnt <= 1) {
                snprintf_s(tempBuf, BUFFER_SIZE_128, "%s#999999 %s#", word, &g_wordBuf[0][strnlen_s(word, GUI_KEYBOARD_CANDIDATE_WORDS_LEN)]);
                lv_label_set_text(letterKb->associateLabel[0], g_wordBuf[0]);
            } else {
                wordcnt = wordcnt > 3 ? 3 : wordcnt;
                for (int i = 0; i < wordcnt; i++) {
                    lv_label_set_text(letterKb->associateLabel[i], g_wordBuf[i]);
                }
                strcpy_s(tempBuf, sizeof(tempBuf), word);
            }
            GuiInputMnemonicKeyBoard(mkb, tempBuf, mkb->currentId, 1);
        } else {
            GuiInputMnemonicKeyBoard(mkb, word, mkb->currentId, 1);
        }
        if (mkb->currentId == mkb->wordCnt) {
            GuiSetLetterBoardConfirm(letterKb, 1);
        } else {
            GuiSetLetterBoardConfirm(letterKb, 0);
        }
    } else if (code == KEY_STONE_KEYBOARD_HIDDEN) {
        lv_obj_set_height(mkb->cont, 400);
    }
}

void GuiSetMnemonicCache(KeyBoard_t *keyBoard, char *word)
{
    lv_textarea_set_text(keyBoard->ta, "");
    GuiKeyBoardRestoreDefault(keyBoard);
    GuiKeyBoardSetMode(keyBoard);
    for (int i = 0; i < 3; i++) {
        memset_s(g_wordBuf[i], sizeof(g_wordBuf[i]), 0, sizeof(g_wordBuf[i]));
        lv_label_set_text(keyBoard->associateLabel[i], "");
    }
}

void GuiMnemonicHintboxClear(void)
{
    GUI_DEL_OBJ(g_noticeHintBox)
}

lv_keyboard_user_mode_t GuiGetMnemonicKbType(int wordCnt)
{
    switch (wordCnt) {
    case 12:
        return KEY_STONE_MNEMONIC_12;
    case 18:
        return KEY_STONE_MNEMONIC_18;
    case 24:
        return KEY_STONE_MNEMONIC_24;
    case 20:
        return KEY_STONE_MNEMONIC_20;
    case 33:
        return KEY_STONE_MNEMONIC_33;
    default:
        break;
    }

    return KEY_STONE_MNEMONIC_12;
}