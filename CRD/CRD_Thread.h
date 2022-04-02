#pragma once

#include "MyWinNT.h"
#include "MyListEntry.h"
#define _CRD_
#include "Transfer.h"

#define PS_CROSS_THREAD_FLAGS_SYSTEM 0x00000010UL 

typedef struct _THREADINFO
{
	SLISTENTRY next;
	ULONG_PTR pethread;
	ULONG_PTR Teb;
	UCHAR State;
	ULONG_PTR UniqueThread;
	ULONG_PTR StartAddress;
	ULONG_PTR Win32StartAddress;
	ULONG_PTR KernelTime;
	ULONG_PTR UserTime;
}THREADINFO,*PTHREADINFO;

PETHREAD GetNextProcessThread(IN PEPROCESS Process,IN PETHREAD Thread OPTIONAL,IN BOOL bIsQueryThread);

SIZE_T ListThreadOfProcess(IN PEPROCESS peprocess);

VOID FreeThreadInfo();

BOOL AddThreadInfo(PETHREAD Thread);

VOID GetThreads(PVOID buff,SIZE_T size);

BOOL KillSystemThread(PETHREAD Thread);

NTSTATUS KillThread(PTransferMsg msg);

typedef NTSTATUS (*PSPTERMINATETHREADBYPOINTER)(PETHREAD, NTSTATUS, BOOLEAN);

ULONG_PTR GetAddress_PspTerminateThreadByPointer_WIN7();

VOID ResumeAllThread(IN PEPROCESS peprocess);
