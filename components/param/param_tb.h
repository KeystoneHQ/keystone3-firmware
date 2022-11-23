/*********************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * name       : param_tb.h
 * Description:
 * author     : stone wang
 * data       : 2022-12-23 15:33
**********************************************************************/

#ifndef _PARAM_TB_H
#define _PARAM_TB_H

#define NORMAL_PARAM_BEGIN                     static const UserParam_t g_normalParamTb[NORMAL_PARAM_ID_MAX - NORMAL_PARAM_ID_START] = {{NORMAL_PARAM_ID_START, PARAM_TYPE_INT, "normalstart", {.valInt = 0}},
#define NORMAL_PARAM_ITEM(paramId, paramType, paramName, paramVal)  {NORMAL_PARAM_##paramId, paramType, paramName, paramVal},
#define NORMAL_PARAM_END                       };

#define PROTECTED_PARAM_BEGIN                   static const UserParam_t g_protectedParamTb[PROTECTED_PARAM_ID_MAX - PROTECTED_PARAM_ID_START] = {{PROTECTED_PARAM_ID_START, PARAM_TYPE_INT, "protectstart", {.valInt = 0}},
#define PROTECTED_PARAM_ITEM(paramId, paramType, paramName, paramVal) {PROTECTED_PARAM_##paramId, paramType, paramName, paramVal},
#define PROTECTED_PARAM_END                     };

#endif /* _PARAM_TB_H */

