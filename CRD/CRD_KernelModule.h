#pragma once

#include "MyWinNT.h"
#include "MyListEntry.h"
#include "CRD_ProcessModule.h"

BOOL AddKernelModuleInfo(MODULEINFO pmi);

VOID FreeKernelModuleInfo();

DWORD CRD_KernelModuleList();

VOID CRD_KernelModuleGet(PVOID buff,SIZE_T size);

VOID FreeTargetKernelModuleInfo();

NTSTATUS CRD_KernelModuleTargetGet(PMODULEINFO buff, const wchar_t* TargetSys);

NTSTATUS AddTargetKernelModuleInfo(PTransferMsg msg);

BOOL IsInTargetKernelModule(ULONG_PTR address);

NTSTATUS HideKernelModule(PTransferMsg msg);