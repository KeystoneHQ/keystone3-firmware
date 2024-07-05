#include "drv_qspi_flash.h"
#include "stdio.h"
#include "mhscpu.h"
#include "assert.h"
#include "mh_rand.h"
#include "mhscpu_qspi.h"

static uint32_t CheckFlashType(void);

QSPI_CommandTypeDef g_cmdType;

/// @brief  QSPI flash init, get model.
/// @param
void QspiFlashInit(void)
{
    uint32_t chipType;

    SYSCTRL_AHBPeriphClockCmd(SYSCTRL_AHBPeriph_DMA | SYSCTRL_AHBPeriph_CRYPT, ENABLE);
    SYSCTRL_AHBPeriphResetCmd(SYSCTRL_AHBPeriph_DMA | SYSCTRL_AHBPeriph_CRYPT, ENABLE);
    SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_TRNG, ENABLE);
    SYSCTRL_APBPeriphResetCmd(SYSCTRL_APBPeriph_TRNG, ENABLE);
    mh_rand_init();
    QSPI_Init(NULL);
    QSPI_SetLatency(0);
    chipType = CheckFlashType();
    if (chipType == QSPI_SUPPORT_CHIP_JEDEC_ID_MICR) {
        g_cmdType.Instruction = PAGE_PROG_CMD;
        g_cmdType.BusMode = QSPI_BUSMODE_111;
        g_cmdType.CmdFormat = QSPI_CMDFORMAT_CMD8_ADDR24_PDAT;
    } else {
        g_cmdType.Instruction = QUAD_INPUT_PAGE_PROG_CMD;
        g_cmdType.BusMode = QSPI_BUSMODE_114;
        g_cmdType.CmdFormat = QSPI_CMDFORMAT_CMD8_ADDR24_PDAT;
    }
}


/// @brief Erase flash sector.
/// @param addr Erase address.
void QspiFlashErase(uint32_t addr)
{
    FLASH_EraseSector(addr);
}


/// @brief
/// @param addr Write flash addr.
/// @param data
/// @param len
void QspiFlashWrite(uint32_t addr, const uint8_t *data, uint32_t len)
{
#ifdef __ARMCC_VERSION
    //FLASH_ProgramPage(&g_cmdType, NULL, addr, len, (uint8_t *)data);
    AES_Program(&g_cmdType, NULL, addr, len, (uint8_t *)data);
#else
    QSPI_ProgramPage(&g_cmdType, NULL, addr, len, (uint8_t *)data);
#endif
}

#include "log_print.h"
/// @brief
/// @param addr Write flash addr.
/// @param data
/// @param len
void QspiFlashEraseAndWrite(uint32_t addr, const uint8_t *data, uint32_t len)
{
    static bool first = true;
    ASSERT(len == 4096);

    do {
        FLASH_EraseSector(addr);
        CACHE_CleanAll(CACHE);
        AES_Program(&g_cmdType, NULL, addr, len, (uint8_t *)data);
        if (first) {
            // first = false;
            if (memcmp(data, (uint8_t *)addr, len) == 0) {
                printf("read back check ok %#x\n", addr);
                break;
            } else {
                printf("encrypt check error....... %#x\n", addr);
                PrintArray("write", data, len);
                PrintArray("read", (uint8_t *)addr, len);
            }
        }
     } while (0);
}


/// @brief Get flash type.
/// @param
/// @return
static uint32_t CheckFlashType(void)
{
    uint32_t chip_type;
    QSPI_CommandTypeDef test_cmd;

    ROM_QSPI_ReleaseDeepPowerDown(NULL);

    chip_type = ROM_QSPI_ReadID(NULL);
    printf("FLASH ID = %#x \n", chip_type);
    if (chip_type == 0xffffff) {
        test_cmd.Instruction = 0x9F;
        test_cmd.BusMode = QSPI_BUSMODE_444;
        test_cmd.CmdFormat = QSPI_CMDFORMAT_CMD8_RREG24;
        chip_type = ROM_QSPI_ReadID(&test_cmd);
        printf("ReRead_FLASH ID = %#x \n", chip_type);
    }

    chip_type = chip_type >> 16;
    switch (chip_type) {
    case QSPI_SUPPORT_CHIP_JEDEC_ID_MICR:      //MICRON
        printf("QSPI Flash chip is MICRON\n");

        //burst
        test_cmd.Instruction = 0x81;
        test_cmd.BusMode = QSPI_BUSMODE_111;
        test_cmd.CmdFormat = QSPI_CMDFORMAT_CMD8_WREG8;
        ROM_QSPI_WriteParam(&test_cmd, 0xF1);

        CACHE->CACHE_CONFIG = (CACHE->CACHE_CONFIG & 0xFF00FFFF) | (0xA5 << 16);

        test_cmd.Instruction = 0x61;
        test_cmd.BusMode = QSPI_BUSMODE_111;
        test_cmd.CmdFormat = QSPI_CMDFORMAT_CMD8_WREG16;
        ROM_QSPI_WriteParam(&test_cmd, 0x4F);
        break;

    case QSPI_SUPPORT_CHIP_JEDEC_ID_MXIC:    //MXIC
        printf("QSPI Flash chip is MXIC\n");

        test_cmd.Instruction = 0xC0;
        test_cmd.BusMode = QSPI_BUSMODE_111;
        test_cmd.CmdFormat = QSPI_CMDFORMAT_CMD8_WREG8;
        ROM_QSPI_WriteParam(&test_cmd, 0x02);       ///< 32Bytes

        CACHE->CACHE_CONFIG = (CACHE->CACHE_CONFIG & 0xFF00FFFF) | (0xA5 << 16);

        test_cmd.Instruction = 0x01;
        test_cmd.BusMode = QSPI_BUSMODE_111;
        test_cmd.CmdFormat = QSPI_CMDFORMAT_CMD8_WREG16;
        ROM_QSPI_WriteParam(&test_cmd, 0x0042);     ///< QE WEL
        break;

    case QSPI_SUPPORT_CHIP_JEDEC_ID_WD:     //Winbond
        printf("QSPI Flash chip is Winbond\n");

        test_cmd.Instruction = WRITE_STATUS_REG1_CMD;
        test_cmd.BusMode = QSPI_BUSMODE_111;
        test_cmd.CmdFormat = QSPI_CMDFORMAT_CMD8_WREG16;
        ROM_QSPI_WriteParam(&test_cmd, 0x0202);

        test_cmd.Instruction = SET_BURST_WITH_WRAP;
        test_cmd.BusMode = QSPI_BUSMODE_144;
        test_cmd.CmdFormat = QSPI_CMDFORMAT_CMD8_DMY24_WREG8;
        ROM_QSPI_WriteParam(&test_cmd, 0x40);

        CACHE->CACHE_CONFIG = (CACHE->CACHE_CONFIG & 0xFF00FFFF) | (0xA5 << 16);
        break;

    case QSPI_SUPPORT_CHIP_JEDEC_ID_GD:      ///< GD
        printf("QSPI Flash chip is GD\n");

        test_cmd.Instruction = WRITE_STATUS_REG1_CMD;         ///< Write Status Register
        test_cmd.BusMode = QSPI_BUSMODE_111;
        test_cmd.CmdFormat = QSPI_CMDFORMAT_CMD8_WREG16;
        ROM_QSPI_WriteParam(&test_cmd, 0x0200);  ///< QE(low byte first, MSB first)

        test_cmd.Instruction = SET_BURST_WITH_WRAP;         ///< Set Burst with Wrap
        test_cmd.BusMode = QSPI_BUSMODE_144;
        test_cmd.CmdFormat = QSPI_CMDFORMAT_CMD8_DMY24_WREG8;
        ROM_QSPI_WriteParam(&test_cmd, 0x40);    ///< Wrap Length:32Bytes

        CACHE->CACHE_CONFIG = (CACHE->CACHE_CONFIG & 0xFF00FFFF) | (0xA5 << 16);
        break;

    default:
        printf("QSPI Flash chip Not Support\n");
        break;
    }
    return chip_type;
}


