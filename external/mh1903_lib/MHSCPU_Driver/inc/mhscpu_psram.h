/************************ (C) COPYRIGHT Megahuntmicro *************************
 * @file                : mhscpu_psram.h
 * @author              : Megahuntmicro
 * @version             : V1.0.0
 * @date                : 21-October-2014
 * @brief               : This file contains all the functions prototypes for the PSRAM firmware library
 *****************************************************************************/

#ifndef __MHSCPU_PSRAM_H__
#define __MHSCPU_PSRAM_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "mhscpu.h"

typedef struct {
    uint32_t MF_ID     :    8;
    uint32_t KGD       :    8;
    uint32_t EID_H16   :    16;
    uint32_t EID_L32   :    32;
} PSRAM_DEVIDTypeDef;

typedef struct {
    uint8_t CsnCycle;
    uint8_t BurstWord;
    uint8_t DummyCycle;
    uint32_t FreqSel;
    uint32_t SampleEdgeSel;
    uint32_t ReadMode;
    uint32_t WriteMode;
} PSRAM_InitTypeDef;


#define PSRAM_CODE_ENTER_QUAD_MODE              (0x35)
#define PSRAM_CODE_EXIT_QUAD_MODE               (0xF5)
#define PSRAM_CODE_RESET_ENABLE                 (0x66)
#define PSRAM_CODE_RESET                        (0x99)
#define PSRAM_CODE_BURST_MODE_TOGGLE            (0xC0)
#define PSRAM_CODE_READ_ID                      (0x9F)


#define PSRAM_CMD_COMMAND_CODE_Pos                  (24)

#define PSRAM_CMD_READ_BUS_MODE_Pos                 (8)
#define PSRAM_CMD_READ_BUS_MODE_READ                ((uint32_t)0x00UL << PSRAM_CMD_READ_BUS_MODE_Pos)
#define PSRAM_CMD_READ_BUS_MODE_FAST_READ           ((uint32_t)0x01UL << PSRAM_CMD_READ_BUS_MODE_Pos)
#define PSRAM_CMD_READ_BUS_MODE_FAST_READ_QUAD      ((uint32_t)0x02UL << PSRAM_CMD_READ_BUS_MODE_Pos)

#define IS_PSRAM_READ_BUS_MODE(MODE)                (((MODE) == PSRAM_CMD_READ_BUS_MODE_READ) || \
                                                    ((MODE) == PSRAM_CMD_READ_BUS_MODE_FAST_READ) || \
                                                    ((MODE) == PSRAM_CMD_READ_BUS_MODE_FAST_READ_QUAD))

#define PSRAM_CMD_WRITE_BUS_MODE_Pos                (6)
#define PSRAM_CMD_WRITE_BUS_MODE_WRITE              ((uint32_t)0x00UL << PSRAM_CMD_WRITE_BUS_MODE_Pos)
#define PSRAM_CMD_WRITE_BUS_MODE_QUAL_WRITE         ((uint32_t)0x02UL << PSRAM_CMD_WRITE_BUS_MODE_Pos)

#define IS_PSRAM_WRITE_BUS_MODE(MODE)               (((MODE) == PSRAM_CMD_WRITE_BUS_MODE_WRITE) || \
                                                    ((MODE) == PSRAM_CMD_WRITE_BUS_MODE_QUAL_WRITE))

#define PSRAM_QSPI_CLK_Pos                          (0)
#define PSRAM_QSPI_CLK_FCLK_2                       ((uint32_t)0x01UL << PSRAM_QSPI_CLK_Pos)
#define PSRAM_QSPI_CLK_FCLK_3                       ((uint32_t)0x02UL << PSRAM_QSPI_CLK_Pos)

#define IS_PSRAM_FREQ_SEL(FREQ)                     (((FREQ) == PSRAM_QSPI_CLK_FCLK_2) || \
                                                    ((FREQ) == PSRAM_QSPI_CLK_FCLK_3))

#define PSRAM_SAMPLE_EDGE_SEL_Pos                   (19)
#define PSRAM_SAMPLE_EDGE_SEL_FALLING               ((uint32_t)0x00UL << PSRAM_SAMPLE_EDGE_SEL_Pos)
#define PSRAM_SAMPLE_EDGE_SEL_RISING                ((uint32_t)0x01UL << PSRAM_SAMPLE_EDGE_SEL_Pos)
#define IS_PSRAM_SAMPLE_EDGE_SEL(EDGE)              (((EDGE) == PSRAM_SAMPLE_EDGE_SEL_FALLING) || \
                                                    ((EDGE) == PSRAM_SAMPLE_EDGE_SEL_RISING))

#define PSRAM_CSN_CYCLE_Pos                         (16)
#define PSRAM_BURST_WORD_Pos                        (8)
#define PSRAM_DUMMY_CYCLE_Pos                       (4)


void PSRAM_SendCmd(uint8_t cmd);
void PSRAM_DeviceReset(void);

void PSRAM_EnterQuadMode(void);
void PSRAM_ExitQuadMode(void);

void PSRAM_Init(PSRAM_InitTypeDef *PSRAM_InitStruct);
PSRAM_DEVIDTypeDef PSRAM_ReadID(void);

void PSRAM_SetReadMode(uint32_t ReadMode);
void PSRAM_SetWriteMode(uint32_t WriteMode);

#ifdef __cplusplus
}
#endif

#endif

/**************************      (C) COPYRIGHT Megahunt    *****END OF FILE****/
