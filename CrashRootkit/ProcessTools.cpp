#include "StdAfx.h"
#include "ProcessTools.h"
#include "ConsoleDll.h"
#include<Tlhelp32.h>

BOOL CheckCRExist()
{
	HANDLE hSnapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	if(hSnapshot==INVALID_HANDLE_VALUE){
		return FALSE;
	}
	PROCESSENTRY32 pe={sizeof(pe)};
	DWORD count = 0;
	for(BOOL fOK=Process32First(hSnapshot,&pe);fOK;fOK=Process32Next(hSnapshot,&pe))
	{
		if(!_wcsicmp(pe.szExeFile,L"fxgame.exe"))
		{
			count++;
		}
	}
	if(count==2)
		return TRUE;
	return FALSE;
}