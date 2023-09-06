/*****************************************************************************
 * Filename : mhscpu_msr.h
 * Description  :
 * Author       : Jin yuanbing
 * ---------------------------------------------------------------------------
 * Modification Record:
 *   Version    Name            date        Content
 *      1.0     Jin yuanbing    2015-12-12  Create this file.
 *
 * Note:
 *
 * ---------------------------------------------------------------------------
 * Copyright (c) 2015 Mega Hunt Micro Technology Inc. All Rights Reserved.
 *****************************************************************************/

#ifndef MHSCPU_MSR_H
#define MHSCPU_MSR_H

#include "mhscpu.h"
//Max track numberic that MH1601 supported
#define MAX_TRACK_NUM   3


//#error("MSR_H");
#ifdef __cplusplus
extern "C" {
#endif

//Track select option
#define TRACK_SELECT_1  0x01
#define TRACK_SELECT_2  0x02
#define TRACK_SELECT_3  0x04
#define TRACK_SELECT_ALL    0x10        //Return read data that all specific track hava data.


//Detect swiping card,detect_swiping_card() return value.
#define DETECT_SWIPING_CARD     0x01
#define DETECT_NO_SWIPING_CARD  0x02
#define DETECT_TIMEOUT          0x03
#define DETECT_HARD_WAKEUP      0x04


//Read card data result
#define SUCCESS         0
#define     INVALID_ADDR        1       //track1 or track2 or track3 address invalid
#define PARITY_ERR          2       //ODD error or LRC error
#define LENGTH_ERR          3       //ODD error or LRC error
#define TRACK_NO_DATA       4       //part track have no data that must readed.
#define HAVE_NO_ZERO        5


#define MAX_TRACK_DATA_SIZE 128
#define     MAX_TRACK_RAW_DATA_SIZE  120

//Set wakeup status
#define     SOFT_WAKEUP 0
#define HARD_WAKEUP 1
#define     HARD_WAKEUP_WITHOUT_SLEEP   2

#define     MAX_SUPPORT_CARD_TYPES  CARD_TYPE_OTHER


typedef enum track_parity_e {
    CARD_PARITY_ODD,
    CARD_PARITY_EVEN,
    CARD_PARITY_NONE
} track_parity;

//Track parameter
typedef struct track_param_s {
    struct {
        uint8_t     head;           //track start sentinel
        uint8_t     tail;               //tack end sentinel
        uint8_t     char_len;       //track character length
        track_parity parity;    //track character parity
    } fmt;
    uint8_t         weight;         //track format weight
} track_param;


typedef struct track_data_s {
    uint8_t len;
    uint8_t buf[MAX_TRACK_DATA_SIZE];
} track_data;


//Swipe direction
typedef enum swiping_direction_e {
    SD_BIDIRECTION          = 0,            //bidirection
    SD_FORWARD_DIRECTION    = 1,
    SD_REVERSE_DIRECTION    = 2
} swiping_direction;


//Card type, detect card type when swipe card.
typedef enum card_type_e {
    CARD_TYPE_ISO_ABA = 0,
    CARD_TYPE_AAMVA = 1,
    CARD_TYPE_IBM = 2,
    CARD_TYPE_IBM_755 = 3,
    CARD_TYPE_JIS = 4,
    CARD_TYPE_777 = 5,
    CARD_TYPE_CADMV = 6,
    CARD_TYPE_OTHER = 7,
    CARD_TYPE_UNKNOW = 0xFF
} card_type;


typedef struct pan_s {
    char v[20];     //Max length of card data is 19B, and 1B end flag('\0')
} pan;


/*****************************************************************************
 * Name     : msr_get_version
 * Function :
 * ---------------------------------------------------------------------------
 * Input Parameters:none
 * Output Parameters:None
 * Return Value: none
 * ---------------------------------------------------------------------------
 * Description:
 * BIT31~BIT24  ---Major_Ver
 * BIT23~BIT16  ---Minor_Ver
 * BIT15~BIT0   ---ReVer
 *****************************************************************************/
uint32_t msr_get_version(void);


/*****************************************************************************
 * Name     : init_dpu
 * Function : initial EMH DPU module.
 * ---------------------------------------------------------------------------
 * Input Parameters:none
 * Output Parameters:None
 * Return Value: none
 * ---------------------------------------------------------------------------
 * Description:
 *
 *****************************************************************************/
void init_dpu(void);

/*****************************************************************************
 * Name     : set_wakeup_status
 * Function : change wakeup type.
 * ---------------------------------------------------------------------------
 * Input Parameters:
 *          status  wakeup state, SOFT_WAKEUP or HARD_WAKEUP.
 * Output Parameters:None
 * Return Value: none
 * ---------------------------------------------------------------------------
 * Description:
 *
 *****************************************************************************/
void set_wakeup_status(uint8_t status);



/*****************************************************************************
 * Name     : detect_swiping_card
 * Function : Detect whether there is a swiping card action.
 * ---------------------------------------------------------------------------
 * Input Parameters:none
 * Output Parameters:None
 * Return Value:
 *          DETECT_SWIPING_CARD     Have swiping card action.
 *          DETECT_NO_SWIPING_CARD  Haven't swiping card action.
 *          DETECT_HARD_WAKEUP      May be set hard wakeup.
 * ---------------------------------------------------------------------------
 * Description:
 *      if return DETECT_HARD_WAKEUP, will must calling set_hard_wakeup() when used hard wakeup.
 *****************************************************************************/
uint8_t detect_swiping_card(void);


/*****************************************************************************
 * Name     : get_decode_data
 * Function : Get decode data that specified track.
 * ---------------------------------------------------------------------------
 * Input Parameters:
 *          option          track select option, bit 0, 1, 2 to track 1, 2, 3. bit 4 to all track.
 * Output Parameters:
 *          ptrack1         track 1,2 and 3 buffer, at least have 3 items.
 *          cfmt            card format
 *                          0       CARD_TYPE_ISO_ABA,
 *                          1       CARD_TYPE_AAMVA,
 *                          2       CARD_TYPE_IBM,
 *                          3       CARD_TYPE_IBM_755,
 *                          4       CARD_TYPE_JIS,
 *                          5       CARD_TYPE_777,
 *                          6       CARD_TYPE_CADMV,
 *                          7       CARD_TYPE_OTHER,
 *                          0xFF    CARD_TYPE_UNKNOW
 *
 *          tflag           bit0,1,2 indicates Track1,2,3 decoding state
 *                          bit3,4,5 indicates Track1,2,3 sampling state
 * Return Value:
 *          SUCCESS     decoding successful finished.
 *          INVALID_ADDR    track buffer address is invalid.
 *          PARITY_ERR      parity error.
 *          LENGTH_ERR      length exceed max length.
 *          TRACK_NO_DATA   part track have no data that must readed
 * ---------------------------------------------------------------------------
 * Description:
 *      Before returned, will automatic configure DPU soft wake.
 *****************************************************************************/
uint8_t get_decode_data(track_data  *ptrack, uint8_t options, uint8_t *cfmt, uint8_t *tflag);


/*****************************************************************************
 * Name     : clear_dpu_int
 * Function : Clear DPU interrupt and discard the swiping card data
 * ---------------------------------------------------------------------------
 * Input Parameters:none
 * Output Parameters:none
 * Return Value: none
 * ---------------------------------------------------------------------------
 * Description:
 *      Before returned, will automatic configure DPU soft wake.
 *****************************************************************************/
void clear_dpu_int(void);


/*****************************************************************************
 * Name     : get_swipe_dir
 * Function : Get current swiping direction.
 * ---------------------------------------------------------------------------
 * Input Parameters:none
 * Output Parameters:None
 * Return Value:
 *      Return current used swiping direction.
 * ---------------------------------------------------------------------------
 * Description:
 *
 *****************************************************************************/
swiping_direction get_swipe_dir(void);


/*****************************************************************************
 * Name     : set_swipe_dir
 * Function : Set swiping direction.
 * ---------------------------------------------------------------------------
 * Input Parameters:
 *          dir     swipe direction
 * Output Parameters:None
 * Return Value: none
 * ---------------------------------------------------------------------------
 * Description:
 *
 *****************************************************************************/
void set_swipe_dir(swiping_direction dir);


/*****************************************************************************
 * Name     : get_track_raw_data
 * Function : Get special track raw data, nothing to do within get operation.
 * ---------------------------------------------------------------------------
 * Input Parameters:
 *      track_idx       track index, 0~2 for track 1~3
 * Output Parameters:
 *      buf         save track raw data buffer address
 * Return Value:
 *
 * ---------------------------------------------------------------------------
 * Description:
 *      buf         Must be save 256B.
 *****************************************************************************/
int get_track_raw_data(uint8_t  *buf, uint8_t track_idx);


/*****************************************************************************
 * Name     : sc_sleep
 * Function : Go to swipe card sleep
 * ---------------------------------------------------------------------------
 * Input Parameters:None
 * Output Parameters:None
 * Return Value: none
 * ---------------------------------------------------------------------------
 * Description:
 *
 *****************************************************************************/
void sc_sleep(void);


/*****************************************************************************
 * Name     : set_hard_wakeup_level
 * Function : Config the register of hard wakeup level.
 * ---------------------------------------------------------------------------
 * Input Parameters:
 *          level       Level of hard wakeup, valid is 0~7, min is 0, max is 7.
 * Output Parameters:None
 * Return Value: none
 * ---------------------------------------------------------------------------
 * Description:
 *
 *****************************************************************************/
void set_hard_wakeup_level(uint8_t level);

/*****************************************************************************
 * Name     : msr_powerdown
 * Function : MSR powerdown.
 * ---------------------------------------------------------------------------
 * Input Parameters: none
 * Output Parameters:None
 * Return Value: none
 * ---------------------------------------------------------------------------
 * Description:
 *
 *****************************************************************************/
void msr_powerdown(void);

/*****************************************************************************
 * Name     : msr_powerdown
 * Function : MSR powerup.
 * ---------------------------------------------------------------------------
 * Input Parameters: none
 * Output Parameters:None
 * Return Value: none
 * ---------------------------------------------------------------------------
 * Description:
 *
 *****************************************************************************/
void msr_powerup(void);


/*****************************************************************************
 * Name     : reset_adc
 * Function : restart ADC.
 * ---------------------------------------------------------------------------
 * Input Parameters: none
 * Output Parameters:None
 * Return Value: none
 * ---------------------------------------------------------------------------
 * Description:
 * If msr_powerup is operated, this must be called after Delay(100ms)
 *****************************************************************************/
void reset_adc(void);



/*****************************************************************************
 * Name     : clear_dpu_state
 * Function : clear SW Status.
 * ---------------------------------------------------------------------------
 * Input Parameters: none
 * Output Parameters:None
 * Return Value: none
 * ---------------------------------------------------------------------------
 * Description:
 *****************************************************************************/
void clear_dpu_state(void);

#ifdef __cplusplus
}
#endif

#endif




