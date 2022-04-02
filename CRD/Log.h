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

VOID SaveToLog(IN PWCHAR logstr);										//保存当前字符串到LOG

#define LogDataSize 512 

VOID PrintLog( __in __format_string NTSTRSAFE_PCWSTR pszFormat,...);	//打印Log信息

PWCHAR CurTimeStr();													//得到当前时间信息

PWCHAR GetLogPath();

#endif