#ifndef EMV_HARD_H
#ifdef __cplusplus
extern "C" {
#endif
#define EMV_HARD_H

#include "emv_core.h"

#define  EMV_TERMINAL_NUM       3


#define SYS_LINUX       (  1  )  /* driver for linux */
#define SYS_NONE        (  2  )  /* driver for monitor */
#define SYS_PLATFORM    SYS_NONE
#define EMV_STD         (  1  )



#define  SM_VERSION_SN          "112"
#define  SM_VERSION_FLAG        "R"
#define  HAS_HIGH_SAM_PATCH     0
#define  MAX_SPEED_LTD          1


#define MH1901_L    (0x112)
#define TARGET_MAC MH1901_L
#define MAC_MODEL  MH1901_L

#define MAC_NAME   "MH1901_L"

#define SM_VERSION_DATE "160604"

/**
 * these following functions is primary for implementing protocol
 * they are implemented in "xxx_emv_hard.c" and "xxx_emv_hard.h".
 * they provide implemetations to apis layer, in "emv_apis.c" or
 * "iso7816_3.c".
 * these data-structures( e.g. struct emv_core ) are defined in
 * "emv_core.h".
 *
 * provide hardware code in "xxx_emv_hard.c" version information.
 */
// char* emv_hard_info( char *emv_hard_info );

/**
 * tell channel is whether valid or not?
 */
int isvalid_channel(int slot);

/**
 * tell slot but not hardware convert
 */
int select_slot(int slot);

/**
 * implement hardware initializing.
 */
int   emv_hard_init(void);

/**
 * release device interruption, avoid to exception after rmmod module.
 */
// int emv_hard_release( void );

/**
 * enable/disable insert/extract card interrupts, avoid disturbing
 * SAM card communication.
 */
int   emv_disturb_interruption(struct emv_core *pdev, int enable);

/**
 * power on with card assigned
 */
int   emv_hard_power_pump(struct emv_core *pdev);

/**
 * executing cold reset with card assigned order by iso7816-3
 */
int   emv_hard_cold_reset(struct emv_core *pdev);

/**
 * executing warn reset with card assigned order by iso7816-3
 */
int   emv_hard_warm_reset(struct emv_core *pdev);

/**
 * executing deactivation with card assigned order by iso7816-3
 */
int   emv_hard_power_dump(struct emv_core *pdev);

/**
 * communicating with card assigned order by iso7816-3
 */
int   emv_hard_xmit(struct emv_core *pdev);

/**
 * detect card with assigned card whether is in socket or not?
 */
int   emv_hard_detect(struct emv_core *pdev);

/**
 * return insert or extract card count.
 */
int   emv_alarm_count(void);

#ifdef __cplusplus
}
#endif
#endif
