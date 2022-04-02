#pragma once
#include "MyWinNT.h"
#include "PETools_R0.h"
#include "KeSysFun.h"

BOOLEAN InitRelocate();
VOID FreeRelocate();

VOID InitNtKrRelocate(PSYSTEM_MODULE_INFORMATION pModule);
VOID InitWin32kRelocate(PSYSTEM_MODULE_INFORMATION pModule,POINTER_TYPE count);

PMEMORYMODULE GetMemoryModuleNtKr();
PMEMORYMODULE GetMemoryModuleWin32k();

ULONG_PTR GetNtKrBase();
ULONG_PTR GetWin32kBase();

FARPROC GetNtKrFunctionByName(LPCSTR lpFunctionName);
FARPROC GetOrignalNtKrFunctionByName(LPCSTR lpFunctionName);
ULONG_PTR NtKrAddressToOrignalNtKrAddress(ULONG_PTR address);

FARPROC GetRelocateFunction(ULONG_PTR INDEX);

typedef VOID (*KEATTACHPROCESS)(PEPROCESS Process);
typedef VOID (*KEDETACHPROCESS)();

typedef VOID (*KESTACKATTACHPROCESS)(IN PEPROCESS Process, OUT PKAPC_STATE ApcState);
typedef VOID (*KEUNSTACKDETACHPROCESS)(IN PKAPC_STATE ApcState);

typedef NTSTATUS (*PSSUSPENDTHREAD)(
									__in PETHREAD Thread,
									__out_opt PULONG PreviousSuspendCount
									);

typedef NTSTATUS (*PSRESUMETHREAD)(
								   __in PETHREAD Thread,
								   __out_opt PULONG PreviousSuspendCount
								   );
typedef NTSTATUS (*KERESUMETHREAD)();

typedef NTSTATUS (*PSRESUMEPROCESS)(PEPROCESS Process);

PSSUSPENDTHREAD GetPsSuspendThreadAddress();

PSRESUMETHREAD GetPsResumeThreadAddress();

#define NtResumeProcess_Index_WinXP 205
#define NtResumeProcess_Index_Win7 303

PSRESUMEPROCESS GetPsResumeProcessAddress();

#define KEATTACHPROCESS_INDEX 1
#define KEDETACHPROCESS_INDEX 2
#define PSSUSPENDTHREAD_INDEX 3
#define PSRESUMETHREAD_INDEX 4
#define KESTACKATTACHPROCESS_INDEX 5
#define KEUNSTACKDETACHPROCESS_INDEX 6
#define PSRESUMEPROCESS_INDEX 7

VOID RelocateSSDTByIndex(ULONG_PTR index);

ULONG_PTR GetRelocateSSDTByIndex(ULONG_PTR index);

VOID RelocateSSDTShadowByIndex(ULONG_PTR index);

ULONG_PTR GetRelocateSSDTShadowByIndex(ULONG_PTR index);