#ifndef _LOG_H_
#define _LOG_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <ntddk.h>
#include <string.h>
#include <ntstrsafe.h>
#ifdef __cplusplus
}; // extern "C"
#endif

#include "MyWinNT.h"

NTSTATUS SetLogPath(PWCHAR path);

VOID SaveToLog(IN PWCHAR logstr);										//���浱ǰ�ַ�����LOG

#define LogDataSize 512 

VOID PrintLog( __in __format_string NTSTRSAFE_PCWSTR pszFormat,...);	//��ӡLog��Ϣ

PWCHAR CurTimeStr();													//�õ���ǰʱ����Ϣ

PWCHAR GetLogPath();

#endif