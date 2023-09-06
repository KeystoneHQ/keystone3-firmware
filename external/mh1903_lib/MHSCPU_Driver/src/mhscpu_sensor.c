/************************ (C) COPYRIGHT Megahuntmicro *************************
 * @file                : mhscpu_sensor.c
 * @author              : Megahuntmicro
 * @version             : V1.0.0
 * @date                : 21-October-2014
 * @brief               : This file provides all the SENSOR firmware functions
 *****************************************************************************/

/* Includes -----------------------------------------------------------------*/
#include "mhscpu_sensor.h"

/******************************************************************************/
/*                                                                            */
/*                      SENSOR Control Unit Block                             */
/*                                                                            */
/******************************************************************************/

/****************  Bit definition for SEN_EXT_CFG register  *******************/
#define SEN_EXT_CFG_EXTS_LEVEL                      (0x0FFFUL)
#define SEN_EXT_CFG_FREQ_POS                        (12)
#define SEN_EXT_CFG_FREQ                            (BIT(13)|BIT(12))
#define SEN_EXT_CFG_FREQ_0                          BIT(12)
#define SEN_EXT_CFG_FREQ_1                          BIT(13)
#define SEN_EXT_CFG_EXTS_PROC                       BIT(15)

#define SEN_EXT_CFG_S_GF_POS                        (16)
#define SEN_EXT_CFG_S_GF                            (BIT(17)|BIT(16))
#define SEN_EXT_CFG_S_GF_0                          BIT(16)
#define SEN_EXT_CFG_S_GF_1                          BIT(17)

#define SEN_EXT_CFG_GF_EN                           BIT(18)
#define SEN_EXT_CFG_PUPU_EN                         BIT(19)
#define SEN_EXT_CFG_PUPU_HOLD_TIME                  (BIT(21)|BIT(20))
#define SEN_EXT_CFG_PUPU_HOLD_TIME_POS              (20)
#define SEN_EXT_CFG_PUPU_HOLD_TIME_0                BIT(20)
#define SEN_EXT_CFG_PUPU_HOLD_TIME_1                BIT(21)
#define SEN_EXT_CFG_D_GF_POS                        (22)
#define SEN_EXT_CFG_D_GF                            (BIT(23)|BIT(22))
#define SEN_EXT_CFG_D_GF_0                          BIT(22)
#define SEN_EXT_CFG_D_GF_1                          BIT(23)
#define SEN_EXT_CFG_PUPU_FREQ                       BIT(24)
#define SEN_EXT_CFG_TRIG_PULL                       BIT(26)


/*****************  Bit definition for SEN_SOFT_EN register  *******************/
#define SEN_SOFT_EN_SOFT_ATTACK_EN                  BIT(0)
/****************  Bit definition for SEN_SOFT_ATTACK register  ********************/
#define SEN_SOFT_ATTACK_SOFT_ATTACK                 BIT(0)
/****************  Bit definition for SEN_SOFT_LOCK register  ********************/
#define SEN_SOFT_LOCK_SOFT_ATTACK_LOCK              BIT(31)


#define SEN_ENABLE                                  ((uint32_t)0xAA)
#define SEN_DISABLE                                 ((uint32_t)0x55)

/**
  * @brief
  * @param
  * @retval None
  */
uint32_t SENSOR_EXTInit(SENSOR_EXTInitTypeDef *SENSOR_EXTInitStruct)
{
    uint32_t i;
    uint32_t tmpSensorExtType = 0, tmpSensorExtCfg = 0;
    uint32_t tmpSensorExtPort = 0, tmpSensorExtPortEn = 0;

    if (SET == SENSOR_EXTIsRuning()) {
        return 3;
    }

    //simple rule check,each pin can be used only one mode
    if (SENSOR_EXTInitStruct->SENSOR_Port_Static & SENSOR_EXTInitStruct->SENSOR_Port_Dynamic) {
        return 1;
    }

    //Set SEN_EXT_TYPE register
    for (i = 0; i < EXT_SENSOR_NUM / 2; i++) {
        if (0 != (SENSOR_EXTInitStruct->SENSOR_Port_Dynamic & (0x03 << (i * 2)))) {
            tmpSensorExtType |= (0x01 << (i * 2));
        }
    }

    tmpSensorExtType |= (SENSOR_EXTInitStruct->SENSOR_Port_Pull & 0xFF) << 12;
    SENSOR->SEN_EXT_TYPE = tmpSensorExtType;

    //set Dynamci Sensor ExtPort Frequency
    SENSOR->SEN_EXT_CFG = (SENSOR->SEN_EXT_CFG & ~SEN_EXT_CFG_FREQ) |
                          (SENSOR_EXTInitStruct->SENSOR_DynamicFrequency << SEN_EXT_CFG_FREQ_POS);

    //set ExtPort Glitch Filter Times
    SENSOR->SEN_EXT_CFG = (SENSOR->SEN_EXT_CFG & ~SEN_EXT_CFG_D_GF) |
                          (SENSOR_EXTInitStruct->SENSOR_Dynamic_Sample << SEN_EXT_CFG_D_GF_POS);

    SENSOR->SEN_EXT_CFG = (SENSOR->SEN_EXT_CFG & ~SEN_EXT_CFG_S_GF) |
                          (SENSOR_EXTInitStruct->SENSOR_Static_Sample << SEN_EXT_CFG_S_GF_POS);

    //enable/disable Dyanmci Sensor Glitch
    if (DISABLE != SENSOR_EXTInitStruct->SENSOR_GlitchEnable) {
        SENSOR->SEN_EXT_CFG |= SEN_EXT_CFG_GF_EN;
    }

    tmpSensorExtCfg = SENSOR->SEN_EXT_CFG;

    //disable ExtPort PUPU resistance
    tmpSensorExtCfg &= ~SEN_EXT_CFG_PUPU_EN;

    tmpSensorExtCfg = (tmpSensorExtCfg & ~SEN_EXT_CFG_PUPU_FREQ) |
                      (SENSOR_EXTInitStruct->SENSOR_PUPU_Frequency << 24);

    tmpSensorExtCfg = (tmpSensorExtCfg & ~SEN_EXT_CFG_PUPU_HOLD_TIME) |
                      (SENSOR_EXTInitStruct->SENSOR_PUPU_HoldTime << SEN_EXT_CFG_PUPU_HOLD_TIME_POS);
    //enable/disable ExtPort PUPU resistance
    if (DISABLE != SENSOR_EXTInitStruct->SENSOR_PUPU_Enable) {
        tmpSensorExtCfg |= SEN_EXT_CFG_PUPU_EN;
    }

    tmpSensorExtCfg &= ~SEN_EXT_CFG_TRIG_PULL;
    if (DISABLE != SENSOR_EXTInitStruct->SENSOR_Trig_Hold_Enable) {
        tmpSensorExtCfg |= SEN_EXT_CFG_TRIG_PULL;
    }
    SENSOR->SEN_EXT_CFG = tmpSensorExtCfg;

    //Enable or Disable Ext Sensors
    if (DISABLE != SENSOR_EXTInitStruct->SENSOR_Port_Enable) {
        tmpSensorExtPortEn = SEN_ENABLE;
    } else {
        tmpSensorExtPortEn = SEN_DISABLE;
    }

    //setting SEN_EN[x] register to effect the Sensor Port
    tmpSensorExtPort = SENSOR_EXTInitStruct->SENSOR_Port_Static | SENSOR_EXTInitStruct->SENSOR_Port_Dynamic;
    for (i = 0; i < EXT_SENSOR_NUM; i++) {
        if (tmpSensorExtPort & SENSOR_Port_S0 << i) {
            SENSOR->SEN_EN[SENSOR_EN_EXT0 + i] = tmpSensorExtPortEn;
        } else {
            SENSOR->SEN_EN[SENSOR_EN_EXT0 + i] = SEN_DISABLE;
        }
    }

    return 0;
}


uint32_t SENSOR_EXTPortCmd(uint32_t SENSOR_Port, FunctionalState NewState)
{
    uint32_t i;
    volatile uint32_t tmpSensorExtPortEn = 0;

    if (SET == SENSOR_EXTIsRuning()) {
        return 3;
    }

    if (DISABLE != NewState) {
        tmpSensorExtPortEn = SEN_ENABLE;
    } else {
        tmpSensorExtPortEn = SEN_DISABLE;
    }

    for (i = 0; i < EXT_SENSOR_NUM; i++) {
        if (SENSOR_Port & SENSOR_Port_S0 << i) {
            SENSOR->SEN_EN[SENSOR_EN_EXT0 + i] = tmpSensorExtPortEn;
        }
    }

    return 0;
}

/**
  * @brief
  * @param
  * @retval None
  */
void SENSOR_AttackRespMode(SENSOR_RespModeTypeDef SENSOR_ResponseMode)
{
    assert_param(IS_SENSOR_RESP_MODE(SENSOR_ResponseMode));

    if (SENSOR_CPUReset == SENSOR_ResponseMode) {
        SENSOR->SEN_EXT_CFG &= ~SEN_EXT_CFG_EXTS_PROC;
    } else if (SENSOR_Interrupt == SENSOR_ResponseMode) {
        SENSOR->SEN_EXT_CFG |= SEN_EXT_CFG_EXTS_PROC;
    }
}

/**
  * @brief
  * @param
  * @retval None
  */
uint32_t SENSOR_EXTCmd(FunctionalState NewState)
{
    if (DISABLE != NewState) {
        SENSOR->SEN_EXTS_START = SEN_ENABLE;
    } else {
        SENSOR->SEN_EXTS_START = SEN_DISABLE;
    }

    return 0;
}

/**
  * @brief
  * @param
  * @retval None
  */
FlagStatus SENSOR_EXTIsRuning(void)
{
    if (SENSOR->SEN_EXTS_START & BIT(31)) {
        return SET;
    } else {
        return RESET;
    }
}

/**
  * @brief
  * @param
  * @retval None
  */
uint32_t SENSOR_ANACmd(uint32_t SENSOR_ANA, FunctionalState NewState)
{
    uint32_t i;
    volatile uint32_t tmpSensorExtPortEn = 0;

    assert_param(IS_SENSOR_ANA(SENSOR_ANA));

    if (DISABLE != NewState) {
        tmpSensorExtPortEn = SEN_ENABLE;
    } else {
        tmpSensorExtPortEn = SEN_DISABLE;
    }

    for (i = 0; i < INNER_SENSOR_NUM; i++) {
        if (SENSOR_ANA & SENSOR_ANA_VOL_HIGH << i) {
            SENSOR->SEN_EN[SENSOR_EN_VH + i] = (SENSOR->SEN_EN[SENSOR_EN_VH + i] & ~0xFF) | tmpSensorExtPortEn;
        }
    }

    if ((SENSOR_ANA & SENSOR_ANA_MESH)) {
        if (ENABLE == NewState) {
            SENSOR->SEN_EN[SENSOR_EN_MESH] &= ~BIT(31);
        } else {
            SENSOR->SEN_EN[SENSOR_EN_MESH] |= BIT(31);
        }

    }

    return 0;
}

void SENSOR_Lock(uint32_t SENSOR_LOCK)
{
    assert_param(IS_SENSOR_LOCK(SENSOR_LOCK));

    SENSOR->SEN_LOCK = SENSOR_LOCK;
}

uint32_t SENSOR_SetRand(uint32_t Rand)
{
    if (0 != Rand && ~0 != Rand) {
        SENSOR->SEN_RNG_INI = Rand;
        return 0;
    } else {
        return 1;
    }
}

/**
  * @brief
  * @param
  * @retval None
  */
void SENSOR_Soft_Enable(void)
{
    SENSOR->SEN_SOFT_EN |= SEN_SOFT_EN_SOFT_ATTACK_EN;
}

/**
  * @brief
  * @param
  * @retval None
  */
void SENSOR_SoftAttack(void)
{
    /* soft attack unlock */
    SENSOR->SEN_SOFT_LOCK &= ~SEN_SOFT_LOCK_SOFT_ATTACK_LOCK;
    /* Start soft attack */
    SENSOR->SEN_SOFT_ATTACK |= SEN_SOFT_ATTACK_SOFT_ATTACK;
}

/**
  * @brief
  * @param  ITState
  * @retval None
  */
int32_t SENSOR_GetITStatus(uint32_t ITState)
{
    assert_param(IS_SENSOR_IT(ITState));

    if ((SENSOR->SEN_STATE & ITState) != 0) {
        return TRUE;
    } else {
        return FALSE;
    }
}


/**
  * @brief
  * @param  None
  * @retval None
  */
int32_t SENSOR_GetITStatusReg(void)
{
    return SENSOR->SEN_STATE;
}

/**
  * @brief
  * @param
  * @retval None
  */
void SENSOR_ClearITPendingBit(void)
{
    /* Write Clear all intp state */
    SENSOR->SEN_STATE = 0;
}

/**
  * @brief
  * @param
  * @retval None
  */
FunctionalState SENSOR_GetEXTTrigPull(void)
{
    if (SENSOR->SEN_EXT_CFG & SEN_EXT_CFG_TRIG_PULL) {
        return ENABLE;
    } else {
        return DISABLE;
    }
}

FunctionalState SENSOR_SetEXTTrigPull(FunctionalState NewState)
{
    if (DISABLE == NewState) {
        SENSOR->SEN_EXT_CFG &= ~SEN_EXT_CFG_TRIG_PULL;
    } else {
        SENSOR->SEN_EXT_CFG |= SEN_EXT_CFG_TRIG_PULL;
    }

    return NewState;
}

FunctionalState SENSOR_GetEXTFilter(void)
{
    if (SENSOR->SEN_EXT_CFG & SEN_EXT_CFG_GF_EN) {
        return ENABLE;
    } else {
        return DISABLE;
    }
}

FunctionalState SENSOR_SetEXTFilter(FunctionalState NewState)
{
    if (DISABLE == NewState) {
        SENSOR->SEN_EXT_CFG &= ~SEN_EXT_CFG_GF_EN;
    } else {
        SENSOR->SEN_EXT_CFG |= SEN_EXT_CFG_GF_EN;
    }

    return NewState;
}

uint32_t SENSOR_GetEXTPulsePeriod(void)
{
    if (SENSOR->SEN_EXT_CFG & SEN_EXT_CFG_PUPU_FREQ) {
        return SENSOR_STATIC_PULSE_PERIOD_1S;
    }

    return SENSOR_STATIC_PULSE_PERIOD_500MS;
}

uint32_t SENSOR_SetEXTPulsePeriod(uint32_t u32Period)
{
    if (u32Period == SENSOR_STATIC_PULSE_PERIOD_1S) {
        SENSOR->SEN_EXT_CFG |= SEN_EXT_CFG_PUPU_FREQ;
    } else {
        SENSOR->SEN_EXT_CFG &= ~SEN_EXT_CFG_PUPU_FREQ;;
    }

    return u32Period;
}

uint32_t SENSOR_GetEXTPulseHold(void)
{
    return ((SENSOR->SEN_EXT_CFG & SEN_EXT_CFG_PUPU_HOLD_TIME) >> SEN_EXT_CFG_PUPU_HOLD_TIME_POS);
}

uint32_t SENSOR_SetEXTPulseHold(uint32_t u32Hold)
{
    SENSOR->SEN_EXT_CFG = (SENSOR->SEN_EXT_CFG & ~SEN_EXT_CFG_PUPU_HOLD_TIME) |
                          ((u32Hold << SEN_EXT_CFG_PUPU_HOLD_TIME_POS) & SEN_EXT_CFG_PUPU_HOLD_TIME);

    return u32Hold;
}

uint32_t SENSOR_GetEXTStaticSample(void)
{
    return ((SENSOR->SEN_EXT_CFG & SEN_EXT_CFG_S_GF) >> SEN_EXT_CFG_S_GF_POS);
}

uint32_t SENSOR_SetEXTStaticSample(uint32_t u32Count)
{
    SENSOR->SEN_EXT_CFG = (SENSOR->SEN_EXT_CFG & ~SEN_EXT_CFG_S_GF) |
                          ((u32Count << SEN_EXT_CFG_S_GF_POS) & SEN_EXT_CFG_S_GF);
    return u32Count;
}

uint32_t SENSOR_GetEXTDynamicSample(void)
{
    return ((SENSOR->SEN_EXT_CFG & SEN_EXT_CFG_D_GF) >> SEN_EXT_CFG_D_GF_POS);
}

uint32_t SENSOR_SetEXTDynamicSample(uint32_t u32Count)
{
    SENSOR->SEN_EXT_CFG = (SENSOR->SEN_EXT_CFG & ~SEN_EXT_CFG_D_GF) |
                          ((u32Count << SEN_EXT_CFG_D_GF_POS) & SEN_EXT_CFG_D_GF);
    return u32Count;
}

uint32_t SENSOR_GetEXTDynamicFreq(void)
{
    return ((SENSOR->SEN_EXT_CFG & SEN_EXT_CFG_FREQ) >> SEN_EXT_CFG_FREQ_POS);
}

uint32_t SENSOR_SetEXTDynamicFreq(uint32_t u32Freq)
{
    SENSOR->SEN_EXT_CFG = (SENSOR->SEN_EXT_CFG & ~SEN_EXT_CFG_FREQ) |
                          ((u32Freq << SEN_EXT_CFG_FREQ_POS) & SEN_EXT_CFG_FREQ);
    return u32Freq;
}


FunctionalState SENSOR_GetEXTPulsePull(void)
{
    if (SENSOR->SEN_EXT_CFG & SEN_EXT_CFG_PUPU_EN) {
        return ENABLE;
    }

    return DISABLE;
}

FunctionalState SENSOR_SetEXTPulsePull(FunctionalState NewState)
{
    if (DISABLE == NewState) {
        SENSOR->SEN_EXT_CFG &= ~SEN_EXT_CFG_PUPU_EN;
    } else {
        SENSOR->SEN_EXT_CFG |= SEN_EXT_CFG_PUPU_EN;
    }
    return NewState;
}

/**************************      (C) COPYRIGHT Megahunt    *****END OF FILE****/
