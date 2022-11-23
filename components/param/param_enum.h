#ifndef _PARAM_ENUM_H_
#define _PARAM_ENUM_H_

#define NORMAL_PARAM_BEGIN      typedef enum {NORMAL_PARAM_ID_START = 0,
#define NORMAL_PARAM_ITEM(paramId, paramType, paramName, paramVal)     NORMAL_PARAM_##paramId,
#define NORMAL_PARAM_END       NORMAL_PARAM_ID_MAX,} NORMAL_PARAM_ENUM;

#define PROTECTED_PARAM_BEGIN    typedef enum{PROTECTED_PARAM_ID_START = 100,
#define PROTECTED_PARAM_ITEM(paramId, paramType, paramName, paramVal)    PROTECTED_PARAM_##paramId,
#define PROTECTED_PARAM_END       PROTECTED_PARAM_ID_MAX,}PROTECTED_PARAM_ENUM;

#endif
