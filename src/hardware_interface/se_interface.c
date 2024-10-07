#include "se_interface.h"
#include "drv_atecc608b.h"
#include "drv_ds28s60.h"
#include "user_utils.h"
#include "cryptoauthlib.h"
#include "assert.h"
#include "drv_trng.h"
#include "drv_mpu.h"

//START: Atecc608b
int32_t SE_EncryptWrite(uint8_t slot, uint8_t block, const uint8_t *data)
{
    //TODO: deal with error;
    MpuSetOtpProtection(false);
    int32_t ret = Atecc608bEncryptWrite(slot, block, data);
    MpuSetOtpProtection(true);
    return ret;
}

int32_t SE_Kdf(uint8_t slot, const uint8_t *authKey, const uint8_t *inData, uint32_t inLen, uint8_t *outData)
{
    //TODO: deal with error;
    MpuSetOtpProtection(false);
    int32_t ret = Atecc608bKdf(slot, authKey, inData, inLen, outData);
    MpuSetOtpProtection(true);
    return ret;
}

int32_t SE_DeriveKey(uint8_t slot, const uint8_t *authKey)
{
    //TODO: deal with error;
    MpuSetOtpProtection(false);
    int32_t ret = Atecc608bDeriveKey(slot, authKey);
    MpuSetOtpProtection(true);
    return ret;
}
//END

//START: DS28S60
int32_t SE_HmacEncryptRead(uint8_t *data, uint8_t page)
{
    int32_t ret = 0;
    MpuSetOtpProtection(false);
    ret = DS28S60_HmacEncryptRead(data, page);
    MpuSetOtpProtection(true);
    return ret;
}

int32_t SE_GetDS28S60Rng(uint8_t *rngArray, uint32_t num)
{
    int32_t ret = 0;
    MpuSetOtpProtection(false);
    ret = DS28S60_GetRng(rngArray, num);
    MpuSetOtpProtection(true);
    return ret;
}

void SE_GetTRng(void *buf, uint32_t len)
{
    return TrngGet(buf, len);
}

int32_t SE_GetAtecc608bRng(uint8_t *rngArray, uint32_t num)
{
    int32_t ret = 0;
    MpuSetOtpProtection(false);
    ret = Atecc608bGetRng(rngArray, num);
    MpuSetOtpProtection(true);
    return ret;
}

int32_t SE_HmacEncryptWrite(const uint8_t *data, uint8_t page)
{
    int32_t ret = 0;
    MpuSetOtpProtection(false);
    ret = DS28S60_HmacEncryptWrite(data, page);
    MpuSetOtpProtection(true);
    return ret;
}

//END