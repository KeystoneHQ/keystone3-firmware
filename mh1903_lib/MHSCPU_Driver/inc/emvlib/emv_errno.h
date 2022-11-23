#ifndef EMV_ERRNO_H
#ifdef __cplusplus
extern "C" {
#endif
#define EMV_ERRNO_H

#ifndef NULL
#define NULL   ( 0 )
#endif

#define OK  ( 0 )


/* *******************************************************

 * emv operation state informations defined.

 * ******************************************************/

#define ICC_ERR_DEVNO              ( -2800 )

#define ICC_ERR_CHIP               ( -2801 )

#define ICC_ERR_NOCARD             ( -2802 )

#define ICC_ERR_PARAM              ( -2803 )

#define ICC_ERR_POWER              ( -2804 )

#define ICC_ERR_OCCUPY             ( -2805 )

#define ICC_ERR_UNINIT             ( -2806 )

#define ICC_ERR_PARS               ( -2807 )/* only be used in communication kenerl */

#define ICC_ERR_BWT                ( -2808 )/* only be used in communication kenerl */

#define ICC_ERR_CWT                ( -2809 )/* only be used in communication kenerl */

#define ICC_ERR_SEND               ( -2810 )


#define ICC_ERR_ATR_SWT            ( -2820 )

#define ICC_ERR_ATR_TWT            ( -2821 )

#define ICC_ERR_ATR_CWT            ( -2822 )

#define ICC_ERR_ATR_TS             ( -2823 )

#define ICC_ERR_ATR_TA1            ( -2824 )

#define ICC_ERR_ATR_TB1            ( -2825 )

#define ICC_ERR_ATR_TC1            ( -2826 )

#define ICC_ERR_ATR_TD1            ( -2827 )

#define ICC_ERR_ATR_TA2            ( -2828 )

#define ICC_ERR_ATR_TB2            ( -2829 )

#define ICC_ERR_ATR_TC2            ( -2830 )

#define ICC_ERR_ATR_TD2            ( -2831 )

#define ICC_ERR_ATR_TA3            ( -2832 )

#define ICC_ERR_ATR_TB3            ( -2833 )

#define ICC_ERR_ATR_TC3            ( -2834 )

#define ICC_ERR_ATR_TCK            ( -2835 )

#define ICC_ERR_T_ORDER            ( -2836 )



#define ICC_ERR_T0_WWT             ( -2840 )

#define ICC_ERR_T0_CREP            ( -2841 )

#define ICC_ERR_T0_PROB            ( -2842 )



#define ICC_ERR_T1_BREP            ( -2850 )

#define ICC_ERR_T1_BWT             ( -2851 )

#define ICC_ERR_T1_CWT             ( -2852 )

#define ICC_ERR_T1_NAD             ( -2853 )

#define ICC_ERR_T1_PCB             ( -2854 )

#define ICC_ERR_T1_LRC             ( -2855 )

#define ICC_ERR_T1_LEN             ( -2856 )

#define ICC_ERR_T1_SRL             ( -2857 )

#define ICC_ERR_T1_SRC             ( -2858 )

#define ICC_ERR_T1_SRA             ( -2859 )



#define ICC_ERR_PPSS               ( -2860 )

#define ICC_ERR_PPS1               ( -2861 )

#define ICC_ERR_PCK                ( -2862 )


#ifdef __cplusplus
}
#endif
#endif
/*
 * =======================================================================
 * CopyRight (C) 2008-2009 PAX Computer Technology(ShenZhen) Co.,Ltd.
 * =======================================================================
 */
