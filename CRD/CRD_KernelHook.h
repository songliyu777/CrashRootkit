#pragma once

#include "MyWinNT.h"
#include "MyListEntry.h"
#include "PETools_R0.h"
#define _CRD_
#include "Transfer.h"

typedef struct _MEMORYHOOK{
	SLISTENTRY next;
	WCHAR ModuleName[MAX_PATH];
	ULONG_PTR Address;
	ULONG_PTR JmpAddress;
	HookType Type;
	DWORD Length;
	PBYTE Origin;
	PBYTE Current;
}MEMORYHOOK,*PMEMORYHOOK;

DWORD NtKrCheck();

VOID NtKrGet(PVOID & buff,SIZE_T size);

VOID NtKrexeCheck(PSYSTEM_MODULE_INFORMATION pModule);

VOID Win32ksysCheck(PSYSTEM_MODULE_INFORMATION pModule,POINTER_TYPE count);

BOOL AddMemoryHookEntry(PWCHAR ModuleName,ULONG_PTR Address,PBYTE Origin,PBYTE Current,SIZE_T Length,HookType hooktype,ULONG_PTR jmpaddress);

DWORD FillMemoryFunction(PBYTE dist,IN PVOID current,OUT PVOID * next);

VOID FreeKernelHook();

NTSTATUS RecoverKernelHook(PTransferMsg msg);

HookType GetHookType(const unsigned char *currentdata, const unsigned char *originaldata, ULONG_PTR hookaddress, SIZE_T length);

ULONG_PTR GetJmpAddress(ULONG_PTR hookaddress, SIZE_T length, HookType type);





