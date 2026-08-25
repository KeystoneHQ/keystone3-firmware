#include "simulator_cmd_server.h"

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "account_public_info.h"
#include "account_manager.h"
#include "device_setting.h"
#include "keystore.h"
#include "slip39.h"
#include "librust_c.h"
#include "gui_wallet.h"

#define DEFAULT_SIMULATOR_COMMAND_PORT 8765
#define MAX_COMMAND_LEN 4096
#define MAX_RESPONSE_LEN 65536
#define MAX_ARGC 32

typedef void (*SimulatorCommandFunc)(int argc, char *argv[]);

static bool g_isHandlingCommand = false;

static void *SimulatorCommandServerThread(void *arg);
static void HandleClient(int clientFd);
static void DispatchCommand(const char *command);
static void AddressTest(int argc, char *argv[]);
static void ImportSlip39(int argc, char *argv[]);
static void ConnectMetaMask(ETHAccountType accountType);
static void ConnectBtc(void);
static void ConnectXrpToolkit(void);
static void PrintAllURFragments(UREncodeResult *ur);
static void PrintSimpleAddressResult(SimpleResponse_c_char *result);
static bool DispatchPrefixedCommand(const char *command, const char *prefix, SimulatorCommandFunc func);
static int ParseArgs(char *input, char *argv[], int maxArgc);
static void TrimLine(char *input);
static int GetCommandPort(void);

void StartSimulatorCommandServer(void)
{
    pthread_t thread;
    int ret = pthread_create(&thread, NULL, SimulatorCommandServerThread, NULL);
    if (ret != 0) {
        printf("simulator command server start failed: %d\n", ret);
        return;
    }
    pthread_detach(thread);
}

bool SimulatorCommandServerIsHandlingCommand(void)
{
    return g_isHandlingCommand;
}

static void *SimulatorCommandServerThread(void *arg)
{
    (void)arg;

    int serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd < 0) {
        perror("simulator command socket");
        return NULL;
    }

    int reuse = 1;
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(GetCommandPort());

    if (bind(serverFd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("simulator command bind");
        close(serverFd);
        return NULL;
    }

    if (listen(serverFd, 4) < 0) {
        perror("simulator command listen");
        close(serverFd);
        return NULL;
    }

    printf("simulator command server listening on 127.0.0.1:%d\n", GetCommandPort());

    while (true) {
        int clientFd = accept(serverFd, NULL, NULL);
        if (clientFd < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("simulator command accept");
            continue;
        }
        HandleClient(clientFd);
        close(clientFd);
    }

    return NULL;
}

static void HandleClient(int clientFd)
{
    char command[MAX_COMMAND_LEN];
    ssize_t total = 0;

    while (total < (ssize_t)sizeof(command) - 1) {
        ssize_t count = recv(clientFd, command + total, sizeof(command) - 1 - total, 0);
        if (count <= 0) {
            break;
        }
        total += count;
        if (memchr(command, '\n', total) != NULL) {
            break;
        }
    }
    command[total] = '\0';
    TrimLine(command);

    int stdoutFd = dup(STDOUT_FILENO);
    FILE *captureFile = tmpfile();
    if (stdoutFd < 0 || captureFile == NULL) {
        const char *error = "simulator command capture failed\n";
        send(clientFd, error, strlen(error), 0);
        if (stdoutFd >= 0) {
            close(stdoutFd);
        }
        if (captureFile != NULL) {
            fclose(captureFile);
        }
        return;
    }

    fflush(stdout);
    if (dup2(fileno(captureFile), STDOUT_FILENO) < 0) {
        const char *error = "simulator command capture redirect failed\n";
        send(clientFd, error, strlen(error), 0);
        close(stdoutFd);
        fclose(captureFile);
        return;
    }

    g_isHandlingCommand = true;
    DispatchCommand(command);
    g_isHandlingCommand = false;

    fflush(stdout);
    dup2(stdoutFd, STDOUT_FILENO);
    close(stdoutFd);

    char response[MAX_RESPONSE_LEN];
    fseek(captureFile, 0, SEEK_SET);
    size_t responseLen = fread(response, 1, sizeof(response), captureFile);
    fclose(captureFile);
    if (responseLen > 0) {
        send(clientFd, response, responseLen, 0);
    }
}

static void DispatchCommand(const char *command)
{
    if (command == NULL || command[0] == '\0') {
        printf("empty simulator command\n");
        return;
    }

    const char *payload = command[0] == '#' ? command + 1 : command;
    if (DispatchPrefixedCommand(payload, "key store test:", KeyStoreTest)) {
        return;
    }
    if (DispatchPrefixedCommand(payload, "import slip39:", ImportSlip39)) {
        return;
    }
    if (DispatchPrefixedCommand(payload, "device settings test:", DeviceSettingsTest)) {
        return;
    }
    if (DispatchPrefixedCommand(payload, "address test:", AddressTest)) {
        return;
    }
    if (strcmp(payload, "rust test connect metamask") == 0) {
        ConnectMetaMask(Bip44Standard);
        return;
    }
    if (strcmp(payload, "rust test connect metamask ledger_live") == 0) {
        ConnectMetaMask(LedgerLive);
        return;
    }
    if (strcmp(payload, "rust test connect metamask ledger_legacy") == 0) {
        ConnectMetaMask(LedgerLegacy);
        return;
    }
    if (strcmp(payload, "rust test connect btc") == 0) {
        ConnectBtc();
        return;
    }
    if (strcmp(payload, "rust test get connect xrp toolkit ur") == 0) {
        ConnectXrpToolkit();
        return;
    }
    printf("unsupported simulator command: %s\n", command);
}

static void ConnectMetaMask(ETHAccountType accountType)
{
    UREncodeResult *ur = GetUnlimitedMetamaskDataForAccountType(accountType);
    if (ur == NULL) {
        printf("error_code is -1\n");
        printf("error_message is null\n");
        return;
    }
    const char *data = ur->data != NULL ? ur->data : "";
    const char *errorMessage = ur->error_message != NULL ? ur->error_message : "";
    printf("is_multi_part is %d\n", ur->is_multi_part);
    printf("data is %s\n", data);
    printf("error_code is %d\n", ur->error_code);
    printf("error_message is %s\n", errorMessage);
    free_ur_encode_result(ur);
}

static void ConnectBtc(void)
{
    UREncodeResult *ur = GuiGetStandardBtcData();
    if (ur == NULL) {
        printf("error_code is -1\n");
        printf("error_message is null\n");
        return;
    }
    PrintAllURFragments(ur);
}

static void ConnectXrpToolkit(void)
{
    char *xpub = GetCurrentAccountPublicKey(XPUB_TYPE_XRP);
    if (xpub == NULL) {
        printf("error_code is -1\n");
        printf("error_message is XRP xpub not available, create a wallet with XRP chain first\n");
        return;
    }
    UREncodeResult *ur = GuiGetXrpToolkitDataByIndex(0);
    if (ur == NULL) {
        printf("error_code is -1\n");
        printf("error_message is null\n");
        return;
    }
    PrintAllURFragments(ur);
}

static void PrintAllURFragments(UREncodeResult *ur)
{
    if (ur == NULL) {
        return;
    }

    printf("is_multi_part is %d\n", ur->is_multi_part);
    printf("data is %s\n", ur->data != NULL ? ur->data : "");

    if (ur->is_multi_part && ur->encoder != NULL) {
        uint32_t fragment_count = 1;
        while (true) {
            UREncodeMultiResult *multi_result = get_next_part(ur->encoder);
            if (multi_result == NULL) {
                break;
            }
            if (multi_result->error_code != 0) {
                free_ur_encode_muilt_result(multi_result);
                break;
            }
            if (multi_result->data != NULL) {
                printf("data is %s\n", multi_result->data);
                fragment_count++;
            }
            free_ur_encode_muilt_result(multi_result);
        }
        printf("fragment_count is %u\n", fragment_count);
    }

    printf("error_code is %d\n", ur->error_code);
    printf("error_message is %s\n", ur->error_message != NULL ? ur->error_message : "");
    free_ur_encode_result(ur);
}

/* Import user-provided SLIP39 shares. Shares use '_' for spaces so each share
 * remains one simulator command argument. */
static void ImportSlip39(int argc, char *argv[])
{
    if (argc < 4 || argc > SLIP39_MAX_SLICE_COUNT + 3) {
        printf("ImportSlip39=-1\n");
        return;
    }
    uint8_t threshold = (uint8_t)atoi(argv[0]);
    uint8_t wordsCount = 0;
    uint8_t ems[SLIP39_EMS_LEN] = {0};
    uint8_t masterSecret[ENTROPY_MAX_LEN] = {0};
    char *shares[SLIP39_MAX_SLICE_COUNT] = {0};
    uint16_t id = 0;
    uint8_t ie = 0;
    bool eb = false;
    int32_t ret = -1;
    for (int i = 1; i < argc - 1; i++) {
        shares[i - 1] = calloc(strlen(argv[i]) + 1, 1);
        strcpy(shares[i - 1], argv[i]);
        for (char *p = shares[i - 1]; *p; p++) {
            if (*p == '_') *p = ' ';
            if (*p == ' ') wordsCount++;
        }
    }
    int shareCount = argc - 3;
    if (shareCount > 0) {
        wordsCount = 0;
        for (char *p = shares[0]; *p; p++) if (*p == ' ') wordsCount++;
        wordsCount++;
        ret = Slip39GetMasterSecret(threshold, wordsCount, ems, masterSecret, shares, &id, &eb, &ie);
    }
    if (ret == SLIP39_OK) {
        uint8_t index = 0;
        ret = GetBlankAccountIndex(&index);
        if (ret == SUCCESS_CODE && index <= 2) {
            ret = CreateNewSlip39Account(index, ems, masterSecret,
                                         wordsCount == 20 ? 16 : 32,
                                         argv[argc - 1], id, eb, ie);
            printf("accountIndex=%u\n", index);
        }
    }
    printf("ImportSlip39=%d\n", ret);
    for (int i = 0; i < shareCount; i++) free(shares[i]);
}

static void AddressTest(int argc, char *argv[])
{
    if (argc < 1) {
        printf("address test arg err\n");
        return;
    }

    if (strcmp(argv[0], "utxo") == 0) {
        if (argc != 3) {
            printf("address test utxo arg err\n");
            return;
        }
        uint32_t xpubType;
        sscanf(argv[1], "%u", &xpubType);
        PrintSimpleAddressResult(utxo_get_address(argv[2], GetCurrentAccountPublicKey(xpubType)));
    } else if (strcmp(argv[0], "eth") == 0) {
        if (argc != 4) {
            printf("address test eth arg err\n");
            return;
        }
        uint32_t xpubType;
        sscanf(argv[1], "%u", &xpubType);
        PrintSimpleAddressResult(eth_get_address(argv[3], GetCurrentAccountPublicKey(xpubType), argv[2]));
    } else if (strcmp(argv[0], "cosmos") == 0) {
        if (argc != 5) {
            printf("address test cosmos arg err\n");
            return;
        }
        uint32_t xpubType;
        sscanf(argv[1], "%u", &xpubType);
        PrintSimpleAddressResult(cosmos_get_address(argv[3], GetCurrentAccountPublicKey(xpubType), argv[2], argv[4]));
    } else if (strcmp(argv[0], "avax_xp") == 0) {
        if (argc != 4) {
            printf("address test avax_xp arg err\n");
            return;
        }
        uint32_t xpubType;
        sscanf(argv[1], "%u", &xpubType);
        PrintSimpleAddressResult(avalanche_get_x_p_address(argv[3], GetCurrentAccountPublicKey(xpubType), argv[2]));
    } else if (strcmp(argv[0], "solana") == 0) {
        if (argc != 4) {
            printf("address test solana arg err\n");
            return;
        }
        int32_t accountIndex;
        uint8_t seed[64] = {0};
        sscanf(argv[1], "%d", &accountIndex);
        int32_t ret = GetAccountSeed(accountIndex, seed, argv[2]);
        if (ret != 0) {
            printf("get seed response=%d\n", ret);
            return;
        }
        // Same seed length as production code (gui_sol.c): BIP39 uses the
        // full 64-byte seed, SLIP39 uses the master secret (entropy length).
        int seedLen = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? (int)sizeof(seed) : (int)GetCurrentAccountEntropyLen();
        SimpleResponse_c_char *pubkey = get_ed25519_pubkey_by_seed(seed, seedLen, argv[3]);
        if (pubkey->error_code != 0) {
            PrintSimpleAddressResult(pubkey);
            return;
        }
        SimpleResponse_c_char *result = solana_get_address(pubkey->data);
        free_simple_response_c_char(pubkey);
        PrintSimpleAddressResult(result);
    } else if (strcmp(argv[0], "tron") == 0) {
        if (argc != 3) {
            printf("address test tron arg err\n");
            return;
        }
        uint32_t xpubType;
        sscanf(argv[1], "%u", &xpubType);
        PrintSimpleAddressResult(tron_get_address(argv[2], GetCurrentAccountPublicKey(xpubType)));
    } else if (strcmp(argv[0], "xrp") == 0) {
        if (argc != 4) {
            printf("address test xrp arg err\n");
            return;
        }
        uint32_t xpubType;
        sscanf(argv[1], "%u", &xpubType);
        PrintSimpleAddressResult(xrp_get_address(argv[3], GetCurrentAccountPublicKey(xpubType), argv[2]));
    } else if (strcmp(argv[0], "sui") == 0) {
        if (argc != 2) {
            printf("address test sui arg err\n");
            return;
        }
        uint32_t xpubType;
        sscanf(argv[1], "%u", &xpubType);
        PrintSimpleAddressResult(sui_generate_address(GetCurrentAccountPublicKey(xpubType)));
    } else if (strcmp(argv[0], "iota") == 0) {
        if (argc != 2) {
            printf("address test iota arg err\n");
            return;
        }
        uint32_t xpubType;
        sscanf(argv[1], "%u", &xpubType);
        PrintSimpleAddressResult(iota_get_address_from_pubkey(GetCurrentAccountPublicKey(xpubType)));
    } else if (strcmp(argv[0], "aptos") == 0) {
        if (argc != 2) {
            printf("address test aptos arg err\n");
            return;
        }
        uint32_t xpubType;
        sscanf(argv[1], "%u", &xpubType);
        PrintSimpleAddressResult(aptos_generate_address(GetCurrentAccountPublicKey(xpubType)));
    } else if (strcmp(argv[0], "stellar") == 0) {
        if (argc != 2) {
            printf("address test stellar arg err\n");
            return;
        }
        uint32_t xpubType;
        sscanf(argv[1], "%u", &xpubType);
        PrintSimpleAddressResult(stellar_get_address(GetCurrentAccountPublicKey(xpubType)));
    } else if (strcmp(argv[0], "ton") == 0) {
        if (argc != 2) {
            printf("address test ton arg err\n");
            return;
        }
        uint32_t xpubType;
        sscanf(argv[1], "%u", &xpubType);
        PrintSimpleAddressResult(ton_get_address(GetCurrentAccountPublicKey(xpubType)));
    } else if (strcmp(argv[0], "cardano") == 0) {
        if (argc != 4) {
            printf("address test cardano arg err\n");
            return;
        }
        uint32_t xpubType;
        uint32_t index;
        sscanf(argv[2], "%u", &xpubType);
        sscanf(argv[3], "%u", &index);
        if (strcmp(argv[1], "base") == 0) {
            PrintSimpleAddressResult(cardano_get_base_address(GetCurrentAccountPublicKey(xpubType), index, 1));
        } else if (strcmp(argv[1], "enterprise") == 0) {
            PrintSimpleAddressResult(cardano_get_enterprise_address(GetCurrentAccountPublicKey(xpubType), index, 1));
        } else if (strcmp(argv[1], "stake") == 0) {
            PrintSimpleAddressResult(cardano_get_stake_address(GetCurrentAccountPublicKey(xpubType), index, 1));
        } else {
            printf("address test cardano type err\n");
        }
    } else if (strcmp(argv[0], "arweave") == 0) {
        if (argc != 3) {
            printf("address test arweave arg err\n");
            return;
        }
        int32_t accountIndex;
        uint8_t seed[64] = {0};
        sscanf(argv[1], "%d", &accountIndex);
        int32_t ret = GetAccountSeed(accountIndex, seed, argv[2]);
        if (ret != 0) {
            printf("get seed response=%d\n", ret);
            return;
        }
        // Same seed length as production code (gui_model.c): BIP39 uses the
        // full 64-byte seed, SLIP39 uses the master secret (entropy length).
        int seedLen = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? (int)sizeof(seed) : (int)GetCurrentAccountEntropyLen();
        SimpleResponse_u8 *secret = generate_arweave_secret(seed, seedLen);
        if (secret == NULL || secret->error_code != 0) {
            if (secret == NULL) {
                printf("address error=null arweave secret\n");
            } else {
                printf("address error_code=%d\n", secret->error_code);
                printf("address error_message=%s\n", secret->error_message);
                free_simple_response_u8(secret);
            }
            return;
        }
        SimpleResponse_c_char *xpub = generate_rsa_public_key(secret->data, 256, secret->data + 256, 256);
        free_simple_response_u8(secret);
        if (xpub == NULL || xpub->error_code != 0) {
            PrintSimpleAddressResult(xpub);
            return;
        }
        SimpleResponse_c_char *address = arweave_get_address(xpub->data);
        free_simple_response_c_char(xpub);
        if (address == NULL || address->error_code != 0) {
            PrintSimpleAddressResult(address);
            return;
        }
        SimpleResponse_c_char *fixedAddress = fix_arweave_address(address->data);
        free_simple_response_c_char(address);
        PrintSimpleAddressResult(fixedAddress);
    } else {
        printf("unsupported address test: %s\n", argv[0]);
    }
}

static void PrintSimpleAddressResult(SimpleResponse_c_char *result)
{
    if (result == NULL) {
        printf("address error=null response\n");
        return;
    }
    if (result->error_code == 0) {
        printf("address=%s\n", result->data);
    } else {
        printf("address error_code=%d\n", result->error_code);
        printf("address error_message=%s\n", result->error_message);
    }
    free_simple_response_c_char(result);
}

static bool DispatchPrefixedCommand(const char *command, const char *prefix, SimulatorCommandFunc func)
{
    size_t prefixLen = strlen(prefix);
    if (strncmp(command, prefix, prefixLen) != 0) {
        return false;
    }

    char args[MAX_COMMAND_LEN];
    snprintf(args, sizeof(args), "%s", command + prefixLen);
    TrimLine(args);

    char *argv[MAX_ARGC];
    int argc = ParseArgs(args, argv, MAX_ARGC);
    func(argc, argv);
    return true;
}

static int ParseArgs(char *input, char *argv[], int maxArgc)
{
    int argc = 0;
    char *cursor = input;
    while (*cursor != '\0' && argc < maxArgc) {
        while (isspace((unsigned char)*cursor)) {
            cursor++;
        }
        if (*cursor == '\0') {
            break;
        }
        argv[argc++] = cursor;
        while (*cursor != '\0' && !isspace((unsigned char)*cursor)) {
            cursor++;
        }
        if (*cursor == '\0') {
            break;
        }
        *cursor++ = '\0';
    }
    return argc;
}

static void TrimLine(char *input)
{
    size_t len = strlen(input);
    while (len > 0 && (input[len - 1] == '\n' || input[len - 1] == '\r')) {
        input[--len] = '\0';
    }
}

static int GetCommandPort(void)
{
    const char *port = getenv("KEYSTONE_SIMULATOR_COMMAND_PORT");
    if (port == NULL || port[0] == '\0') {
        return DEFAULT_SIMULATOR_COMMAND_PORT;
    }
    return atoi(port);
}
