#ifndef EMV_CORE_H
#ifdef __cplusplus
extern "C" {
#endif
#define EMV_CORE_H

//#include "mac_params.h"



/* definitions for parameters used by driver */
#define MAX_QBUF_SIZE   ( 512 )
#define MAX_APDU_SIZE   ( 256 )

#define T1_REPEAT_LMT   ( 4   )
#define INCONV_CH       ( 0x3F )
#define CONV_CH         ( 0x3B )

#define SET_BIT( x, y ) (x) |=  ( 1 << (y) )
#define CLR_BIT( x, y ) (x) &= ~( 1 << (y) )
#define TST_BIT( x, y ) ( (x) & ( 1 << (y) ) )

#define IMR_BIT          ( 5 )
#define ISN_BIT          ( 6 )
#define RSN_BIT          ( 4 )
#define SRS_BIT          ( 5 )

#define CMP_IRSN_EQU( i, r )    ( ( ( (i) & ( 1 << ISN_BIT ) ) >> 2 ) == ( ( r ) & ( 1 << RSN_BIT ) ) )
#define ISN_INC( x )            ( (x) ^ ( 1 << ISN_BIT ) )
#define ISN_DEC( x )            ISN_INC( x )

#define ISN_RSN( x )    ( ( (x) & ( 1 << ISN_BIT ) ) >> 2 )
#define RSN_ISN( x )    ( ( (x) & ( 1 << RSN_BIT ) ) << 2 )

/*communication buffer*/
#define INVALID_INDEX ( -1 )
struct emv_queue {
    volatile unsigned char qbuf[ MAX_QBUF_SIZE ];
    volatile int           sf;/*the first status word*/
    volatile int           pf;/*the index of the first status word*/
    volatile int           ip;/*the input index of queue*/
    volatile int           op;/*the output index of queue*/
};

/*the core data structure for EMV and ISO7816-3*/
struct emv_core {
    int           terminal_ch;/*hardware channel*/

    int           terminal_exist;/*card existence*/
    int           terminal_open;/*device operating*/
    int           terminal_mode;/*Work Mode: 1 - Synchronize Mode; 0 - Asynchronize Mode*/

    int           terminal_state;/*current card will be or is status*/
#define EMV_IDLE        ( 1 )
#define EMV_COLD_RESET  ( 2 )
#define EMV_WARM_RESET  ( 3 )
#define EMV_READY       ( 4 )
    int           terminal_vcc;/*voltage, by mV*/
    int           terminal_auto;/*auto-send GET RESPONSE Command for T = 0*/
    int           terminal_pps;
    int           terminal_enforce_pps;
    int           terminal_spec;

    int           terminal_conv;/*conversion logical : 0 - direction; 1 - reverse direction*/
    int           terminal_ptype;/*the type of protocol: 1 - T = 1; 0 - T = 0*/
    int           terminal_fi;
    int           terminal_di;

    int           terminal_implict_fi;/*defined by ISO7816-3 in TA2.*/
    int           terminal_implict_di;

    unsigned int  terminal_cgt;
    unsigned int  terminal_igt;

    unsigned int  terminal_bwt;
    unsigned int  terminal_wwt;
    unsigned int  terminal_cwt;

    int           terminal_ifs;
    int           emv_card_ifs;

    int           allow_stop_clock;     /* 0 - not support; 1 - stop low; 2 - stop high; 3 - no preference */
    int           allow_ops_condition;  /* 0 - A; 1 - B; 2 - C; 3 - A & B; 4 - B & C; 5 - A, B & C */
    int           card_use_spu;         /* 0 - not use; 1 - proprietary; other value - voltage */

    unsigned char emv_card_nad;
    unsigned char emv_repeat;

    unsigned char terminal_ipcb;
    unsigned char terminal_pcb;

    struct emv_queue *queue;/*the pointer of data buffer*/

    unsigned char emv_card_ipcb;
    unsigned char emv_card_pcb;
    unsigned char terminal_enforce_pps_param;
};

/*the ATR data structure*/
struct emv_atr {
    unsigned char       ts;
    unsigned char       t0;
    unsigned char       ta_flag;
    unsigned char       tb_flag;
    unsigned char       tc_flag;
    unsigned char       td_flag;
    unsigned char       ta[ 8 ];
    unsigned char       tb[ 8 ];
    unsigned char       tc[ 8 ];
    unsigned char       td[ 8 ];
    unsigned char       hbytes[ 15 ];
};

/*the Command-Response pair data structure under TAL*/
struct emv_apdu_req {
    unsigned char cmd[ 4 ];
    int           lc;
    int           le;
    unsigned char data[ MAX_APDU_SIZE ];
};

struct emv_apdu_rsp {
    int           len;
    unsigned char swa;
    unsigned char swb;
    unsigned char data[ MAX_APDU_SIZE + 2 ];/*more than 2 bytes for receiving block
                                              in t=1 to process swa/swb.*/
};

/*the temperory data structure when process in T=1*/
struct emv_t1_block {
    unsigned char nad;
    unsigned char pcb;
    unsigned char len;
    unsigned char data[ MAX_APDU_SIZE ];
    unsigned char lrc;
};

/* ===================================================================
 * ����Ӧ�ò�Э�������������
 * -------------------------------------------------------------------
 * ������
 * -------------------------------------------------------------------
 * [����]apdu_req : APDU����ṹ��ָ��
 * -------------------------------------------------------------------
 * ���أ�
 * -------------------------------------------------------------------
 *        1 - CASE1
 *        2 - CASE2
 *        3 - CASE3
 *        4 - CASE4
 * ===================================================================
 */
int     emv_tell_case(struct emv_apdu_req *apdu_req);

/* ===================================================================
 * ����ISO7816�淶��ȡ��Ƭ��ATR
 * -------------------------------------------------------------------
 * ������
 * -------------------------------------------------------------------
 * [����]terminal  : �߼��豸��Ϣ
 * -------------------------------------------------------------------
 * [���]su_atr    : ��Ƭ��ATR��Ϣ
 * -------------------------------------------------------------------
 * [���]atr       : ��Ƭ��ATR��Ϣ��ʽ���ַ���
 * -------------------------------------------------------------------
 * ���أ�
 * -------------------------------------------------------------------
 *    ����Ĵ������ĵ�"emv_errno.h"
 * ===================================================================
 */
int     emv_atr_analyser(struct emv_core *terminal, struct emv_atr *su_atr, unsigned char *atr);

/* ===================================================================
 * ����EMV�淶������ȡ����ATR�ĺϷ���
 * -------------------------------------------------------------------
 * ������
 * -------------------------------------------------------------------
 * [����]terminal  : �߼��豸��Ϣ
 * -------------------------------------------------------------------
 * [����]su_atr    : ��Ƭ��ATR��Ϣ
 * -------------------------------------------------------------------
 * ���أ�
 * -------------------------------------------------------------------
 *    ����Ĵ������ĵ�"emv_errno.h"
 * ===================================================================
 */
int     emv_atr_parse(struct emv_core *terminal, struct emv_atr *su_atr);

/* ===================================================================
 * T=0�����µ���Ϣ����
 * -------------------------------------------------------------------
 * ������
 * -------------------------------------------------------------------
 * [����]terminal  : �߼��豸��Ϣ
 * -------------------------------------------------------------------
 * [����]apdu_req  : �����͵�����ṹ��ָ��
 * -------------------------------------------------------------------
 * [���]apdu_resp : �洢��Ƭ��Ӧ��Ϣ�Ľṹ��ָ��
 * -------------------------------------------------------------------
 * ���أ�
 * -------------------------------------------------------------------
 *    ����Ĵ������ĵ�"emv_errno.h"
 * ===================================================================
 */
int     emv_t0_exchange(struct emv_core *terminal, struct emv_apdu_req *apdu_req, struct emv_apdu_rsp *apdu_resp);

/* ===================================================================
 * ����T=1�п��β��У���ֽ�LRC.
 * -------------------------------------------------------------------
 * ������
 * -------------------------------------------------------------------
 * [����/���]t1_block : �����  : ͷ�� + ������
 *                       ���β��: LRC
 * -------------------------------------------------------------------
 * ���أ�
 * -------------------------------------------------------------------
 *   ʼ��Ϊ0
 * ===================================================================
 */
int     emv_t1_compute_lrc(struct emv_t1_block *t1_block);

/* ===================================================================
 * ����APDU�Ĺ涨����APDU����ṹ��֯�ɸ�ʽ�����ַ���
 * -------------------------------------------------------------------
 * ������
 * -------------------------------------------------------------------
 * [����]apdu_req : ����ṹ��ָ�룬��ѭAPDU�涨
 * -------------------------------------------------------------------
 * [���]buf      : ��Ÿ�ʽ�����ַ������
 * -------------------------------------------------------------------
 * ���أ�
 * -------------------------------------------------------------------
 *        �����ַ����е���Ч�����ֽ���
 * ===================================================================
 */
int     emv_t1_extract(struct emv_apdu_req *apdu_req, unsigned char *buf);

/* ===================================================================
 * ��֯I��
 * -------------------------------------------------------------------
 * ������
 * -------------------------------------------------------------------
 * [����]terminal : �߼��豸��Ϣ
 * -------------------------------------------------------------------
 * [����]buf      : ����֯�����ݻ�����
 * -------------------------------------------------------------------
 * [����]len      : ����֯����Ч�����ֽ���
 * -------------------------------------------------------------------
 * [���]t1_block : ��֯��Ŀ�
 * -------------------------------------------------------------------
 * ���أ�
 * -------------------------------------------------------------------
 *    ʼ�շ���0
 * ===================================================================
 */
int     emv_t1_iblock_pack(struct emv_core *terminal, const unsigned char *buf, int len,
                           struct emv_t1_block  *t1_block);

/* ===================================================================
 * ���Ϳ�( �����Ƿ���T=1���������Ϳ� )
 * -------------------------------------------------------------------
 * ������
 * -------------------------------------------------------------------
 * [����]terminal : �߼��豸��Ϣ
 * -------------------------------------------------------------------
 * [����]t1_block : �����͵���Ϣ��
 * -------------------------------------------------------------------
 * ���أ�
 * -------------------------------------------------------------------
 *    ����Ĵ������ĵ�"emv_errno.h"
 * ===================================================================
 */
int     emv_t1_block_xmit(struct emv_core *terminal, struct emv_t1_block  *t1_block);

/* ===================================================================
 * ���տ�( �����Ƿ���T=1���������Ϳ� )
 * -------------------------------------------------------------------
 * ������
 * -------------------------------------------------------------------
 * [����]terminal : �߼��豸��Ϣ
 * -------------------------------------------------------------------
 * [���]t1_block : ���յ���Ϣ��
 * -------------------------------------------------------------------
 * ���أ�
 * -------------------------------------------------------------------
 *    ����Ĵ������ĵ�"emv_errno.h"
 * ===================================================================
 */
int     emv_t1_block_recv(struct emv_core *terminal, struct emv_t1_block  *t1_block);

/* ===================================================================
 * T=1�����µ���Ϣ����
 * -------------------------------------------------------------------
 * ������
 * -------------------------------------------------------------------
 * [����]terminal  : �߼��豸��Ϣ
 * -------------------------------------------------------------------
 * [����]apdu_req  : �����͵�����ṹ��ָ��
 * -------------------------------------------------------------------
 * [���]apdu_resp : �洢��Ƭ��Ӧ��Ϣ�Ľṹ��ָ��
 * -------------------------------------------------------------------
 * ���أ�
 * -------------------------------------------------------------------
 *    ����Ĵ������ĵ�"emv_errno.h"
 * ===================================================================
 */
int     emv_t1_exchange(struct emv_core *terminal, struct emv_apdu_req *apdu_req, struct emv_apdu_rsp *apdu_resp);

/* ===================================================================
 * ����IFSD��( ����IFSD��terminal�����е�terminal_ifsdָ�� )
 * -------------------------------------------------------------------
 * ������
 * -------------------------------------------------------------------
 * [����]terminal : �߼��豸��Ϣ
 * -------------------------------------------------------------------
 * ���أ�
 * -------------------------------------------------------------------
 *    ����Ĵ������ĵ�"emv_errno.h"
 * ===================================================================
 */
int     emv_t1_ifsd_request(struct emv_core *terminal);

#ifdef __cplusplus
}
#endif
#endif

