#include "test_cmd.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "cmsis_os.h"
#include "rtos_expand.h"
#include "task.h"
#include "user_memory.h"
#include "drv_spi.h"
#include "drv_trng.h"
#include "log_print.h"
#include "drv_virtual_touch.h"
#include "drv_ds28s60.h"
#include "drv_gd25qxx.h"
#include "drv_rtc.h"
#include "qrdecode_task.h"
#include "hmac.h"
#include "user_fatfs.h"
// #include "user_param.h"
#include "crc.h"
#include "user_msg.h"
#include "bip39.h"
#include "drv_battery.h"
#include "drv_cst726.h"
#include "user_utils.h"
#include "drv_lcd_bright.h"
#include "drv_i2c_io.h"
#include "cryptoauthlib.h"
#include "drv_atecc608b.h"
#include "drv_power.h"
#include "drv_tamper.h"
#include "drv_aw32001.h"
#include "drv_bpk.h"
#include "ctaes.h"
#include "hash_and_salt.h"
#include "sha512.h"
#include "keystore.h"
#include "gui_framework.h"
#include "secret_cache.h"
#include "anti_tamper.h"
#include "account_manager.h"
#ifndef EXCLUDE_RUSTC
#include "librust_c.h"
#endif
#include "log.h"
#include "background_task.h"
#include "account_public_info.h"
#include "version.h"
#include "drv_motor.h"
#include "hal_lcd.h"
#include "low_power.h"
#include "fingerprint_process.h"
#include "slip39.h"
#include "user_sqlite3.h"
#include "protocol_codec.h"
#include "presetting.h"
#include "usb_task.h"
#include "device_setting.h"

#define CMD_MAX_ARGC 16

typedef void (*UartTestCmdFunc_t)(int argc, char *argv[]);

typedef struct
{
    const char *cmdString;  // Compare string for testing
    UartTestCmdFunc_t func; // add the function for testing
} UartTestCmdItem_t;

static void TestFunc(int argc, char *argv[]);
static void AllTaskInfoFunc(int argc, char *argv[]);
static void ClrCpuPercentFunc(int argc, char *argv[]);
static void GetChipTempFunc(int argc, char *argv[]);
static void GetClockInfoFunc(int argc, char *argv[]);
static void HeapInfoFunc(int argc, char *argv[]);
static void GetTickFunc(int argc, char *argv[]);
static void MemoryTestFunc(int argc, char *argv[]);
static void PsramTestFunc(int argc, char *argv[]);
static void TrngTestFunc(int argc, char *argv[]);
static void HardfaultFunc(int argc, char *argv[]);
static void TouchFunc(int argc, char *argv[]);
static void TouchReleaseFunc(int argc, char *argv[]);
static void QrDecodeStateFunc(int argc, char *argv[]);
static void Gd25FlashOperateFunc(int argc, char *argv[]);
static void Sha256TestFunc(int argc, char *argv[]);
static void Sha256HmacFunc(int argc, char *argv[]);
static void Ds28s60TestFunc(int argc, char *argv[]);
static void FatfsLsFunc(int argc, char *argv[]);
static void FatfsCatFunc(int argc, char *argv[]);
static void FatfsFileMd5Func(int argc, char *argv[]);
static void FatfsFileSha256Func(int argc, char *argv[]);
static void FatfsFileWriteFunc(int argc, char *argv[]);
static void FatfsFileDeleteFunc(int argc, char *argv[]);
static void FatfsFileCopyFunc(int argc, char *argv[]);
// static void ParamReadFunc(int argc, char *argv[]);
// static void ParamWriteFunc(int argc, char *argv[]);
static void ReadAddrFunc(int argc, char *argv[]);
static void GetCurrentTimeFunc(int argc, char *argv[]);
static void SetCurrentTimeFunc(int argc, char *argv[]);
static void GetMnemonicTestFunc(int argc, char *argv[]);
static void BatteryTestFunc(int argc, char *argv[]);
static void TouchpadTestFunc(int argc, char *argv[]);
static void SetBrightFunc(int argc, char *argv[]);
static void CryptoAuthLibTestFunc(int argc, char *argv[]);
static void PowerCtrlFunc(int argc, char *argv[]);
static void TampTestFunc(int argc, char *argv[]);
static void Aw32001TestFunc(int argc, char *argv[]);
static void AesEncryptTestFunc(int argc, char *argv[]);
static void HashAndSaltFunc(int argc, char *argv[]);
static void Sha512Func(int argc, char *argv[]);
static void KeyStoreTestFunc(int argc, char *argv[]);
static void RustGetMasterFingerprint(int argc, char *argv[]);
static void GuiFrameDebugTestFunc(int argc, char *argv[]);
static void RustTestParseCryptoPSBT(int argc, char *argv[]);
static void RustTestParseBTCKeystone(int argc, char *argv[]);
static void RustTestCheckFailedBTCKeystone(int argc, char *argv[]);
static void RustTestCheckSucceedBCHKeystone(int argc, char *argv[]);
static void RustTestParseDASHKeystone(int argc, char *argv[]);
static void RustTestParseBCHKeystone(int argc, char *argv[]);
static void RustTestParseLTCKeystone(int argc, char *argv[]);
static void RustTestParseTronKeystone(int argc, char *argv[]);
static void RustTestCheckTronKeystoneSucceed(int argc, char *argv[]);
static void RustTestCheckTronKeystoneFailed(int argc, char *argv[]);
static void RustTestSignTronKeystone(int argc, char *argv[]);
static void RustTestEncodeCryptoPSBT(int argc, char *argv[]);
static void RustTestDecodeCryptoPSBT(int argc, char *argv[]);
static void RustTestDecodeMultiCryptoPSBT(int argc, char *argv[]);
static void RustTestDecodeKeystoneBTCSignResult(int argc, char *argv[]);
static void RustTestDecodeKeystoneSignRequest(int argc, char *argv[]);
static void LogTestFunc(int argc, char *argv[]);
static void RustTestSignPSBT(int argc, char *argv[]);
static void RustTestSignBTCKeystone(int argc, char *argv[]);
static void RustTestSignLTCKeystone(int argc, char *argv[]);
static void RustTestSignDASHKeystone(int argc, char *argv[]);
static void RustTestSignBCHKeystone(int argc, char *argv[]);
static void BackgroundTestFunc(int argc, char *argv[]);
static void RustTestDecodeUr(int argc, char *argv[]);
static void RustGetConnectBlueWalletUR(int argc, char *argv[]);
static void RustGetConnectKeplrUR(int argc, char *argv[]);
static void RustGetConnectXrpToolKitUR(int argc, char *argv[]);
static void RustGetConnectMetaMaskUR(int argc, char *argv[]);
static void RustTestKeyDerivation(int argc, char *argv[]);
static void RustTestMemory(int argc, char *argv[]);
static void RustGetSyncCompanionAppUR(int argc, char *argv[]);
static void RustTestGetAddressLTCSucceed(int argc, char *argv[]);
static void RustTestGetAddressTronSucceed(int argc, char *argv[]);
static void RustTestGetAddressSolanaSucceed(int argc, char *argv[]);
static void RustTestGetAddressLTCFailed(int argc, char *argv[]);
static void RustParseEthPersonalMessage(int argc, char *argv[]);
static void RustParseEthContractData(int argc, char *argv[]);
static void GetReceiveAddress(int argc, char *argv[]);
static void testEthTx(int argc, char *argv[]);
static void testSolanaTx(int argc, char *argv[]);
static void testSolanaCheckTx(int argc, char *argv[]);
static void testSolanaParseTx(int argc, char *argv[]);
static void testNearParseTx(int argc, char *argv[]);
static void testXrpParseTx(int argc, char *argv[]);
static void testNearGetAddress(int argc, char *argv[]);
static void testCosmosGetAddress(int argc, char *argv[]);
static void testNearTx(int argc, char *argv[]);
static void testNearCheckTx(int argc, char *argv[]);
static void testCardanoTx(int argc, char *argv[]);
static void AccountPublicInfoTestFunc(int argc, char *argv[]);
static void ETHDBContractsTest(int argc, char *argv[]);
static void FingerTestFunc(int argc, char *argv[]);
static void MotorTestFunc(int argc, char *argv[]);
static void LcdTestFunc(int argc, char *argv[]);
static void LowPowerTestFunc(int argc, char *argv[]);
static void RustGetEthAddress(int argc, char *argv[]);
static void Slip39SliceWordTestFunc(int argc, char *argv[]);
static void Sqlite3TestFunc(int argc, char *argv[]);
static void testXRPGetAddress(int argc, char *argv[]);
static void CrcTestFunc(int argc, char *argv[]);
static void ProtocolCodeTestFunc(int argc, char *argv[]);
static void testXRPSignTx(int argc, char *argv[]);
static void RustTestCosmosSignTx(int argc, char *argv[]);
static void RustTestCosmosParseTx(int argc, char *argv[]);
static void RustGetConnectSolanaWalletUR(int argc, char *argv[]);
static void RustGetConnectAptosWalletUR(int argc, char *argv[]);
static void RustSolanaMessage(int argc, char *argv[]);
static void RustTestCosmosCheckTx(int argc, char *argv[]);
static void RustTestCosmosEvmSignTx(int argc, char *argv[]);
static void RustTestK1SignMeessageByKey(int argc, char *argv[]);
static void RustTestK1VerifySignature(int argc, char *argv[]);
static void testWebAuth(int argc, char *argv[]);
static void PresettingTestFunc(int argc, char *argv[]);
static void UsbTestFunc(int argc, char *argv[]);
static void DeviceSettingsTestFunc(int argc, char *argv[]);
static void RustTestParseAptosTx(int argc, char *argv[]);
static void ScreenShotFunc(int argc, char *argv[]);
static void FatfsCopyFunc(int argc, char *argv[]);
static void BpkPrintFunc(int argc, char *argv[]);
static void RustTestSuiParseTx(int argc, char *argv[]);
static void RustTestSuiCheckTx(int argc, char *argv[]);
static void RustTestSuiSignTx(int argc, char *argv[]);
static void RustTestAptosCheckTx(int argc, char *argv[]);
static void RustTestAptosParseTx(int argc, char *argv[]);
static void RustADATest(int argc, char *argv[]);

const static UartTestCmdItem_t g_uartTestCmdTable[] =
{
    {"test", TestFunc},
    {"all task info", AllTaskInfoFunc},
    {"clr cpu per", ClrCpuPercentFunc},
    {"get chip temp", GetChipTempFunc},
    {"get clock info", GetClockInfoFunc},
    {"heap info", HeapInfoFunc},
    {"get tick", GetTickFunc},
    {"memory test:", MemoryTestFunc},
    {"psram test:", PsramTestFunc},
    {"trng test:", TrngTestFunc},
    {"hardfault:", HardfaultFunc},
    {"touch:", TouchFunc},
    {"touch release", TouchReleaseFunc},
    {"qr decode state:", QrDecodeStateFunc},
    {"spi flash:", Gd25FlashOperateFunc},
    {"sha256 test", Sha256TestFunc},
    {"sha256 hmac:", Sha256HmacFunc},
    {"ds28s60 test:", Ds28s60TestFunc},
    {"ls:", FatfsLsFunc},
    {"cat:", FatfsCatFunc},
    {"write:", FatfsFileWriteFunc},
    {"rm:", FatfsFileDeleteFunc},
    {"md5:", FatfsFileMd5Func},
    {"sha256:", FatfsFileSha256Func},
    {"copy:", FatfsFileCopyFunc},
    {"copy ota", FatfsCopyFunc},
    // {"param read:", ParamReadFunc},
    // {"param write:", ParamWriteFunc},
    {"read addr:", ReadAddrFunc},
    {"get time", GetCurrentTimeFunc},
    {"set time:", SetCurrentTimeFunc},
    {"get mnemonic test:", GetMnemonicTestFunc},
    {"battery test:", BatteryTestFunc},
    {"touch pad test:", TouchpadTestFunc},
    {"set bright:", SetBrightFunc},
    {"cryptoauthlib:", CryptoAuthLibTestFunc},
    {"power ctrl:", PowerCtrlFunc},
    {"tamper test:", TampTestFunc},
    {"aw32001 test:", Aw32001TestFunc},
    {"aes test", AesEncryptTestFunc},
    {"hash and salt:", HashAndSaltFunc},
    {"sha512:", Sha512Func},
    {"key store test:", KeyStoreTestFunc},
    {"gui debug", GuiFrameDebugTestFunc},
#ifndef EXCLUDE_RUSTC
    {"rust get mfp:", RustGetMasterFingerprint},
    {"rust test parse psbt", RustTestParseCryptoPSBT},
    {"rust test parse bitcoin keystone", RustTestParseBTCKeystone},
    {"rust test check bitcoin keystone failed", RustTestCheckFailedBTCKeystone},
    {"rust test check bch succeed", RustTestCheckSucceedBCHKeystone},
    {"rust test parse ltc", RustTestParseLTCKeystone},
    {"rust test parse tron keystone", RustTestParseTronKeystone},
    {"rust test check tron keystone succeed:", RustTestCheckTronKeystoneSucceed},
    {"rust test check tron keystone failed", RustTestCheckTronKeystoneFailed},
    {"rust test sign tron keystone:", RustTestSignTronKeystone},
    {"rust test parse dash", RustTestParseDASHKeystone},
    {"rust test parse bch", RustTestParseBCHKeystone},
    {"rust test encode psbt", RustTestEncodeCryptoPSBT},
    {"rust test decode psbt", RustTestDecodeCryptoPSBT},
    {"rust test decode keystone btc sign result", RustTestDecodeKeystoneBTCSignResult},
    {"rust test decode keystone sign request", RustTestDecodeKeystoneSignRequest},
    {"rust test decode multi psbt", RustTestDecodeMultiCryptoPSBT},
    {"rust test sign psbt:", RustTestSignPSBT},
    {"rust test sign btc keystone:", RustTestSignBTCKeystone},
    {"rust test sign ltc:", RustTestSignLTCKeystone},
    {"rust test sign dash:", RustTestSignDASHKeystone},
    {"rust test sign bch:", RustTestSignBCHKeystone},
    {"rust test decode ur", RustTestDecodeUr},
    {"rust test get connect blue wallet ur", RustGetConnectBlueWalletUR},
    {"rust test get connect keplr wallet ur", RustGetConnectKeplrUR},
    {"rust test get connect xrp toolkit ur", RustGetConnectXrpToolKitUR},
    {"rust test derivation:", RustTestKeyDerivation},
    {"rust test memory", RustTestMemory},
    {"rust test get connect companion app ur", RustGetSyncCompanionAppUR},
    {"rust test connect metamask", RustGetConnectMetaMaskUR},
    {"rust test get ltc address succeed", RustTestGetAddressLTCSucceed},
    {"rust test get tron address succeed", RustTestGetAddressTronSucceed},
    {"rust test get solana address succeed:", RustTestGetAddressSolanaSucceed},
    {"rust test get ltc address failed", RustTestGetAddressLTCFailed},
    {"rust test eth tx:", testEthTx},
    {"rust test solana tx:", testSolanaTx},
    {"rust test solana check", testSolanaCheckTx},
    {"rust test solana parse:", testSolanaParseTx},
    {"rust test near parse:", testNearParseTx},
    {"rust test xrp parse:", testXrpParseTx},
    {"rust test near get address:", testNearGetAddress},
    {"rust test cosmos get address:", testCosmosGetAddress},
    {"rust test near tx:", testNearTx},
    {"rust test near check:", testNearCheckTx},
    {"rust test cardano tx:", testCardanoTx},
    {"rust test get eth address", RustGetEthAddress},
    {"rust test parse eth personal message:", RustParseEthPersonalMessage},
    {"rust test parse eth contract data", RustParseEthContractData},
    {"rust test xrp get address:", testXRPGetAddress},
    {"rust test xrp sign:", testXRPSignTx},
    {"rust test cosmos check:", RustTestCosmosCheckTx},
    {"rust test cosmos sign:", RustTestCosmosSignTx},
    {"rust test cosmos parse:", RustTestCosmosParseTx},
    {"rust test solana sync", RustGetConnectSolanaWalletUR},
    {"rust test aptos sync", RustGetConnectAptosWalletUR},
    {"rust test solana sign message:", RustSolanaMessage},
    {"rust test cosmos evm sign:", RustTestCosmosEvmSignTx},
    {"rust test sign message by private key:", RustTestK1SignMeessageByKey},
    {"rust test verify signature:", RustTestK1VerifySignature},
    {"rust test web auth", testWebAuth},
    {"rust get mfp:",           RustGetMasterFingerprint},
    {"rust test parse psbt",    RustTestParseCryptoPSBT},
    {"rust test parse bitcoin keystone",           RustTestParseBTCKeystone        },
    {"rust test check bitcoin keystone failed",    RustTestCheckFailedBTCKeystone  },
    {"rust test check bch succeed",                     RustTestCheckSucceedBCHKeystone },
    {"rust test parse ltc",                             RustTestParseLTCKeystone        },
    {"rust test parse tron keystone",              RustTestParseTronKeystone       },
    {"rust test check tron keystone succeed:",     RustTestCheckTronKeystoneSucceed},
    {"rust test check tron keystone failed",       RustTestCheckTronKeystoneFailed },
    {"rust test sign tron keystone:",              RustTestSignTronKeystone        },
    {"rust test parse dash",                            RustTestParseDASHKeystone       },
    {"rust test parse bch",                             RustTestParseBCHKeystone        },
    {"rust test encode psbt",                           RustTestEncodeCryptoPSBT            },
    {"rust test decode psbt",                           RustTestDecodeCryptoPSBT            },
    {"rust test decode keystone btc sign result",  RustTestDecodeKeystoneBTCSignResult },
    {"rust test decode keystone sign request",     RustTestDecodeKeystoneSignRequest   },
    {"rust test decode multi psbt",                     RustTestDecodeMultiCryptoPSBT       },
    {"rust test sign psbt:",                            RustTestSignPSBT                    },
    {"rust test sign btc keystone:",               RustTestSignBTCKeystone         },
    {"rust test sign ltc:",                             RustTestSignLTCKeystone         },
    {"rust test sign dash:",                            RustTestSignDASHKeystone        },
    {"rust test sign bch:",                             RustTestSignBCHKeystone         },
    {"rust test decode ur",                             RustTestDecodeUr                    },
    {"rust test get connect blue wallet ur",            RustGetConnectBlueWalletUR          },
    {"rust test derivation:",                           RustTestKeyDerivation               },
    {"rust test memory",                                RustTestMemory                      },
    {"rust test get connect companion app ur",          RustGetSyncCompanionAppUR           },
    {"rust test get ltc address succeed",               RustTestGetAddressLTCSucceed        },
    {"rust test get tron address succeed",              RustTestGetAddressTronSucceed       },
    {"rust test get solana address succeed:",           RustTestGetAddressSolanaSucceed     },
    {"rust test get ltc address failed",                RustTestGetAddressLTCFailed         },
    {"rust test eth tx:",                               testEthTx                           },
    {"rust test solana tx:",                            testSolanaTx                        },
    {"rust test solana check",                          testSolanaCheckTx                   },
    {"rust test cardano tx:",                           testCardanoTx                       },
    {"rust test get eth address",                       RustGetEthAddress                   },
    {"rust test parse aptos tx",                        RustTestParseAptosTx                },
    {"rust test sui check tx:",                         RustTestSuiCheckTx                  },
    {"rust test sui parse tx:",                         RustTestSuiParseTx                  },
    {"rust test sui sign tx:",                          RustTestSuiSignTx                   },
    {"rust test aptos check tx:",                       RustTestAptosCheckTx                },
    {"rust test aptos parse tx:",                       RustTestAptosParseTx                },
    {"rust ada test",                                   RustADATest                         },

#endif
    {"log test:", LogTestFunc},
    {"eth db contract test:", ETHDBContractsTest},
    {"background test:", BackgroundTestFunc},
    {"get receive address:", GetReceiveAddress},
    {"account public info test:", AccountPublicInfoTestFunc},
    {"finger test:", FingerTestFunc},
    {"motor:", MotorTestFunc},
    {"lcd:", LcdTestFunc},
    {"low power:", LowPowerTestFunc},
    {"slip39 test", Slip39SliceWordTestFunc},
    {"sqlite test:", Sqlite3TestFunc},
    {"crc:", CrcTestFunc},
    {"protocol codec:", ProtocolCodeTestFunc},
    {"presetting:", PresettingTestFunc},
    {"usb test:", UsbTestFunc},
    {"device settings test:", DeviceSettingsTestFunc},
    {"screen shot", ScreenShotFunc},
    {"bpk print:", BpkPrintFunc},
};

bool CompareAndRunTestCmd(const char *inputString)
{
    uint32_t tableSize, compareLen, argc, argvLen;
    char *inputHead, *argvHead, *argv[CMD_MAX_ARGC];
    bool content = false;

    tableSize = sizeof(g_uartTestCmdTable) / sizeof(g_uartTestCmdTable[0]);
    for (uint32_t i = 0; i < tableSize; i++)
    {
        inputHead = strstr(g_uartTestCmdTable[i].cmdString, ":");
        if (inputHead != NULL)
        {
            // partial compare
            inputHead++;
            compareLen = inputHead - g_uartTestCmdTable[i].cmdString;
            if (strncmp(inputString, g_uartTestCmdTable[i].cmdString, compareLen) == 0)
            {
                inputHead = (char *)inputString + compareLen;
                argc = 0;
                for (uint32_t j = 0;; j++)
                {
                    if (content)
                    {
                        // content mode, finding space.
                        if (inputHead[j] == ' ' || inputHead[j] == 0)
                        {
                            argvLen = &inputHead[j] - argvHead + 1; // including zero tail.
                            argv[argc] = SRAM_MALLOC(argvLen);
                            strncpy(argv[argc], argvHead, argvLen - 1);
                            argv[argc][argvLen - 1] = 0;
                            argc++;
                            content = false;
                        }
                    }
                    else
                    {
                        // space mode, finding content.
                        if (inputHead[j] != ' ')
                        {
                            argvHead = &inputHead[j];
                            content = true;
                        }
                    }
                    if (inputHead[j] == 0)
                    {
                        break;
                    }
                }
                g_uartTestCmdTable[i].func(argc, argv);
                for (uint32_t j = 0; j < argc; j++)
                {
                    SRAM_FREE(argv[j]);
                }
                return true;
            }
        }
        else
        {
            // full compare
            if (strcmp(inputString, g_uartTestCmdTable[i].cmdString) == 0)
            {
                g_uartTestCmdTable[i].func(0, NULL);
                return true;
            }
        }
    }
    return false;
}

static void TestFunc(int argc, char *argv[])
{
    printf("test!!\r\n");
}

static void AllTaskInfoFunc(int argc, char *argv[])
{
    PrintTasksStatus();
}

static void ClrCpuPercentFunc(int argc, char *argv[])
{
    printf("clear cpu percent to 0\r\n");
    ClrRunTimeStats();
}

static void GetChipTempFunc(int argc, char *argv[])
{
    ;
}

static void GetClockInfoFunc(int argc, char *argv[])
{
    // printf("HAL_RCC_GetSysClockFreq()=%d\r\n", HAL_RCC_GetSysClockFreq());
    // printf("HAL_RCC_GetHCLKFreq()=%d\r\n", HAL_RCC_GetHCLKFreq());
    // printf("HAL_RCC_GetPCLK1Freq()=%d\r\n", HAL_RCC_GetPCLK1Freq());
    // printf("HAL_RCC_GetPCLK2Freq()=%d\r\n", HAL_RCC_GetPCLK2Freq());
}

static void HeapInfoFunc(int argc, char *argv[])
{
    PrintHeapInfo();
}

static void GetTickFunc(int argc, char *argv[])
{
    uint32_t osTick = osKernelGetTickCount();
    uint32_t remainingSec = osTick / 1000;
    uint32_t day, hour, min, sec;
    day = remainingSec / 86400;
    remainingSec %= 86400;
    hour = remainingSec / 3600;
    remainingSec %= 3600;
    min = remainingSec / 60;
    remainingSec %= 60;
    sec = remainingSec;
    printf("os tick=%d\r\n", osTick);
    printf("sys start for %ddays %dhours %dminute %dsecond\r\n", day, hour, min, sec);
}

static void MemoryTestFunc(int argc, char *argv[])
{
    int32_t byteNum, times;
    uint32_t startTick, endTick;
    uint8_t *tempAddr;
    uint8_t *sramAddr, *sramAddrSource;
    uint8_t *psramAddr, *psramAddrSource;
    uint32_t i;

    VALUE_CHECK(argc, 2);
    sscanf(argv[0], "%d", &byteNum);
    sscanf(argv[1], "%d", &times);

    printf("Prepare running %d bytes * %d times memory test\r\n", byteNum, times);
    sramAddr = SRAM_MALLOC(byteNum);
    if (sramAddr == NULL)
    {
        printf("malloc err\r\n");
        return;
    }
    sramAddrSource = SRAM_MALLOC(byteNum);
    if (sramAddrSource == NULL)
    {
        printf("malloc2 err\r\n");
        return;
    }

    psramAddr = EXT_MALLOC(byteNum);
    if (psramAddr == NULL)
    {
        printf("malloc3 err\r\n");
        return;
    }
    psramAddrSource = EXT_MALLOC(byteNum);
    if (psramAddrSource == NULL)
    {
        printf("malloc4 err\r\n");
        return;
    }

    printf("\r\nstart SRAM to SRAM test:\r\n");
    startTick = osKernelGetTickCount();
    for (i = 0; i < times; i++)
    {
        memcpy(sramAddr, sramAddrSource, byteNum);
    }
    endTick = osKernelGetTickCount();
    printf("used tick: %d ms, %d bytes/s\r\n", endTick - startTick, (uint32_t)(((uint64_t)byteNum * 1000 * times) / (endTick - startTick)));

    printf("\r\nstart QSPI FLASH to SRAM test:\r\n");
    tempAddr = (uint8_t *)(0x01010000);
    startTick = osKernelGetTickCount();
    for (i = 0; i < times; i++)
    {
        memcpy(sramAddr, tempAddr, byteNum);
    }
    endTick = osKernelGetTickCount();
    printf("used tick: %d ms, %d bytes/s\r\n", endTick - startTick, (uint32_t)(((uint64_t)byteNum * 1000 * times) / (endTick - startTick)));

    printf("\r\nstart QSPI FLASH to PSRAM test:\r\n");
    tempAddr = (uint8_t *)(0x01010000);
    startTick = osKernelGetTickCount();
    for (i = 0; i < times; i++)
    {
        memcpy(psramAddr, tempAddr, byteNum);
    }
    endTick = osKernelGetTickCount();
    printf("used tick: %d ms, %d bytes/s\r\n", endTick - startTick, (uint32_t)(((uint64_t)byteNum * 1000 * times) / (endTick - startTick)));

    printf("\r\nstart SRAM to PSRAM test:\r\n");
    startTick = osKernelGetTickCount();
    for (i = 0; i < times; i++)
    {
        memcpy(psramAddr, sramAddrSource, byteNum);
    }
    endTick = osKernelGetTickCount();
    printf("used tick: %d ms, %d bytes/s\r\n", endTick - startTick, (uint32_t)(((uint64_t)byteNum * 1000 * times) / (endTick - startTick)));

    printf("\r\nstart PSRAM to SRAM test:\r\n");
    startTick = osKernelGetTickCount();
    for (i = 0; i < times; i++)
    {
        memcpy(sramAddr, psramAddrSource, byteNum);
    }
    endTick = osKernelGetTickCount();
    printf("used tick: %d ms, %d bytes/s\r\n", endTick - startTick, (uint32_t)(((uint64_t)byteNum * 1000 * times) / (endTick - startTick)));

    printf("\r\nstart PSRAM to PSRAM test:\r\n");
    startTick = osKernelGetTickCount();
    for (i = 0; i < times; i++)
    {
        memcpy(psramAddr, psramAddrSource, byteNum);
    }
    endTick = osKernelGetTickCount();
    printf("used tick: %d ms, %d bytes/s\r\n", endTick - startTick, (uint32_t)(((uint64_t)byteNum * 1000 * times) / (endTick - startTick)));

    SRAM_FREE(sramAddr);
    SRAM_FREE(sramAddrSource);
}

static void PsramTestFunc(int argc, char *argv[])
{
    int32_t byteNum, i;
    uint8_t *mem, *memSram;
    VALUE_CHECK(argc, 1);
    sscanf(argv[0], "%d", &byteNum);
    printf("EXT_MALLOC %d bytes\r\n", byteNum);
    mem = EXT_MALLOC(byteNum);
    printf("addr=0x%08X\r\n", (uint32_t)mem);
    memSram = SRAM_MALLOC(byteNum);
    TrngGet(memSram, byteNum);
    memcpy(mem, memSram, byteNum);
    PrintArray("mem", mem, byteNum);
    for (i = 0; i < byteNum; i++)
    {
        if (mem[i] != memSram[i])
        {
            break;
        }
    }
    if (i == byteNum)
    {
        printf("PSRAM test succ\r\n");
    }
    else
    {
        printf("PSRAM test fault, i=%d\r\n", i);
    }
}

static void TrngTestFunc(int argc, char *argv[])
{
    int32_t byteNum;
    uint8_t *mem;
    VALUE_CHECK(argc, 1);
    sscanf(argv[0], "%d", &byteNum);
    mem = SRAM_MALLOC(byteNum);
    if (mem == NULL)
    {
        printf("malloc err\r\n");
        return;
    }
    memset(mem, 0, byteNum);
    TrngGet(mem, byteNum);
    PrintArray("trng", mem, byteNum);
    SRAM_FREE(mem);
}

static void HardfaultFunc(int argc, char *argv[])
{
    uint32_t type;
    uint32_t *errAddr;
    VALUE_CHECK(argc, 1);
    sscanf(argv[0], "%d", &type);
    printf("going to hardfault,type=%d\r\n", type);
    switch (type)
    {
    case 0:
    {
        errAddr = (uint32_t *)0x1001000;
        *errAddr = 0x12345678;
        printf("*errAddr=0x%08X\r\n", *errAddr);
    }
    break;
    default:
        break;
    }
}

static void TouchFunc(int argc, char *argv[])
{
    int32_t x, y;
    VALUE_CHECK(argc, 2);
    sscanf(argv[0], "%d", &x);
    sscanf(argv[1], "%d", &y);
    SetVirtualTouchCoord(x, y);
    SetVirtualTouchState(true);
}

static void TouchReleaseFunc(int argc, char *argv[])
{
    SetVirtualTouchState(false);
}

static void QrDecodeStateFunc(int argc, char *argv[])
{
    int32_t state;
    VALUE_CHECK(argc, 1);
    sscanf(argv[0], "%d", &state);
    printf("state=%d\r\n", state);
    if (state == 1)
    {
        StartQrDecode();
    }
    else if (state == 0)
    {
        StopQrDecode();
    }
}

static void Gd25FlashOperateFunc(int argc, char *argv[])
{
    uint32_t opearte;
    uint32_t addr = 0;
    uint32_t size;
    char *readBuf;

    VALUE_CHECK(argc, 3);
    sscanf(argv[0], "%d", &opearte);
    sscanf(argv[1], "%x", &addr);

    // tempBuf[strlen(tempBuf) - 4]  = '\0';
    switch (opearte)
    {
    case GD25_FLASH_ERASE:
        if (SUCCESS_CODE == Gd25FlashSectorErase(addr))
        {
            printf("erase %#x success\r\n", addr);
        }
        break;
    case GD25_FLASH_READ:
        size = atoi(argv[2]);
        printf("size=%d\r\n", size);
        readBuf = SRAM_MALLOC(size);
        if (readBuf == NULL)
        {
            printf("malloc err\r\n");
            return;
        }
        if (size == Gd25FlashReadBuffer(addr, (uint8_t *)readBuf, size))
        {
            printf("read %#x success\r\n", addr);
        }
        PrintArray("data", (uint8_t *)readBuf, size);
        SRAM_FREE(readBuf);
        break;
    case GD25_FLASH_WRITE:
        Gd25FlashSectorErase(addr);
        size = strlen(argv[2]);
        if (size == Gd25FlashWriteBuffer(addr, (uint8_t *)argv[2], size))
        {
            printf("write %#x success\r\n", addr);
        }
        break;
    default:
        break;
    }
}

static void Sha256TestFunc(int argc, char *argv[])
{
    uint8_t privateKey[] =
    {
        0x29, 0x71, 0x7E, 0x8C, 0xB4, 0x55, 0xA4, 0x46,
        0xA4, 0x35, 0x16, 0x86, 0x3A, 0x43, 0xD4, 0x57,
        0xD0, 0x91, 0x58, 0x8C, 0xE3, 0xD0, 0x31, 0xA5,
        0x9B, 0x05, 0x7E, 0x90, 0xA8, 0x9B, 0x16, 0x1C
    };
    uint8_t msg[] =
    {
        0x0F, 0x5C, 0x83, 0xC6, 0x76, 0xE0, 0x88, 0x63,
        0x55, 0x2E, 0xD9, 0xA7, 0x01, 0x01, 0x1C, 0x83,
        0x18, 0x00, 0x00, 0x4B
    };
    uint8_t hmac[32] = {0};
    uint8_t encryptData[] =
    {
        0x5C, 0x24, 0x23, 0xB6, 0x03, 0x78, 0xD0, 0xF4,
        0x88, 0x5D, 0xB3, 0xC9, 0x27, 0xEF, 0x39, 0x40,
        0x8F, 0x8B, 0x96, 0xED, 0x0F, 0x7E, 0x34, 0x14,
        0x51, 0x1F, 0xE6, 0xAA, 0xE5, 0x95, 0xBF, 0xF6
    };
    uint8_t decryptData[32];
    printf("sha256 test!\r\n");
    hmac_sha256(privateKey, 32, msg, sizeof(msg), hmac);
    PrintArray("mac", hmac, 32);
    for (uint32_t i = 0; i < 32; i++)
    {
        decryptData[i] = encryptData[i] ^ hmac[i];
    }
    PrintArray("decryptData", decryptData, 32);
    //
}

static void Sha256HmacFunc(int argc, char *argv[])
{
    // example:
    // #sha256 hmac:3b015f84131b10db4352c31fe60a371e58fb639a64c313f22d65e9b4c5913e15 552ed9a701011c836e0752cbeb52df38b9fe057667aea507f2ef4acb6a1e929af7aeb0920f25c3af1fe9e0113f6cf08208c53f9b9e857ae6887de143d970051c87837b34ed032e621400003c\r\n
    uint32_t keyLen, msgLen;
    uint8_t *privateKey, *msg;
    uint8_t hmac[32] = {0};

    VALUE_CHECK(argc, 2);

    privateKey = SRAM_MALLOC(strlen(argv[0]) / 2 + 1);
    msg = SRAM_MALLOC(strlen(argv[1]) / 2 + 1);
    keyLen = StrToHex(privateKey, argv[0]);
    msgLen = StrToHex(msg, argv[1]);
    PrintArray("privateKey", privateKey, keyLen);
    PrintArray("msg", msg, msgLen);

    hmac_sha256(privateKey, keyLen, msg, msgLen, hmac);
    PrintArray("hmac", hmac, 32);

    SRAM_FREE(privateKey);
    SRAM_FREE(msg);
}

static void Ds28s60TestFunc(int argc, char *argv[])
{
    DS28S60_Test(argc, argv);
}

static void FatfsLsFunc(int argc, char *argv[])
{
    VALUE_CHECK(argc, 1);
    FatfsDirectoryListing(argv[0]);
}

static void FatfsCopyFunc(int argc, char *argv[])
{
    CopyToFlash();
}

static void FatfsCatFunc(int argc, char *argv[])
{
    VALUE_CHECK(argc, 1);
    FatfsCatFile(argv[0]);
}

static void FatfsFileMd5Func(int argc, char *argv[])
{
    VALUE_CHECK(argc, 1);
    FatfsFileMd5(argv[0]);
}

static void FatfsFileSha256Func(int argc, char *argv[])
{
    VALUE_CHECK(argc, 1);
    uint8_t sha256[32];
    FatfsFileSha256(argv[0], sha256);
}

static void FatfsFileWriteFunc(int argc, char *argv[])
{
    VALUE_CHECK(argc, 2);
    FatfsFileWrite(argv[0], (const uint8_t *)argv[1], strlen(argv[1]));
}

static void FatfsFileDeleteFunc(int argc, char *argv[])
{
    VALUE_CHECK(argc, 1);
    FatfsFileDelete(argv[0]);
}

static void FatfsFileCopyFunc(int argc, char *argv[])
{
    VALUE_CHECK(argc, 2);
    FatfsFileCopy(argv[0], argv[1]);
}

static void BpkPrintFunc(int argc, char *argv[])
{
    VALUE_CHECK(argc, 1);
    PrintBpkValue(atoi(argv[0]));
}
// static void ParamReadFunc(int argc, char *argv[])
// {
//     uint32_t id;
//     VALUE_CHECK(argc, 1);
//     sscanf(argv[0], "%d", &id);
//     ParamPrintf(id);
// }

// static void ParamWriteFunc(int argc, char *argv[])
// {
//     uint32_t id;
//     VALUE_CHECK(argc, 2);
//     sscanf(argv[0], "%d", &id);
//     ParamWriteParamBuf(id, argv[1]);
// }

static void ReadAddrFunc(int argc, char *argv[])
{
    uint32_t addr, len;

    VALUE_CHECK(argc, 2);
    sscanf(argv[0], "%X", &addr);
    sscanf(argv[1], "%d", &len);
    printf("addr=0x%08X,len=%d\r\n", addr, len);
    PrintArray("read data", (uint8_t *)addr, len);
}

static void GetCurrentTimeFunc(int argc, char *argv[])
{
    printf("current time : %s\n", GetCurrentTime());
}

static void SetCurrentTimeFunc(int argc, char *argv[])
{
    uint32_t stampTime;

    VALUE_CHECK(argc, 1);
    sscanf(argv[0], "%d", &stampTime);
    SetCurrentStampTime(stampTime);
}

static void GetMnemonicTestFunc(int argc, char *argv[])
{
    int32_t entropyByte, ret;
    uint8_t *entropy, seed[64], entropyCalc[32];
    size_t entropySize;

    VALUE_CHECK(argc, 1);
    sscanf(argv[0], "%d", &entropyByte);
    entropy = SRAM_MALLOC(entropyByte);
    TrngGet(entropy, entropyByte);
    PrintArray("entropy", entropy, entropyByte);
    char *mnemonic = NULL;
    ret = bip39_mnemonic_from_bytes(NULL, entropy, entropyByte, &mnemonic);
    printf("bip39_mnemonic_from_bytes=%d\r\n", ret);
    if (mnemonic == NULL)
    {
        printf("Get mnemonic err.\r\n");
    }
    else
    {
        // PrintArray("mnemonics", mnemonic, strlen(mnemonic));
        printf("mnemonic:\r\n%s\r\n", mnemonic);
    }
    ret = bip39_mnemonic_to_bytes(NULL, mnemonic, entropyCalc, 32, &entropySize);
    printf("bip39_mnemonic_to_bytes=%d\r\n", ret);
    PrintArray("entropyCalc", entropyCalc, entropySize);
    ret = bip39_mnemonic_to_seed(mnemonic, NULL, seed, 64, NULL);
    PrintArray("seed", seed, 64);
    SRAM_FREE(entropy);
    SRAM_FREE(mnemonic);
}

static void BatteryTestFunc(int argc, char *argv[])
{
    BatteryTest(argc, argv);
}

static void TouchpadTestFunc(int argc, char *argv[])
{
    TouchPadTest(argc, argv);
}

static void SetBrightFunc(int argc, char *argv[])
{
    uint32_t bright;

    VALUE_CHECK(argc, 1);
    sscanf(argv[0], "%d", &bright);
    printf("bright=%d\r\n", bright);
    SetLcdBright(bright);
}

static void CryptoAuthLibTestFunc(int argc, char *argv[])
{
    Atecc608bTest(argc, argv);
}

static void PowerCtrlFunc(int argc, char *argv[])
{
    VALUE_CHECK(argc, 2);
    PowerTest(argc, argv);
}

static void TampTestFunc(int argc, char *argv[])
{
    TamperTest(argc, argv);
}

static void Aw32001TestFunc(int argc, char *argv[])
{
    Aw32001Test(argc, argv);
}

static void AesEncryptTestFunc(int argc, char *argv[])
{
    uint8_t key[32], iv[16], plain[32], encrpyted[32], decrypted[32];
    AES256_CBC_ctx ctx;

    TrngGet(key, sizeof(key));
    TrngGet(iv, sizeof(iv));
    TrngGet(plain, sizeof(plain));

    PrintArray("key", key, sizeof(key));
    PrintArray("iv", iv, sizeof(iv));
    PrintArray("plain", plain, sizeof(plain));

    AES256_CBC_init(&ctx, key, iv);

    AES256_CBC_encrypt(&ctx, 2, encrpyted, plain);
    PrintArray("encrpyted", encrpyted, sizeof(encrpyted));

    AES256_CBC_init(&ctx, key, iv);
    AES256_CBC_decrypt(&ctx, 2, decrypted, encrpyted);
    PrintArray("decrypted", decrypted, sizeof(decrypted));

    if (memcmp(plain, decrypted, 32) == 0)
    {
        printf("aes test succ\r\n");
    }
    else
    {
        printf("aes test err\r\n");
    }
}

static void HashAndSaltFunc(int argc, char *argv[])
{
    uint8_t hash[32], *inData;
    uint32_t inLen;

    VALUE_CHECK(argc, 2);
    inData = SRAM_MALLOC(strlen(argv[0]) / 2 + 1);
    inLen = StrToHex(inData, argv[0]);
    HashWithSalt(hash, inData, inLen, argv[1]);
    PrintArray("hash with salt", hash, 32);
}

static void Sha512Func(int argc, char *argv[])
{
    uint8_t hash[64], *inData;
    uint32_t inLen;

    VALUE_CHECK(argc, 1);
    inData = SRAM_MALLOC(strlen(argv[0]) / 2 + 1);
    inLen = StrToHex(inData, argv[0]);
    sha512((struct sha512 *)hash, inData, inLen);
    PrintArray("sha512 hash", hash, sizeof(hash));
}

static void KeyStoreTestFunc(int argc, char *argv[])
{
    KeyStoreTest(argc, argv);
}

static void GuiFrameDebugTestFunc(int argc, char *argv[])
{
    GuiFrameDebugging();
}

#ifndef EXCLUDE_RUSTC

static void RustTestK1SignMeessageByKey(int argc, char *argv[])
{
    uint8_t privateKey[] =
    {
        0xf2, 0x54, 0xb0, 0x30, 0xd0, 0x4c, 0xdd, 0x90,
        0x2e, 0x92, 0x19, 0xd8, 0x39, 0x0e, 0x1d, 0xeb,
        0x5a, 0x58, 0x5f, 0x3c, 0x25, 0xba, 0xcf, 0x5c,
        0x74, 0xbd, 0x07, 0x80, 0x3a, 0x8d, 0xd8, 0x73
    };

    uint8_t msg[] =
    {
        0x0D, 0x94, 0xD0, 0x45, 0xA7, 0xE0, 0xD4, 0x54,
        0x7E, 0x16, 0x1A, 0xC3, 0x60, 0xC7, 0x35, 0x81,
        0xA9, 0x53, 0x83, 0x43, 0x5A, 0x48, 0xD8, 0x86,
        0x9A, 0xB0, 0x8F, 0xF3, 0x4A, 0x8D, 0xB5, 0xE7
    };

    SimpleResponse_c_char *simpleResponse = k1_sign_message_hash_by_private_key(privateKey, msg);
    printf("get signature %s\r\n", simpleResponse->data);
    free_simple_response_c_char(simpleResponse);
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void RustTestK1VerifySignature(int argc, char *argv[])
{
    uint8_t msg[] =
    {
        0x0D, 0x94, 0xD0, 0x45, 0xA7, 0xE0, 0xD4, 0x54,
        0x7E, 0x16, 0x1A, 0xC3, 0x60, 0xC7, 0x35, 0x81,
        0xA9, 0x53, 0x83, 0x43, 0x5A, 0x48, 0xD8, 0x86,
        0x9A, 0xB0, 0x8F, 0xF3, 0x4A, 0x8D, 0xB5, 0xE7
    };

    uint8_t pubkey[] =
    {
        0x04, 0xb5, 0xc8, 0xd5, 0xf8, 0xde, 0xfd, 0x3b,
        0xe0, 0x89, 0xc0, 0xed, 0xda, 0xfc, 0xb2, 0x75,
        0xa5, 0xab, 0x15, 0xa4, 0xf7, 0x1b, 0xc4, 0x0f,
        0x86, 0x41, 0x56, 0x1e, 0x89, 0x7a, 0x85, 0xd7,
        0x50, 0x42, 0xc1, 0xa1, 0x68, 0xef, 0x84, 0xa1,
        0x47, 0x34, 0x36, 0x2f, 0xf7, 0x4b, 0x26, 0xf8,
        0xc4, 0xc1, 0x9f, 0x62, 0xb3, 0xa3, 0x6e, 0x3d,
        0xdd, 0x55, 0xfb, 0xae, 0xc2, 0x11, 0xd9, 0x68,
        0x02
    };

    uint8_t sig[] =
    {
        0x26, 0x93, 0x70, 0xa9, 0x76, 0xb8, 0x27, 0xaa,
        0xa6, 0x31, 0x6a, 0x01, 0xbc, 0xbb, 0x7e, 0x41,
        0xbe, 0xdf, 0xbc, 0x45, 0xba, 0xea, 0xe0, 0x7a,
        0x86, 0x8d, 0xe3, 0xc5, 0xd9, 0x4d, 0x6b, 0x04,
        0x09, 0xad, 0x26, 0x33, 0xc7, 0x86, 0x68, 0x13,
        0x16, 0xe5, 0x56, 0xa3, 0xba, 0x15, 0x59, 0x65,
        0xab, 0x50, 0x78, 0xe3, 0x53, 0x14, 0x34, 0xca,
        0x96, 0xa1, 0xf7, 0xa4, 0xb8, 0x60, 0x99, 0x70
    };

    bool result = k1_verify_signature(sig, msg, pubkey);
    printf("signature verification passed is: %s", result ? "true" : "false");
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void RustGetMasterFingerprint(int argc, char *argv[])
{
    printf("RustGetMasterFingerprint\r\n");
    int32_t index;
    VALUE_CHECK(argc, 2);
    sscanf(argv[0], "%d", &index);
    uint8_t seed[64] = {0};
    GetAccountSeed(index, seed, argv[1]);
    SimpleResponse_u8 *simpleResponse = get_master_fingerprint(seed, 64);
    uint8_t *master_fingerprint = simpleResponse->data;
    free_simple_response_u8(simpleResponse);
    PrintArray("get mfp", master_fingerprint, 4);
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}
static void RustTestParseCryptoPSBT(int argc, char *argv[])
{
    printf("RustTestParseCryptoPSBT\r\n");
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize()); // remaining heap size
    URParseResult *ur = test_get_crypto_psbt();
    void *crypto_psbt = ur->data;
    uint8_t mfp[4] = {0x70, 0x7e, 0xed, 0x6c};
    PtrT_CSliceFFI_ExtendedPublicKey public_keys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
    ExtendedPublicKey keys[3];
    public_keys->data = keys;
    public_keys->size = 3;
    keys[0].path = "m/44'/0'/0'";
    keys[0].xpub = "xpub6BhcvYN2qwQKRviMKKBTfRcK1RmCTmM7JHsg67r3rwvymhUEt8gPHhnkugQaQ7UN8M5FfhEUfyVuSaK5fQzfUpvAcCxN4bAT9jyySbPGsTs";
    keys[1].path = "m/49'/0'/0'";
    keys[1].xpub = "xpub6CkG15Jdw866GKs84e7ysjxAhBQUJBdLZTVbQERCjwh2z6wZSSdjfmaXaMvf6Vm5sbWemK43d7HJMicz41G3vEHA9Sa5N2J9j9vgwyiHdMj";
    keys[2].path = "m/84'/0'/0'";
    keys[2].xpub = "xpub6Bm9M1SxZdzL3TxdNV8897FgtTLBgehR1wVNnMyJ5VLRK5n3tFqXxrCVnVQj4zooN4eFSkf6Sma84reWc5ZCXMxPbLXQs3BcaBdTd4YQa3B";
    TransactionParseResult_DisplayTx *result = btc_parse_psbt(crypto_psbt, mfp, sizeof(mfp), public_keys);
    VecFFI_DisplayTxDetailInput *inputs = result->data->detail->from;
    VecFFI_DisplayTxOverviewOutput *overview_to = result->data->overview->to;
    for (size_t i = 0; i < result->data->overview->from->size; i++)
    {
        printf("result: overview output #%d\r\n", i);
        printf("result: overview output address %s\r\n", overview_to->data[i].address);
    }
    for (size_t i = 0; i < result->data->detail->to->size; i++)
    {
        printf("result: input #%d\r\n", i);
        printf("result: input address %s\r\n", inputs->data[i].address);
        printf("result: input amount %s\r\n", inputs->data[i].amount);
        printf("result: input path %s\r\n", inputs->data[i].path);
    }
    printf("result: fee value %s\r\n", result->data->detail->fee_amount);
    printf("result: input length %d\r\n", result->data->detail->from->size);
    free_ur_parse_result(ur);
    free_TransactionParseResult_DisplayTx(result);
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void RustTestParseBTCKeystone(int argc, char *argv[])
{
    printf("RustTestParseBTCKeystone 11\r\n");
    URParseResult *ur = test_get_btc_keystone_bytes();
    void *crypto_bytes = ur->data;
    char *xpub = "xpub6BosfCnifzxcFwrSzQiqu2DBVTshkCXacvNsWGYJVVhhawA7d4R5WSWGFNbi8Aw6ZRc1brxMyWMzG3DSSSSoekkudhUd9yLb6qx39T9nMdj";
    uint8_t mfp[4] = {0x70, 0x7e, 0xed, 0x6c};
    printf("RustTestParseBTCKeystone crypto_bytes %p\r\n", crypto_bytes);
    TransactionParseResult_DisplayTx *result = utxo_parse_keystone(crypto_bytes, Bytes, mfp, sizeof(mfp), xpub);
    printf("RustTestParseBTCKeystone 22\r\n");
    VecFFI_DisplayTxDetailInput *inputs = result->data->detail->from;
    VecFFI_DisplayTxOverviewOutput *overview_to = result->data->overview->to;
    for (size_t i = 0; i < result->data->overview->to->size; i++)
    {
        printf("result: overview output #%d\r\n", i);
        printf("result: overview output address %s\r\n", overview_to->data[i].address);
    }
    for (size_t i = 0; i < result->data->detail->from->size; i++)
    {
        printf("result: input #%d\r\n", i);
        printf("result: input address %s\r\n", inputs->data[i].address);
        printf("result: input amount %s\r\n", inputs->data[i].amount);
        printf("result: input path %s\r\n", inputs->data[i].path);
    }
    printf("result: fee value %s\r\n", result->data->detail->fee_amount);
    printf("result: input length %d\r\n", result->data->detail->from->size);
    free_ur_parse_result(ur);
    free_TransactionParseResult_DisplayTx(result);
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void RustTestCheckFailedBTCKeystone(int argc, char *argv[])
{
    printf("RustTestCheckFailedBTCKeystone 11\r\n");
    URParseResult *ur = test_get_btc_keystone_bytes();
    void *crypto_bytes = ur->data;
    char *xpub = "xpub6BosfCnifzxcFwrSzQiqu2DBVTshkCXacvNsWGYJVVhhawA7d4R5WSWGFNbi8Aw6ZRc1brxMyWMzG3DSSSSoekkudhUd9yLb6qx39T9nMdj";
    uint8_t mfp[4] = {0x70, 0x7e, 0xed, 0x6c};
    printf("RustTestCheckFailedBTCKeystone crypto_bytes %p\r\n", crypto_bytes);
    TransactionCheckResult *result = utxo_check_keystone(crypto_bytes, Bytes, mfp, sizeof(mfp), xpub);
    printf("error_code: %d\r\n", result->error_code);
    printf("error_message, %s\r\n", result->error_message);
    free_ur_parse_result(ur);
    free_TransactionCheckResult(result);
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void RustTestCheckSucceedBCHKeystone(int argc, char *argv[])
{
    printf("RustTestCheckSucceedBCHKeystone 11\r\n");
    URParseResult *ur = test_get_bch_keystone_succeed_bytes();
    void *crypto_bytes = ur->data;
    char *xpub = "xpub6ByHsPNSQXTWZ7PLESMY2FufyYWtLXagSUpMQq7Un96SiThZH2iJB1X7pwviH1WtKVeDP6K8d6xxFzzoaFzF3s8BKCZx8oEDdDkNnp4owAZ";
    uint8_t mfp[4] = {0x70, 0x7e, 0xed, 0x6c};
    printf("RustTestCheckSucceedBCHKeystone crypto_bytes %p\r\n", crypto_bytes);
    TransactionCheckResult *result = utxo_check_keystone(crypto_bytes, Bytes, mfp, sizeof(mfp), xpub);
    printf("error_code: %d\r\n", result->error_code);
    printf("error_message, %s\r\n", result->error_message);
    free_ur_parse_result(ur);
    free_TransactionCheckResult(result);
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void RustTestParseLTCKeystone(int argc, char *argv[])
{
    printf("RustTestParseLTCKeystone 11\r\n");
    URParseResult *ur = test_get_ltc_keystone_bytes();
    void *crypto_bytes = ur->data;
    char *xpub = "Mtub2rz9F1pkisRsSZX8sa4Ajon9GhPP6JymLgpuHqbYdU5JKFLBF7Qy8b1tZ3dccj2fefrAxfrPdVkpCxuWn3g72UctH2bvJRkp6iFmp8aLeRZ";
    ViewType view_type = ur->t;
    printf("RustTestParseLTCKeystone view_type %d\r\n", view_type);
    uint8_t mfp[4] = {0x70, 0x7e, 0xed, 0x6c};
    printf("RustTestParseLTCKeystone crypto_bytes %p\r\n", crypto_bytes);
    TransactionParseResult_DisplayTx *result = utxo_parse_keystone(crypto_bytes, Bytes, mfp, sizeof(mfp), xpub);
    VecFFI_DisplayTxDetailInput *inputs = result->data->detail->from;
    VecFFI_DisplayTxOverviewOutput *overview_to = result->data->overview->to;
    printf("RustTestParseLTCKeystone to->size %d\r\n", result->data->overview->to->size);
    for (size_t i = 0; i < result->data->overview->to->size; i++)
    {
        printf("result: overview output #%d\r\n", i);
        printf("result: overview output address %s\r\n", overview_to->data[i].address);
    }
    printf("RustTestParseLTCKeystone from->size %d\r\n", result->data->detail->from->size);
    for (size_t i = 0; i < result->data->detail->from->size; i++)
    {
        printf("result: input #%d\r\n", i);
        printf("result: input address %s\r\n", inputs->data[i].address);
        printf("result: input amount %s\r\n", inputs->data[i].amount);
        printf("result: input path %s\r\n", inputs->data[i].path);
    }
    printf("result: fee value %s\r\n", result->data->detail->fee_amount);
    printf("result: input length %d\r\n", result->data->detail->from->size);
    free_ur_parse_result(ur);
    free_TransactionParseResult_DisplayTx(result);
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void RustTestParseTronKeystone(int argc, char *argv[])
{
    printf("RustTestParseTronKeystone 11\r\n");
    URParseResult *ur = test_get_tron_keystone_bytes();
    void *crypto_bytes = ur->data;
    char *xpub = "xpub6C3ndD75jvoARyqUBTvrsMZaprs2ZRF84kRTt5r9oxKQXn5oFChRRgrP2J8QhykhKACBLF2HxwAh4wccFqFsuJUBBcwyvkyqfzJU5gfn5pY";
    ViewType view_type = ur->t;
    printf("RustTestParseTronKeystone view_type %d\r\n", view_type);
    uint8_t mfp[4] = {0x70, 0x7e, 0xed, 0x6c};
    printf("RustTestParseTronKeystone crypto_bytes %p\r\n", crypto_bytes);
    TransactionParseResult_DisplayTron *result = tron_parse_keystone(crypto_bytes, Bytes, mfp, sizeof(mfp), xpub);
    printf("error_code: %d\r\n", result->error_code);
    printf("error_message, %s\r\n", result->error_message);
    printf("parse result overview: \r\n");
    printf("parse result overview from: %s\r\n", result->data->overview->from);
    printf("parse result overview to: %s\r\n", result->data->overview->to);
    printf("parse result overview value: %s\r\n", result->data->overview->value);
    printf("parse result overview method: %s\r\n", result->data->overview->method);
    printf("parse result overview network: %s\r\n", result->data->overview->network);
    printf("parse result detail: \r\n");
    printf("parse result detail token: %s\r\n", result->data->detail->token);
    printf("parse result detail contract_address: %s\r\n", result->data->detail->contract_address);
    free_ur_parse_result(ur);
    free_TransactionParseResult_DisplayTron(result);
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void RustTestCheckTronKeystoneFailed(int argc, char *argv[])
{
    printf("RustTestCheckTronKeystoneFailed 11\r\n");
    URParseResult *ur = test_get_tron_check_failed_keystone_bytes();
    void *crypto_bytes = ur->data;
    char *xpub = "xpub6C3ndD75jvoARyqUBTvrsMZaprs2ZRF84kRTt5r9oxKQXn5oFChRRgrP2J8QhykhKACBLF2HxwAh4wccFqFsuJUBBcwyvkyqfzJU5gfn5pY";
    ViewType view_type = ur->t;
    printf("RustTestCheckTronKeystoneFailed view_type %d\r\n", view_type);
    uint8_t mfp[4] = {0x70, 0x7e, 0xed, 0x6c};
    printf("RustTestCheckTronKeystoneFailed crypto_bytes %p\r\n", crypto_bytes);
    TransactionCheckResult *result = tron_check_keystone(crypto_bytes, Bytes, mfp, sizeof(mfp), xpub);
    printf("error_code: %d\r\n", result->error_code);
    printf("error_message, %s\r\n", result->error_message);
    free_ur_parse_result(ur);
    free_TransactionCheckResult(result);
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void RustTestCheckTronKeystoneSucceed(int argc, char *argv[])
{
    printf("RustTestCheckTronKeystoneSucceed 11\r\n");
    URParseResult *ur = test_get_tron_keystone_bytes();
    void *crypto_bytes = ur->data;
    char *xpub = "xpub6C3ndD75jvoARyqUBTvrsMZaprs2ZRF84kRTt5r9oxKQXn5oFChRRgrP2J8QhykhKACBLF2HxwAh4wccFqFsuJUBBcwyvkyqfzJU5gfn5pY";
    ViewType view_type = ur->t;
    printf("RustTestCheckTronKeystoneSucceed view_type %d\r\n", view_type);
    uint8_t mfp[4] = {0x70, 0x7e, 0xed, 0x6c};
    printf("RustTestCheckTronKeystoneSucceed crypto_bytes %p\r\n", crypto_bytes);
    TransactionCheckResult *result = tron_check_keystone(crypto_bytes, Bytes, mfp, sizeof(mfp), xpub);
    printf("error_code: %d\r\n", result->error_code);
    printf("error_message, %s\r\n", result->error_message);
    free_ur_parse_result(ur);
    free_TransactionCheckResult(result);
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void RustTestSignTronKeystone(int argc, char *argv[])
{
    printf("RustTestSignBTCKeystone\r\n");
    int32_t index;
    VALUE_CHECK(argc, 2);
    sscanf(argv[0], "%d", &index);
    printf("RustTestSignTronKeystone 11\r\n");
    URParseResult *ur = test_get_tron_keystone_bytes();
    void *crypto_bytes = ur->data;
    char *xpub = "xpub6C3ndD75jvoARyqUBTvrsMZaprs2ZRF84kRTt5r9oxKQXn5oFChRRgrP2J8QhykhKACBLF2HxwAh4wccFqFsuJUBBcwyvkyqfzJU5gfn5pY";
    ViewType view_type = ur->t;
    printf("RustTestSignTronKeystone view_type %d\r\n", view_type);
    uint8_t mfp[4] = {0x70, 0x7e, 0xed, 0x6c};
    int32_t cold_version = SOFTWARE_VERSION;
    printf("RustTestSignTronKeystone crypto_bytes %p\r\n", crypto_bytes);
    uint8_t seed[64];
    GetAccountSeed(index, seed, argv[1]);
    UREncodeResult *result = tron_sign_keystone(crypto_bytes, Bytes, mfp, sizeof(mfp), xpub, cold_version, seed, sizeof(seed));
    printf("is multi part: %d\r\n", result->is_multi_part);
    printf("data, %s\r\n", result->data);
    free_ur_parse_result(ur);
    free_ur_encode_result(result);
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void RustTestCosmosSignTx(int argc, char *argv[])
{
    //argv[0]: wallet index
    //argv[1]: wallet password
    //argv[2]: CosmosSignRequest cbor
    //Example(kid): #rust test cosmos sign: 0 111111 a601d82550327bc24abdc04d9aa377b119c7a5c9c70259016d7b226163636f756e745f6e756d626572223a2231363734363731222c22636861696e5f6964223a22636f736d6f736875622d34222c22666565223a7b22616d6f756e74223a5b7b22616d6f756e74223a2232353833222c2264656e6f6d223a227561746f6d227d5d2c22676173223a22313033333031227d2c226d656d6f223a22222c226d736773223a5b7b2274797065223a22636f736d6f732d73646b2f4d736753656e64222c2276616c7565223a7b22616d6f756e74223a5b7b22616d6f756e74223a223132303030222c2264656e6f6d223a227561746f6d227d5d2c2266726f6d5f61646472657373223a22636f736d6f733137753032663830766b61666e65396c61347779706478336b78787878776d3666327174636a32222c22746f5f61646472657373223a22636f736d6f73316b776d6c37797434656d34656e37677579366865743271333330387537336466663938337333227d7d5d2c2273657175656e6365223a2232227d03010481d90130a2018a182cf51876f500f500f400f4021a52744703058178286637316561343964656362373533336339376664616238383136396133363331386336373666343906654b65706c72
    int32_t index;
    VALUE_CHECK(argc, 3);
    sscanf(argv[0], "%d", &index);
    uint8_t seed[64];
    GetAccountSeed(index, seed, argv[1]);
    URParseResult *crypto_bytes = test_get_cosmos_sign_request(argv[2]);
    printf("RustTestCosmosSignTx cbor, %s\r\n", crypto_bytes->data);
    printf("RustTestCosmosSignTx ur_type, %s\r\n", crypto_bytes->ur_type);
    UREncodeResult *sign_result = cosmos_sign_tx(crypto_bytes->data, crypto_bytes->
                                  ur_type, seed, sizeof(seed));
    if (sign_result->error_message != NULL)
    {
        printf("error_message, %s\r\n", sign_result->error_message);
    }
    else
    {
        printf("Cosmos sign result error_code: %d\r\n", sign_result->error_code);
        printf("Cosmos sign result data: %s\r\n", sign_result->data);
    }
    free_ur_parse_result(crypto_bytes);
    free_ur_encode_result(sign_result);
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void RustTestCosmosEvmSignTx(int argc, char *argv[])
{
    //argv[0]: wallet index
    //argv[1]: wallet password
    //argv[2]: CosmosSignRequest cbor
    //Example(kid): #rust test cosmos evm sign: 0 111111 a701d825500980b7424e444691a2b9d5854ac53ffa025902247b226163636f756e745f6e756d626572223a2230222c22636861696e5f6964223a22222c22666565223a7b22616d6f756e74223a5b5d2c22676173223a2230227d2c226d656d6f223a22222c226d736773223a5b7b2274797065223a227369676e2f4d73675369676e44617461222c2276616c7565223a7b2264617461223a225647686c49475a766247787664326c755a79427063794230614755676157356d62334a7459585270623234675a6d397949456c44546c4d67636d566e61584e30636d46306157397549475a766369423661474676625756755a334a314d6a41784e53356c646d31766379344b436b4e6f59576c7549476c6b4f6942766332317663326c7a4c54454b5132397564484a68593351675157526b636d567a637a6f6762334e74627a4634617a427a4f48686e613352754f586731646e646a5a3352715a486878656d466b5a7a67345a6d64754d7a4e774f485535593235775a4868335a57313265484e6a646d467a644455795932526b436b3933626d56794f694276633231764d5464314d444a6d4f4442326132466d626d55356247453064336c775a48677a613368346548683362545a6d656d316a5a336c6a436c4e6862485136494463334d4467774e6a41314f4445774f4463794d54453d222c227369676e6572223a2265766d6f7331747173647a37383573716a6e6c67676565306c77786a77666b36646c33366165327566396572227d7d5d2c2273657175656e6365223a2230227d03010419232905d90130a2018a182cf5183cf500f500f400f4021a527447030658283538323064313738663438303235336661313139636266656533343963396236396266386562623907654b65706c72
    int32_t index;
    VALUE_CHECK(argc, 3);
    sscanf(argv[0], "%d", &index);
    uint8_t seed[64];
    GetAccountSeed(index, seed, argv[1]);
    URParseResult *crypto_bytes = test_get_cosmos_evm_sign_request(argv[2]);
    printf("RustTestCosmosEvmSignTx cbor, %s\r\n", crypto_bytes->data);
    printf("RustTestCosmosEvmSignTx ur_type, %d\r\n", crypto_bytes->ur_type);
    UREncodeResult *sign_result = cosmos_sign_tx(crypto_bytes->data, crypto_bytes->
                                  ur_type, seed, sizeof(seed));
    if (sign_result->error_message != NULL)
    {
        printf("error_message, %s\r\n", sign_result->error_message);
    }
    else
    {
        printf("Cosmos sign result error_code: %d\r\n", sign_result->error_code);
        printf("Cosmos sign result data: %s\r\n", sign_result->data);
    }
    free_ur_parse_result(crypto_bytes);
    free_ur_encode_result(sign_result);
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void RustTestParseBCHKeystone(int argc, char *argv[])
{
    printf("RustTestParseBCHKeystone 11\r\n");
    URParseResult *ur = test_get_bch_keystone_bytes();
    void *crypto_bytes = ur->data;
    char *xpub = "xpub6ByHsPNSQXTWZ7PLESMY2FufyYWtLXagSUpMQq7Un96SiThZH2iJB1X7pwviH1WtKVeDP6K8d6xxFzzoaFzF3s8BKCZx8oEDdDkNnp4owAZ";
    ViewType view_type = ur->t;
    printf("RustTestParseBCHKeystone view_type %d\r\n", view_type);
    uint8_t mfp[4] = {0x70, 0x7e, 0xed, 0x6c};
    printf("RustTestParseBCHKeystone crypto_bytes %p\r\n", crypto_bytes);
    TransactionParseResult_DisplayTx *result = utxo_parse_keystone(crypto_bytes, Bytes, mfp, sizeof(mfp), xpub);
    VecFFI_DisplayTxDetailInput *inputs = result->data->detail->from;
    VecFFI_DisplayTxOverviewOutput *overview_to = result->data->overview->to;
    printf("RustTestParseBCHKeystone to->size %d\r\n", result->data->overview->to->size);
    printf("RustTestParseBCHKeystone overview->total_output_amount %s\r\n", result->data->overview->total_output_amount);
    printf("RustTestParseBCHKeystone overview->fee_amount %s\r\n", result->data->overview->fee_amount);
    printf("RustTestParseBCHKeystone detail->total_input_amount %s\r\n", result->data->detail->total_input_amount);
    printf("RustTestParseBCHKeystone detail->total_output_amount %s\r\n", result->data->detail->total_output_amount);
    printf("RustTestParseBCHKeystone detail->fee_amount %s\r\n", result->data->detail->fee_amount);
    for (size_t i = 0; i < result->data->overview->to->size; i++)
    {
        printf("result: overview output #%d\r\n", i);
        printf("result: overview output address %s\r\n", overview_to->data[i].address);
    }
    printf("RustTestParseBCHKeystone from->size %d\r\n", result->data->detail->from->size);
    for (size_t i = 0; i < result->data->detail->from->size; i++)
    {
        printf("result: input #%d\r\n", i);
        printf("result: input address %s\r\n", inputs->data[i].address);
        printf("result: input amount %s\r\n", inputs->data[i].amount);
        printf("result: input path %s\r\n", inputs->data[i].path);
    }
    printf("result: fee value %s\r\n", result->data->detail->fee_amount);
    printf("result: input length %d\r\n", result->data->detail->from->size);
    free_ur_parse_result(ur);
    free_TransactionParseResult_DisplayTx(result);
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void RustTestParseDASHKeystone(int argc, char *argv[])
{
    printf("RustTestParseDASHKeystone 11\r\n");
    URParseResult *ur = test_get_dash_keystone_bytes();
    ViewType view_type = ur->t;
    printf("RustTestParseDASHKeystone view_type %d\r\n", view_type);
    void *crypto_bytes = ur->data;
    char *xpub = "xpub6CYEjsU6zPM3sADS2ubu2aZeGxCm3C5KabkCpo4rkNbXGAH9M7rRUJ4E5CKiyUddmRzrSCopPzisTBrXkfCD4o577XKM9mzyZtP1Xdbizyk";
    uint8_t mfp[4] = {0x70, 0x7e, 0xed, 0x6c};
    printf("RustTestParseDASHKeystone crypto_bytes %p\r\n", crypto_bytes);
    TransactionParseResult_DisplayTx *result = utxo_parse_keystone(crypto_bytes, Bytes, mfp, sizeof(mfp), xpub);
    VecFFI_DisplayTxDetailInput *inputs = result->data->detail->from;
    VecFFI_DisplayTxOverviewOutput *overview_to = result->data->overview->to;
    printf("RustTestParseDASHKeystone to->size %d\r\n", result->data->overview->to->size);
    for (size_t i = 0; i < result->data->overview->to->size; i++)
    {
        printf("result: overview output #%d\r\n", i);
        printf("result: overview output address %s\r\n", overview_to->data[i].address);
    }
    printf("RustTestParseDASHKeystone from->size %d\r\n", result->data->detail->from->size);
    for (size_t i = 0; i < result->data->detail->from->size; i++)
    {
        printf("result: input #%d\r\n", i);
        printf("result: input address %s\r\n", inputs->data[i].address);
        printf("result: input amount %s\r\n", inputs->data[i].amount);
        printf("result: input path %s\r\n", inputs->data[i].path);
    }
    printf("result: fee value %s\r\n", result->data->detail->fee_amount);
    printf("result: input length %d\r\n", result->data->detail->from->size);
    free_ur_parse_result(ur);
    free_TransactionParseResult_DisplayTx(result);
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void RustTestMemory(int argc, char *argv[])
{
    printf("RustTestMemory\r\n");
    URParseResult *ur = test_get_crypto_psbt();
    free_ur_parse_result(ur);
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void RustTestEncodeCryptoPSBT(int argc, char *argv[])
{
    printf("RustTestEncodeCryptoPSBT\r\n");
    UREncodeResult *ur = test_encode_crypto_psbt();
    printf("encode ur\r\n");
    printf("is_multi_part is %d\r\n", ur->is_multi_part);
    printf("data is %s\r\n", ur->data);
    printf("error_code is %d\r\n", ur->error_code);
    printf("error_message is %s\r\n", ur->error_message);
}

static void RustTestDecodeCryptoPSBT(int argc, char *argv[])
{
    printf("RustTestDecodeCryptoPSBT\r\n");
    URParseResult *ur = test_decode_crypto_psbt();
    printf("decode ur\r\n");
    printf("error_code is %d\r\n", ur->error_code);
    printf("data is %d\r\n", ur->data);
    printf("progress is %d\r\n", ur->progress);
}

static void RustTestDecodeKeystoneBTCSignResult(int argc, char *argv[])
{
    printf("RustTestDecodeKeystoneBTCSignResult\r\n");
    URParseResult *ur = test_decode_btc_keystone_sign_result();
    printf("decode ur\r\n");
    printf("error_code is %d\r\n", ur->error_code);
    printf("error_message is %s\r\n", ur->error_message);
}

static void RustTestDecodeKeystoneSignRequest(int argc, char *argv[])
{
    printf("RustTestDecodeKeystoneSignRequest\r\n");
    URParseResult *ur = test_decode_keystone_sign_request();
    printf("decode ur\r\n");
    printf("error_code is %d\r\n", ur->error_code);
    printf("view type is %d\r\n", ur->t);
    printf("progress is %d\r\n", ur->progress);
}

static void RustTestDecodeMultiCryptoPSBT(int argc, char *argv[])
{
    printf("RustTestDecodeMultiCryptoPSBT\r\n");
    URParseResult *ur = test_decode_crypto_psbt_1();
    printf("decode ur\r\n");
    printf("error_code is %d\r\n", ur->error_code);
    printf("data is %d\r\n", ur->data);
    printf("progress is %d\r\n", ur->progress);

    URParseMultiResult *ur2 = test_decode_crypto_psbt_2(ur->decoder);

    printf("decode ur2\r\n");
    printf("error_code is %d\r\n", ur2->error_code);
    printf("progress is %d\r\n", ur2->progress);
    printf("data is %d\r\n", ur2->data);

    URParseMultiResult *ur3 = test_decode_crypto_psbt_3(ur->decoder);

    printf("decode ur3\r\n");
    printf("error_code is %d\r\n", ur3->error_code);
    printf("progress is %d\r\n", ur3->progress);
    printf("data is %d\r\n", ur3->data);
}

static void RustTestSignPSBT(int argc, char *argv[])
{
    printf("RustTestSignPSBT\r\n");
    int32_t index;
    VALUE_CHECK(argc, 2);
    sscanf(argv[0], "%d", &index);
    URParseResult *ur = test_get_crypto_psbt();
    PtrUR *crypto_psbt = ur->data;
    uint8_t seed[64];
    int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();
    GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
    UREncodeResult *result = btc_sign_psbt(crypto_psbt, seed, len);
    printf("is multi part: %d\r\n", result->is_multi_part);
    printf("data, %s\r\n", result->data);
}

static void RustTestGetAddressLTCSucceed(int argc, char *argv[])
{
    printf("RustTestGetAddressLTCSucceed\r\n");
    char *xpub = "Mtub2rz9F1pkisRsSZX8sa4Ajon9GhPP6JymLgpuHqbYdU5JKFLBF7Qy8b1tZ3dccj2fefrAxfrPdVkpCxuWn3g72UctH2bvJRkp6iFmp8aLeRZ";
    char *hdPath = "M/49'/2'/0'/0/0";
    SimpleResponse_c_char *result = utxo_get_address(hdPath, xpub);
    printf("get address \r\n");
    printf("address is %s\r\n", result->data);
    printf("error_code is %d\r\n", result->error_code);
    printf("error_message is %s\r\n", result->error_message);
    free_simple_response_c_char(result);
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void RustTestGetAddressTronSucceed(int argc, char *argv[])
{
    printf("RustTestGetAddressTronSucceed\r\n");
    char *xpub = "xpub6D1AabNHCupeiLM65ZR9UStMhJ1vCpyV4XbZdyhMZBiJXALQtmn9p42VTQckoHVn8WNqS7dqnJokZHAHcHGoaQgmv8D45oNUKx6DZMNZBCd";
    char *hdPath = "M/44'/195'/0'/0/0";
    SimpleResponse_c_char *result = tron_get_address(hdPath, xpub);
    printf("get address \r\n");
    printf("address is %s\r\n", result->data);
    printf("error_code is %d\r\n", result->error_code);
    printf("error_message is %s\r\n", result->error_message);
    free_simple_response_c_char(result);
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void RustTestGetAddressSolanaSucceed(int argc, char *argv[])
{
    printf("RustTestGetAddressSolanaSucceed\r\n");
    int32_t index;
    VALUE_CHECK(argc, 3);
    sscanf(argv[0], "%d", &index);
    // get publickey
    uint8_t seed[64];
    GetAccountSeed(index, seed, argv[1]);
    SimpleResponse_c_char *pubkey = get_ed25519_pubkey_by_seed(seed, sizeof(seed), argv[2]);
    char *pubkeyStr = pubkey->data;
    printf("pubkey: %s\r\n", pubkey->data);
    SimpleResponse_c_char *result = solana_get_address(pubkeyStr);
    printf("get address \r\n");
    printf("address is %s\r\n", result->data);
    printf("error_code is %d\r\n", result->error_code);
    printf("error_message is %s\r\n", result->error_message);
    free_simple_response_c_char(pubkey);
    free_simple_response_c_char(result);
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void RustTestGetAddressLTCFailed(int argc, char *argv[])
{
    printf("RustTestGetAddressLTCFailed\r\n");
    char *xpub = "Mtub2rz9F1pkisRsSZX8sa4Ajon9GhPP6JymLgpuHqbYdU5JKFLBF7Qy8b1tZ3dccj2fefrAxfrPdVkpCxuWn3g72UctH2bvJRkp6iFmp8aLeRZ";
    char *hdPath = "M/49'/3'/0'/0/0";
    SimpleResponse_c_char *result = utxo_get_address(hdPath, xpub);
    printf("get address \r\n");
    printf("address is %s\r\n", result->data);
    printf("error_code is %d\r\n", result->error_code);
    printf("error_message is %s\r\n", result->error_message);
    free_simple_response_c_char(result);
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void RustTestSignBTCKeystone(int argc, char *argv[])
{
    printf("RustTestSignBTCKeystone\r\n");
    int32_t index;
    VALUE_CHECK(argc, 2);
    sscanf(argv[0], "%d", &index);
    URParseResult *ur = test_get_btc_keystone_bytes();
    void *crypto_bytes = ur->data;
    char *xpub = "xpub6BosfCnifzxcFwrSzQiqu2DBVTshkCXacvNsWGYJVVhhawA7d4R5WSWGFNbi8Aw6ZRc1brxMyWMzG3DSSSSoekkudhUd9yLb6qx39T9nMdj";
    uint8_t mfp[4] = {0x70, 0x7e, 0xed, 0x6c};
    int32_t cold_version = SOFTWARE_VERSION;
    printf("RustTestSignBTCKeystone crypto_bytes %p\r\n", crypto_bytes);
    uint8_t seed[64];
    int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();
    GetAccountSeed(index, seed, argv[1]);
    UREncodeResult *result = utxo_sign_keystone(crypto_bytes, Bytes, mfp, sizeof(mfp), xpub, cold_version, seed, len);
    printf("is multi part: %d\r\n", result->is_multi_part);
    printf("data, %s\r\n", result->data);
    free_ur_parse_result(ur);
    free_ur_encode_result(result);
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void RustTestSignLTCKeystone(int argc, char *argv[])
{
    printf("RustTestSignLTCKeystone\r\n");
    int32_t index;
    VALUE_CHECK(argc, 2);
    sscanf(argv[0], "%d", &index);
    URParseResult *ur = test_get_ltc_keystone_bytes();
    void *crypto_bytes = ur->data;
    char *xpub = "Mtub2rz9F1pkisRsSZX8sa4Ajon9GhPP6JymLgpuHqbYdU5JKFLBF7Qy8b1tZ3dccj2fefrAxfrPdVkpCxuWn3g72UctH2bvJRkp6iFmp8aLeRZ";
    int32_t cold_version = SOFTWARE_VERSION;
    uint8_t mfp[4] = {0x70, 0x7e, 0xed, 0x6c};
    printf("RustTestSignLTCKeystone crypto_bytes %p\r\n", crypto_bytes);
    uint8_t seed[64];
    int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();
    GetAccountSeed(index, seed, argv[1]);
    UREncodeResult *result = utxo_sign_keystone(crypto_bytes, Bytes, mfp, sizeof(mfp), xpub, cold_version, seed, len);
    printf("is multi part: %d\r\n", result->is_multi_part);
    printf("data, %s\r\n", result->data);
    free_ur_parse_result(ur);
    free_ur_encode_result(result);
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void RustTestSignDASHKeystone(int argc, char *argv[])
{
    printf("RustTestSignDASHKeystone\r\n");
    int32_t index;
    VALUE_CHECK(argc, 2);
    sscanf(argv[0], "%d", &index);
    URParseResult *ur = test_get_dash_keystone_bytes();
    void *crypto_bytes = ur->data;
    char *xpub = "xpub6CYEjsU6zPM3sADS2ubu2aZeGxCm3C5KabkCpo4rkNbXGAH9M7rRUJ4E5CKiyUddmRzrSCopPzisTBrXkfCD4o577XKM9mzyZtP1Xdbizyk";
    uint8_t mfp[4] = {0x70, 0x7e, 0xed, 0x6c};
    int32_t cold_version = SOFTWARE_VERSION;
    printf("RustTestSignDASHKeystone crypto_bytes %p\r\n", crypto_bytes);
    uint8_t seed[64];
    int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();
    GetAccountSeed(index, seed, argv[1]);
    UREncodeResult *result = utxo_sign_keystone(crypto_bytes, Bytes, mfp, sizeof(mfp), xpub, cold_version, seed, len);
    printf("is multi part: %d\r\n", result->is_multi_part);
    printf("data, %s\r\n", result->data);
    free_ur_parse_result(ur);
    free_ur_encode_result(result);
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void RustTestSignBCHKeystone(int argc, char *argv[])
{
    printf("RustTestSignBCHKeystone\r\n");
    int32_t index;
    VALUE_CHECK(argc, 2);
    sscanf(argv[0], "%d", &index);
    URParseResult *ur = test_get_bch_keystone_bytes();
    void *crypto_bytes = ur->data;
    char *xpub = "xpub6ByHsPNSQXTWZ7PLESMY2FufyYWtLXagSUpMQq7Un96SiThZH2iJB1X7pwviH1WtKVeDP6K8d6xxFzzoaFzF3s8BKCZx8oEDdDkNnp4owAZ";
    uint8_t mfp[4] = {0x70, 0x7e, 0xed, 0x6c};
    int32_t cold_version = SOFTWARE_VERSION;
    printf("RustTestSignBCHKeystone crypto_bytes %p\r\n", crypto_bytes);
    uint8_t seed[64];
    int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();
    GetAccountSeed(index, seed, argv[1]);
    UREncodeResult *result = utxo_sign_keystone(crypto_bytes, Bytes, mfp, sizeof(mfp), xpub, cold_version, seed, len);
    printf("is multi part: %d\r\n", result->is_multi_part);
    printf("data, %s\r\n", result->data);
    free_ur_parse_result(ur);
    free_ur_encode_result(result);
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void RustTestDecodeUr(int argc, char *argv[])
{
    printf("RustTestDecodeUr\r\n");
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
    char *ur = SRAM_MALLOC(1000);
    char *xpub = SRAM_MALLOC(124);
    char *nestedAddress = SRAM_MALLOC(124);
    char *nativeAddress = SRAM_MALLOC(124);
    strcpy(ur, "UR:CRYPTO-PSBT/HDRKJOJKIDJYZMADAEGMAOAEAEAEADJNFPVALTEEISYAHTZMKOTSJONYMUQZJSLAWDATLRWEPKSTFDCPLGDWFLFXMTSGGOAEAEAEAEAEZCZMZMZMADNBBSAEAEAEAEAEAECMAEBBIYCNLFLKCTLTRNKSFPPTPASFENBTETPLBKLUJTTIAEAEAEAEAEADADCTISCHAEAEAEAEAEAECMAEBBTISSOTWSASWLMSRPWLNNESKBGYMYVLVECYBYLKOYCPAMAOVDPYDAEMRETYNNMSAXASPKVTJTNNGAWFJZVYSOZERKTYGLSPVTTTSFNBQZYTSRCFCSJKSKTNBKGHAEAELAADAEAELAAEAEAELAAEAEAEAEAEAEAEAEAEAEMNSFFGMW");
    strcpy(xpub, "xpub6BhcvYN2qwQKRviMKKBTfRcK1RmCTmM7JHsg67r3rwvymhUEt8gPHhnkugQaQ7UN8M5FfhEUfyVuSaK5fQzfUpvAcCxN4bAT9jyySbPGsTs");
    strcpy(nestedAddress, "xpub6CkG15Jdw866GKs84e7ysjxAhBQUJBdLZTVbQERCjwh2z6wZSSdjfmaXaMvf6Vm5sbWemK43d7HJMicz41G3vEHA9Sa5N2J9j9vgwyiHdMj");
    strcpy(nativeAddress, "xpub6Bm9M1SxZdzL3TxdNV8897FgtTLBgehR1wVNnMyJ5VLRK5n3tFqXxrCVnVQj4zooN4eFSkf6Sma84reWc5ZCXMxPbLXQs3BcaBdTd4YQa3B");
    URParseResult *result = parse_ur(ur);
    free_ur_parse_result(result);
    SRAM_FREE(ur);
    SRAM_FREE(xpub);
    SRAM_FREE(nestedAddress);
    SRAM_FREE(nativeAddress);
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void RustGetConnectBlueWalletUR(int argc, char *argv[])
{
    printf("RustGetConnectBlueWalletUR\r\n");
    UREncodeResult *ur = test_connect_blue_wallet();
    printf("encode ur\r\n");
    printf("is_multi_part is %d\r\n", ur->is_multi_part);
    printf("data is %s\r\n", ur->data);
    printf("error_code is %d\r\n", ur->error_code);
    printf("error_message is %s\r\n", ur->error_message);
}

static void RustGetConnectKeplrUR(int argc, char *argv[])
{
    printf("RustGetConnectKeplrUR\r\n");
    uint8_t mfp[4] = {0x73, 0xC5, 0xDA, 0x0A};

    struct KeplrAccount atom = {"ATOM-0", "M/44'/118'/0'/0/0", "xpub6GWQSCxh2ug91DcDwj4zYF1JckmwpVx3EAG8SKgfw4wKHSUHqCXUecRcfhWgEwD27Sg7cFFDN45HtVAxxx2XnRdtdVNr8JLFy8YraWDYBb4" };
    struct KeplrAccount secret = {"SCRT-0", "M/44'/529'/0'/0/0", "xpub6FkBDHpJffDAoeVPCFkVHqVNaUNxho4gMGs1uqdGwQSSwS2KqBY3wrM8XCNyfBFaeYiR7rD3VfTH3qAmnDQDcsgMAmqqHgfbQrfmPKXM1sS" };
    struct KeplrAccount cro = {"CRO-0", "M/44'/394'/0'/0/0", "xpub6GJghQLb21eJ93DXQphpvCc8Yk4tcLZeFqg1PYvF2Jqb2sk49xrU41HFrhGdbNS6XjGQmdiTSzCPCQgWndCtagWhWTktPn2DkDRfMYEaRZ2" };
    struct KeplrAccount starname = {"IOV-0", "M/44'/234'/0'/0/0", "xpub6GBAJVsAHvJqFEWYNEpYSVQ9Q23M9axEejprJ2yMRi4zgC7BkfXtZxxLYqS3u3nqoruoyVYGUtQfDztjvSiQPhA6bC5CWGMaSCBk8z8GZ83" };
    struct KeplrAccount agoric = {"BLD-0", "M/44'/564'/0'/0/0", "xpub6FaKwCG15yoCcc8BB5MZsK5S5H82q7fbp8XLWzaJRtBjpTH4EF67922fNQjXdTFuAEtX5kWwPtTaxnUVKPNocD174xDv3j9LeWde418zWSD" };
    struct KeplrAccount kava = {"KAVA-0", "M/44'/459'/0'/0/0", "xpub6GynGvfkyXFFMF6u9shUamN1hoJPJzXhteVKqRpN7WuwkSCV463nxTuG178z7TTxDic9PgUnf4b3bMpGBGRnGS4CgmWvqbw7PsK9pKAvdBc" };
    struct KeplrAccount evmos = {"EVMOS-0", "M/44'/60'/0'/0/0", "xpub6H6LG2We64bdwqNF7gNkUJ5EvDibiT2gbs77oonbawV86XE3eMxZf9czGQ9CPdSzsdsHLnLEjiJJEDnFMAyLrWATesaVbTYeggBXMHaFKLg" };
    KeplrAccount accounts[] = {atom, secret, cro, starname, agoric, kava, evmos};
    PtrT_CSliceFFI_KeplrAccount keplr_accounts = SRAM_MALLOC(sizeof(CSliceFFI_KeplrAccount));
    keplr_accounts->size = 7;
    keplr_accounts->data = accounts;
    UREncodeResult *ur = get_connect_keplr_wallet_ur(mfp, sizeof(mfp), keplr_accounts);
    printf("encode ur\r\n");
    if (ur->error_code == 0)
    {
        printf("Keplr is_multi_part is %d\r\n", ur->is_multi_part);
        printf("Keplr data is %s\r\n", ur->data);
    }
    else
    {
        printf("Keplr error_code is %s\r\n", ur->error_code);
        printf("Keplr error_message is %s\r\n", ur->error_message);
    }
    free_ur_encode_result(ur);
    SRAM_FREE(keplr_accounts);
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void RustGetConnectXrpToolKitUR(int argc, char *argv[])
{
    printf("RustGetConnectXrpToolKitUR\r\n");
    char *hd_path = "44'/144'/0'/0/0";
    char *root_x_pub = "xpub6CFKyZTfzj3cyeRLUDKwQQ5s1tqTTdVgywKMVkrB2i1taGFbhazkxDzWVsfBHZpv7rg6qpDBGYR5oA8iazEfa44CdQkkknPFHJ7YCzncCS9";
    char *root_path = "44'/144'/0'";
    UREncodeResult *ur = get_connect_xrp_toolkit_ur(hd_path, root_x_pub, root_path);
    printf("encode ur\r\n");
    if (ur->error_code == 0)
    {
        printf("XrpToolkit is_multi_part is %d\r\n", ur->is_multi_part);
        printf("XrpToolkit data is %s\r\n", ur->data);
    }
    else
    {
        printf("XrpToolkit error_code is %s\r\n", ur->error_code);
        printf("XrpToolkit error_message is %s\r\n", ur->error_message);
    }
    free_ur_encode_result(ur);
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void RustGetConnectMetaMaskUR(int argc, char *argv[])
{
    uint8_t mfp[4] = {0x75, 0x7e, 0x6f, 0xc9};
    struct ExtendedPublicKey key0 = {"", "xpub6C8zKiZZ8V75XynjThhvdjy7hbnJHAFkhW7jL9EvBCsRFSRov4sXUJATU6CqUF9BxAbryiU3eghdHDLbwgF8ASE4AwHTzkLHaHsbwiCnkHc"};
    struct ExtendedPublicKey key1 = {"", "xpub6C8zKiZZ8V75aXTgYAswqCgYUHeBYg1663a4Ri6zdJ4GW58r67Kmj4Fr8pbBK9usq45o8iQ8dBM75o67ct1M38yeb6RhFTWBxiYeq8Kg84Z"};
    struct ExtendedPublicKey key2 = {"", "xpub6C8zKiZZ8V75e1B4yeB8hpw9itcx9aKWvxfWH5jmX9xf1YeqaUM1zotxXg2pX8Jin4NUS7L1KKGTUuuCr1kMf2ox6c5ebQ14XTZa9seWuCW"};
    struct ExtendedPublicKey key3 = {"", "xpub6C8zKiZZ8V75gaav5YzPCb19oSARjXNWHrBRtoQA7Fx7uMSXSnkgCrwviTPqHswrP5JSxzMshS2jetwo7V9sR3mbjMqr3bukHy3She7adEX"};
    struct ExtendedPublicKey key4 = {"", "xpub6C8zKiZZ8V75hek8cS7byAQtfQBWov8osxA4bREa8JLvXBkmynFd8fttUVX723ZCHvVaYnJMgQsE35E7JBQr5eqYSwYwGBYa2cjFXYi9Z6u"};
    struct ExtendedPublicKey key5 = {"", "xpub6C8zKiZZ8V75nabUFPJEspYfLqsJRqbmQ5LBvUy8nDRzj9SejBCGCKbAT127LQjKQVoFasayzam5ozijG3f2kCf7uZQrPBQG4zAfSFJ6c2Z"};
    struct ExtendedPublicKey key6 = {"", "xpub6C8zKiZZ8V75phnqeXtW6Qm8duCmHXUE18PkYHHJqmCPmcf5iEdqLPfSVNsPKm7HsUGPorG5KSxmWc3nAkfbhmWN7PuqFzpYbmHocAiqMH1"};
    struct ExtendedPublicKey key7 = {"", "xpub6C8zKiZZ8V75qcUxEpYCXebRF23VMcJqcWdCSvZjcfMWYU4dcQnKZ8LiXPXBwYioYc62wC6F1B6UhnWYtX1Ss9ZfT3dC6e63Bzfq4AHULQh"};
    struct ExtendedPublicKey key8 = {"", "xpub6C8zKiZZ8V75tgxbNbnWfAbC63k1s6tQvfpYLNLaEzq3aUhhZu798JUyMJVL3fNK2CcF2vKauyBdF73TxjLyfU9Cfb4SLKxLDr3SUVVSAQL"};
    struct ExtendedPublicKey key9 = {"", "xpub6C8zKiZZ8V75xAiMExpbA2YBTvgC3o5sLkHjv6jm1zdWrAQ2pCq9CUnGQgBiVd2xrUh2yxBSUccXguJgdJ3SdnJvhAD5KGhWa6fjKsyt8Wy"};
    ExtendedPublicKey keys[] = {key0, key1, key2, key3, key4, key5, key6, key7, key8, key9};
    PtrT_CSliceFFI_ExtendedPublicKey public_keys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
    public_keys->size = 10;
    public_keys->data = keys;
    PtrT_UREncodeResult ur = get_connect_metamask_ur(mfp, sizeof(mfp), LedgerLive, public_keys);
    printf("encode ur\r\n");
    printf("is_multi_part is %d\r\n", ur->is_multi_part);
    printf("data is %s\r\n", ur->data);
    printf("error_code is %d\r\n", ur->error_code);
    printf("error_message is %s\r\n", ur->error_message);
}

static void RustTestKeyDerivation(int argc, char *argv[])
{
    int32_t index;
    VALUE_CHECK(argc, 2);
    sscanf(argv[0], "%d", &index);
    uint8_t seed[64];
    GetAccountSeed(index, seed, argv[1]);
    SimpleResponse_c_char *ed25519_key = get_ed25519_pubkey_by_seed(seed, sizeof(seed), "m/0'");
    printf("ed25519 pubkey: %s\r\n", ed25519_key);
    SimpleResponse_c_char *xpub = get_extended_pubkey_by_seed(seed, sizeof(seed), "m/0'");
    printf("xpub: %s\r\n", xpub->data);
    free_simple_response_c_char(ed25519_key);
    free_simple_response_c_char(xpub);
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}
static void RustGetSyncCompanionAppUR(int argc, char *argv[])
{
    printf("RustGetSyncCompanionAppUR\r\n");

    uint8_t mfp[4] = {0x70, 0x7e, 0xed, 0x6c};

    AccountConfig account;
    account.hd_path = "M/49'/0'/0'";
    account.x_pub = "xpub6C6nQwHaWbSrzs5tZ1q7m5R9cPK9eYpNMFesiXsYrgc1P8bvLLAet9JfHjYXKjToD8cBRswJXXbbFpXgwsswVPAZzKMa1jUp2kVkGVUaJa7";
    account.address_length = 20;
    account.is_multi_sign = false;

    CoinConfig coin;
    coin.is_active = true;
    coin.coin_code = "BTC";
    coin.accounts = &account;
    coin.accounts_length = 1;

    AccountConfig account1;
    account1.hd_path = "M/44'/0'/0'";
    account1.x_pub = "xpub6BosfCnifzxcFwrSzQiqu2DBVTshkCXacvNsWGYJVVhhawA7d4R5WSWGFNbi8Aw6ZRc1brxMyWMzG3DSSSSoekkudhUd9yLb6qx39T9nMdj";
    account1.address_length = 20;
    account1.is_multi_sign = false;

    CoinConfig coin1;
    coin1.is_active = true;
    coin1.coin_code = "BTC_LEGACY";
    coin1.accounts = &account1;
    coin1.accounts_length = 1;
    CoinConfig coins[2] = {coin, coin1};

    UREncodeResult *result = get_connect_companion_app_ur(mfp, sizeof(mfp), SOFTWARE_VERSION, coins, 2);
    printf("is_multi_part is %d\r\n", result->is_multi_part);
    printf("data is %s\r\n", result->data);
    printf("error_code is %d\r\n", result->error_code);
    printf("error_message is %s\r\n", result->error_message);

    free_ur_encode_result(result);

    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void testEthTx(int argc, char *argv[])
{
    int32_t index;
    VALUE_CHECK(argc, 2);
    sscanf(argv[0], "%d", &index);
    URParseResult *result = test_get_eth_sign_request();
    char *xpub = "xpub6ELHKXNimKbxMCytPh7EdC2QXx46T9qLDJWGnTraz1H9kMMFdcduoU69wh9cxP12wDxqAAfbaESWGYt5rREsX1J8iR2TEunvzvddduAPYcY";
    TransactionParseResult_DisplayETH *eth = eth_parse(result->data, xpub);
    printf("parse result: \r\n");
    printf("parse result overview from: %s\r\n", eth->data->overview->from);
    printf("parse result overview to: %s\r\n", eth->data->overview->to);
    printf("parse result overview max txn fee: %s\r\n", eth->data->overview->max_txn_fee);
    printf("parse result overview value: %s\r\n", eth->data->overview->value);
    uint8_t seed[64];
    int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();
    GetAccountSeed(index, seed, argv[1]);
    UREncodeResult *sign_result = eth_sign_tx(result->data, seed, len);
    printf("sign result error_code: %d\r\n", sign_result->error_code);
    printf("sign result error_message: %s\r\n", sign_result->error_message);
    printf("sign result data: %s\r\n", sign_result->data);
}

static void testSolanaTx(int argc, char *argv[])
{
    // Example: #rust test solana tx: 0 111111 a501d825509b1deb4d3b7d4bad9bdd2b0d7b3dcb6d02589601000103876c762c4c83532f82966935ba1810659a96237028a2af6688dadecb0155ae071c7d0930a08193e702b0f24ebba96f179e9c186ef1208f98652ee775001744490000000000000000000000000000000000000000000000000000000000000000a7516fe1d3af3457fdc54e60856c0c3c87f4e5be3d10ffbc7a5cce8bf96792a101020200010c02000000881300000000000003d90130a20188182cf51901f5f500f500f5021a123456780568736f6c666c6172650601
    int32_t index;
    VALUE_CHECK(argc, 3);
    sscanf(argv[0], "%d", &index);
    // transfer a501d825509b1deb4d3b7d4bad9bdd2b0d7b3dcb6d02589601000103876c762c4c83532f82966935ba1810659a96237028a2af6688dadecb0155ae071c7d0930a08193e702b0f24ebba96f179e9c186ef1208f98652ee775001744490000000000000000000000000000000000000000000000000000000000000000a7516fe1d3af3457fdc54e60856c0c3c87f4e5be3d10ffbc7a5cce8bf96792a101020200010c02000000881300000000000003d90130a20188182cf51901f5f500f500f5021a123456780568736f6c666c6172650601
    URParseResult *crypto_bytes = test_get_sol_sign_request(argv[2]);
    uint8_t seed[64];
    GetAccountSeed(index, seed, argv[1]);
    // sign result
    UREncodeResult *sign_result = solana_sign_tx(crypto_bytes->data, seed, sizeof(seed));
    printf("sign result error_code: %d\r\n", sign_result->error_code);
    printf("sign result data: %s\r\n", sign_result->data);
    free_ur_parse_result(crypto_bytes);
    free_ur_encode_result(sign_result);
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void testSolanaCheckTx(int argc, char *argv[])
{
    // transfer a501d825509b1deb4d3b7d4bad9bdd2b0d7b3dcb6d02589601000103876c762c4c83532f82966935ba1810659a96237028a2af6688dadecb0155ae071c7d0930a08193e702b0f24ebba96f179e9c186ef1208f98652ee775001744490000000000000000000000000000000000000000000000000000000000000000a7516fe1d3af3457fdc54e60856c0c3c87f4e5be3d10ffbc7a5cce8bf96792a101020200010c02000000881300000000000003d90130a20188182cf51901f5f500f500f5021a123456780568736f6c666c6172650601
    URParseResult *crypto_bytes = test_get_sol_sign_request(argv[0]);
    // check failed
    uint8_t failed_mfp[4] = {0x73, 0xC5, 0xDA, 0x0A};
    TransactionCheckResult *failed_result = solana_check(crypto_bytes->data, failed_mfp, sizeof(failed_mfp));
    printf("transaction check failed result : %d\r\n", failed_result->error_code);
    printf("transaction check failed error_message, %s\r\n", failed_result->error_message);
    // check succeed
    uint8_t succeed_mfp[4] = {0x70, 0x7e, 0xed, 0x6c};
    TransactionCheckResult *succeed_result = solana_check(crypto_bytes->data, succeed_mfp, sizeof(succeed_mfp));
    printf("transaction check succeed result : %d\r\n", succeed_result->error_code);
    free_TransactionCheckResult(failed_result);
    free_TransactionCheckResult(succeed_result);
    free_ur_parse_result(crypto_bytes);
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void testSolanaParseTx(int argc, char *argv[])
{
    printf("RustTestSolanaParseTx\r\n");
    URParseResult *ur = test_get_sol_sign_request(argv[0]);
    void *crypto_bytes = ur->data;
    printf("RustTestSolanaParseTx crypto_bytes %s\r\n", crypto_bytes);
    ViewType view_type = ur->t;
    printf("RustTestSolanaParseTx view_type %d\r\n", view_type);
    TransactionParseResult_DisplaySolanaTx *result = solana_parse_tx(crypto_bytes);
    printf("error_code: %d\r\n", result->error_code);
    if (result->error_message != NULL)
    {
        printf("error_message, %s\r\n", result->error_message);
    }
    printf("solana parse result overview: \r\n");
    printf("solana parse result overview display_type: %s\r\n", result->data->overview->display_type);
    if (result->data->overview->transfer_value != NULL)
    {
        printf("solana parse result overview transfer_value: %s\r\n", result->data->overview->transfer_value);
    }
    if (result->data->overview->transfer_from != NULL)
    {
        printf("solana parse result overview transfer_from: %s\r\n", result->data->overview->transfer_from);
    }
    if (result->data->overview->transfer_to != NULL)
    {
        printf("solana parse result overview transfer_to: %s\r\n", result->data->overview->transfer_to);
    }
    if (result->data->overview->main_action != NULL)
    {
        printf("solana parse result overview main_action: %s\r\n", result->data->overview->main_action);
    }
    if (result->data->overview->votes_on != NULL)
    {
        for (size_t i = 0; i < result->data->overview->votes_on->size; i++)
        {
            printf("solana parse result: overview votes on #%d\r\n", i);
            printf("solana parse result: overview votes on %s\r\n", result->data->overview->votes_on->data[i].slot);
        }
    }
    if (result->data->overview->vote_account != NULL)
    {
        printf("solana parse result overview vote_account: %s\r\n", result->data->overview->vote_account);
    }
    if (result->data->overview->general != NULL)
    {
        for (size_t i = 0; i < result->data->overview->general->size; i++)
        {
            printf("solana parse result: overview general #%d\r\n", i);
            printf("solana parse result: overview general program %s\r\n",
                   result->data->overview->general->data[i].program);
            printf("solana parse result: overview general method %s\r\n",
                   result->data->overview->general->data[i].method);
        }
    }
    printf("solana parse result network: %s\r\n", result->data->network);
    printf("solana parse result detail: %s\r\n", result->data->detail);
    free_ur_parse_result(ur);
    free_TransactionParseResult_DisplaySolanaTx(result);
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void RustTestSuiCheckTx(int argc, char *argv[])
{
    // Example:
    // #rust test sui check tx: a501d825507afd5e09926743fba02e08c4a09417ec0258dc00000000000200201ff915a5e9e32fdbe0135535b6c69a00a9809aaf7f7c0275d3239ca79db20d6400081027000000000000020200010101000101020000010000ebe623e33b7307f1350f8934beb3fb16baef0fc1b3f1b92868eec3944093886901a2e3e42930675d9571a467eb5d4b22553c93ccb84e9097972e02c490b4e7a22ab73200000000000020176c4727433105da34209f04ac3f22e192a2573d7948cb2fabde7d13a7f4f149ebe623e33b7307f1350f8934beb3fb16baef0fc1b3f1b92868eec39440938869e8030000000000006400000000000000000381d90130a2018a182cf5190310f500f500f500f5021a5274470304815820504886c9ec43bff70af37f55865094cc3a799cb54479f252d30cd3717f15ecdc056a5375692057616c6c6574
    URParseResult *crypto_bytes = test_get_sui_sign_request(argv[0]);
    // check failed
    uint8_t failed_mfp[4] = {0x52, 0x74, 0x47, 0x00};
    TransactionCheckResult *failed_result = sui_check_request(crypto_bytes->data, failed_mfp, sizeof(failed_mfp));
    printf("transaction check failed result : %d\r\n", failed_result->error_code);
    printf("transaction check failed error_message, %s\r\n", failed_result->error_message);
    // check succeed
    uint8_t succeed_mfp[4] = {0x52, 0x74, 0x47, 0x03};
    TransactionCheckResult *succeed_result = sui_check_request(crypto_bytes->data, succeed_mfp, sizeof(succeed_mfp));
    printf("transaction check succeed result : %d\r\n", succeed_result->error_code);
    free_TransactionCheckResult(failed_result);
    free_TransactionCheckResult(succeed_result);
    free_ur_parse_result(crypto_bytes);
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void RustTestSuiParseTx(int argc, char *argv[])
{
    // Example:
    // #rust test sui parse tx: a501d825507afd5e09926743fba02e08c4a09417ec0258dc00000000000200201ff915a5e9e32fdbe0135535b6c69a00a9809aaf7f7c0275d3239ca79db20d6400081027000000000000020200010101000101020000010000ebe623e33b7307f1350f8934beb3fb16baef0fc1b3f1b92868eec3944093886901a2e3e42930675d9571a467eb5d4b22553c93ccb84e9097972e02c490b4e7a22ab73200000000000020176c4727433105da34209f04ac3f22e192a2573d7948cb2fabde7d13a7f4f149ebe623e33b7307f1350f8934beb3fb16baef0fc1b3f1b92868eec39440938869e8030000000000006400000000000000000381d90130a2018a182cf5190310f500f500f500f5021a5274470304815820504886c9ec43bff70af37f55865094cc3a799cb54479f252d30cd3717f15ecdc056a5375692057616c6c6574
    printf("RustTestSuiTx\r\n");
    URParseResult *ur = test_get_sui_sign_request(argv[0]);
    void *crypto_bytes = ur->data;
    printf("RustTestSuiTx crypto_bytes: %s\r\n", crypto_bytes);
    ViewType view_type = ur->t;
    printf("RustTestSuiTx view_type: %d\r\n", view_type);
    TransactionParseResult_DisplaySuiIntentMessage *result = sui_parse_intent(crypto_bytes);
    printf("error_code: %d\r\n", result->error_code);
    if (result->error_message != NULL)
    {
        printf("error_message, %s\r\n", result->error_message);
    }
    printf("sui parse result detail: %s\r\n", result->data->detail);
    free_ur_parse_result(ur);
    free_TransactionParseResult_DisplaySuiIntentMessage(result);
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void RustTestSuiSignTx(int argc, char *argv[])
{
    // Example:
    // #rust test sui sign tx: 0 111111 a501d825507afd5e09926743fba02e08c4a09417ec0258dc00000000000200201ff915a5e9e32fdbe0135535b6c69a00a9809aaf7f7c0275d3239ca79db20d6400081027000000000000020200010101000101020000010000ebe623e33b7307f1350f8934beb3fb16baef0fc1b3f1b92868eec3944093886901a2e3e42930675d9571a467eb5d4b22553c93ccb84e9097972e02c490b4e7a22ab73200000000000020176c4727433105da34209f04ac3f22e192a2573d7948cb2fabde7d13a7f4f149ebe623e33b7307f1350f8934beb3fb16baef0fc1b3f1b92868eec39440938869e8030000000000006400000000000000000381d90130a2018a182cf5190310f500f500f500f5021a5274470304815820504886c9ec43bff70af37f55865094cc3a799cb54479f252d30cd3717f15ecdc056a5375692057616c6c6574
    int32_t index;
    VALUE_CHECK(argc, 3);
    sscanf(argv[0], "%d", &index);
    URParseResult *crypto_bytes = test_get_sui_sign_request(argv[2]);
    uint8_t seed[64];
    GetAccountSeed(index, seed, argv[1]);
    // sign result
    UREncodeResult *sign_result = sui_sign_intent(crypto_bytes->data, seed, sizeof(seed));
    printf("sign result error_code: %d\r\n", sign_result->error_code);
    printf("sign result data: %s\r\n", sign_result->data);
    free_ur_parse_result(crypto_bytes);
    free_ur_encode_result(sign_result);
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void RustTestAptosCheckTx(int argc, char *argv[])
{
    // Example:
    // #rust test aptos check tx: a601d825509b1deb4d3b7d4bad9bdd2b0d7b3dcb6d0258208e53e7b10656816de70824e3016fc1a277e77825e12825dc4f239f418ab2e04e0381d90130a2018a182cf519027df500f500f500f5021a5274470304815820aa7420c68c16645775ecf69a5e2fdaa4f89d3293aee0dd280e2d97ad7b879650056b6170746f7357616c6c65740601
    URParseResult *crypto_bytes = test_get_aptos_sign_request(argv[0]);
    // check failed
    uint8_t failed_mfp[4] = {0x52, 0x74, 0x47, 0x00};
    TransactionCheckResult *failed_result = aptos_check_request(crypto_bytes->data, failed_mfp, sizeof(failed_mfp));
    printf("transaction check failed result: %d\r\n", failed_result->error_code);
    printf("transaction check failed error_message: %s\r\n", failed_result->error_message);
    // check succeed
    uint8_t succeed_mfp[4] = {0x52, 0x74, 0x47, 0x03};
    TransactionCheckResult *succeed_result = aptos_check_request(crypto_bytes->data, succeed_mfp, sizeof(succeed_mfp));
    printf("transaction check succeed result: %d\r\n", succeed_result->error_code);
    free_TransactionCheckResult(failed_result);
    free_TransactionCheckResult(succeed_result);
    free_ur_parse_result(crypto_bytes);
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void RustTestAptosParseTx(int argc, char *argv[])
{
    // Example:
    // #rust test aptos parse tx: a601d825501a76927b7bf4464d91db40a5e97a28d202590102b5e97db07fa0bd0e5598aa3643a9bc6f6693bddc1a9fec9e674a461eaa00b193fe370d5c5e5d22f3f6decbf975bda3ad20c22677feff745e753938505708de2b00000000000000000200000000000000000000000000000000000000000000000000000000000000010d6170746f735f6163636f756e740e7472616e736665725f636f696e73010700000000000000000000000000000000000000000000000000000000000000010a6170746f735f636f696e094170746f73436f696e000220fe370d5c5e5d22f3f6decbf975bda3ad20c22677feff745e753938505708de2b0880969800000000000a00000000000000640000000000000042f22d6500000000020381d90130a2018a182cf519027df507f500f500f5021a527447030480056550657472610601
    printf("RustTestAptosTx\r\n");
    URParseResult *ur = test_get_aptos_sign_request(argv[0]);
    void *crypto_bytes = ur->data;
    printf("RustTestAptosTx crypto_bytes: %s\r\n", crypto_bytes);
    ViewType view_type = ur->t;
    printf("RustTestAptosTx view_type: %d\r\n", view_type);
    TransactionParseResult_DisplayAptosTx *result = aptos_parse(crypto_bytes);
    printf("error_code: %d\r\n", result->error_code);
    if (result->error_message != NULL) {
        printf("error_message, %s\r\n", result->error_message);
    }
    printf("aptos parse result detail: %s\r\n", result->data->detail);
    printf("aptos parse result is_msg: %d\r\n", result->data->is_msg);
    free_ur_parse_result(ur);
    free_TransactionParseResult_DisplayAptosTx(result);
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void RustADATest(int argc, char *argv[])
{
    printf("ADA test\r\n");
    printf("ADA get xpub\r\n");
    uint8_t entropy[64];
    uint8_t accountIndex = GetCurrentAccountIndex();
    char* paths[24] =
    {
        "m/1852'/1815'/0'",
        "m/1852'/1815'/1'",
        "m/1852'/1815'/2'",
        "m/1852'/1815'/3'",
        "m/1852'/1815'/4'",
        "m/1852'/1815'/5'",
        "m/1852'/1815'/6'",
        "m/1852'/1815'/7'",
        "m/1852'/1815'/8'",
        "m/1852'/1815'/9'",
        "m/1852'/1815'/10'",
        "m/1852'/1815'/11'",
        "m/1852'/1815'/12'",
        "m/1852'/1815'/13'",
        "m/1852'/1815'/14'",
        "m/1852'/1815'/15'",
        "m/1852'/1815'/16'",
        "m/1852'/1815'/17'",
        "m/1852'/1815'/18'",
        "m/1852'/1815'/19'",
        "m/1852'/1815'/20'",
        "m/1852'/1815'/21'",
        "m/1852'/1815'/22'",
        "m/1852'/1815'/23'",
    };
    uint8_t entropyLen = 0;
    GetAccountEntropy(accountIndex, entropy, &entropyLen, "123456");
    char* rootKey;

    SimpleResponse_c_char* root = get_icarus_master_key(entropy, (uint32_t)entropyLen, GetPassphrase(accountIndex));
    printf("get root key \r\n");
    printf("root key: %s\r\n", root->data);
    rootKey = root->data;

    for (size_t i = 0; i < 24; i++)
    {
        SimpleResponse_c_char* result = derive_bip32_ed25519_extended_pubkey(rootKey, paths[i]);
        printf("get result \r\n");
        printf("xpub %d: %s\r\n", i, result->data);
        free_simple_response_c_char(result);
    }

    free_simple_response_c_char(root);
}

static void testNearParseTx(int argc, char *argv[])
{
    printf("RustTestNearParseTx\r\n");
    URParseResult *ur = test_get_near_sign_request(argv[0]);
    void *crypto_bytes = ur->data;
    printf("RustTestNearParseTx crypto_bytes %s\r\n", crypto_bytes);
    ViewType view_type = ur->t;
    printf("RustTestNearParseTx view_type %d\r\n", view_type);
    TransactionParseResult_DisplayNearTx *result = near_parse_tx(crypto_bytes);
    printf("error_code: %d\r\n", result->error_code);
    if (result->error_message != NULL)
    {
        printf("error_message, %s\r\n", result->error_message);
    }
    printf("near parse result overview: \r\n");
    printf("near parse result overview display_type: %s\r\n", result->data->overview->display_type);
    if (result->data->overview->transfer_value != NULL)
    {
        printf("near parse result overview transfer_value: %s\r\n", result->data->overview->transfer_value);
    }
    if (result->data->overview->transfer_from != NULL)
    {
        printf("near parse result overview transfer_from: %s\r\n", result->data->overview->transfer_from);
    }
    if (result->data->overview->transfer_to != NULL)
    {
        printf("near parse result overview transfer_to: %s\r\n", result->data->overview->transfer_to);
    }
    if (result->data->overview->main_action != NULL)
    {
        printf("near parse result overview main_action: %s\r\n", result->data->overview->main_action);
    }
    if (result->data->overview->action_list != NULL)
    {
        for (size_t i = 0; i < result->data->overview->action_list->size; i++)
        {
            printf("near parse result: overview action #%d\r\n", i);
            printf("solana parse result: overview action %s\r\n", result->data->overview->action_list->data[i].action);
        }
    }
    printf("near parse result network: %s\r\n", result->data->network);
    printf("near parse result detail: %s\r\n", result->data->detail);
    free_ur_parse_result(ur);
    free_TransactionParseResult_DisplayNearTx(result);
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void testNearGetAddress(int argc, char *argv[])
{
    // arguments
    // argv[0]: wallet index
    // argv[1]: wallet password
    // argv[2]: hd path
    printf("RustTestGetAddressNearSucceed\r\n");
    int32_t index;
    VALUE_CHECK(argc, 3);
    sscanf(argv[0], "%d", &index);
    // get publickey
    uint8_t seed[64];
    GetAccountSeed(index, seed, argv[1]);
    SimpleResponse_c_char *pubkey = get_ed25519_pubkey_by_seed(seed, sizeof(seed), argv[2]);
    printf("get address \r\n");
    printf("address is %s\r\n", pubkey->data);
    printf("error_code is %d\r\n", pubkey->error_code);
    free_simple_response_c_char(pubkey);
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void testCosmosGetAddress(int argc, char *argv[])
{
    // Command Example: #rust test cosmos get address: 0 111111 44'/564'/0'/0/0 44'/564'/0' agoric
    // arguments
    // argv[0]: wallet index
    // argv[1]: wallet password
    // argv[2]: hd path
    // argv[3]: root path
    // argv[4]: prefix
    printf("testCosmosGetAddress\r\n");
    int32_t index;
    sscanf(argv[0], "%d", &index);
    // get publickey
    uint8_t seed[64];
    int32_t  getSeedRet = GetAccountSeed(index, seed, argv[1]);
    printf("get seed response is %d \r\n", getSeedRet);
    SimpleResponse_c_char *pubkey = get_extended_pubkey_by_seed(seed, sizeof(seed), argv[3]);
    printf("extended pubkey is %s\r\n", pubkey->data);
    printf("error_code is %d\r\n", pubkey->error_code);
    char *pubkeyStr = pubkey->data;
    SimpleResponse_c_char *result = cosmos_get_address(argv[2], pubkeyStr, argv[3], argv[4]);
    printf("get address \r\n");
    if (result->error_code == 0)
    {
        printf("cosmos address is %s\r\n", result->data);
    }
    else
    {
        printf("error_code is %s\r\n", result->error_code);
        printf("error_message is %s\r\n", result->error_message);
    }
    free_simple_response_c_char(pubkey);
    free_simple_response_c_char(result);
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void testNearTx(int argc, char *argv[])
{
    // arguments:
    // argv[0]: wallet index
    // argv[1]: wallet password
    // argv[2]: near sign request
    // Example: #rust test near tx: 0 111111 a301d82550e34bf7bef35e4f68aa8c4b9a509c25d2028159016b40000000333138323466626632343335666231656361346466633339373734313833636232356631336231303335326435643533323736313662353963333565616539660031824fbf2435fb1eca4dfc39774183cb25f13b10352d5d5327616b59c35eae9f442d16f48e3f00003c000000613062383639393163363231386233366331643139643461326539656230636533363036656234382e666163746f72792e6272696467652e6e65617258f2d224d30588f6208387939ace31edea27537c724d57897aed7861f43bd4ab01000000020f00000073746f726167655f6465706f7369746a0000007b226163636f756e745f6964223a2238616562363163396634653431623264666664653965353666343661316639303632316332363730633137343865373339663736383465643963643938386534222c22726567697374726174696f6e5f6f6e6c79223a747275657d00e057eb481b00000000485637193cc3430000000000000003d90130a20186182cf519018df500f5021a707eed6c
    int32_t index;
    VALUE_CHECK(argc, 3);
    sscanf(argv[0], "%d", &index);
    URParseResult *crypto_bytes = test_get_near_sign_request(argv[2]);
    // sign result
    uint8_t seed[64];
    GetAccountSeed(index, seed, argv[1]);
    UREncodeResult *sign_result = near_sign_tx(crypto_bytes->data, seed, sizeof(seed));
    printf("sign result error_code: %d\r\n", sign_result->error_code);
    printf("sign result data: %s\r\n", sign_result->data);
    free_ur_parse_result(crypto_bytes);
    free_ur_encode_result(sign_result);
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void testNearCheckTx(int argc, char *argv[])
{
    // arguments:
    // argv[0]: near sign request
    URParseResult *crypto_bytes = test_get_near_sign_request(argv[0]);
    // check failed
    uint8_t failed_mfp[4] = {0x73, 0xC5, 0xDA, 0x0A};
    TransactionCheckResult *failed_result = solana_check(crypto_bytes->data, failed_mfp, sizeof(failed_mfp));
    printf("transaction check failed result : %d\r\n", failed_result->error_code);
    printf("transaction check failed error_message, %s\r\n", failed_result->error_message);
    // check succeed
    uint8_t succeed_mfp[4] = {0x70, 0x7e, 0xed, 0x6c};
    TransactionCheckResult *succeed_result = solana_check(crypto_bytes->data, succeed_mfp, sizeof(succeed_mfp));
    printf("transaction check succeed result : %d\r\n", succeed_result->error_code);
    free_TransactionCheckResult(failed_result);
    free_TransactionCheckResult(succeed_result);
    free_ur_parse_result(crypto_bytes);
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void testCardanoTx(int argc, char *argv[])
{
    int32_t index;
    VALUE_CHECK(argc, 2);
    sscanf(argv[0], "%d", &index);
    URParseResult *result = test_get_cardano_sign_request();
    uint8_t mfp[4] = {0x73, 0xC5, 0xDA, 0x0A};
    char *xpub = "beb7e770b3d0f1932b0a2f3a63285bf9ef7d3e461d55446d6a3911d8f0ee55c0b0e2df16538508046649d0e6d5b32969555a23f2f1ebf2db2819359b0d88bd16";
    TransactionParseResult_DisplayCardanoTx *cardano_tx = cardano_parse_tx(result->data, mfp, xpub);
    printf("result: %s\r\n", cardano_tx->data->fee);
    uint8_t entropy[64];
    uint8_t entropyLen = sizeof(entropy);
    GetAccountEntropy(index, entropy, &entropyLen, argv[1]);
    UREncodeResult *sign_result = cardano_sign_tx(result->data, mfp, xpub, entropy, sizeof(entropy), "");
    if (sign_result->error_code == 0)
    {
        printf("sign result: %s \r\n", sign_result->data);
    }
    else
    {
        printf("error_code is %s\r\n", sign_result->error_code);
        printf("error_message is %s\r\n", sign_result->error_message);
    }
    free_ur_encode_result(sign_result);
    free_TransactionParseResult_DisplayCardanoTx(cardano_tx);
    free_ur_parse_result(result);
}

static void RustGetEthAddress(int argc, char *argv[])
{
    char *standardHdPath = "44'/60'/0'/0/0";
    char *rootPath = "44'/60'/0'";
    char *rootXPub = "xpub6BtigCpsVJrCGVhsuMuAshHuQctVUKUeumxP4wkUtypFpXatQ44ZCHwZi6w4Gf5kMN3vpfyGnHo5hLvgjs2NnkewYHSdVHX4oUbR1Xzxc7E";

    SimpleResponse_c_char *result = eth_get_address(standardHdPath, rootXPub, rootPath);

    if (result->error_code == 0)
    {
        printf("bip44 standard address is %s\r\n", result->data);
    }
    else
    {
        printf("error_code is %s\r\n", result->error_code);
        printf("error_message is %s\r\n", result->error_message);
    }
    free_simple_response_c_char(result);

    char *ledgerLegacyHdPath = "44'/60'/0'/0";
    SimpleResponse_c_char *result1 = eth_get_address(ledgerLegacyHdPath, rootXPub, rootPath);

    if (result1->error_code == 0)
    {
        printf("ledger legacy address is %s\r\n", result1->data);
    }
    else
    {
        printf("error_code is %s\r\n", result1->error_code);
        printf("error_message is %s\r\n", result1->error_message);
    }
    free_simple_response_c_char(result1);

    char *ledgerLiveHdPath = "44'/60'/1'/0/0";
    char *root_path1 = "44'/60'/1'";
    char *root_x_pub1 = "xpub6BtigCpsVJrCJZsM7fwcwCX8dhAn5Drg5QnMQY1wgzX1BMHGHPHB9qjmvnqgK6BECXyVTkGdr4CTyNyhaMXKdSmEVkSd4w7ePqaBvzjMxJ9";
    SimpleResponse_c_char *result2 = eth_get_address(ledgerLiveHdPath, root_x_pub1, root_path1);

    if (result2->error_code == 0)
    {
        printf("ledger live address is %s\r\n", result2->data);
    }
    else
    {
        printf("error_code is %s\r\n", result2->error_code);
        printf("error_message is %s\r\n", result2->error_message);
    }
    free_simple_response_c_char(result2);

    // error case
    SimpleResponse_c_char *result3 = eth_get_address(ledgerLiveHdPath, root_x_pub1, rootPath);

    if (result3->error_code == 0)
    {
        printf("ledger live address is %s\r\n", result3->data);
    }
    else
    {
        printf("error_code is %s\r\n", result3->error_code);
        printf("error_message is %s\r\n", result3->error_message);
    }
    free_simple_response_c_char(result3);

    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void RustTestParseAptosTx(int argc, char *argv[])
{

    SimpleResponse_c_char *result = test_aptos_parse();

    if (result->error_code == 0)
    {
        printf("json is %s\r\n", result->data);
    }
    else
    {
        printf("error_code is %s\r\n", result->error_code);
        printf("error_message is %s\r\n", result->error_message);
    }
    free_simple_response_c_char(result);

    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}


static void RustParseEthPersonalMessage(int argc, char *argv[])
{

    int32_t index;
    VALUE_CHECK(argc, 2);
    sscanf(argv[0], "%d", &index);
    URParseResult *result = test_get_eth_sign_request_for_personal_message();
    char *xpub = "xpub6ELHKXNimKbxMCytPh7EdC2QXx46T9qLDJWGnTraz1H9kMMFdcduoU69wh9cxP12wDxqAAfbaESWGYt5rREsX1J8iR2TEunvzvddduAPYcY";

    PtrT_TransactionParseResult_DisplayETHPersonalMessage eth = eth_parse_personal_message(result->data, xpub);

    printf("parse result: \r\n");
    printf("parse result raw message: %s\r\n", eth->data->raw_message);
    printf("parse result utf8_message : %s\r\n", eth->data->utf8_message);
    printf("parse result from: %s\r\n", eth->data->from);

    free_TransactionParseResult_DisplayETHPersonalMessage(eth);
    uint8_t seed[64];
    int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();
    GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
    UREncodeResult *sign_result = eth_sign_tx(result->data, seed, len);
    printf("sign result error_code: %d\r\n", sign_result->error_code);
    printf("sign result error_message: %s\r\n", sign_result->error_message);
    printf("sign result data: %s\r\n", sign_result->data);

    free_ur_encode_result(sign_result);
    free_ur_parse_result(result);

    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void RustParseEthContractData(int argc, char* argv[])
{
    char* tx_input_data = "3593564c000000000000000000000000000000000000000000000000000000000000006000000000000000000000000000000000000000000000000000000000000000a00000000000000000000000000000000000000000000000000000000064996e5f00000000000000000000000000000000000000000000000000000000000000020b000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000002000000000000000000000000000000000000000000000000000000000000004000000000000000000000000000000000000000000000000000000000000000a000000000000000000000000000000000000000000000000000000000000000400000000000000000000000000000000000000000000000000000000000000002000000000000000000000000000000000000000000000000002386f26fc1000000000000000000000000000000000000000000000000000000000000000001000000000000000000000000000000000000000000000000000000000000000001000000000000000000000000000000000000000000000000002386f26fc10000000000000000000000000000000000000000000000000000f84605ccc515414000000000000000000000000000000000000000000000000000000000000000a00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000002bc02aaa39b223fe8d0a0e5c4f27ead9083c756cc20001f46b175474e89094c44da98b954eedeac495271d0f000000000000000000000000000000000000000000";
    char* contract_abi = "{\"name\":\"UniversalRouter\",\"address\":\"0x3fC91A3afd70395Cd496C647d5a6CC9D4B2b7FAD\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"components\":[{\"internalType\":\"address\",\"name\":\"permit2\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"weth9\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"seaportV1_5\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"seaportV1_4\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"openseaConduit\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"nftxZap\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"x2y2\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"foundation\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"sudoswap\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"elementMarket\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"nft20Zap\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"cryptopunks\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"looksRareV2\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"routerRewardsDistributor\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"looksRareRewardsDistributor\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"looksRareToken\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"v2Factory\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"v3Factory\",\"type\":\"address\"},{\"internalType\":\"bytes32\",\"name\":\"pairInitCodeHash\",\"type\":\"bytes32\"},{\"internalType\":\"bytes32\",\"name\":\"poolInitCodeHash\",\"type\":\"bytes32\"}],\"internalType\":\"struct RouterParameters\",\"name\":\"params\",\"type\":\"tuple\"}],\"stateMutability\":\"nonpayable\",\"type\":\"constructor\"},{\"inputs\":[],\"name\":\"BalanceTooLow\",\"type\":\"error\"},{\"inputs\":[],\"name\":\"BuyPunkFailed\",\"type\":\"error\"},{\"inputs\":[],\"name\":\"ContractLocked\",\"type\":\"error\"},{\"inputs\":[],\"name\":\"ETHNotAccepted\",\"type\":\"error\"},{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"commandIndex\",\"type\":\"uint256\"},{\"internalType\":\"bytes\",\"name\":\"message\",\"type\":\"bytes\"}],\"name\":\"ExecutionFailed\",\"type\":\"error\"},{\"inputs\":[],\"name\":\"FromAddressIsNotOwner\",\"type\":\"error\"},{\"inputs\":[],\"name\":\"InsufficientETH\",\"type\":\"error\"},{\"inputs\":[],\"name\":\"InsufficientToken\",\"type\":\"error\"},{\"inputs\":[],\"name\":\"InvalidBips\",\"type\":\"error\"},{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"commandType\",\"type\":\"uint256\"}],\"name\":\"InvalidCommandType\",\"type\":\"error\"},{\"inputs\":[],\"name\":\"InvalidOwnerERC1155\",\"type\":\"error\"},{\"inputs\":[],\"name\":\"InvalidOwnerERC721\",\"type\":\"error\"},{\"inputs\":[],\"name\":\"InvalidPath\",\"type\":\"error\"},{\"inputs\":[],\"name\":\"InvalidReserves\",\"type\":\"error\"},{\"inputs\":[],\"name\":\"InvalidSpender\",\"type\":\"error\"},{\"inputs\":[],\"name\":\"LengthMismatch\",\"type\":\"error\"},{\"inputs\":[],\"name\":\"SliceOutOfBounds\",\"type\":\"error\"},{\"inputs\":[],\"name\":\"TransactionDeadlinePassed\",\"type\":\"error\"},{\"inputs\":[],\"name\":\"UnableToClaim\",\"type\":\"error\"},{\"inputs\":[],\"name\":\"UnsafeCast\",\"type\":\"error\"},{\"inputs\":[],\"name\":\"V2InvalidPath\",\"type\":\"error\"},{\"inputs\":[],\"name\":\"V2TooLittleReceived\",\"type\":\"error\"},{\"inputs\":[],\"name\":\"V2TooMuchRequested\",\"type\":\"error\"},{\"inputs\":[],\"name\":\"V3InvalidAmountOut\",\"type\":\"error\"},{\"inputs\":[],\"name\":\"V3InvalidCaller\",\"type\":\"error\"},{\"inputs\":[],\"name\":\"V3InvalidSwap\",\"type\":\"error\"},{\"inputs\":[],\"name\":\"V3TooLittleReceived\",\"type\":\"error\"},{\"inputs\":[],\"name\":\"V3TooMuchRequested\",\"type\":\"error\"},{\"anonymous\":false,\"inputs\":[{\"indexed\":false,\"internalType\":\"uint256\",\"name\":\"amount\",\"type\":\"uint256\"}],\"name\":\"RewardsSent\",\"type\":\"event\"},{\"inputs\":[{\"internalType\":\"bytes\",\"name\":\"looksRareClaim\",\"type\":\"bytes\"}],\"name\":\"collectRewards\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"},{\"inputs\":[{\"internalType\":\"bytes\",\"name\":\"commands\",\"type\":\"bytes\"},{\"internalType\":\"bytes[]\",\"name\":\"inputs\",\"type\":\"bytes[]\"}],\"name\":\"execute\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"},{\"inputs\":[{\"internalType\":\"bytes\",\"name\":\"commands\",\"type\":\"bytes\"},{\"internalType\":\"bytes[]\",\"name\":\"inputs\",\"type\":\"bytes[]\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"}],\"name\":\"execute\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"},{\"inputs\":[{\"internalType\":\"address\",\"name\":\"\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"\",\"type\":\"address\"},{\"internalType\":\"uint256[]\",\"name\":\"\",\"type\":\"uint256[]\"},{\"internalType\":\"uint256[]\",\"name\":\"\",\"type\":\"uint256[]\"},{\"internalType\":\"bytes\",\"name\":\"\",\"type\":\"bytes\"}],\"name\":\"onERC1155BatchReceived\",\"outputs\":[{\"internalType\":\"bytes4\",\"name\":\"\",\"type\":\"bytes4\"}],\"stateMutability\":\"pure\",\"type\":\"function\"},{\"inputs\":[{\"internalType\":\"address\",\"name\":\"\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"\",\"type\":\"uint256\"},{\"internalType\":\"bytes\",\"name\":\"\",\"type\":\"bytes\"}],\"name\":\"onERC1155Received\",\"outputs\":[{\"internalType\":\"bytes4\",\"name\":\"\",\"type\":\"bytes4\"}],\"stateMutability\":\"pure\",\"type\":\"function\"},{\"inputs\":[{\"internalType\":\"address\",\"name\":\"\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"\",\"type\":\"uint256\"},{\"internalType\":\"bytes\",\"name\":\"\",\"type\":\"bytes\"}],\"name\":\"onERC721Received\",\"outputs\":[{\"internalType\":\"bytes4\",\"name\":\"\",\"type\":\"bytes4\"}],\"stateMutability\":\"pure\",\"type\":\"function\"},{\"inputs\":[{\"internalType\":\"bytes4\",\"name\":\"interfaceId\",\"type\":\"bytes4\"}],\"name\":\"supportsInterface\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"pure\",\"type\":\"function\"},{\"inputs\":[{\"internalType\":\"int256\",\"name\":\"amount0Delta\",\"type\":\"int256\"},{\"internalType\":\"int256\",\"name\":\"amount1Delta\",\"type\":\"int256\"},{\"internalType\":\"bytes\",\"name\":\"data\",\"type\":\"bytes\"}],\"name\":\"uniswapV3SwapCallback\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"},{\"stateMutability\":\"payable\",\"type\":\"receive\"}]}},\"version\":1,\"checkPoints\":[]}";
    Response_DisplayContractData *display_contract_data = eth_parse_contract_data(tx_input_data, contract_abi);

    if (display_contract_data->error_code == 0)
    {
        printf("contract name: %s\r\n", display_contract_data->data->contract_name);
        printf("method name: %s\r\n", display_contract_data->data->method_name);
        for (size_t i = 0; i < display_contract_data->data->params->size; i++)
        {
            printf("param name: %s\r\n", display_contract_data->data->params->data[i].name);
            printf("param value: %s\r\n", display_contract_data->data->params->data[i].value);
        }
    }
}


#endif

static void LogTestFunc(int argc, char *argv[])
{
    LogTest(argc, argv);
}

static void ETHDBContractsTest(int argc, char* argv[])
{
    char functionABI[2000];
    char name[64];
    if (GetDBContract(argv[0], argv[1], 1, functionABI, name))
    {
        printf("functionABI: %s\r\n", functionABI);
        printf("name: %s\r\n", name);
    }
}

static int32_t __BackgroundTestFunc(const void *inData, uint32_t inDataLen)
{
    PrintArray("background execute data", inData, inDataLen);
    return 0;
}

static void BackgroundTestFunc(int argc, char *argv[])
{
    uint32_t delay;
    uint8_t data[] = {0x11, 0x22, 0x33};
    VALUE_CHECK(argc, 1);
    sscanf(argv[0], "%d", &delay);
    AsyncDelayExecute(__BackgroundTestFunc, data, sizeof(data), delay);
}

static void GetReceiveAddress(int argc, char *argv[])
{
    char *xpub;
    uint32_t chain;

    VALUE_CHECK(argc, 2);
    sscanf(argv[0], "%d", &chain);
    xpub = GetCurrentAccountPublicKey(chain);
    printf("xpub addr=0x%08X\r\n", xpub);
    ASSERT(xpub);
    printf("chain=%d\r\n", chain);
    printf("xpub=%s\r\n", xpub);
    printf("hd path=%s\r\n", argv[1]);
    SimpleResponse_c_char *result = utxo_get_address(argv[1], xpub);
    free_simple_response_c_char(result);
    printf("address is %s\r\n", result->data);
}

static void AccountPublicInfoTestFunc(int argc, char *argv[])
{
    AccountPublicInfoTest(argc, argv);
}

static void FingerTestFunc(int argc, char *argv[])
{
    FingerTest(argc, argv);
}

static void MotorTestFunc(int argc, char *argv[])
{
    uint32_t pwm, tick;
    VALUE_CHECK(argc, 2);
    sscanf(argv[0], "%d", &pwm);
    sscanf(argv[1], "%d", &tick);
    MotorCtrl(pwm, tick);
}

static void LcdTestFunc(int argc, char *argv[])
{
    LcdTest(argc, argv);
}


static void LowPowerTestFunc(int argc, char *argv[])
{
    LowPowerTest(argc, argv);
}

static void Slip39SliceWordTestFunc(int argc, char *argv[])
{
    uint8_t threshold = 0;
    int ret = Slip39OneSliceCheck("river flea academic academic civil duke kidney cinema insect engage explain unknown welfare rhythm branch elite vampire cover airline boring", 20,
                                  20, 20, &threshold);
    printf("ret = 0 threshold = %d\n", ret, threshold);

    ret = Slip39OneSliceCheck("river flea academic academic civil duke kidney cinema insect engage explain unknown welfare rhythm branch elite vampire cover airline boring", 20,
                              24459, 0, &threshold);
    printf("ret = 0 threshold = %d\n", ret, threshold);
}

static void Sqlite3TestFunc(int argc, char *argv[])
{
    Sqlite3Test(argc, argv);
}

static void testXRPGetAddress(int argc, char *argv[])
{
    // arguments
    // argv[0]: hd_path
    // Example: #rust test xrp get address: 44'/144'/0'/0/0
    printf("testXRPGetAddress\r\n");
    char *rootPath = "44'/144'/0'";
    char *rootXPub = "xpub6CFKyZTfzj3cyeRLUDKwQQ5s1tqTTdVgywKMVkrB2i1taGFbhazkxDzWVsfBHZpv7rg6qpDBGYR5oA8iazEfa44CdQkkknPFHJ7YCzncCS9";

    SimpleResponse_c_char *result = xrp_get_address(argv[0], rootXPub, rootPath);

    if (result->error_code == 0)
    {
        printf("xrp address is %s\r\n", result->data);
    }
    else
    {
        printf("error_code is %d\r\n", result->error_code);
        printf("error_message is %s\r\n", result->error_message);
    }
    free_simple_response_c_char(result);
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}


static void CrcTestFunc(int argc, char *argv[])
{
    uint8_t *hex;
    uint32_t len, crc32;
    uint16_t crc16;

    VALUE_CHECK(argc, 1);
    hex = SRAM_MALLOC(strlen(argv[0]) / 2 + 1);
    len = StrToHex(hex, argv[0]);
    PrintArray("hex", hex, len);
    crc32 = crc32_ieee(0, hex, len);
    crc16 = crc16_ccitt(hex, len);
    printf("crc32=0x%08X,crc16=0x%04X\r\n", crc32, crc16);
    SRAM_FREE(hex);
}


static void ProtocolCodeTestFunc(int argc, char *argv[])
{
    ProtocolCodecTest(argc, argv);
}

static void testXRPSignTx(int argc, char *argv[])
{
    printf("testXRPSignTx\r\n");
    int32_t index;
    VALUE_CHECK(argc, 2);
    sscanf(argv[0], "%d", &index);
    URParseResult *ur = test_get_xrp_sign_request();
    uint8_t seed[64];
    GetAccountSeed(index, seed, argv[1]);
    UREncodeResult *result = xrp_sign_tx(ur->data, "m/44'/144'/0'/0/0", seed, sizeof(seed));

    if (result->error_code == 0)
    {
        printf("xrp sign result is %s\r\n", result->data);
    }
    else
    {
        printf("error_code is %d\r\n", result->error_code);
        printf("error_message is %s\r\n", result->error_message);
    }
    free_ur_parse_result(ur);
    free_ur_encode_result(result);
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}


static void PresettingTestFunc(int argc, char *argv[])
{
    PresettingTest(argc, argv);
}

static void testXrpParseTx(int argc, char *argv[])
{
    //    #rust test xrp parse: 7B225472616E73616374696F6E54797065223A225061796D656E74222C22416D6F756E74223A223130303030303030222C2244657374696E6174696F6E223A22724478516F597A635172707A56487554345778366261634A5958794754457462766D222C22466C616773223A323134373438333634382C224163636F756E74223A227247556D6B794C627671474633687758347177474864727A4C6459325170736B756D222C22466565223A223132222C2253657175656E6365223A37393939313836352C224C6173744C656467657253657175656E6365223A38303838323630322C225369676E696E675075624B6579223A22303346354335424231443139454337313044334437464144313939414631304346384243314431313334384535423337363543304230423943304245433332383739227D
    printf("testXrpParseTx\r\n");
    URParseResult *ur = test_get_xrp_parse_request(argv[0]);
    void *crypto_bytes = ur->data;
    printf("testXrpParseTx crypto_bytes %s\r\n", crypto_bytes);
    ViewType view_type = ur->t;
    printf("testXrpParseTx view_type %d\r\n", view_type);
    TransactionParseResult_DisplayXrpTx *result = xrp_parse_tx(crypto_bytes);
    printf("error_code: %d\r\n", result->error_code);
    if (result->error_message != NULL)
    {
        printf("error_message, %s\r\n", result->error_message);
    }
    printf("xrp parse result overview: \r\n");
    printf("xrp parse result overview display_type: %s\r\n", result->data->overview->display_type);
    if (result->data->overview->value != NULL)
    {
        printf("xrp parse result overview transfer_value: %s\r\n", result->data->overview->value);
    }
    if (result->data->overview->from != NULL)
    {
        printf("xrp parse result overview transfer_from: %s\r\n", result->data->overview->from);
    }
    if (result->data->overview->to != NULL)
    {
        printf("xrp parse result overview transfer_to: %s\r\n", result->data->overview->to);
    }
    if (result->data->overview->transaction_type != NULL)
    {
        printf("xrp parse result overview transaction_type: %s\r\n", result->data->overview->transaction_type);
    }
    if (result->data->overview->fee != NULL)
    {
        printf("xrp parse result overview fee: %s\r\n", result->data->overview->fee);
    }
    if (result->data->overview->sequence != NULL)
    {
        printf("xrp parse result overview sequence: %s\r\n", result->data->overview->sequence);
    }
    printf("xrp parse result network: %s\r\n", result->data->network);
    printf("xrp parse result detail: %s\r\n", result->data->detail);
    free_ur_parse_result(ur);
    free_TransactionParseResult_DisplayXrpTx(result);
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void testWebAuth(int argc, char *argv[])
{
    URParseResult *_res =  parse_ur("UR:BYTES/1-3/LPADAXCFAXHLCYYNJKWLGOHKADCTHKAXHTKGCPKOIHJPJKINJLJTCPFTEHDWCPIEIHJKIAJPINJOJYINJLJTCPFTCPJEIHKKJKJYJLJTIHCXJSJPIAJLIEIHCXJOJPJLJYJLIAJLJZCPDWCPIEHSJYHSCPFTKGCPJYKKJOIHCPFTCPKTIHIDFPKPJYISCPDWCPIEHSJYHSCPFTCPGSGUKTHDINKPKTHKKPHDDYHKFEKSFWKTDLJZGTGUFPFPGLGOJLHGEHENHTKTFXIDHTDLHGGOGSGAGTIMEYGAIOINIYGHGTGUFYFXFDJTEYDLGWEMKNJNKNFPIAJSGAETJNKTFEJZJLEOGUKNGOINFEHSJLISFPISHDESHTIHKPFGFWECKKEMEOGAJTESJTEEFWGWFYIAFDKKHDJYIHEEGOKKKPHSFPINIOFYJSIYKNKKIMJTFGKSKSIHDLGEFLFXHKIHFWESFPEOJEJYDYHGEHFPFLGUFEIEGEINJTEYKOGDECFEJTECGYGTHFFEEHGEECGSIDGHESKKGRKOHFETGUJPGRJKFLJKENEHGMIHGOHKGOFEGDFXEYKSGEKPGTJNJYGUJSJKJLRSTTIMJO");
    receive("UR:BYTES/2-3/LPAOAXCFAXHLCYYNJKWLGOHKADCTJNDLIAINJNENIYKNGDFWFYFEGYKPECIMHSGMKSJEKNKKIOGEEOGAJPJLJNJZGLFXHFJEEHENFEDYJTJTJYIAEYGOEHEHJLFGEEFXHKGLJEFDIEHGEHFWDNEMIADNDYDLIEEOGSFLDLIHIDIHEHEMKNJKECGRFXISGTGDFYJLKTGYISGHGTIEFPENEYKOKSEOGWHGGTIHFLGYJSDYGWFDHKJYJYGUESHSFDKNJKJPESGLJPGRJTGTHDGHINGDJOENIEHGJSEYEOGDJEGRFYDNGLDNJZFPHFJEFDESJODLJZGYFEGOIEIDEEGYIEGAHFGTIHFGIMJEGWFLINGSENJNGHGEJNETGWECGOJNFEEEKPGEETGRGRHFHKDLDLJEJYGYJSGEKKGOGDFLDNIHKTFEJYHTJYKPJSIYGAJKFLEEFLHDEOIEHKEEHKJKFGIHGYHGGUEMINGOIOKKFGFXFGENEHHKJLHSKNIEIEFGIAGSESFYJPGOFLGUKSGMIMHGEHIDFGEOIMDLJSJPHSHTETKPHGKPJOHFFGKOJSGDGMIDFYJZETHKGAHDJOKNIHHDJLKNFHIYUE", _res->decoder);
    URParseMultiResult *_multi = receive("UR:BYTES/3-3/LPAXAXCFAXHLCYYNJKWLGOHKADCTJLEHIMIHGUHSIEDLKPJOJOFWHKGAFXGLESGOIHHGJLETINGMHFFXFWFDGAJLJLIOJTDYEMKSISFWFDFXJSFPENENJYGYJTIYHKGTIEJPHKFPFXDYHKECJYFEHDJKJKJOEHGRJOIDGSKSKTFLIEJSHSGMJEKKGSGLIOJYJNGDJKESISIEGAJKJOIYGMGRFGGADLGTGHHSFYGYGLJKEOESJOJSJNGTIEGLGDFEKPHSECFDGMEOHGDNENENINJOJNGWJPISIOJZGWIOKSIDHKFPIMGEFDFYJKKNFDFPIEFGDLEEJSHFGDFXDYHTIDFEKNGEKKFEGWFPDYFYGWFYJPFYKNFEFXDLHFGEKTKTJEJODNGAETEEIOJPDYINGMIAKSJPFDGLFLFDFWFPIHIHIYEHKNGYHTHSJOENENJOIMGTHGFGISJZGWFWHGGSJSETHGHKGSIMHSDLFLKOJYECGTJPGTGLGTIHIOKOGTIMHTHKHTEYFPGAIAIEGRGLHFFLHTGYETECEEIDECDYFEENKTKKIMKKGYKKJTGAKSKNGAGUIAIMJTINIHHFJEKNCPKIKIHEKKKOME", _res->decoder);
    uint8_t *key = SRAM_MALLOC(WEB_AUTH_RSA_KEY_LEN);
    GetWebAuthRsaKey(key);
    char *result = calculate_auth_code(_multi->data, key, 512, &key[512], 512);
    printf("auth_code: %s\r\n", result);
}


static void UsbTestFunc(int argc, char *argv[])
{
    UsbTest(argc, argv);
}


static void DeviceSettingsTestFunc(int argc, char *argv[])
{
    DeviceSettingsTest(argc, argv);
}

static void RustTestCosmosCheckTx(int argc, char *argv[])
{
    // Example #rust test cosmos check: a601d82550376d126699f54b08b6f3583bb3ddab510259017f7b226163636f756e745f6e756d626572223a2237363431222c22636861696e5f6964223a226f736d6f2d746573742d35222c22666565223a7b22616d6f756e74223a5b7b22616d6f756e74223a2239353132222c2264656e6f6d223a22756f736d6f227d5d2c22676173223a22323337373838227d2c226d656d6f223a22222c226d736773223a5b7b2274797065223a22636f736d6f732d73646b2f4d7367556e64656c6567617465222c2276616c7565223a7b22616d6f756e74223a7b22616d6f756e74223a2232303030303030222c2264656e6f6d223a22756f736d6f227d2c2264656c656761746f725f61646472657373223a226f736d6f3137753032663830766b61666e65396c61347779706478336b78787878776d36667a6d63677963222c2276616c696461746f725f61646472657373223a226f736d6f76616c6f7065723168683067357866323365357a656b673435636d65726339376873346e32303034647932743236227d7d5d2c2273657175656e6365223a2231227d03010481d90130a2018a182cf51876f500f500f400f4021a52744703058178286637316561343964656362373533336339376664616238383136396133363331386336373666343906654b65706c72
    URParseResult *crypto_bytes = test_get_cosmos_sign_request(argv[0]);
    // check failed
    uint8_t failed_mfp[4] = {0x73, 0xC5, 0xDA, 0x0A};
    TransactionCheckResult *failed_result = cosmos_check_tx(crypto_bytes->data, crypto_bytes->ur_type, failed_mfp, sizeof(failed_mfp));
    printf("transaction check failed result : %d\r\n", failed_result->error_code);
    printf("transaction check failed error_message, %s\r\n", failed_result->error_message);
    // check succeed
    uint8_t succeed_mfp[4] = {0x52, 0x74, 0x47, 0x03};
    TransactionCheckResult *succeed_result = cosmos_check_tx(crypto_bytes->data, crypto_bytes->ur_type, succeed_mfp, sizeof(succeed_mfp));
    printf("transaction check succeed result : %d\r\n", succeed_result->error_code);
    free_TransactionCheckResult(failed_result);
    free_TransactionCheckResult(succeed_result);
    free_ur_parse_result(crypto_bytes);
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void RustTestCosmosParseTx(int argc, char *argv[])
{
    // Example:
    // #rust test cosmos parse: a601d82550376d126699f54b08b6f3583bb3ddab510259017f7b226163636f756e745f6e756d626572223a2237363431222c22636861696e5f6964223a226f736d6f2d746573742d35222c22666565223a7b22616d6f756e74223a5b7b22616d6f756e74223a2239353132222c2264656e6f6d223a22756f736d6f227d5d2c22676173223a22323337373838227d2c226d656d6f223a22222c226d736773223a5b7b2274797065223a22636f736d6f732d73646b2f4d7367556e64656c6567617465222c2276616c7565223a7b22616d6f756e74223a7b22616d6f756e74223a2232303030303030222c2264656e6f6d223a22756f736d6f227d2c2264656c656761746f725f61646472657373223a226f736d6f3137753032663830766b61666e65396c61347779706478336b78787878776d36667a6d63677963222c2276616c696461746f725f61646472657373223a226f736d6f76616c6f7065723168683067357866323365357a656b673435636d65726339376873346e32303034647932743236227d7d5d2c2273657175656e6365223a2231227d03010481d90130a2018a182cf51876f500f500f400f4021a52744703058178286637316561343964656362373533336339376664616238383136396133363331386336373666343906654b65706c72
    printf("RustTestCosmosParseTx\r\n");
    URParseResult *ur = test_get_cosmos_sign_request(argv[0]);
    void *crypto_bytes = ur->data;
    printf("RustTestCosmosParseTx crypto_bytes %s\r\n", crypto_bytes);
    ViewType view_type = ur->t;
    printf("RustTestCosmosParseTx view_type %d\r\n", view_type);
    TransactionParseResult_DisplayCosmosTx *result = cosmos_parse_tx(crypto_bytes, ur->ur_type);
    printf("error_code: %d\r\n", result->error_code);
    if (result->error_message != NULL)
    {
        printf("error_message, %s\r\n", result->error_message);
    }
    printf("cosmos parse result overview: \r\n");
    printf("cosmos parse result overview display_type: %s\r\n", result->data->overview->display_type);
    if (result->data->overview->send_value != NULL)
    {
        printf("cosmos parse result overview send_value: %s\r\n", result->data->overview->send_value);
    }
    if (result->data->overview->send_from != NULL)
    {
        printf("cosmos parse result overview send_from: %s\r\n", result->data->overview->send_from);
    }
    if (result->data->overview->send_to != NULL)
    {
        printf("cosmos parse result overview send_to: %s\r\n", result->data->overview->send_to);
    }
    if (result->data->overview->delegate_value != NULL)
    {
        printf("cosmos parse result overview delegate_value: %s\r\n", result->data->overview->delegate_value);
    }
    if (result->data->overview->delegate_from != NULL)
    {
        printf("cosmos parse result overview delegate_from: %s\r\n", result->data->overview->delegate_from);
    }
    if (result->data->overview->delegate_to != NULL)
    {
        printf("cosmos parse result overview delegate_to: %s\r\n", result->data->overview->delegate_to);
    }
    if (result->data->overview->undelegate_value != NULL)
    {
        printf("cosmos parse result overview undelegate_value: %s\r\n", result->data->overview->undelegate_value);
    }
    if (result->data->overview->undelegate_to != NULL)
    {
        printf("cosmos parse result overview undelegate_to: %s\r\n", result->data->overview->undelegate_to);
    }
    if (result->data->overview->undelegate_validator != NULL)
    {
        printf("cosmos parse result overview undelegate_validator: %s\r\n", result->data->overview->undelegate_validator);
    }
    if (result->data->overview->redelegate_value != NULL)
    {
        printf("cosmos parse result overview redelegate_value: %s\r\n", result->data->overview->redelegate_value);
    }
    if (result->data->overview->redelegate_to != NULL)
    {
        printf("cosmos parse result overview redelegate_to: %s\r\n", result->data->overview->redelegate_to);
    }
    if (result->data->overview->redelegate_new_validator != NULL)
    {
        printf("cosmos parse result overview redelegate_new_validator: %s\r\n", result->data->overview->redelegate_new_validator);
    }
    if (result->data->overview->vote_voted != NULL)
    {
        printf("cosmos parse result overview vote_voted: %s\r\n", result->data->overview->vote_voted);
    }
    if (result->data->overview->vote_proposal != NULL)
    {
        printf("cosmos parse result overview vote_proposal: %s\r\n", result->data->overview->vote_proposal);
    }
    if (result->data->overview->vote_voter != NULL)
    {
        printf("cosmos parse result overview vote_voter: %s\r\n", result->data->overview->vote_voter);
    }

    if (result->data->overview->withdraw_reward_to != NULL)
    {
        printf("cosmos parse result overview withdraw_reward_to: %s\r\n", result->data->overview->withdraw_reward_to);
    }
    if (result->data->overview->withdraw_reward_validator != NULL)
    {
        printf("cosmos parse result overview withdraw_reward_validator: %s\r\n", result->data->overview->withdraw_reward_validator);
    }
    if (result->data->overview->overview_list != NULL)
    {
        printf("cosmos parse result overview overview_list: %s\r\n", result->data->overview->overview_list);
    }
    printf("cosmos parse result detail: %s\r\n", result->data->detail);
    free_ur_parse_result(ur);
    free_TransactionParseResult_DisplayCosmosTx(result);
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void RustGetConnectSolanaWalletUR(int argc, char *argv[])
{
    printf("RustGetConnectSolanaWalletUR\r\n");
    uint8_t mfp[4] = {0x52, 0x74, 0x47, 0x03};

    PtrT_CSliceFFI_ExtendedPublicKey public_keys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
    ExtendedPublicKey keys[7];
    public_keys->data = keys;
    public_keys->size = 7;
    keys[0].path = "m/44'/501'/0'/0'";
    keys[0].xpub = "e671e524ef43ccc5ef0006876f9a2fd66681d5abc5871136b343a3e4b073efde";

    keys[1].path = "m/44'/501'/1'/0'";
    keys[1].xpub = "dccf89e7b4992967e6b8ac31a03be3f8228d916048571d62d5db7eac6e83f728";

    keys[2].path = "m/44'/501'/2'/0'";
    keys[2].xpub = "81659cdd1f832de09b65d64d1fc36d62fe5094a35bfdf892c0447f477e75a6eb";

    keys[3].path = "m/44'/501'/7'";
    keys[3].xpub = "22816e1251a79cdce1ac4c787302df95510bffc3c22a92474a9c1b20d81d9c77";

    keys[4].path = "m/44'/501'/8'";
    keys[4].xpub = "90232ade711630651ba71f89aac472a26c8ceeb64c493a1e24a7bec1ead89511";

    keys[5].path = "m/44'/501'/9'";
    keys[5].xpub = "f65e650a5d24cc0fbd1df3d76b4d643a2aab9b169ead90560c8e764a5f11b7ba";

    keys[6].path = "m/44'/501'";
    keys[6].xpub = "f85c46dbd5652a4143b651dc162ac9a37a03ac015e749b4bae8f0c38bee54c48";

    UREncodeResult *ur = get_connect_solana_wallet_ur(mfp, sizeof(mfp), public_keys);
    printf("encode ur\r\n");
    if (ur->error_code == 0)
    {
        printf("solana is_multi_part is %d\r\n", ur->is_multi_part);
        printf("solana data is %s\r\n", ur->data);
    }
    else
    {
        printf("solana error_code is %s\r\n", ur->error_code);
        printf("solana error_message is %s\r\n", ur->error_message);
    }
    free_ur_encode_result(ur);
    SRAM_FREE(public_keys);
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void RustGetConnectAptosWalletUR(int argc, char *argv[])
{
    printf("RustGetConnectAptosWalletUR\r\n");
    uint8_t mfp[4] = {0x52, 0x74, 0x47, 0x03};

    PtrT_CSliceFFI_ExtendedPublicKey public_keys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
    ExtendedPublicKey keys[3];
    public_keys->data = keys;
    public_keys->size = 3;
    keys[0].path = "m/44'/637'/0'/0'/0'";
    keys[0].xpub = "7ed1e1d5656675c2424d2cf9a7d5142d85e0afed3780c5f9c684f69b8f875775";

    keys[1].path = "m/44'/637'/1'/0'/0'";
    keys[1].xpub = "a40ec03893fd955692221f5512f872e4cfd171a5c0d9822a64d4303779fa62be";

    keys[2].path = "m/44'/637'/2'/0'/0'";
    keys[2].xpub = "e72182e5e8e9f329d7febd8f17e3bb808ab085c408dcb2a7cd15638bb12821cb";

    UREncodeResult *ur = get_connect_aptos_wallet_ur(mfp, sizeof(mfp), public_keys);
    printf("encode ur\r\n");
    if (ur->error_code == 0)
    {
        printf("aptos is_multi_part is %d\r\n", ur->is_multi_part);
        printf("aptos data is %s\r\n", ur->data);
    }
    else
    {
        printf("aptos error_code is %s\r\n", ur->error_code);
        printf("aptos error_message is %s\r\n", ur->error_message);
    }
    free_ur_encode_result(ur);
    SRAM_FREE(public_keys);
    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}

static void RustSolanaMessage(int argc, char *argv[])
{

    URParseResult *result = test_get_sol_sign_message(argv[2]);
    char *pubkey = "e671e524ef43ccc5ef0006876f9a2fd66681d5abc5871136b343a3e4b073efde";

    PtrT_TransactionParseResult_DisplaySolanaMessage sol_msg = solana_parse_message(result->data, pubkey);
    if (sol_msg->error_message != NULL)
    {
        printf("error_message, %s\r\n", sol_msg->error_message);
    }
    else
    {
        printf("RustSolanaMessage parse result raw message: %s\r\n", sol_msg->data->raw_message);
        printf("RustSolanaMessage parse result utf8_message : %s\r\n", sol_msg->data->utf8_message);
        printf("RustSolanaMessage parse result from: %s\r\n", sol_msg->data->from);
    }

    int32_t index;
    sscanf(argv[0], "%d", &index);
    uint8_t seed[64];
    GetAccountSeed(index, seed, argv[1]);
    UREncodeResult *sign_result = solana_sign_tx(result->data, seed, sizeof(seed));
    if (sign_result->error_message != NULL)
    {
        printf("RustSolanaMessage sign result error_code: %d\r\n", sign_result->error_code);
        printf("RustSolanaMessage sign result error_message: %s\r\n", sign_result->error_message);
    }
    else
    {
        printf("RustSolanaMessage sign result data: %s\r\n", sign_result->data);
    }
    free_TransactionParseResult_DisplaySolanaMessage(sol_msg);
    free_ur_encode_result(sign_result);
    free_ur_parse_result(result);

    PrintRustMemoryStatus();
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
}


static void ScreenShotFunc(int argc, char *argv[])
{
    PubValueMsg(UI_MSG_SCREEN_SHOT, 0);
}
