// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include "Log.h"
#include "CRString.h"
#include "HookTool.h"

void UpdataToken()
{
	HANDLE hToken;
	LUID DebugNameValue;
	TOKEN_PRIVILEGES Privileges;
	DWORD dwRet;
	OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);
	LookupPrivilegeValue(NULL, L"SeDebugPrivilege", &DebugNameValue);
	Privileges.PrivilegeCount=1;
	Privileges.Privileges[0].Luid=DebugNameValue;
	Privileges.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
	AdjustTokenPrivileges(hToken, FALSE,&Privileges, sizeof(Privileges), NULL, &dwRet);
	CloseHandle(hToken);
}

void InitProtectHook()
{
	HookLdrLoadDll();
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			UpdataToken();
			InitLogPath(TRUE);
			InitProtectHook();
		}
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

