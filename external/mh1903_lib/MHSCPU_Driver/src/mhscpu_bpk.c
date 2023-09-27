/************************ (C) COPYRIGHT Megahuntmicro *************************
 * @file                : mhscpu_bpk.c
 * @author              : Megahuntmicro
 * @version             : V1.0.0
 * @date                : 21-October-2014
 * @brief               : This file provides all the BPK firmware functions
 *****************************************************************************/

/* Includes -----------------------------------------------------------------*/
#include "mhscpu_bpk.h"

/* Exported functions -------------------------------------------------------*/
/**
  * @brief  Check whether bpk is ready or not
  * @param  None
  * @retval The status of BPK ready flag
  */
FlagStatus BPK_IsReady(void)
{
	if (BPK->BPK_RDY & BPK_RDY_READY)
	{
		return SET;	
	}
	
	return RESET;
}

/**
  * @brief  Write user key to the specified Data backup register
  * @param  Key: pointer to the data to write
  * @param  Key_Len: length of user key
  * @param  Key_Offset: offset of the Data backup register
  * @retval result of operation
  */
ErrorStatus BPK_WriteKey(uint32_t *Key, uint32_t Key_Len, uint32_t Key_Offset)
{
	uint32_t index = 0;
	
	if (BPK_IsReady() == RESET)
	{
		return ERROR;	
	}
	
	if (Key_Offset + Key_Len > BPK_KEY_WORD_SIZE)
	{
		return ERROR;	
	}

	for (index = 0; index < Key_Len; index++)
	{
		BPK->KEY[Key_Offset + index] = Key[index];
	}
    
	return SUCCESS;
}

/**
  * @brief  read user key from the specified Data backup register
  * @param  Buf: pointer to the data to read
  * @param  Key_Len: length of user key
  * @param  Key_Offset: offset of the Data backup register
  * @retval result of operation
  */
ErrorStatus BPK_ReadKey(uint32_t *Buf, uint32_t Key_Len, uint32_t Key_Offset)
{
	uint32_t index = 0;
	
	if (BPK_IsReady() == RESET)
	{
		return ERROR;	
	}

	if (Key_Offset + Key_Len > BPK_KEY_WORD_SIZE)
	{
		return ERROR;	
	}
	
	for (index = 0; index < Key_Len; index++)
	{
		Buf[index] = BPK->KEY[Key_Offset + index];
	}
	
	return SUCCESS;
}

/**
  * @brief  Clear the specified Data backup region
  * @param  BPK_KEY_Region: Data backup region
  *         This parameter can be one of the following values: 
  *         BPK_KEY_REGION_0, BPK_KEY_REGION_1, BPK_KEY_REGION_2 or BPK_KEY_REGION_3.
  * @retval None
  */
void BPK_KeyClear(uint16_t BPK_KEY_Region)
{
    assert_param(IS_BPK_KEY_REGION(BPK_KEY_Region));
	
    BPK->BPK_CLR |= BPK_KEY_Region;
	while (BPK->BPK_CLR);
}

/**
  * @brief  Enable or disable Data backup register write lock
  * @param  BPK_KEY_Region: Data backup region
  *         This parameter can be one of the following values: 
  *      @arg BPK_KEY_REGION_0
  *      @arg BPK_KEY_REGION_1
  * @param  NewState: new state of the BPK region write lock
  *			This parameter can be ENABLE or DISABLE
  * @retval result of operation
  */
void BPK_KeyWriteLock(uint16_t BPK_KEY_Region, FunctionalState NewState)
{
    assert_param(IS_BPK_KEY_REGION(BPK_KEY_Region));
	
    if (DISABLE != NewState)
    {
        BPK->BPK_LWA |= BPK_KEY_Region;
    }
	else
	{
        BPK->BPK_LWA &= ~BPK_KEY_Region;
    }
}

/**
  * @brief  Enable or disable Data backup register read lock
  * @param  BPK_KEY_Region: Data backup region
  *         This parameter can be one of the following values:
  *      @arg BPK_KEY_REGION_0
  *      @arg BPK_KEY_REGION_1
  * @param  NewState: new state of the BPK region read lock
  *			This parameter can be ENABLE or DISABLE
  * @retval result of operation
  */
void BPK_KeyReadLock(uint16_t BPK_KEY_Region, FunctionalState NewState)
{
    assert_param(IS_BPK_KEY_REGION(BPK_KEY_Region));
	
    if (DISABLE != NewState)
    {
        BPK->BPK_LRA |= BPK_KEY_Region;
    }
	else
	{
        BPK->BPK_LRA &= ~BPK_KEY_Region;
    }
}

/**
  * @brief  Enable or disable the specified BPK lock
  * @param  BPK_LOCK: specify the bpk lock
  *         This parameter can be one of the following values: 
  *      @arg BPK_LR_LOCK_RESET
  *      @arg BPK_LR_LOCK_KEYWRITE
  *      @arg BPK_LR_LOCK_KEYREAD
  *      @arg BPK_LR_LOCK_KEYCLEAR
  *      @arg BPK_LR_LOCK_SETSCRAMBER
  * @param  NewState: new state of the specified BPK lock
  *			This parameter can be ENABLE or DISABLE
  * @retval None
  */
void BPK_Lock(uint32_t BPK_LOCK, FunctionalState NewState)
{
	assert_param((BPK_LOCK != BPK_LR_LOCK_SELF) && IS_BPK_LOCK(BPK_LOCK));
	
	if (DISABLE != NewState)
	{
		BPK->BPK_LR |= BPK_LOCK;
	}
	else
	{
		BPK->BPK_LR &= ~BPK_LOCK;
	}
}

/**
  * @brief  Lock the BPK_LR register
  *      Note: If the BPK_LR register is locked, only through backup battery domain power on again can be unlocked
  * @param  None
  * @retval None
  */
void BPK_LockSelf(void)
{
	BPK->BPK_LR |= BPK_LR_LOCK_SELF;
}

/**
  * @brief  Get the specified BPK lock status
  * @param  BPK_LOCK: specify the bpk lock
  *         This parameter can be one of the following values: 
  *      @arg BPK_LR_LOCK_SELF
  *      @arg BPK_LR_LOCK_RESET
  *      @arg BPK_LR_LOCK_KEYWRITE
  *      @arg BPK_LR_LOCK_KEYREAD
  *      @arg BPK_LR_LOCK_KEYCLEAR
  *      @arg BPK_LR_LOCK_SETSCRAMBER
  * @retval None
  */
FlagStatus BPK_GetLockStatus(uint32_t BPK_Locks)
{
	assert_param(IS_BPK_LOCK(BPK_Locks));
	
	if ((BPK->BPK_LR & BPK_Locks) != (uint16_t)RESET)
	{
		return SET;
	}
	else
	{
		return RESET;
	}
}

/**************************      (C) COPYRIGHT Megahunt    *****END OF FILE****/
