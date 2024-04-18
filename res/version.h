#ifndef __VERSION_H__
#define __VERSION_H__

#define FILE_VERSION           1,0,0,0418        /* 文件版本 */
#define FILE_VERSION_STR       "1.0.0.0418"
#define COMPANY_NAME           "CZTEK"
#define INTERNAL_NAME          "CodeDesigner.exe"
#ifdef _WIN64 /* x64在RC编译器中添加预处理宏_WIN64 */
    #ifdef _DEBUG
        #define FILE_DESCRIPTION       "ATE Tool CodeDesigner(x64_Debug)"  /* 文件说明，数字为编译时间 */
    #else
        #define FILE_DESCRIPTION       "ATE Tool CodeDesigner(x64_Release)"
    #endif
#else
    #ifdef _DEBUG
        #define FILE_DESCRIPTION       "ATE Tool CodeDesigner(x86_Debug)"
    #else
        #define FILE_DESCRIPTION       "ATE Tool CodeDesigner(x86_Release)"
    #endif
#endif
#define LEGAL_COPYRIGHT        "Copyright 2013-2024 Shenzhen CZTEK Co., Ltd. All rights reserved." /* 版权 */
#define ORIGINAL_FILE_NAME     "CodeDesigner.exe"           /* 原始文件名 */
#define PRODUCT_NAME           "CodeDesigner"               /* 产品名称 */
#define ORGANIZATION_DOMAIN    "http://www.cztek.cn/"   /* 域名 */



#endif /* __VERSION_H__ */
