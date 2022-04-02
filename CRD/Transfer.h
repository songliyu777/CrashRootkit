#pragma once

#ifndef _CRD_
#include <vector>
using namespace std;
#endif

enum MESSAGETYPE
{
	FREEALL = 0,
	TERMINATE_PROCESS,
	KILL_DPCTIMER,
	START_IOTIMER,
	STOP_IOTIMER,
	RECOVER_SSDT,
	RECOVER_SSDTSHADOW,
	RECOVER_KERNELHOOK,
	KILL_THREAD,
	ANTI_PROTECT_1,
	ANTI_PROTECT_2,
	KDENABLEDEBUGGER,
	KDDISABLEDEBUGGER,
	HIDEKERNELDEBUGGER,
	MONITORDRIVERLOAD,
	ADDTARGETPROCESSID,
	RECOVEROBJECT,
	ADDPROTECTPROCESS,
	ADDTARGETKERNELMODULE,
	ADDFILTERDRVIOINFO,
	HIDEPROCESSMODULE,
	HIDEPROCESSMEMORY,
	SETKESPEED,
	SETKEEXCEPTION,
	ADDSEPARATEPROCESS,
	SUSPENDTHREAD,
	RESUMETHREAD,
	SHUTDOWN,
	GETPORT,
	SETPORT,
	INPUTSIMULATE,
	HIDEKERNELMODULE,
	MODIFYKERNELMEMORY
};

enum HookType
{
	IAT_HOOK,
	EAT_HOOK,
	INLINE_HOOK,
	SSDT_HOOK,
	OBJECT_HOOK,
	UNKNOW_HOOK
};

typedef struct _TransferMsg
{
	ULONG dwMsgId;
	ULONG size;		
	UCHAR buff[4096];
}TransferMsg,*PTransferMsg;

typedef struct _MemoryHook
{
	_MemoryHook()
	{
		Address = 0;
		JmpAddress = 0;
		Length = 0;
		Origin = NULL;
		Current = NULL;
		Type = IAT_HOOK;
	}
	WCHAR ModuleName[MAX_PATH];
	WCHAR JmpModuleName[MAX_PATH];
	ULONG_PTR Address;
	ULONG_PTR JmpAddress;
	HookType Type;
	DWORD Length;
	PBYTE Origin;
	PBYTE Current;
}MemoryHook,*PMemoryHook;

typedef struct _ProcessInfo
{
	ULONG_PTR ProcessId;
	ULONG_PTR ParentProcessId;
	ULONG_PTR PEProcess;
	UCHAR ImageFileName[16];
	WCHAR ImagePath[MAX_PATH];
}ProcessInfo,*PProcessInfo;

typedef struct _ModuleInfo
{
	ULONG_PTR BaseAddress;
	ULONG_PTR EntryPoint;
	ULONG_PTR SizeOfImage;
	ULONG_PTR ParentProcessId;
	WCHAR FullDllName[MAX_PATH];
	PVOID PEImage;
	_ModuleInfo * SameNameNext;
}ModuleInfo,*PModuleInfo;

typedef struct _ThreadInfo
{
	ULONG_PTR pethread;
	ULONG_PTR Teb;
	UCHAR State;
	ULONG_PTR UniqueThread;
	ULONG_PTR StartAddress;
	ULONG_PTR Win32StartAddress;
	ULONG_PTR KernelTime;
	ULONG_PTR UserTime;
	WCHAR FullDllName[MAX_PATH];
}ThreadInfo,*PThreadInfo;

typedef struct _ReadMemoryInfo
{
	HANDLE ProcessHandle;
	ULONG_PTR StartAddress;
	SIZE_T nSize;
}ReadMemoryInfo,*PReadMemoryInfo;

typedef struct _WriteMemoryInfo
{
	HANDLE ProcessHandle;
	ULONG_PTR StartAddress;
	SIZE_T nSize;
}WriteMemoryInfo,*PWriteMemoryInfo;

typedef struct _SSDTInfo
{
	ULONG_PTR CurrentAddress;
	ULONG_PTR OriginalAddress;
	ULONG Index;
	CHAR FunctionName[60];
	WCHAR FullDllName[MAX_PATH];
}SSDTInfo,*PSSDTInfo;

typedef struct _DpcTimerInfo{
	ULONG_PTR TimerAddress;
	ULONG_PTR DpcAddress;
	ULONG_PTR DpcRoutineAddress;
	ULONG_PTR Period;
	WCHAR FullDllName[MAX_PATH];
}DpcTimerInfo,*PDpcTimerInfo;

typedef struct _IoTimerInfo{
	ULONG_PTR DeviceObject;
	ULONG_PTR IoTimerRoutineAddress;
	SHORT ulStatus;
	WCHAR FullDllName[MAX_PATH];
}IoTimerInfo,*PIoTimerInfo;

typedef struct _FilterDrvInfo
{
	ULONG_PTR ProcessId;
	WCHAR ImageFileName[32];
}FilterDrvInfo,*PFilterDrvInfo;

#ifndef _CRD_
typedef vector<MemoryHook> MemoryHookVector;
typedef vector<ProcessInfo> ProcessInfoVector;
typedef vector<ModuleInfo> ModuleInfoVector;
typedef vector<ThreadInfo> ThreadInfoVector;
typedef vector<SSDTInfo> SSDTInfoVector;
typedef vector<DpcTimerInfo> DpcTimerInfoVector;
typedef vector<IoTimerInfo> IoTimerInfoVector;
#endif

#ifdef _CRD_
typedef struct tagMOUSEINPUT {
	LONG    dx;
	LONG    dy;
	DWORD   mouseData;
	DWORD   dwFlags;
	DWORD   time;
	ULONG_PTR dwExtraInfo;
} MOUSEINPUT, *PMOUSEINPUT, FAR* LPMOUSEINPUT;

typedef struct tagKEYBDINPUT {
	WORD    wVk;
	WORD    wScan;
	DWORD   dwFlags;
	DWORD   time;
	ULONG_PTR dwExtraInfo;
} KEYBDINPUT, *PKEYBDINPUT, FAR* LPKEYBDINPUT;

typedef struct tagHARDWAREINPUT {
	DWORD   uMsg;
	WORD    wParamL;
	WORD    wParamH;
} HARDWAREINPUT, *PHARDWAREINPUT, FAR* LPHARDWAREINPUT;

typedef struct tagINPUT {
	DWORD   type;
	union
	{
		MOUSEINPUT      mi;
		KEYBDINPUT      ki;
		HARDWAREINPUT   hi;
	};
} INPUT, *PINPUT, FAR* LPINPUT;
#endif

typedef struct _TransferSendInput
{
	unsigned int nInputs;
	LPINPUT pInputs;
	int cbSize;
}TransferSendInput, *PTransferSendInput;

typedef struct _TransferFindWindowEx
{
	LONG hwndParent;
	LONG hwndChild;
	ULONG_PTR strClassName;
	ULONG_PTR strWindowName;
	DWORD dwType;
}TransferFindWindowEx, *PTransferFindWindowEx;

typedef struct _TransferOpenProcess
{
	DWORD dwDesiredAccess;
	BOOL bInheritHandle;
	DWORD dwProcessId;
	HANDLE resultHandle;
}TransferOpenProcess, *PTransferOpenProcess;

#pragma pack(1)
struct tagPort32Struct
{
	unsigned short wPortAddr;
	unsigned char dwPortVal;
	unsigned char bSize;
};
