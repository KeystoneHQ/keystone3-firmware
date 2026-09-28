
/**
 * @file main
 *
 */

/*********************
 *      INCLUDES
 *********************/
#define _DEFAULT_SOURCE /* needed for usleep() */
#include <stdlib.h>
#include <unistd.h>
#define SDL_MAIN_HANDLED /*To fix SDL's "undefined reference to WinMain" \
                            issue*/
#include "lv_drivers/sdl/sdl.h"
#include "lvgl/examples/lv_examples.h"
#include "lvgl/lvgl.h"
#include <SDL2/SDL.h>

#include "device_setting.h"
#include "account_public_info.h"
#include "account_manager.h"
#include "gui.h"
#include "gui_api.h"
#include "gui_framework.h"
#include "gui_lock_widgets.h"
#include "gui_views.h"
#include "gui_wallet.h"
#include "keystore.h"
#include "librust_c.h"
#include "log_print.h"
#include "secret_cache.h"
#include "user_memory.h"
#include "user_utils.h"
#include "simulator_cmd_server.h"

#ifndef _WIN32
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#endif

/*********************
 *      DEFINES
 *********************/
#define SIMULATOR_COMMAND_PORT 8765
#define SIMULATOR_COMMAND_BUFFER_SIZE 8192
#define SIMULATOR_COMMAND_MAX_ARGC 16

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void hal_init(void);
#ifndef _WIN32
static void *command_server_main(void *arg);
static void start_command_server(void);
static bool simulator_dispatch_command(char *command);
static void simulator_process_pending_command(void);
#endif

struct URParseResult *test_get_eth_sign_request(void);
struct URParseResult *test_get_eth_sign_request_for_c_path(char *path);
struct URParseResult *test_get_eth_sign_request_for_personal_message(void);
struct URParseResult *test_get_eth_sign_request_for_personal_message_c_path(char *path);
struct URParseResult *test_get_eth_eip1559_sign_request(void);
struct URParseResult *test_get_eth_eip1559_sign_request_for_c_path(char *path);
struct URParseResult *test_get_eth_typed_data_sign_request(char *sign_data_hex);
struct URParseResult *test_get_eth_typed_data_sign_request_for_c_path(char *sign_data_hex, char *path);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *      VARIABLES
 **********************/
#ifndef _WIN32
static pthread_mutex_t g_command_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_command_cond = PTHREAD_COND_INITIALIZER;
static char g_pending_command[SIMULATOR_COMMAND_BUFFER_SIZE];
static int g_pending_client_fd = -1;
static bool g_command_pending = false;
static bool g_command_done = false;
static bool g_is_handling_command = false;

bool SimulatorCommandServerIsHandlingCommand(void)
{
    return g_is_handling_command;
}
#endif

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
int hexstr_to_uint8_array(const char *hexstr, uint8_t *buf, size_t buf_len) {
    int count = 0;
    while (*hexstr && count < buf_len) {
        unsigned int byte;
        if (sscanf(hexstr, "%2x", &byte) != 1) {
            break;
        }
        buf[count++] = byte;
        hexstr += 2;
    }
    return count;
}

int main(int argc, char **argv)
{
    (void)argc; /*Unused*/
    (void)argv; /*Unused*/

    printf("start");
    setvbuf(stdout, NULL, _IONBF, 0);
#ifndef _WIN32
    signal(SIGPIPE, SIG_IGN);
#endif

    /*Initialize LVGL*/
    lv_init();

    /*Initialize the HAL (display, input devices, tick) for LVGL*/
    hal_init();

    DeviceSettingsInit();
    GuiStyleInit();
    LanguageInit();

    GuiFrameOpenView(&g_initView);
#ifndef _WIN32
    start_command_server();
#endif
    //  lv_example_calendar_1();
    //  lv_example_btnmatrix_2();
    //  lv_example_checkbox_1();
    //  lv_example_colorwheel_1();
    //  lv_example_chart_6();
    //  lv_example_table_2();
    //  lv_example_scroll_2();
    //  lv_example_textarea_1();
    //  lv_example_msgbox_1();
    //  lv_example_dropdown_2();
    //  lv_example_btn_1();
    //  lv_example_scroll_1();
    //  lv_example_tabview_1();
    //  lv_example_tabview_1();
    //  lv_example_flex_3();
    //  lv_example_label_1();

    while (1)
    {
        /* Periodically call the lv_task handler.
         * It could be done in a timer interrupt or an OS task too.*/
        lv_timer_handler();
#ifndef _WIN32
        simulator_process_pending_command();
#endif
        usleep(5 * 1000);
    }

    return 0;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Initialize the Hardware Abstraction Layer (HAL) for the LVGL graphics
 * library
 */
static void hal_init(void)
{
    /* Use the 'monitor' driver which creates window on PC's monitor to simulate a
     * display*/
    sdl_init();

    /*Create a display buffer*/
    static lv_disp_draw_buf_t disp_buf1;
    static lv_color_t buf1_1[SDL_HOR_RES * 100];
    lv_disp_draw_buf_init(&disp_buf1, buf1_1, NULL, SDL_HOR_RES * 100);

    /*Create a display*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv); /*Basic initialization*/
    disp_drv.draw_buf = &disp_buf1;
    disp_drv.flush_cb = sdl_display_flush;
    disp_drv.hor_res = SDL_HOR_RES;
    disp_drv.ver_res = SDL_VER_RES;

    lv_disp_t *disp = lv_disp_drv_register(&disp_drv);

    lv_theme_t *th = lv_theme_default_init(
        disp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED),
        LV_THEME_DEFAULT_DARK, LV_FONT_DEFAULT);
    lv_disp_set_theme(disp, th);

    lv_group_t *g = lv_group_create();
    lv_group_set_default(g);

    /* Add the mouse as input device
     * Use the 'mouse' driver which reads the PC's mouse*/
    static lv_indev_drv_t indev_drv_1;
    lv_indev_drv_init(&indev_drv_1); /*Basic initialization*/
    indev_drv_1.type = LV_INDEV_TYPE_POINTER;

    /*This function will be called periodically (by the library) to get the mouse
     * position and state*/
    indev_drv_1.read_cb = sdl_mouse_read;
    lv_indev_t *mouse_indev = lv_indev_drv_register(&indev_drv_1);

    static lv_indev_drv_t indev_drv_2;
    lv_indev_drv_init(&indev_drv_2); /*Basic initialization*/
    indev_drv_2.type = LV_INDEV_TYPE_KEYPAD;
    indev_drv_2.read_cb = sdl_keyboard_read;
    lv_indev_t *kb_indev = lv_indev_drv_register(&indev_drv_2);
    lv_indev_set_group(kb_indev, g);

    static lv_indev_drv_t indev_drv_3;
    lv_indev_drv_init(&indev_drv_3); /*Basic initialization*/
    indev_drv_3.type = LV_INDEV_TYPE_ENCODER;
    indev_drv_3.read_cb = sdl_mousewheel_read;
    lv_indev_t *enc_indev = lv_indev_drv_register(&indev_drv_3);
    lv_indev_set_group(enc_indev, g);
}

#ifndef _WIN32
static void trim_command(char *command)
{
    size_t len = strlen(command);
    while (len > 0 && (command[len - 1] == '\n' || command[len - 1] == '\r')) {
        command[len - 1] = '\0';
        len--;
    }
}

static int split_command_args(char *input, char *argv[], int max_argc)
{
    int argc = 0;
    while (*input != '\0' && argc < max_argc) {
        while (*input == ' ') {
            input++;
        }
        if (*input == '\0') {
            break;
        }

        argv[argc++] = input;
        while (*input != '\0' && *input != ' ') {
            input++;
        }
        if (*input == ' ') {
            *input = '\0';
            input++;
        }
    }
    return argc;
}

static bool dispatch_prefixed_command(char *command, const char *prefix, void (*handler)(int argc, char *argv[]))
{
    size_t prefix_len = strlen(prefix);
    if (strncmp(command, prefix, prefix_len) != 0) {
        return false;
    }

    char *argv[SIMULATOR_COMMAND_MAX_ARGC];
    int argc = split_command_args(command + prefix_len, argv, SIMULATOR_COMMAND_MAX_ARGC);
    handler(argc, argv);
    return true;
}

static void simulator_save_s39_auto(int argc, char *argv[])
{
    int entropyLen = 0;
    unsigned int slip39Id = 0;
    unsigned int slip39Extendable = 0;
    unsigned int slip39IterationExponent = 0;
    uint8_t accountIndex = 0;
    uint8_t entropy[32] = {0};
    uint8_t ems[32] = {0};

    if (argc != 6 || sscanf(argv[1], "%d", &entropyLen) != 1 || entropyLen <= 0 || entropyLen > (int)sizeof(entropy) ||
        strlen(argv[2]) != (size_t)entropyLen * 2 || strlen(argv[3]) != (size_t)entropyLen * 2 ||
        sscanf(argv[4], "%u:%u:%u", &slip39Id, &slip39Extendable, &slip39IterationExponent) != 3 || slip39Id > UINT16_MAX ||
        slip39IterationExponent > UINT8_MAX) {
        printf("input err!\r\n");
        return;
    }
    if (StrToHex(entropy, argv[2]) != (uint32_t)entropyLen || StrToHex(ems, argv[3]) != (uint32_t)entropyLen) {
        printf("input length err\r\n");
        CLEAR_ARRAY(entropy);
        CLEAR_ARRAY(ems);
        return;
    }

    int32_t ret = GetBlankAccountIndex(&accountIndex);
    printf("GetBlankAccountIndex=%d\r\n", ret);
    if (ret != SUCCESS_CODE || accountIndex > 2) {
        printf("CreateNewSlip39Account=%d\r\n", ERR_GENERAL_FAIL);
        CLEAR_ARRAY(entropy);
        CLEAR_ARRAY(ems);
        return;
    }
    printf("next blank account=%d\r\n", accountIndex);
    PrintArray("entropy", entropy, (uint16_t)entropyLen);
    ret = CreateNewSlip39Account(accountIndex, ems, entropy, (uint8_t)entropyLen, argv[5], (uint16_t)slip39Id, slip39Extendable != 0,
                                 (uint8_t)slip39IterationExponent);
    printf("CreateNewSlip39Account=%d\r\n", ret);
    CLEAR_ARRAY(entropy);
    CLEAR_ARRAY(ems);
}

static void simulator_key_store_test(int argc, char *argv[])
{
    if (argc >= 3 && strcmp(argv[0], "set_passphrase") == 0 && strcmp(argv[1], "__EMPTY__") == 0) {
        argv[1] = "";
    }
    if (argc > 0 && (strcmp(argv[0], "new_entropy") == 0 ||
                     strcmp(argv[0], "new_slip39_entropy") == 0 ||
                     strcmp(argv[0], "save_new_entropy") == 0 ||
                     strcmp(argv[0], "save_slip39_entropy") == 0 ||
                     strcmp(argv[0], "save_s39_auto") == 0 ||
                     strcmp(argv[0], "set_passphrase") == 0) &&
        GuiLockScreenIsTop()) {
        GuiLockScreenTurnOff();
        GuiFrameOpenView(&g_homeView);
    }
    if (argc > 0 && strcmp(argv[0], "save_s39_auto") == 0) {
        simulator_save_s39_auto(argc, argv);
        return;
    }
    KeyStoreTest(argc, argv);
}

static bool parse_optional_eth_path(int argc, char *argv[], int path_arg_index, char **path)
{
    *path = "m/44'/60'/0'/0/0";
    if (argc == path_arg_index) {
        return true;
    }
    if (argc != path_arg_index + 1) {
        return false;
    }
    *path = argv[path_arg_index];
    return true;
}

static void simulator_eth_sign(URParseResult *request, int argc, char *argv[])
{
    if (argc != 2 || request == NULL) {
        printf("input err!\r\n");
        if (request != NULL) {
            free_ur_parse_result(request);
        }
        return;
    }

    int32_t index;
    sscanf(argv[0], "%d", &index);
    uint8_t seed[64] = {0};
    uint32_t seed_len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();
    int32_t seed_ret = GetAccountSeed(index, seed, argv[1]);
    if (seed_ret == 0) {
        UREncodeResult *sign_result = eth_sign_tx(request->data, seed, seed_len);
        printf("sign result error_code: %d\r\n", sign_result->error_code);
        printf("sign result error_message: %s\r\n", sign_result->error_message);
        printf("sign result data: %s\r\n", sign_result->data);
        free_ur_encode_result(sign_result);
    } else {
        printf("GetAccountSeed=%d\r\n", seed_ret);
    }
    memset_s(seed, sizeof(seed), 0, sizeof(seed));
    free_ur_parse_result(request);
}

static void simulator_eth_legacy_sign(int argc, char *argv[])
{
    char *path;
    if (!parse_optional_eth_path(argc, argv, 2, &path)) {
        printf("input err!\r\n");
        return;
    }
    char *sign_argv[2] = {argv[0], argv[1]};
    simulator_eth_sign(test_get_eth_sign_request_for_c_path(path), 2, sign_argv);
}

static void simulator_eth_personal_sign(int argc, char *argv[])
{
    char *path;
    if (!parse_optional_eth_path(argc, argv, 2, &path)) {
        printf("input err!\r\n");
        return;
    }
    char *sign_argv[2] = {argv[0], argv[1]};
    simulator_eth_sign(test_get_eth_sign_request_for_personal_message_c_path(path), 2, sign_argv);
}

static void simulator_eth_eip1559_sign(int argc, char *argv[])
{
    char *path;
    if (!parse_optional_eth_path(argc, argv, 2, &path)) {
        printf("input err!\r\n");
        return;
    }
    char *sign_argv[2] = {argv[0], argv[1]};
    simulator_eth_sign(test_get_eth_eip1559_sign_request_for_c_path(path), 2, sign_argv);
}

static uint32_t simulator_get_ur_fragment_count(const char *data)
{
    if (data == NULL) {
        return 0;
    }

    const char *sequence = strchr(data, '/');
    if (sequence == NULL) {
        return 0;
    }
    uint32_t index = 0;
    uint32_t count = 0;
    if (sscanf(sequence + 1, "%u-%u/", &index, &count) != 2 || count == 0) {
        return 0;
    }
    return count;
}

static void simulator_print_ur_result(UREncodeResult *ur)
{
    if (ur == NULL) {
        printf("error_code is -1\r\n");
        printf("error_message is null\r\n");
        return;
    }

    printf("is_multi_part is %d\r\n", ur->is_multi_part);
    printf("data is %s\r\n", ur->data != NULL ? ur->data : "");
    if (ur->is_multi_part && ur->encoder != NULL) {
        uint32_t fragment_count = 1;
        uint32_t expected_count = simulator_get_ur_fragment_count(ur->data);
        uint32_t max_fragment_count = expected_count > 1 ? expected_count : 256;
        while (fragment_count < max_fragment_count) {
            UREncodeMultiResult *part = get_next_part(ur->encoder);
            if (part == NULL) {
                break;
            }
            if (part->error_code != 0) {
                free_ur_encode_muilt_result(part);
                break;
            }
            if (part->data != NULL) {
                if (ur->data != NULL && strcmp(part->data, ur->data) == 0) {
                    free_ur_encode_muilt_result(part);
                    break;
                }
                printf("data is %s\r\n", part->data);
                fragment_count++;
            }
            free_ur_encode_muilt_result(part);
        }
        printf("fragment_count is %u\r\n", fragment_count);
    }
    printf("error_code is %d\r\n", ur->error_code);
    printf("error_message is %s\r\n", ur->error_message != NULL ? ur->error_message : "");
    free_ur_encode_result(ur);
}

static void simulator_connect_metamask(ETHAccountType account_type)
{
    if (GetCurrentAccountIndex() > 2) {
        printf("error_code is -1\r\n");
        printf("error_message is wallet not ready\r\n");
        return;
    }
    simulator_print_ur_result(GetUnlimitedMetamaskDataForAccountType(account_type));
}

static void simulator_connect_btc(void)
{
    if (GetCurrentAccountIndex() > 2) {
        printf("error_code is -1\r\n");
        printf("error_message is wallet not ready\r\n");
        return;
    }
    simulator_print_ur_result(GuiGetStandardBtcData());
}

static void simulator_connect_xrp_toolkit(void)
{
    if (GetCurrentAccountIndex() > 2) {
        printf("error_code is -1\r\n");
        printf("error_message is wallet not ready\r\n");
        return;
    }
    simulator_print_ur_result(GuiGetXrpToolkitDataByIndex(0));
}

static void simulator_connect_solflare(int argc, char *argv[])
{
    if (argc != 1) {
        printf("error_code is -1\r\n");
        printf("error_message is input err\r\n");
        return;
    }
    int path_index = 0;
    if (sscanf(argv[0], "%d", &path_index) != 1 || path_index < 0 || path_index > 2) {
        printf("error_code is -1\r\n");
        printf("error_message is input err\r\n");
        return;
    }
    if (GetCurrentAccountIndex() > 2) {
        printf("error_code is -1\r\n");
        printf("error_message is wallet not ready\r\n");
        return;
    }

    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);

    ExtendedPublicKey keys[10];
    PtrT_CSliceFFI_ExtendedPublicKey public_keys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
    public_keys->data = keys;
    if (path_index == 0) {
        public_keys->size = 10;
        for (int i = XPUB_TYPE_SOL_BIP44_0; i <= XPUB_TYPE_SOL_BIP44_9; i++) {
            int key_index = i - XPUB_TYPE_SOL_BIP44_0;
            keys[key_index].path = SRAM_MALLOC(BUFFER_SIZE_32);
            snprintf_s(keys[key_index].path, BUFFER_SIZE_32, "m/44'/501'/%d'", key_index);
            keys[key_index].xpub = GetCurrentAccountPublicKey(i);
        }
    } else if (path_index == 1) {
        public_keys->size = 1;
        keys[0].path = SRAM_MALLOC(BUFFER_SIZE_32);
        snprintf_s(keys[0].path, BUFFER_SIZE_32, "m/44'/501'");
        keys[0].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_SOL_BIP44_ROOT);
    } else {
        public_keys->size = 10;
        for (int i = XPUB_TYPE_SOL_BIP44_CHANGE_0; i <= XPUB_TYPE_SOL_BIP44_CHANGE_9; i++) {
            int key_index = i - XPUB_TYPE_SOL_BIP44_CHANGE_0;
            keys[key_index].path = SRAM_MALLOC(BUFFER_SIZE_32);
            snprintf_s(keys[key_index].path, BUFFER_SIZE_32, "m/44'/501'/%d'/0'", key_index);
            keys[key_index].xpub = GetCurrentAccountPublicKey(i);
        }
    }

    simulator_print_ur_result(generate_common_crypto_multi_accounts_ur(mfp, sizeof(mfp), public_keys, "SOL"));
    for (int i = 0; i < public_keys->size; i++) {
        SRAM_FREE(public_keys->data[i].path);
    }
    SRAM_FREE(public_keys);
}

static void simulator_connect_keystone_nexus(void)
{
    UREncodeResult *ur = NULL;
    if (GetMnemonicType() == MNEMONIC_TYPE_SLIP39) {
        ur = GuiGetKeystoneConnectWalletDataSlip39();
    } else {
        ur = GuiGetKeystoneConnectWalletDataBip39();
    }
    simulator_print_ur_result(ur);
}

static void simulator_eth_typed_data_sign(int argc, char *argv[])
{
    char *path;
    if (!parse_optional_eth_path(argc, argv, 3, &path)) {
        printf("input err!\r\n");
        return;
    }

    char *sign_argv[2] = {argv[0], argv[1]};
    simulator_eth_sign(test_get_eth_typed_data_sign_request_for_c_path(argv[2], path), 2, sign_argv);
}

static void simulator_print_address_result(SimpleResponse_c_char *result)
{
    if (result == NULL) {
        printf("address error_code: -1\r\n");
        return;
    }
    printf("address error_code: %d\r\n", result->error_code);
    if (result->error_message != NULL) {
        printf("address error_message: %s\r\n", result->error_message);
    }
    if (result->error_code == 0 && result->data != NULL) {
        printf("address=%s\r\n", result->data);
    }
    free_simple_response_c_char(result);
}

static bool simulator_validate_xpub(char *xpub)
{
    if (xpub == NULL) {
        printf("address error_code: -1\r\n");
        printf("address error_message: missing xpub\r\n");
        return false;
    }
    return true;
}

static char *simulator_get_xpub_arg(char *arg)
{
    int32_t xpub_type;
    sscanf(arg, "%d", &xpub_type);
    return GetCurrentAccountPublicKey(xpub_type);
}

static void simulator_address_utxo(int argc, char *argv[])
{
    if (argc != 3) {
        printf("input err!\r\n");
        return;
    }
    char *xpub = simulator_get_xpub_arg(argv[1]);
    if (!simulator_validate_xpub(xpub)) {
        return;
    }
    simulator_print_address_result(utxo_get_address(argv[2], xpub));
}

static void simulator_address_eth(int argc, char *argv[])
{
    if (argc != 4) {
        printf("input err!\r\n");
        return;
    }
    char *xpub = simulator_get_xpub_arg(argv[1]);
    if (!simulator_validate_xpub(xpub)) {
        return;
    }
    simulator_print_address_result(eth_get_address(argv[3], xpub, argv[2]));
}

static void simulator_address_avax_xp(int argc, char *argv[])
{
    if (argc != 4) {
        printf("input err!\r\n");
        return;
    }
    char *xpub = simulator_get_xpub_arg(argv[1]);
    if (!simulator_validate_xpub(xpub)) {
        return;
    }
    simulator_print_address_result(avalanche_get_x_p_address(argv[3], xpub, argv[2]));
}

static void simulator_address_cosmos(int argc, char *argv[])
{
    if (argc != 5) {
        printf("input err!\r\n");
        return;
    }
    char *xpub = simulator_get_xpub_arg(argv[1]);
    if (!simulator_validate_xpub(xpub)) {
        return;
    }
    simulator_print_address_result(cosmos_get_address(argv[3], xpub, argv[2], argv[4]));
}

static void simulator_address_tron(int argc, char *argv[])
{
    if (argc != 3) {
        printf("input err!\r\n");
        return;
    }
    char *xpub = simulator_get_xpub_arg(argv[1]);
    if (!simulator_validate_xpub(xpub)) {
        return;
    }
    simulator_print_address_result(tron_get_address(argv[2], xpub));
}

static void simulator_address_xrp(int argc, char *argv[])
{
    if (argc != 4) {
        printf("input err!\r\n");
        return;
    }
    char *xpub = simulator_get_xpub_arg(argv[1]);
    if (!simulator_validate_xpub(xpub)) {
        return;
    }
    simulator_print_address_result(xrp_get_address(argv[3], xpub, argv[2]));
}

static void simulator_address_xpub_only(int argc, char *argv[])
{
    if (argc != 2) {
        printf("input err!\r\n");
        return;
    }

    char *xpub = simulator_get_xpub_arg(argv[1]);
    if (!simulator_validate_xpub(xpub)) {
        return;
    }
    if (strcmp(argv[0], "sui") == 0) {
        simulator_print_address_result(sui_generate_address(xpub));
    } else if (strcmp(argv[0], "iota") == 0) {
        simulator_print_address_result(iota_get_address_from_pubkey(xpub));
    } else if (strcmp(argv[0], "aptos") == 0) {
        simulator_print_address_result(aptos_generate_address(xpub));
    } else if (strcmp(argv[0], "stellar") == 0) {
        simulator_print_address_result(stellar_get_address(xpub));
    } else if (strcmp(argv[0], "ton") == 0) {
        simulator_print_address_result(ton_get_address(xpub));
    } else {
        printf("unknown address command: %s\r\n", argv[0]);
    }
}

static void simulator_address_cardano(int argc, char *argv[])
{
    if (argc != 4) {
        printf("input err!\r\n");
        return;
    }

    uint32_t index;
    sscanf(argv[3], "%u", &index);
    char *xpub = simulator_get_xpub_arg(argv[2]);
    if (!simulator_validate_xpub(xpub)) {
        return;
    }
    if (strcmp(argv[1], "base") == 0) {
        simulator_print_address_result(cardano_get_base_address(xpub, index, 1));
    } else if (strcmp(argv[1], "enterprise") == 0) {
        simulator_print_address_result(cardano_get_enterprise_address(xpub, index, 1));
    } else if (strcmp(argv[1], "stake") == 0) {
        simulator_print_address_result(cardano_get_stake_address(xpub, index, 1));
    } else {
        printf("unknown cardano address type: %s\r\n", argv[1]);
    }
}

static void simulator_address_arweave(int argc, char *argv[])
{
    if (argc != 3) {
        printf("input err!\r\n");
        return;
    }
    (void)argv[1];
    (void)argv[2];
    char *xpub = GetCurrentAccountPublicKey(XPUB_TYPE_ARWEAVE);
    if (!simulator_validate_xpub(xpub)) {
        return;
    }
    simulator_print_address_result(arweave_get_address(xpub));
}

static void simulator_address_solana(int argc, char *argv[])
{
    if (argc != 4) {
        printf("input err!\r\n");
        return;
    }

    int32_t index;
    sscanf(argv[1], "%d", &index);
    uint8_t seed[64] = {0};
    uint32_t seed_len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();
    int32_t seed_ret = GetAccountSeed(index, seed, argv[2]);
    if (seed_ret != 0) {
        printf("GetAccountSeed=%d\r\n", seed_ret);
        printf("address error_code: %d\r\n", seed_ret);
        printf("address error_message: get seed failed\r\n");
        memset_s(seed, sizeof(seed), 0, sizeof(seed));
        return;
    }

    SimpleResponse_c_char *pubkey = get_ed25519_pubkey_by_seed(seed, seed_len, argv[3]);
    memset_s(seed, sizeof(seed), 0, sizeof(seed));
    if (pubkey == NULL || pubkey->error_code != 0 || pubkey->data == NULL) {
        simulator_print_address_result(pubkey);
        return;
    }
    SimpleResponse_c_char *result = solana_get_address(pubkey->data);
    free_simple_response_c_char(pubkey);
    simulator_print_address_result(result);
}

static void simulator_address_test(int argc, char *argv[])
{
    if (argc <= 0) {
        printf("input err!\r\n");
        return;
    }

    if (strcmp(argv[0], "utxo") == 0) {
        simulator_address_utxo(argc, argv);
    } else if (strcmp(argv[0], "eth") == 0) {
        simulator_address_eth(argc, argv);
    } else if (strcmp(argv[0], "avax_xp") == 0) {
        simulator_address_avax_xp(argc, argv);
    } else if (strcmp(argv[0], "cosmos") == 0) {
        simulator_address_cosmos(argc, argv);
    } else if (strcmp(argv[0], "tron") == 0) {
        simulator_address_tron(argc, argv);
    } else if (strcmp(argv[0], "xrp") == 0) {
        simulator_address_xrp(argc, argv);
    } else if (strcmp(argv[0], "solana") == 0) {
        simulator_address_solana(argc, argv);
    } else if (strcmp(argv[0], "cardano") == 0) {
        simulator_address_cardano(argc, argv);
    } else if (strcmp(argv[0], "arweave") == 0) {
        simulator_address_arweave(argc, argv);
    } else {
        simulator_address_xpub_only(argc, argv);
    }
}

static bool simulator_dispatch_command(char *command)
{
    if (dispatch_prefixed_command(command, "device settings test:", DeviceSettingsTest)) {
        return true;
    }
    if (strncmp(command, "key store test:", strlen("key store test:")) == 0 && SecretCacheGetWalletName() == NULL) {
        SecretCacheSetWalletIndex(0);
        SecretCacheSetWalletName("Wallet 1");
    }
    if (dispatch_prefixed_command(command, "key store test:", simulator_key_store_test)) {
        return true;
    }
    if (dispatch_prefixed_command(command, "address test:", simulator_address_test)) {
        return true;
    }
    if (dispatch_prefixed_command(command, "rust test eth tx:", simulator_eth_legacy_sign)) {
        return true;
    }
    if (dispatch_prefixed_command(command, "rust test parse eth personal message:", simulator_eth_personal_sign)) {
        return true;
    }
    if (dispatch_prefixed_command(command, "rust test eth eip1559 tx:", simulator_eth_eip1559_sign)) {
        return true;
    }
    if (dispatch_prefixed_command(command, "rust test eth typed data:", simulator_eth_typed_data_sign)) {
        return true;
    }
    if (strcmp(command, "rust test connect metamask") == 0) {
        simulator_connect_metamask(Bip44Standard);
        return true;
    }
    if (strcmp(command, "rust test connect metamask ledger_live") == 0) {
        simulator_connect_metamask(LedgerLive);
        return true;
    }
    if (strcmp(command, "rust test connect metamask ledger_legacy") == 0) {
        simulator_connect_metamask(LedgerLegacy);
        return true;
    }
    if (strcmp(command, "rust test connect btc") == 0) {
        simulator_connect_btc();
        return true;
    }
    if (strcmp(command, "rust test get connect xrp toolkit ur") == 0) {
        simulator_connect_xrp_toolkit();
        return true;
    }
    if (dispatch_prefixed_command(command, "rust test connect solflare ", simulator_connect_solflare)) {
        return true;
    }
    if (strcmp(command, "rust test connect keystone nexus") == 0) {
        simulator_connect_keystone_nexus();
        return true;
    }
    return false;
}

static void execute_simulator_command(char *command, int client_fd)
{
    trim_command(command);

    char *test_command = command;
    if (test_command[0] == '#') {
        test_command++;
    }

    int stdout_fd = dup(STDOUT_FILENO);
    if (stdout_fd < 0) {
        dprintf(client_fd, "command server dup stdout failed: %s\r\n", strerror(errno));
        return;
    }

    fflush(stdout);
    dup2(client_fd, STDOUT_FILENO);
    clearerr(stdout);
    bool handled = simulator_dispatch_command(test_command);
    if (!handled) {
        printf("unknown simulator command: %s\r\n", test_command);
    }
    fflush(stdout);
    dup2(stdout_fd, STDOUT_FILENO);
    clearerr(stdout);
    close(stdout_fd);
}

static void queue_command_for_main_thread(char *command, int client_fd)
{
    pthread_mutex_lock(&g_command_mutex);
    while (g_command_pending) {
        pthread_cond_wait(&g_command_cond, &g_command_mutex);
    }

    strncpy(g_pending_command, command, sizeof(g_pending_command) - 1);
    g_pending_command[sizeof(g_pending_command) - 1] = '\0';
    g_pending_client_fd = client_fd;
    g_command_pending = true;
    g_command_done = false;
    pthread_cond_signal(&g_command_cond);

    while (!g_command_done) {
        pthread_cond_wait(&g_command_cond, &g_command_mutex);
    }
    pthread_mutex_unlock(&g_command_mutex);
}

static void simulator_process_pending_command(void)
{
    pthread_mutex_lock(&g_command_mutex);
    if (!g_command_pending) {
        pthread_mutex_unlock(&g_command_mutex);
        return;
    }

    g_is_handling_command = true;
    execute_simulator_command(g_pending_command, g_pending_client_fd);
    g_is_handling_command = false;
    g_command_pending = false;
    g_command_done = true;
    g_pending_client_fd = -1;
    pthread_cond_broadcast(&g_command_cond);
    pthread_mutex_unlock(&g_command_mutex);
}

static void handle_command_client(int client_fd)
{
    char command[SIMULATOR_COMMAND_BUFFER_SIZE];
    size_t used = 0;
    while (used < sizeof(command) - 1) {
        ssize_t n = recv(client_fd, command + used, sizeof(command) - 1 - used, 0);
        if (n <= 0) {
            break;
        }
        used += (size_t)n;
        if (memchr(command, '\n', used) != NULL) {
            break;
        }
    }
    command[used] = '\0';
    queue_command_for_main_thread(command, client_fd);
}

static void *command_server_main(void *arg)
{
    (void)arg;
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        printf("simulator command server socket failed: %s\r\n", strerror(errno));
        return NULL;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#ifdef SO_REUSEPORT
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
#endif

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = htons(SIMULATOR_COMMAND_PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        printf("simulator command server bind failed: %s\r\n", strerror(errno));
        close(server_fd);
        return NULL;
    }

    if (listen(server_fd, 4) < 0) {
        printf("simulator command server listen failed: %s\r\n", strerror(errno));
        close(server_fd);
        return NULL;
    }

    printf("simulator command server listening on 127.0.0.1:%d\r\n", SIMULATOR_COMMAND_PORT);
    while (1) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            continue;
        }
        handle_command_client(client_fd);
        close(client_fd);
    }
    return NULL;
}

static void start_command_server(void)
{
    pthread_t thread;
    int ret = pthread_create(&thread, NULL, command_server_main, NULL);
    if (ret != 0) {
        printf("simulator command server thread failed: %s\r\n", strerror(ret));
        return;
    }
    pthread_detach(thread);
}
#endif
