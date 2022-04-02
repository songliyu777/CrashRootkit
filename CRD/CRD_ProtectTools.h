#pragma once

#include "MyWinNT.h"
#include "MyListEntry.h"
#include "CRD_Process.h"
#define _CRD_
#include "Transfer.h"

NTSTATUS AddTargetProcessId(PTransferMsg msg);

BOOLEAN AddTargetProcessIdEx(HANDLE ProcessId);

BOOLEAN IsProtectTools(ULONG_PTR tableadr, ULONG index);

NTSTATUS AddProtectProcess(PTransferMsg msg);

BOOLEAN AddProtectProcessIdEx(HANDLE ProcessId);

VOID FreeProtectProcessInfo();

VOID FreeTargetProcessInfo();

VOID FreeSeparateProcessInfo();

BOOLEAN IsProtectProcessId(HANDLE ProcessId);

BOOLEAN IsProtectProcess(PEPROCESS peprocess);

BOOLEAN IsTargetProcessId(HANDLE ProcessId);

BOOLEAN IsTargetProcess(PEPROCESS peprocess);

NTSTATUS AddSeparateProcess(PTransferMsg msg);

BOOLEAN AddSeparateProcessIdEx(HANDLE ProcessId);

BOOLEAN IsSeparateProcessId(HANDLE ProcessId);

BOOLEAN IsSeparateProcess(PEPROCESS peprocess);

BOOLEAN CreateProtectThread();

VOID TerminalProtectThread();

VOID ProtectThreadFunc(IN PVOID context);