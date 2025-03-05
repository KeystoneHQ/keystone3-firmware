typedef struct {
    char verifyCode[BUFFER_SIZE_16];
    uint32_t index;
} MultiSigReceiveIndex_t;

static MultiSigReceiveIndex_t g_multiSigReceiveIndex[4];

static void ConvertXPub(char *dest, ChainType chainType);
uint32_t GetAccountMultiReceiveIndexFromFlash(char *verifyCode);

static void LoadCurrentAccountMultiReceiveIndex(void)
{
    for (int i = 0; i < 4; i++) {
        memset_s(&g_multiSigReceiveIndex[i], sizeof(MultiSigReceiveIndex_t), 0, sizeof(MultiSigReceiveIndex_t));
        if (GetCurrenMultisigWalletByIndex(i) == NULL) {
            continue;
        }
        g_multiSigReceiveIndex[i].index = GetAccountMultiReceiveIndexFromFlash(GetCurrenMultisigWalletByIndex(i)->verifyCode);
        strcpy(g_multiSigReceiveIndex[i].verifyCode, GetCurrenMultisigWalletByIndex(i)->verifyCode);
    }
}

static void replace(char *str, const char *old_str, const char *new_str)
{
    char *pos = strstr(str, old_str);
    if (pos != NULL) {
        size_t old_len = strlen(old_str);
        size_t new_len = strlen(new_str);
        size_t tail_len = strlen(pos + old_len);

        memmove(pos + new_len, pos + old_len, tail_len + 1);
        memcpy(pos, new_str, new_len);

        replace(pos + new_len, old_str, new_str);
    }
}

void ExportMultiSigXpub(ChainType chainType)
{
    ASSERT(chainType >= XPUB_TYPE_BTC_MULTI_SIG_P2SH);
    ASSERT(chainType <= XPUB_TYPE_BTC_MULTI_SIG_P2WSH_TEST);

    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    char mfpHexStr[9] = {0};
    ByteArrayToHexStr(mfp, sizeof(mfp), mfpHexStr);

    char path[64] = {0};
    strcpy(path, GetXPubPath(chainType));
    replace(path, "M", "m");

    char xpub[128] = {0};
    ConvertXPub(xpub, chainType);

    char *jsonString = NULL;
    cJSON *rootJson;
    rootJson = cJSON_CreateObject();
    cJSON_AddItemToObject(rootJson, "xfp", cJSON_CreateString(mfpHexStr));
    cJSON_AddItemToObject(rootJson, "xpub", cJSON_CreateString(xpub));
    cJSON_AddItemToObject(rootJson, "path", cJSON_CreateString(path));
    jsonString = cJSON_PrintBuffered(rootJson, 1024, false);
    RemoveFormatChar(jsonString);

    char exportFileName[32] = {0};

    switch (chainType) {
    case XPUB_TYPE_BTC_MULTI_SIG_P2SH:
    case XPUB_TYPE_BTC_MULTI_SIG_P2SH_TEST:
        sprintf(exportFileName, "0:%s_%s.json", mfpHexStr, "P2SH");
        break;
    case XPUB_TYPE_BTC_MULTI_SIG_P2WSH_P2SH:
    case XPUB_TYPE_BTC_MULTI_SIG_P2WSH_P2SH_TEST:
        sprintf(exportFileName, "0:%s_%s.json", mfpHexStr, "P2SH-P2WSH");
        break;
    case XPUB_TYPE_BTC_MULTI_SIG_P2WSH:
    case XPUB_TYPE_BTC_MULTI_SIG_P2WSH_TEST:
        sprintf(exportFileName, "0:%s_%s.json", mfpHexStr, "P2WSH");
        break;
    default:
        break;
    }

    int res = FatfsFileWrite(exportFileName, (uint8_t *)jsonString, strlen(jsonString));

    printf("export data is %s\r\n", jsonString);

    if (res == RES_OK) {
        printf("multi sig write to sdcard success\r\n");
    } else {
        printf("multi sig write to sdcard fail\r\n");
    }

    cJSON_Delete(rootJson);
    EXT_FREE(jsonString);
}

static void ConvertXPub(char *dest, ChainType chainType)
{
    SimpleResponse_c_char *result;

    char *xpub = GetCurrentAccountPublicKey(chainType);
    char head[] = "xpub";
    switch (chainType) {
    case XPUB_TYPE_BTC_MULTI_SIG_P2SH:
        sprintf(dest, "%s", xpub);
        break;
    case XPUB_TYPE_BTC_MULTI_SIG_P2WSH_P2SH:
        head[0] = 'Y';
        break;
    case XPUB_TYPE_BTC_MULTI_SIG_P2WSH:
        head[0] = 'Z';
        break;
    case XPUB_TYPE_BTC_MULTI_SIG_P2SH_TEST:
        head[0] = 't';
        break;
    case XPUB_TYPE_BTC_MULTI_SIG_P2WSH_P2SH_TEST:
        head[0] = 'U';
        break;
    case XPUB_TYPE_BTC_MULTI_SIG_P2WSH_TEST:
        head[0] = 'V';
        break;
    default:
        break;
    }
    result = xpub_convert_version(xpub, head);
    ASSERT(result);
    sprintf(dest, "%s", result->data);
    free_simple_response_c_char(result);
}

void ExportMultiSigWallet(char *verifyCode, uint8_t accountIndex)
{
    ASSERT(accountIndex >= 0);
    ASSERT(accountIndex <= 2);

    MultiSigWalletItem_t *multiSigWalletItem = GetMultisigWalletByVerifyCode(verifyCode);
    if (multiSigWalletItem == NULL) {
        printf("multiSigWalletItem == NULL\r\n");
        return;
    }

    char exportFileName[32] = {0};
    sprintf(exportFileName, "0:exprot-%s.txt", multiSigWalletItem->name);
    int res =  FatfsFileWrite(exportFileName, (uint8_t *)multiSigWalletItem->walletConfig, strlen(multiSigWalletItem->walletConfig));
    printf("export file name  is %s\r\n", exportFileName);
    printf("export data is %s\r\n", multiSigWalletItem->walletConfig);
    if (res == RES_OK) {
        printf("multi sig write to sdcard success\r\n");
    } else {
        printf("multi sig write to sdcard fail\r\n");
    }
}

uint32_t GetAccountMultiReceiveIndex(char *verifyCode)
{
    for (int i = 0; i < 4; i++) {
        if (strcmp(g_multiSigReceiveIndex[i].verifyCode, verifyCode) == 0) {
            return g_multiSigReceiveIndex[i].index;
        }
    }
    return 0;
}

uint32_t GetAccountMultiReceiveIndexFromFlash(char *verifyCode)
{
    char key[BUFFER_SIZE_64] = {0};
    sprintf(key, "multiRecvIndex_%s", verifyCode);
    printf("key = %s.\n", key);
    return GetTemplateWalletValue("BTC", key);
}

void SetAccountMultiReceiveIndex(uint32_t index, char *verifyCode)
{
    char key[BUFFER_SIZE_64] = {0};
    sprintf(key, "multiRecvIndex_%s", verifyCode);
    printf("key = %s.\n", key);
    for (int i = 0; i < 4; i++) {
        if (strlen(g_multiSigReceiveIndex[i].verifyCode) == 0) {
            g_multiSigReceiveIndex[i].index = index;
            strcpy(g_multiSigReceiveIndex[i].verifyCode, verifyCode);
            break;
        } else if (strcmp(g_multiSigReceiveIndex[i].verifyCode, verifyCode) == 0) {
            g_multiSigReceiveIndex[i].index = index;
            break;
        }
    }
    SetTemplateWalletValue("BTC", key, index);
}

void DeleteAccountMultiReceiveIndex(const char* chain, char *verifyCode)
{
    uint32_t addr;
    const char *chainName = "BTC";
    char key[BUFFER_SIZE_64] = {0};
    sprintf(key, "multiRecvIndex_%s", verifyCode);
    printf("key = %s.\n", key);
    cJSON* rootJson = ReadAndParseAccountJson(&addr, NULL);

    cJSON* item = cJSON_GetObjectItem(rootJson, chainName);
    if (item == NULL) {
        item = cJSON_CreateObject();
        cJSON_AddItemToObject(rootJson, chainName, item);
    }

    cJSON* valueItem = cJSON_GetObjectItem(item, key);
    if (valueItem != NULL) {
        cJSON_DeleteItemFromObject(item, key);
    }

    WriteJsonToFlash(addr, rootJson);
    CleanupJson(rootJson);
    LoadCurrentAccountMultiReceiveIndex();
}

uint32_t GetAccountTestReceiveIndex(const char* chainName)
{
    return GetTemplateWalletValue(chainName, "testRecvIndex");
}

void SetAccountTestReceiveIndex(const char* chainName, uint32_t index)
{
    SetTemplateWalletValue(chainName, "testRecvIndex", index);
}

uint32_t GetAccountTestReceivePath(const char* chainName)
{
    return GetTemplateWalletValue(chainName, "testRecvPath");
}

void SetAccountTestReceivePath(const char* chainName, uint32_t index)
{
    SetTemplateWalletValue(chainName, "testRecvPath", index);
}