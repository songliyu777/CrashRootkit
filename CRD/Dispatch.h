#pragma once

#include "MyWinNT.h"
#define _CRD_
#include "Transfer.h"

NTSTATUS DispatchMessage(PTransferMsg msg);

NTSTATUS KernelHookCheck(SIZE_T & size);

NTSTATUS KernelHookGet(PVOID & buff,SIZE_T size);

NTSTATUS ProcessList(SIZE_T & size);

NTSTATUS ProcessListGet(PVOID buff,SIZE_T size);

NTSTATUS ModulesList(ULONG_PTR pid,SIZE_T & size);

NTSTATUS ModulesListGet(PVOID buff,SIZE_T size);

NTSTATUS ThreadsList(ULONG_PTR pid,SIZE_T & size);

NTSTATUS ThreadsListGet(PVOID buff,SIZE_T size);

NTSTATUS ModulePeInfoGet(PVOID buff,SIZE_T & size);

NTSTATUS CR4Get(ULONG & cr4);

NTSTATUS ReadProcessMemory(PVOID buff,SIZE_T insize,SIZE_T & outsize);

NTSTATUS WriteProcessMemory(PVOID buff,SIZE_T insize,SIZE_T & outsize);

NTSTATUS NtOpenProcessEx(PVOID buff,OUT PHANDLE pHandle);

NTSTATUS KernelModuleList(SIZE_T & size);

NTSTATUS KernelModuleListGet(PVOID buff,SIZE_T size);

NTSTATUS SSDTList(SIZE_T & size);

NTSTATUS SSDTGet(PVOID buff,SIZE_T size);

NTSTATUS SSDTShadowList(SIZE_T & size);

NTSTATUS SSDTShadowGet(PVOID buff,SIZE_T size);

NTSTATUS DpcTimerList(SIZE_T & size);

NTSTATUS DpcTimerGet(PVOID buff,SIZE_T size);

NTSTATUS IoTimerList(SIZE_T & size);

NTSTATUS IoTimerGet(PVOID buff,SIZE_T size);

NTSTATUS SetSynchronizaEvent(HANDLE hEvent);

NTSTATUS FreeSynchronizaEvent();

NTSTATUS WaitForRing3Event();

NTSTATUS NotifyRing3();

VOID SetSynchronizaInfo(PWCHAR pinfo,SIZE_T size);

NTSTATUS GetSynchronizaInfo(PVOID buff,SIZE_T size);

NTSTATUS GetPort(PTransferMsg msg);

NTSTATUS SetPort(PTransferMsg msg);

NTSTATUS FreeAll();

