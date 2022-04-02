#pragma once

#include "MyWinNT.h"
#include "MyListEntry.h"
#define _CRD_
#include "Transfer.h"

#define PROCESS_VM_READ           (0x0010)  // winnt
#define PROCESS_VM_WRITE          (0x0020)  // winnt

typedef NTSTATUS (*MMCOPYVIRTUALMEMORY)(
					IN PEPROCESS FromProcess,
					IN CONST VOID *FromAddress,
					IN PEPROCESS ToProcess,
					OUT PVOID ToAddress,
					IN SIZE_T BufferSize,
					IN KPROCESSOR_MODE PreviousMode,
					OUT PSIZE_T NumberOfBytesCopied
					);

NTSTATUS CRD_ReadProcessMemory(HANDLE hProcess,PVOID lpBaseAddress,PVOID lpBuffer,SIZE_T BufferSize, SIZE_T * lpNumberOfBytesRead);

NTSTATUS CRD_WriteProcessMemory(HANDLE hProcess,PVOID lpBaseAddress,PVOID lpBuffer,SIZE_T nSize, SIZE_T * lpNumberOfBytesWritten);

NTSTATUS CRD_OpenProcess(IN ULONG ProcessId, OUT PHANDLE pHandle);

NTSTATUS HideProcessMemory(PTransferMsg msg);

VOID FreeHideMemoryInfo();

BOOL IsHideMemory(ULONG_PTR address);

BOOL AddHideMemoryInfo(PModuleInfo pmi);

BOOL InitMmCopyVirtualMemory();

NTSTATUS ModifyKernelMemory(PTransferMsg msg);