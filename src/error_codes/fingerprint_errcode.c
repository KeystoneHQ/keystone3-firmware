#include "define.h"
#include "err_code.h"
#include "fingerprint_process.h"

static const ErrCodeDesc_t g_fpFaults[] = {
    {FP_SUCCESS_CODE, "Success"},
    {ERR_FP_NO_FINGER_PRESS, "No finger press"},

    {ERR_FP_LIFT_FINGER_TOO_FAST, "Please lift your finger only after\neach vibration feedback."},
    {ERR_FP_FINGER_LEAVE, "Finger leave"},

    {ERR_FP_NUMBER_OF_FINGER_ZERO, "Number of finger zero"},
    {ERR_FP_ALGORITHMIC_PROCESSING_FAILURE, "Algorithmic processing failure"},

    {ERR_FP_AREA_TOO_WET, "Moisture detected. Please wipe your finger\nand try again."},
    {ERR_FP_RECORDING_AREA_SMALL, "Partial fingerprint detected. Please try\nagain"},
    {ERR_FP_ARE_POOR_QUALITY, "Fingerprint unclear. Please restart the\nprocess and use a different finger to try\nagain."},
    {ERR_FP_RECORDING_AREA_OVERLAPS_TOO_MUCH, "Duplicate area, please adjust your\nfinger's position slightly"},
    {ERR_FP_TOO_MUCH_WEIGHT_ON_RECORDING_AREA, "Too much weight on recording area"},

    {ERR_FP_TAKEN_YOUR_FINGER_OFF_IT, "Taken your finger off it"},
    {ERR_FP_FIRMWARE_UPGRADE_FAILURE, "Firmware upgrade failure"},
    {ERR_FP_FIRMWARE_OVERSIZE, "Firmware oversize"},
    {ERR_FP_TEMPLATE_SAVING_ERROR, "Template saving error"},
    {ERR_FP_ENROLL_ERROR, "Enroll error"},
    {ERR_FP_ENROLL_FINISH_ERROR, "Enroll finish error"},
    {ERR_FP_GENERAL_FAIL, "General fail"},

    {ERR_FP_FLASH_MESSAGE_FAILURE, "Flash message failure"},
    {ERR_FP_NO_TEMPLATES_WERE_MATCHED, "No templates were matched"},
    {ERR_FP_NON_EXISTENT_FINGERPRINT, "Non existent fingerprint"},
    {ERR_FP_AES_KEY_ALREADY_EXISTS, "AES key already exists"},

    {ERR_FP_REPEAT_FINGER, "Duplicate finger, Please change another\nfinger"},
};

const char *GetFpErrorMessage(FingerError_Code errCode)
{
    if (errCode < FP_SUCCESS_CODE || errCode >= ERR_FP_END) {
        return "Unknown Error";
    }
    for (int i = 0; i < NUMBER_OF_ARRAYS(g_fpFaults); i++) {
        if (g_fpFaults[i].errCode == errCode) {
            return g_fpFaults[i].errDesc;
        }
    }
    return "Unknown Error";
}