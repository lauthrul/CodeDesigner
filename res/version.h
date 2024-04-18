#ifndef __VERSION_H__
#define __VERSION_H__

#define FILE_VERSION           1,0,0,0418        /* �ļ��汾 */
#define FILE_VERSION_STR       "1.0.0.0418"
#define COMPANY_NAME           "CZTEK"
#define INTERNAL_NAME          "CodeDesigner.exe"
#ifdef _WIN64 /* x64��RC�����������Ԥ�����_WIN64 */
    #ifdef _DEBUG
        #define FILE_DESCRIPTION       "ATE Tool CodeDesigner(x64_Debug)"  /* �ļ�˵��������Ϊ����ʱ�� */
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
#define LEGAL_COPYRIGHT        "Copyright 2013-2024 Shenzhen CZTEK Co., Ltd. All rights reserved." /* ��Ȩ */
#define ORIGINAL_FILE_NAME     "CodeDesigner.exe"           /* ԭʼ�ļ��� */
#define PRODUCT_NAME           "CodeDesigner"               /* ��Ʒ���� */
#define ORGANIZATION_DOMAIN    "http://www.cztek.cn/"   /* ���� */



#endif /* __VERSION_H__ */
