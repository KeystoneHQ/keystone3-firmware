#include "err_code.h"

static const ErrCodeDesc_t g_faults[] = {
    {SUCCESS_CODE, "Success"},
    {ERR_GENERAL_FAIL, "General Failed"},

    {ERR_DS28S60_OVERTIME, "DS28S60 Overtime"},
    {ERR_DS28S60_UNEXPECTLEN, "DS28S60 Unexpected Length"},
    {ERR_DS28S60_INFO, "DS28S60 Get Info Failed"},
    {ERR_DS28S60_AUTH, "DS28S60 Auth Failed"},
    {ERR_DS28S60_BIND, "DS28S60 Bind Failed"},

    {ERR_BYTEWORDS_LEN, "ByteWords Length Error"},
    {ERR_BYTEWORDS_WORD, "ByteWords Word Error"},
    {ERR_BYTEWORDS_CRC, "ByteWords CRC Error"},

    {ERR_URDECODE_HEAD, "URDecode Head Error"},
    {ERR_URDECODE_END, "URDecode End Error"},
    {ERR_URDECODE_LEN, "URDecode Length Error"},

    {ERR_TOUCHPAD_NODATA, "TouchPad No Data"},
    {ERR_TOUCHPAD_NOREG, "TouchPad No Reg"},

    {ERR_I2CIO_NOACK, "I2CIO No ACK"},

    {ERR_ATECC608B_UNEXPECT_LOCK, "ATECC608B Unexpected Lock"},
    {ERR_ATECC608B_UNEXPECT_UNLOCK, "ATECC608B Unexpected Unlock"},
    {ERR_ATECC608B_SLOT_NUM_ERR, "ATECC608B Slot Num Error"},
    {ERR_ATECC608B_BIND, "ATECC608B Bind Failed"},

    {ERR_KEYSTORE_AUTH, "Keystore Auth Failed"},
    {ERR_KEYSTORE_PASSWORD_ERR, "Keystore Password Error"},
    {ERR_KEYSTORE_REPEAT_PASSWORD, "Keystore Repeat Password Error"},
    {ERR_KEYSTORE_NOT_LOGIN, "Keystore Not Login"},
    {ERR_KEYSTORE_MNEMONIC_REPEAT, "Keystore Mnemonic Repeat"},
    {ERR_KEYSTORE_SAVE_LOW_POWER, "keystore save low power"},
    {ERR_KEYSTORE_SAVE_FP_LOW_POWER, "keystore save fp low power"},
    {ERR_KEYSTORE_MNEMONIC_INVALID, "Keystore Mnemonic Invalid"},
    {ERR_KEYSTORE_MNEMONIC_NOT_MATCH_WALLET, "Keystore Mnemonic not match"},
    {ERR_KEYSTORE_EXTEND_PUBLIC_KEY_NOT_MATCH, "Keystore extend public key not match"},

    {ERR_GD25_BAD_PARAM, "GD25 Bad Param"},
    {ERR_GD25_WEL_FAILED, "GD25 GET WEL State Failed"},
    {ERR_PSRAM_OPERATE_FAILED, "PSRAM operate Failed"},

    {ERR_GUI_ERROR, "GUI Error"},
    {ERR_GUI_UNHANDLED, "GUI Unhandled"},

    {ERR_SQLITE3_FAILED, "Sqlite3 Failed"},

    {ERR_SLIP39_DECODE_FAILED, "SLIP39 Decode Failed"},

    {ERR_SERIAL_NUMBER_NOT_EXIST, "Serial number not exist"},
    {ERR_SERIAL_NUMBER_INVALID, "Serial number invalid"},
    {ERR_SERIAL_NUMBER_ALREADY_EXIST, "Serial number already exist"},

    {ERR_WEB_AUTH_KEY_NOT_EXIST, "Web auth key not exist"},
    {ERR_WEB_AUTH_KEY_ALREADY_EXIST, "Web auth key already exist"},

    {ERR_UPDATE_PUB_KEY_NOT_EXIST, "Update pub key not exist"},
    {ERR_UPDATE_PUB_KEY_NO_SPACE, "Update pub key no space"},

    {ERR_UPDATE_FIRMWARE_NOT_DETECTED, "Update firmware not detected"},
    {ERR_UPDATE_SDCARD_NOT_DETECTED, "Update SD card not detected"},
    {ERR_UPDATE_CHECK_VERSION_FAILED, "Update check version failed"},
    {ERR_UPDATE_NO_UPGRADABLE_FIRMWARE, "Update no upgradable firmware"},
    {ERR_UPDATE_CHECK_FILE_EXIST, "Update check file exist"},
    {ERR_UPDATE_CHECK_CRC_FAILED, "Update check crc failed"},
    {ERR_UPDATE_CHECK_SIGNATURE_FAILED, "Update check signature failed"},

    {ERR_END, "Unknown Error"},
};

const char *GetErrorMessage(Error_Code errCode)
{
    if (errCode < SUCCESS_CODE || errCode >= ERR_END) {
        return "Unknown Error";
    }
    return g_faults[errCode].errDesc;
}