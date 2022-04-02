#include "stdafx.h"
#include "Log.h"

#pragma warning(disable:4996)

extern WCHAR wlogPath[MAX_PATH] = L"C:\\CRlog.txt";

VOID InitLogPath(BOOL bClear)
{
	FILE *fp;
	WCHAR szDriverImagePath[MAX_PATH] = {0};

	GetModuleFileName(NULL,szDriverImagePath,MAX_PATH); //得到当前模块路径
	for (int i=wcslen(szDriverImagePath);i>0;i--)
	{
		if ('\\'==szDriverImagePath[i])
		{
			szDriverImagePath[i+1]='\0';
			break;
		}
	}
	
	wcscpy_s(wlogPath,szDriverImagePath);
	wcscat_s(wlogPath,L"log.txt");
	if(bClear)
	{
		fp = _wfopen(wlogPath, L"w+, ccs=UNICODE");
		if(fp == NULL)
			return;
		WORD a = 0xFEFF;
		fwrite(&a, sizeof(WORD), 1, fp);
		fclose(fp);
	}
}

VOID SaveToLog(IN PWCHAR logstr)
{
	return;
	FILE *fp;
	unsigned char *data=NULL;
	fp = _wfopen(wlogPath, L"a+, ccs=UNICODE");
	if (fp == NULL)
	{
		return;
	}
	fwrite(logstr, wcslen(logstr)*sizeof(WCHAR), 1, fp);
	fclose(fp);
}

#define MAX_LINE 512

CONSOLEDLL_API VOID PrintLog(const wchar_t * info, ...)
{
	if (NULL == info)
	{
		return;
	}

	wchar_t	printData[MAX_LINE];
	wchar_t strData[MAX_LINE];
	wchar_t timeData[64];

	va_list		args;

	va_start(args, info);

	::_vsnwprintf(strData, MAX_LINE, info, args);
	SYSTEMTIME systime;
	GetLocalTime(&systime);
	::_snwprintf(timeData,64,L"%4d-%02d-%02d %02d:%02d:%02d",systime.wYear,systime.wMonth,systime.wDay,systime.wHour,systime.wMinute,systime.wSecond);
	::_snwprintf(printData,MAX_LINE,L"[%ws]%ws",timeData,strData);
	//::_snwprintf(printData,MAX_LINE,L"%ws",strData);
#ifndef RELEASE_VERSION
	SaveToLog(printData);
#endif
	va_end(args);
}

CONSOLEDLL_API VOID PrintDbgString(const wchar_t * info, ...)
{
	if (NULL == info)
	{
		return;
	}

	wchar_t strData[MAX_PATH];

	va_list		args;

	va_start(args, info);

	::_vsnwprintf(strData, MAX_PATH, info, args);

	OutputDebugStringW(strData);

	va_end(args);
}

