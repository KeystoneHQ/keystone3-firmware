#ifndef ISO7816_3_H
#ifdef __cplusplus
extern "C" {
#endif
#define ISO7816_3_H

/**
 * ======================== APDU structure =====================================
 */
/**
 * You must keep its with "osal.h"
 */
typedef struct {
    unsigned char   cmd[4];
    int             lc;
    unsigned char   data_in[512];
    int             le;
} ST_APDU_REQ;

typedef struct {
    int             len_out;
    unsigned char   data_out[512];
    unsigned char   swa;
    unsigned char   swb;
} ST_APDU_RSP;

/**
 * ============================ interfacces declarations ================================
 */
int iso7816_device_init(void);

#define VCC_MASK    ( 3 )
#define VCC_5000mV  ( 3 )
#define VCC_3000mV  ( 2 )
#define VCC_1800mV  ( 1 )
#define VCC_1200mV  ( 0 )
#define VCC_CONFIG( x, v )  ( x ) &= ~( VCC_MASK );\
                            ( x ) |=  ( v )

#define PPS_SUPPORT ( 4 )
#define PPS_CONFIG( x, s )  ( x ) &= ~( PPS_SUPPORT );\
                            ( x ) |=  ( s )

#define ENFORCE_PPS_SUPPORT     ( 64 )  //bit6


#define SPD_MASK    ( 24 )
#define SPD_1X      ( 0  )
#define SPD_2X      ( 8  )
#define SPD_4X      ( 16 )
#define SPD_CONFIG( x, p )  ( x ) &= ~( SPD_MASK );\
                            ( x ) |=  ( p )

#define ISO_SPEC    ( 32 )
#define EMV_SPEC    ( 0  )
#define SPEC_CONFIG( x, s ) ( x ) &= ~( ISO_SPEC );\
                            ( x ) |=  ( s )
int  iso7816_init(unsigned int slot,
                  unsigned int option,
                  unsigned char *out_atr
                 );
#define AUTO_GET_RSP  ( 1 )
#define NON_AUTO_RSP  ( 0 )
int  iso7816_exchange(unsigned int slot,
                      int mode,
                      ST_APDU_REQ *apdu_req,
                      ST_APDU_RSP *apdu_rsp
                     );
void iso7816_close(unsigned int slot);
int  iso7816_detect(unsigned int slot);
int  iso7816_poweroff(void);
int iso7816_getlibversion(void);
/**
 * Memory card interfaces
 */
int Mc_Clk_Enable(unsigned char slot, unsigned char mode);
int Mc_Io_Dir(unsigned char slot, unsigned char mode);
int Mc_Io_Read(unsigned char slot);
int Mc_Io_Write(unsigned char slot, unsigned char mode);
int Mc_Vcc(unsigned char slot, unsigned char mode);
int Mc_Reset(unsigned char slot, unsigned char mode);
int Mc_Clk(unsigned char slot, unsigned char mode);
int Mc_C4_Write(unsigned char slot, unsigned char mode);
int Mc_C8_Write(unsigned char slot, unsigned char mode);
int Mc_C4_Read(unsigned char slot);
int Mc_C8_Read(unsigned char slot);
int Read_CardSlotInfo(unsigned char slot);
int Write_CardSlotInfo(unsigned char slot, unsigned char slotopen);

#ifdef __cplusplus
}
#endif
#endif
