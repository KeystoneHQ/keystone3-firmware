/************************ (C) COPYRIGHT Megahuntmicro *************************
 * @file                : mhscpu_sci.h
 * @author              : Megahuntmicro
 * @version             : V1.0.0
 * @date                : 21-October-2014
 * @brief               : This file contains all the functions prototypes for the SCI firmware library
 *****************************************************************************/

#ifndef __MHSCPU_SCI_H
#define __MHSCPU_SCI_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "mhscpu.h"
#include <arm_math.h>


#define SCI_UNCONFIG                (-1)
#define SCI_ICC_CLOCK_ERR           (-2)
#define SCI_REF_CLOCK_ERR           (-3)
#define SCI_IMPRECISION_CLK         (-4)
#define SCI_EMV_F_D_ERR             (-5)
#define SCI_EMV_TS_ERR              (-6)
#define SCI_EMV_ATR_ERR             (-7)
#define SCI_CARD_OUT_ERR            (-8)


int32_t SCI_ConfigEMV(uint8_t SCI_Bitmap, uint32_t SCIx_Clk);


#ifdef __cplusplus
}
#endif

#endif   ///< __MHSCPU_SCI_H


/**************************      (C) COPYRIGHT Megahunt    *****END OF FILE****/
