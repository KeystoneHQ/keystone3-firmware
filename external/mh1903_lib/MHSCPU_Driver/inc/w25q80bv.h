/**
  ************************ (C) COPYRIGHT Megahuntmicro *************************
  * File Name            : w25q80bv.h
  * Author               : Megahuntmicro
  * Version              : V1.0.0
  * Date                 : 29-October-2015
  * Description          : This file contains all the description of the w25q80 QSPI Flash.
  *****************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __W25Q80BV_H
#define __W25Q80BV_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Components
  * @{
  */

/** @addtogroup W25Q80BV
  * @{
  */

/** @defgroup W25Q80BV_Exported_Types
  * @{
  */

/**
  * @}
  */

/** @defgroup W25Q80BV_Exported_Constants
  * @{
  */

/**
  * @brief  W25Q80BV Configuration
  */
#define W25Q80BV_FLASH_SIZE                 0x100000 /* 8MBits => 1MBytes */
#define W25Q80BV_SECTOR_SIZE            0x1000   /* 256 sectors of 4KBytes */
#define W25Q80BV_SUBSECTOR_SIZE          0x100     /* 4096 subsectors of 4kBytes */
#define W25Q80BV_PAGE_SIZE                   0x100    /* 65536 pages of 256 bytes */

#define W25Q80BV_DUMMY_CYCLES_READ           4
#define W25Q80BV_DUMMY_CYCLES_READ_QUAD      10

#define W25Q80BV_BULK_ERASE_MAX_TIME         250000
#define W25Q80BV_SECTOR_ERASE_MAX_TIME       3000
#define W25Q80BV_SUBSECTOR_ERASE_MAX_TIME    800

/**
  * @brief  W25Q80BV Commands
  */
/* Reset Operations */
#define RESET_ENABLE_CMD                    0x66
#define RESET_MEMORY_CMD                        0x99

#define ENTER_QPI_MODE_CMD                      0x38
#define EXIT_QPI_MODE_CMD                       0xFF

/* Identification Operations */
#define READ_ID_CMD                             0x90
#define DUAL_READ_ID_CMD                        0x92
#define QUAD_READ_ID_CMD                        0x94
#define READ_JEDEC_ID_CMD                       0x9F

/* Read Operations */
#define READ_CMD                                    0x03
#define FAST_READ_CMD                           0x0B
#define DUAL_OUT_FAST_READ_CMD      0x3B
#define DUAL_INOUT_FAST_READ_CMD        0xBB
#define QUAD_OUT_FAST_READ_CMD              0x6B
#define QUAD_INOUT_FAST_READ_CMD        0xEB

/* Write Operations */
#define WRITE_ENABLE_CMD                0x06
#define WRITE_DISABLE_CMD               0x04

/* Register Operations */
#define READ_STATUS_REG1_CMD                  0x05
#define READ_STATUS_REG2_CMD                  0x35
#define READ_STATUS_REG3_CMD                  0x15

#define WRITE_STATUS_REG1_CMD                 0x01
#define WRITE_STATUS_REG2_CMD                 0x31
#define WRITE_STATUS_REG3_CMD                 0x11


/* Program Operations */
#define PAGE_PROG_CMD                       0x02
#define QUAD_INPUT_PAGE_PROG_CMD                0x32


/* Erase Operations */
#define SECTOR_ERASE_CMD                        0x20
#define CHIP_ERASE_CMD                          0xC7

#define PROG_ERASE_RESUME_CMD             0x7A
#define PROG_ERASE_SUSPEND_CMD           0x75

#define SET_BURST_WITH_WRAP         0x77


/* Flag Status Register */
#define W25Q80BV_FSR_BUSY                    ((uint8_t)0x01)    /*!< busy */
#define W25Q80BV_FSR_WREN                    ((uint8_t)0x02)    /*!< write enable */
#define W25Q80BV_FSR_QE                      ((uint8_t)0x02)    /*!< quad enable */

/**
  * @}
  */

/** @defgroup W25Q80BV_Exported_Functions
  * @{
  */
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __W25Q80BV_H */

/************************ (C) COPYRIGHT 2014 Megahuntmicro ****END OF FILE****/
