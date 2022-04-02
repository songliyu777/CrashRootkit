#pragma once

enum SYSTEM_VERSION
{
	UNKNOWN = -1,
	WINDOWSXP = 0,
	WINDOWS2003 = 1,
	WINDOWSVISTA = 2,
	WINDOWS7 = 3,
	WINDOWS8 = 4
};

SYSTEM_VERSION GetSystemVersion();

ULONG GetUniqueProcessId_Offset();
ULONG GetImageFileName_Offset();
ULONG GetInheritedFromUniqueProcessId_Offset();
ULONG GetPeb_Offset();
ULONG GetObjectTable_Offset();
ULONG GetActiveProcessLinks_Offset();
ULONG GetThreadListHead_Offset();
ULONG GetThreadListEntry_Offset();
ULONG GetTeb_Offset();
ULONG GetState_Offset();
ULONG GetCid_Offset();
ULONG GetStartAddress_Offset();
ULONG GetWin32StartAddress_Offset();
ULONG GetKernelTime_Offset();
ULONG GetUserTime_Offset();
ULONG GetDirectoryTableBase_Offset();
ULONG GetFlags_Offset();
ULONG GetNextFreeTableEntry_Offset();
ULONG GetNextHandleNeedingPool_Offset();
ULONG GetCrossThreadFlags_Offset();
ULONG GetActiveExWorker_Offset();

