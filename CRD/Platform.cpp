#ifdef __cplusplus
extern "C" {
#endif
#include <ntddk.h>
#include <string.h>
#ifdef __cplusplus
}; // extern "C"
#endif

#include "Platform.h"

SYSTEM_VERSION GetSystemVersion()
{
	UNICODE_STRING CSDVersion;
	ULONG majorVersion,minorVersion,buildnum;
	PsGetVersion(&majorVersion, &minorVersion, &buildnum, &CSDVersion);
	if(majorVersion == 6 && minorVersion == 2)
	{
		//KdPrint(("comint32: Running on Windows 8 32 Build:%d CSDVersion:%ws\r\n",buildnum,CSDVersion.Buffer));
		return WINDOWS8;
	}
	if(majorVersion == 6 && minorVersion == 1)
	{
		//KdPrint(("comint32: Running on Windows 7 32 Build:%d CSDVersion:%ws\r\n",buildnum,CSDVersion.Buffer));
		return WINDOWS7;
	}
	if(majorVersion == 6 && minorVersion == 0)
	{
		//KdPrint(("comint32: Running on Windows Vista 32 Build:%d CSDVersion:%ws\r\n",buildnum,CSDVersion.Buffer));
		return WINDOWSVISTA;
	}
	if ( majorVersion == 5 && minorVersion == 2 )
	{
		//KdPrint(("comint32: Running on Windows 2003 32 Build:%d CSDVersion:%ws\r\n",buildnum,CSDVersion.Buffer));
		return WINDOWS2003;
	}
	if ( majorVersion == 5 && minorVersion == 1 )
	{
		//KdPrint(("comint32: Running on Windows XP 32 Build:%d CSDVersion:%ws\r\n",buildnum,CSDVersion.Buffer));
		return WINDOWSXP;
	}
	return UNKNOWN;
}

ULONG GetUniqueProcessId_Offset()
{
	SYSTEM_VERSION version = GetSystemVersion();
	switch(version)
	{
	case WINDOWSXP:
		return 0x84;
	case WINDOWS7:
		return 0xb4;
	}
	return 0;
}

ULONG GetImageFileName_Offset()
{
	SYSTEM_VERSION version = GetSystemVersion();
	switch(version)
	{
	case WINDOWSXP:
		return 0x174;
	case WINDOWS7:
		return 0x16c;
	}
	return 0;
}

ULONG GetInheritedFromUniqueProcessId_Offset()
{
	SYSTEM_VERSION version = GetSystemVersion();
	switch(version)
	{
	case WINDOWSXP:
		return 0x14c;
	case WINDOWS7:
		return 0x140;
	}
	return 0;
}

ULONG GetPeb_Offset()
{
	SYSTEM_VERSION version = GetSystemVersion();
	switch(version)
	{
	case WINDOWSXP:
		return 0x1b0;
	case WINDOWS2003:
		return 0x1a0;
	case WINDOWS7:
		return 0x1a8;
	case WINDOWSVISTA:
		return 0x188;
	case WINDOWS8:
		return 0x140;
	}
	return 0;
}

ULONG GetObjectTable_Offset()
{
	SYSTEM_VERSION version = GetSystemVersion();
	switch(version)
	{
	case WINDOWSXP:
		return 0xc4;
	case WINDOWS7:
		return 0xf4;
	}
	return 0;
}

ULONG GetActiveProcessLinks_Offset()
{
	SYSTEM_VERSION version = GetSystemVersion();
	switch(version)
	{
	case WINDOWSXP:
		return 0x088;
	case WINDOWS7:
		return 0x0b8;
	}
	return 0;
}

ULONG GetThreadListHead_Offset()
{
	SYSTEM_VERSION version = GetSystemVersion();
	switch(version)
	{
	case WINDOWSXP:
		return 0x190;
	case WINDOWS7:
		return 0x188;
	}
	return 0;
}

ULONG GetThreadListEntry_Offset()
{
	SYSTEM_VERSION version = GetSystemVersion();
	switch(version)
	{
	case WINDOWSXP:
		return 0x22c;
	case WINDOWS7:
		return 0x268;
	}
	return 0;
}

ULONG GetTeb_Offset()
{
	SYSTEM_VERSION version = GetSystemVersion();
	switch(version)
	{
	case WINDOWSXP:
		return 0x20;
	case WINDOWS7:
		return 0x88;
	}
	return 0;
}

ULONG GetState_Offset()
{
	SYSTEM_VERSION version = GetSystemVersion();
	switch(version)
	{
	case WINDOWSXP:
		return 0x2d;
	case WINDOWS7:
		return 0x68;
	}
	return 0;
}

ULONG GetCid_Offset()
{
	SYSTEM_VERSION version = GetSystemVersion();
	switch(version)
	{
	case WINDOWSXP:
		return 0x1ec;
	case WINDOWS7:
		return 0x22c;
	}
	return 0;
}

ULONG GetStartAddress_Offset()
{
	SYSTEM_VERSION version = GetSystemVersion();
	switch(version)
	{
	case WINDOWSXP:
		return 0x224;
	case WINDOWS7:
		return 0x218;
	}
	return 0;
}

ULONG GetWin32StartAddress_Offset()
{
	SYSTEM_VERSION version = GetSystemVersion();
	switch(version)
	{
	case WINDOWSXP:
		return 0x228;
	case WINDOWS7:
		return 0x260;
	}
	return 0;
}

ULONG GetKernelTime_Offset()
{
	SYSTEM_VERSION version = GetSystemVersion();
	switch(version)
	{
	case WINDOWSXP:
		return 0x144;
	case WINDOWS7:
		return 0x198;
	}
	return 0;
}

ULONG GetUserTime_Offset()
{
	SYSTEM_VERSION version = GetSystemVersion();
	switch(version)
	{
	case WINDOWSXP:
		return 0x148;
	case WINDOWS7:
		return 0x1c4;
	}
	return 0;
}

ULONG GetDirectoryTableBase_Offset()
{
	SYSTEM_VERSION version = GetSystemVersion();
	switch(version)
	{
	case WINDOWSXP:
		return 0x18;
	case WINDOWS7:
		return 0x18;
	}
	return 0;
}

ULONG GetFlags_Offset()
{
	SYSTEM_VERSION version = GetSystemVersion();
	switch(version)
	{
	case WINDOWSXP:
		return 0x248;
	case WINDOWS7:
		return 0x270;
	}
	return 0;
}

ULONG GetNextFreeTableEntry_Offset()
{
	SYSTEM_VERSION version = GetSystemVersion();
	switch(version)
	{
	case WINDOWSXP:
		return 0x4;
	case WINDOWS7:
		return 0x4;
	}
	return 0;
}

ULONG GetNextHandleNeedingPool_Offset()
{
	SYSTEM_VERSION version = GetSystemVersion();
	switch(version)
	{
	case WINDOWSXP:
		return 0x38;
	case WINDOWS7:
		return 0x34;
	}
	return 0;
}

ULONG GetCrossThreadFlags_Offset()
{
	SYSTEM_VERSION version = GetSystemVersion();
	switch(version)
	{
	case WINDOWSXP:
		return 0x248;
	case WINDOWS7:
		return 0x280;
	}
	return 0;
}

ULONG GetActiveExWorker_Offset()
{
	SYSTEM_VERSION version = GetSystemVersion();
	switch(version)
	{
	case WINDOWSXP:
		return 0x24c;
	case WINDOWS7:
		return 0x284;
	}
	return 0;
}