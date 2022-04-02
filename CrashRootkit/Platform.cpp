#include "StdAfx.h"
#include "Platform.h"

SYSTEM_VERSION GetSystemVersion()   
{   
	BOOL   bOsVersionInfoEx;   
	OSVERSIONINFOEX   osvi;   

	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));   
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);   
	if(!(bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO*)&osvi)))   
	{   
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);   
		if(!GetVersionEx((OSVERSIONINFO*)&osvi))     
			return UNKNOWN;   
	}

	if(osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 2)
	{
		//KdPrint(("comint32: Running on Windows 8 32 Build:%d CSDVersion:%ws\r\n",buildnum,CSDVersion.Buffer));
		return WINDOWS8;
	}
	if(osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1)
	{
		//KdPrint(("comint32: Running on Windows 7 32 Build:%d CSDVersion:%ws\r\n",buildnum,CSDVersion.Buffer));
		return WINDOWS7;
	}
	if(osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0)
	{
		//KdPrint(("comint32: Running on Windows Vista 32 Build:%d CSDVersion:%ws\r\n",buildnum,CSDVersion.Buffer));
		return WINDOWSVISTA;
	}
	if(osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2)
	{
		//KdPrint(("comint32: Running on Windows 2003 32 Build:%d CSDVersion:%ws\r\n",buildnum,CSDVersion.Buffer));
		return WINDOWS2003;
	}
	if(osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1)
	{
		//KdPrint(("comint32: Running on Windows XP 32 Build:%d CSDVersion:%ws\r\n",buildnum,CSDVersion.Buffer));
		return WINDOWSXP;
	}
	return UNKNOWN;
}

BOOL WINAPI IsWow64()
{
	
	LPFN_ISWOW64PROCESS fnIsWow64Process; 
	BOOL bIsWow64 = FALSE; 
	fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle(L"kernel32"),"IsWow64Process"); 
	if (NULL != fnIsWow64Process) 
	{ 
		fnIsWow64Process(GetCurrentProcess(),&bIsWow64);
	}
	return bIsWow64;
}