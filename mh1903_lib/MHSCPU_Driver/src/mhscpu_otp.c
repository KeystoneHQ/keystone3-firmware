/************************ (C) COPYRIGHT Megahuntmicro *************************
 * @file                : mhscpu_otp.c
 * @author              : Megahuntmicro
 * @version             : V1.0.0
 * @date                : 21-October-2014
 * @brief               : This file provides all the OTP firmware functions
 *****************************************************************************/

/* Include ------------------------------------------------------------------*/
#include "mhscpu_otp.h"


#define OTP_DONE_FLAG               BIT(0)
#define OTP_START                   BIT(0)
#define OTP_WAKEUPEN                BIT(1)

#define OTP_TIM_EN                  (0xA5)

/*OTP unlock key*/
#define OTP_KEY1                    (0xABCD00A5)
#define OTP_KEY2                    (0x1234005A)


static uint32_t gu32OTP_Key1 = 0;
static uint32_t gu32OTP_Key2 = 0;


/******************************************************************************
* Function Name  : OTP_Operate
* Description    : 启动对OTP的操作
* Input          : NONE
* Return         : NONE
******************************************************************************/
static void OTP_Operate(void)
{
    OTP->CS |= OTP_START;
}

/******************************************************************************
 * Function Name  : OTP_WakeUp
 * Description    : 如果OTP进入休眠状态，唤醒OTP。
                    从超低功耗唤醒时需要唤醒OTP；
 * Input          : NONE
 * Return         : NONE
******************************************************************************/
void OTP_WakeUp(void)
{
    OTP->CFG |= OTP_WAKEUPEN;
}

/******************************************************************************
 * Function Name  : OTP_SetLatency
 * Description    : 设置OTP时序寄存器，分别设置1Us和10ns的时钟个数。
                    如果传入0，则按照当前时钟频率计算出一个值写进去。
                    当otp_tim_en寄存器为A5时（OTP_TimCmd()函数传入ENABLE时），
                    才使用这个函数的配置，否则（OTP_TimCmd()函数传入DISENABLE）
                    使用核里面自己的配置，推荐使用核自己的配置（OTP_TimCmd()函数传入DISENABLE）。
 * Input          : u8_1UsClk,u8_10NsCLK
 * Return         : NONE
******************************************************************************/
void OTP_SetLatency(uint8_t u8_1UsClk, uint8_t u8_10NsCLK)
{
    if (0 == u8_1UsClk) {
        OTP->TIM = ((OTP->TIM & ~(0xFF)) | (SYSCTRL->HCLK_1MS_VAL + 1000 - 1) / 1000);
    } else {
        OTP->TIM = ((OTP->TIM & ~(0xFF)) | u8_1UsClk);
    }
    if (0 == u8_10NsCLK) {
        OTP->TIM = ((OTP->TIM & ~(0x7 << 8)) | (SYSCTRL->HCLK_1MS_VAL / 100000 + 1));
    } else {
        OTP->TIM = ((OTP->TIM & ~(0x7 << 8)) | u8_10NsCLK);
    }
}

/******************************************************************************
* Function Name  : OTP_Unlock
* Description    : 获取写操作用到的密钥
* Input          : NONE
* Return         : NONE
******************************************************************************/
void OTP_Unlock(void)
{
    gu32OTP_Key1 = OTP_KEY1;
    gu32OTP_Key2 = OTP_KEY2;
}

/******************************************************************************
* Function Name  : OTP_Lock
* Description    : 对写操作加锁
* Input          : NONE
* Return         : NONE
******************************************************************************/
void OTP_Lock(void)
{
    gu32OTP_Key1 = ~OTP_KEY1;
    gu32OTP_Key2 = ~OTP_KEY2;
    OTP->PROT = OTP_KEY2;
}

/******************************************************************************
* Function Name  : OTP_IsWriteDone
* Description    : 判断OTP写入操作是否完成
* Input          : NONE
* Return         : Boolean:FALSE/TRUE
******************************************************************************/
Boolean OTP_IsWriteDone(void)
{
    if ((OTP->CS & OTP_DONE_FLAG) == OTP_DONE_FLAG) {
        return FALSE;
    } else {
        return TRUE;
    }
}

/******************************************************************************
 * Function Name  : OTP_GetFlag
 * Description    : 获取操作完成后的状态。
 * Input          : NONE
 * Return         : OTP_StatusTypeDef:
                    OTP_Complete此次操作无异常；
                    OTP_ReadOnProgramOrSleep在编程/休眠状态下对OTP进行读操作
                    OTP_ProgramIn_HiddenOrRO_Block对只读区域进行编程
                    OTP_ProgramOutOfAddr编程地址超出OTP范围
                    OTP_ProgramOnSleep在休眠状态下进行编程
                    OTP_WakeUpOnNoSleep在非休眠状态下进行唤醒操作
******************************************************************************/
OTP_StatusTypeDef OTP_GetFlag(void)
{
    return (OTP_StatusTypeDef)((OTP->CS >> 1) & 0x7);
}

/******************************************************************************
 * Function Name  : OTP_ClearStatus
 * Description    : 清除状态寄存器的值
 * Input          : NONE
 * Return         : NONE
******************************************************************************/
void OTP_ClearStatus(void)
{
    OTP->CS &= ~(0x07 << 1);
}

/******************************************************************************
* Function Name  : WriteOtpWord
* Description    : OTP区编程
* Input          : u32Addr：编程地址
                   u32Data：写入的数据
* Return         : OTP_StatusTypeDef
******************************************************************************/
OTP_StatusTypeDef OTP_WriteWord(uint32_t addr, uint32_t w)
{
    uint32_t Delay = 0;
    OTP_StatusTypeDef otp_status;

    assert_param(IS_OTP_ADDRESS(addr));
    assert_param(0 == (addr & 0x03));

    OTP->PDATA = w;
    OTP->ADDR = addr;

    OTP->PROT = gu32OTP_Key1;
    OTP->PROT = gu32OTP_Key2;

    OTP_Operate();

    Delay = 0xFFFF;
    while ((OTP_IsWriteDone() == FALSE) && (0 != --Delay));
    if (0 == Delay) {
        return OTP_TimeOut;
    }

    otp_status = OTP_GetFlag();
    if (OTP_Complete != otp_status) {
        OTP_ClearStatus();
        return otp_status;
    }

    while (FALSE == OTP_IsReadReady());
    if ((*(uint32_t *)addr) != w) {
        return OTP_DataWrong;
    }

    return OTP_Complete;
}

/******************************************************************************
* Function Name  : OTP_TimCmd
* Description    : 是否使能otp_tim寄存器
* Input          : FunctionalState
* Return         : NONE
******************************************************************************/
void OTP_TimCmd(FunctionalState NewState)
{
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (ENABLE == NewState) {
        OTP->TIM_EN = OTP_TIM_EN;
    } else {
        OTP->TIM_EN = ~OTP_TIM_EN;
    }
}

/******************************************************************************
* Function Name  : OTP_GetProtect
* Description    : 读取整个OTP区是否为只读状态
* Input          : NONE
* Return         : 返回只读锁的状态
******************************************************************************/
uint32_t OTP_GetProtect(void)
{
    return OTP->RO;
}

/******************************************************************************
* Function Name  : OTP_GetProtectLock
* Description    : 读取只读锁的对应的硬件锁的状态
* Input          : NONE
* Return         : 读取只读锁的对应的硬件锁的状态
******************************************************************************/
uint32_t OTP_GetProtectLock(void)
{
    return OTP->ROL;
}

/******************************************************************************
* Function Name  : OTP_SetProtect
* Description    : 设置对应OTP地址为只读
* Input          : u32Addr
* Return         : NONE
******************************************************************************/
void OTP_SetProtect(uint32_t u32Addr)
{
    uint32_t pu32RO;
    assert_param(IS_OTP_ADDRESS(u32Addr));

    pu32RO = (u32Addr - OTP_BASE) / 0x100;

    OTP->RO |= BIT(pu32RO);
}

/******************************************************************************
* Function Name  : OTP_SetProtectLock
* Description    : 设置OTP地址对应寄存器的只读锁的硬件锁，置1后软件无法清0，
                   复位后硬件自动清0
* Input          : u32Addr
* Return         : NONE
******************************************************************************/
void OTP_SetProtectLock(uint32_t u32Addr)
{
    uint32_t pu32ROL;
    assert_param(IS_OTP_ADDRESS(u32Addr));

    OTP_SetProtect(u32Addr);
    pu32ROL = (u32Addr - OTP_BASE) / 0x100;

    OTP->ROL |= BIT(pu32ROL);
}

/******************************************************************************
* Function Name  : OTP_UnProtect
* Description    : 设置对应OTP地址为可擦写
* Input          : u32Addr
* Return         : NONE
******************************************************************************/
void OTP_UnProtect(uint32_t u32Addr)
{
    uint32_t pu32RO;
    assert_param(IS_OTP_ADDRESS(u32Addr));

    pu32RO = (u32Addr - OTP_BASE) / 0x100;

    OTP->RO &= ~BIT(pu32RO);
}

/******************************************************************************
* Function Name  : OTP_IsReadReady
* Description    : 是否可以读操作，OTP处于编程/休眠状态时不可读
* Input          : void
* Return         : Boolean:TRUE/FALSE
******************************************************************************/
Boolean OTP_IsReadReady(void)
{
    if (BIT(31) == (OTP->CS & BIT(31))) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/******************************************************************************
* Function Name  : OTP_IsProtect
* Description    : 读取对应地址的只读锁是否为锁定状态
* Input          : u32Addr
* Return         : Boolean:TRUE/FALSE
******************************************************************************/
Boolean OTP_IsProtect(uint32_t u32Addr)
{
    uint32_t pu32RO;

    assert_param(IS_OTP_ADDRESS(u32Addr));

    pu32RO = (u32Addr - OTP_BASE) / 0x100;

    if (BIT(pu32RO) == (OTP->RO & BIT(pu32RO))) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/******************************************************************************
* Function Name  : OTP_IsProtectLock
* Description    : 读取对应地址的只读锁的硬件锁是否为锁定状态
* Input          : u32Addr
* Return         : Boolean:TRUE/FALSE
******************************************************************************/
Boolean OTP_IsProtectLock(uint32_t u32Addr)
{
    uint32_t pu32ROL;

    assert_param(IS_OTP_ADDRESS(u32Addr));

    pu32ROL = (u32Addr - OTP_BASE) / 0x100;

    if (BIT(pu32ROL) == (OTP->ROL & BIT(pu32ROL))) {
        return TRUE;
    } else {
        return FALSE;
    }
}


/******************************************************************************
* Function Name  : OTP_PowerOn
* Description    : 打开OTP电源, 上电后需要等2us以后才能读取OTP内容
* Input          :
* Return         :
******************************************************************************/
void OTP_PowerOn(void)
{
    //考虑OTP的时间余量, 实际延时比要求长很多
    uint32_t n = SYSCTRL->HCLK_1MS_VAL / 250;

    SYSCTRL->LDO25_CR &= ~BIT5; //打开OTP电源
    while (n--);
    SYSCTRL_AHBPeriphClockCmd(SYSCTRL_AHBPeriph_OTP, ENABLE);
}

/************************ (C) COPYRIGHT 2014 Megahuntmicro ****END OF FILE****/
