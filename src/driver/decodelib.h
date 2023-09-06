#ifndef _DECODELIB_H_
#define _DECODELIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"
#include "stdlib.h"
#include "mhscpu.h"

/************************************************************************************************/
/*图像翻转类型设置*/
typedef enum {
    FLIP_NORMAL,    //原始图像
    FLIP_VER,       //垂直翻转
    FLIP_HOZ,       //水平翻转
    FLIP_VERHOZ     //垂直水平翻转，即原始图像旋转180度
} SensorImageFlipType;

/************************************************************************************************/
/*解码CodeID*/
typedef enum {
    ID_NONE = 0,        /*未解到码*/
    ID_CODE128,         /*code128*/
    ID_CODE39,          /*code39*/
    ID_CODE93,          /*code93*/
    ID_EAN13,           /*EAN 13*/
    ID_EAN8,            /*EAN 8*/
    ID_UPC_A,           /*UPC_A*/
    ID_UPC_E0,          /*UPC_E0*/
    ID_UPC_E1,          /*UPC_E1*/
    ID_ISBN13,          /*ISBN13*/
    ID_ITF25,           /*交叉二五码*/
    ID_QRCODE,          /*QRCode*/
    ID_PDF417,          /*PDF417*/
    ID_DATAMATRIX       /*DataMatrix*/
} DecodeIDType;
/*解码数据结构体*/
typedef struct {
    DecodeIDType id;    /*译码结果id*/
    uint8_t AIM[4];     /*AIM ID*/
    uint8_t *result;    /*保存解码结果数组首地址*/
    uint32_t resn;      /*返回解码结果个数*/
    uint32_t maxn;      /*保存解码结果最大个数*/
} DecodeResultTypeDef;
/************************************************************************************************/
/*解码初始化配置结构体*/
typedef struct {
    void            *pool;                  /*内存池首地址 */
    unsigned int    size;                   /*内存池大小   大于等于 410 * 1024kbyte,栈空间大于等于4k */
    GPIO_TypeDef*   CAM_PWDN_GPIOx;         /*camera PWDN gpio */
    uint16_t        CAM_PWDN_GPIO_Pin;      /*camera PWDN gpio pin */
    GPIO_TypeDef*   CAM_RST_GPIOx;          /*camera RESET gpio */
    uint16_t        CAM_RST_GPIO_Pin;       /*camera RESET gpio pin */
    I2C_TypeDef*    CAM_I2Cx;               /*连接摄像头使用的I2C */
    uint32_t        CAM_I2CClockSpeed;      /*I2C时钟速率设置，速率范围[100000, 400000]*/
    uint8_t (*SensorConfig)[2];             /*自定义摄像头设置数组首地址，为NULL时使用默认配置参数*/
    uint32_t        SensorCfgSize;          /*自定义摄像头设置数组大小*/
} DecodeInitTypeDef;

/*解码库初始化返回状态信息*/
typedef enum {
    DecodeInitSuccess,              /*解码库初始化成功*/
    DecodeInitCheckError,           /*解码库校验失败，请联系芯片厂商*/
    DecodeInitMemoryError,          /*给解码库的buff过小*/
    DecodeInitSensorError,          /*摄像头初始化失败*/
} DecodeFlagTypeDef;

/*DeocdeStart 错误返回值状态信息*/
enum {
    DecodeResOverflowError = -1,    /*解码结果超过buff大小或者buff大小小于256byte*/
    DecodeImageNoDoneError = -2,    /*图片未采集完成*/
    DecodeLibNoInitError   = -3,    /*解码库未初始化*/
};
/*解码配置结构体*/
typedef struct {
    /*一维码配置*/
    uint32_t cfgCODE128;            /*code 128解码设置      */
    uint32_t cfgCODE39;             /*code 39解码设置       */
    uint32_t cfgCODE93;             /*code 93解码设置       */
    uint32_t cfgEAN13;              /*EAN13 解码设置        */
    uint32_t cfgEAN8;               /*EAN8 解码设置         */
    uint32_t cfgUPC_A;              /*UPC_A 解码设置        */
    uint32_t cfgUPC_E0;             /*UPC_E0 解码设置(UPC_E前置码为0)*/
    uint32_t cfgUPC_E1;             /*UPC_E1 解码设置(UPC_E前置码为1)*/
    uint32_t cfgISBN13;             /*ISBN13 解码设置       */
    uint32_t cfgInterleaved2of5;    /*交叉二五码解码设置    */
    /*二维码配置*/
    uint32_t cfgQRCode;             /*qr code 解码设置      */
    uint32_t cfgPDF417;             /*pdf 417 解码设置      */
    uint32_t cfgDataMatrix;         /*data matrix 解码设置  */
    /*全局解码设置*/
    uint32_t cfgGlobal;             /*解码全局设置*/
} DecodeConfigTypeDef;

/*配置状态*/
typedef enum {
    DECODE_DISABLE = 0,                 /*失能配置*/
    DECODE_ENABLE = !DECODE_DISABLE     /*使能当前配置*/
} DecodeStatus_t;

/*条码解码状态配置*/
typedef enum {
    DE_STATUS  = 0,             /*0bit 条码状态位 包括一维码，二维码，此位均是使能与失能*/
    DE_EMIT_CHECK = 1,          /*1bit 使能条码输出校验位 (除code128,所有条码)*/
    DE_ADD_CHECK = 2,           /*2bit 使能条码校验       (code39, Interleaved2of5)*/
    //code128
    DE_CODE128_ENHANCE = 7,     /*7bit 使能code128识读增强，主要提升code128条空比不标准条码*/
    //code39
    DE_ASCII     = 3,           /*3bit 使能条码full ASCII模式输出 (code39)*/
    DE_EMIT_START = 6,          /*6bit 使能code39起始符输出('*')*/
    DE_EMIT_END   = 7,          /*7bit 使能code39结束符输出('*')*/
    //EAN UPC
    DE_UPC2EAN   = 4,           /*4bit UPC_A转EAN13输出使能(UPC_A)*/
    DE_MUSTADDN  = 5,           /*5bit 必须输出带附加码条码数据 (EAN13,UPC_A,EAN8,UPC_E0,UPC_E1)*/
    DE_EANADDON2 = 6,           /*6bit 输出2位附加码 (EAN13,UPC_A,EAN8,UPC_E0,UPC_E1)*/
    DE_EANADDON5 = 7,           /*7bit 输出5位附加码 (EAN13,UPC_A,EAN8,UPC_E0,UPC_E1)*/

    DE_MIN_LEN   = 16,          /*16bit-23bit 共8bit 解码最小长度*/
    DE_MAX_LEN   = 24           /*24bit-31bit 共8bit 解码最长长度*/
} DeBarCodeStatus_t;

/*QRCode解码状态配置*/
typedef enum {
    DEQR_STATUS        = 0,     /*0bit QRCode解码状态使能位*/
    DEQR_MISSINGCORNER = 1,     /*1bit QRCode缺角算法使能*/
    DEQR_CURVE         = 3,     /*3bit QRCode曲面解码算法使能*/
    DEQR_ECICONFIG     = 7,     /*16bit QRCode输出ECI开关*/
} DeQRCodeStatus_t;

/*条码解码全局设置*/
typedef enum {
    DEBAR_PRECISION   = 0,      /*0bit 条码精度码解码配置位*/
    DECODE_ANTICOLOR  = 1,      /*1bit 支持反色码解码配置位*/
    DEBAR_DAMAGE      = 2,      /*2bit 条码污损划线解码配置位*/
    DECODE_MIRROR     = 3,      /*3-4bit 条码（DM）镜像设置配置0/1为解正常码，2为镜像码，3为两种码都可以解*/
} DecodeGlobalStatus_t;
/****************************************************************************************************/
/**
 * 码配置结构体初始化函数
 *
 */
void DecodeConfigInit(DecodeConfigTypeDef *cfg);
/**
 * BCTC认真测试推荐配置结构体初始化函数
 *
 */
void BCTCTestConfigInit(DecodeConfigTypeDef *cfg);
/****************************************************************************************************/
/**
 *外设初始化及内存池初始化
 * 初始化结构体
 *返回值为 1 代表初始化成功，0 代表初始化失败
 */
DecodeFlagTypeDef DecodeInit(DecodeInitTypeDef* DecodeInitStruct);

/**
 *  开始解码函数
 *  返回值为解码结果个数
 *  返回 0 解码失败
 *       DecodeResOverflowError 代表解码结果超过给定的buffer范围，或者给定解码buff小于256byte
 *       DecodeImageNoDoneError 采图未完成
 *       DecodeLibNoInitError 未初始化或者初始化失败
 */
int DecodeStart(DecodeConfigTypeDef *cfg, DecodeResultTypeDef *res);

/**
 * 使能DCMI采图
 *
 *
 */
void DecodeDcmiStart(void);

/**
 * 判断采图是否完成
 * 必须在完成中断调用此函数(DCMI_CallBackFrame())
 * 返回值 0未采集完成 1 采集完成，非必须调用函数
 */
int DecodeDcmiFinish(void);

/**
 * 清除全部解码buff标志，以让当前保存图像buff数据失效
 *
 */
void CleanDecodeBuffFlag(void);

/**
 * 获取当前解码buff是否都是空闲状态
 * 返回FALSE表示当前所有图像buff都为无效状态，需要重新采图
 * 返回TRUE表示当前图像buff至少有一块buff不是无效状态
 */
Boolean GetDecodeBuffFlag(void);

/**
 * DCMI_IT_FRAME 中断回调函数
 */
void DCMI_CallBackFrame(void);

/**
 * 设置图像翻转
 *
 */
void SetSensorImageFlip(SensorImageFlipType fliptype);

/**
 * 释放解码所需要的软硬件资源
 *
 */
void CloseDecode(void);

/**
  * 获取当前算法版本号
  * 16-31bit为主版本号，8-15bit为次版本号，0-7bit为修正版本号
  */
uint32_t GetDecodeLibVerison(void);


/**
  * Get Gray array of image,640*480
  *
  * const char * const GetImageBuffAddr(void)`返回的是图片地址，从此地址开始的640 * 480数据，only y的图；
  * 只有当当前图像采集完成后并此次还没有进入解码调用此函数有效，此时返回图像首地址，其余状态则返回为NULL；
  * 调用此函数需先进行声明: const char * const GetImageBuffAddr(void);
  *
  */
char *GetImageBuffAddr(void);

#ifdef __cplusplus
}
#endif

#endif
