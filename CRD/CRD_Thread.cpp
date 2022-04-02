#ifdef __cplusplus
extern "C" {
#endif
#include <ntddk.h>
#include <string.h>
#ifdef __cplusplus
}; // extern "C"
#endif

#include "CRD_Thread.h"
#include "FuncAddrValid.h"
#include "CRD_Process.h"
#include "Platform.h"
#include "KeSysFun.h"
#include "MyListEntry.h"
#include "PETools_R0.h"
#include "CRD_KernelHook.h"
#include "CRD_SSDT.h"
#include "Relocate.h"
#define _CRD_
#include "Transfer.h"

SLISTENTRY g_sleThreadInfoHead = {0};

PSPTERMINATETHREADBYPOINTER g_PspTerminateThreadByPointer = NULL;

PETHREAD GetNextProcessThread(
							  IN PEPROCESS Process,
							  IN PETHREAD Thread OPTIONAL,
							  IN BOOL bIsQueryThread
							  )
{
	PETHREAD FoundThread = NULL;
	PLIST_ENTRY ListHead, Entry;
	ULONG ThreadListHead = GetThreadListHead_Offset();
	ULONG ThreadListEntry = GetThreadListEntry_Offset();

	if (!MmIsAddressValidEx(Process)){
		return NULL;
	}
	if (Thread){
		if (!MmIsAddressValidEx(Thread)){
			return NULL;
		}
	}
	if (Thread)
	{
		//  Entry = Thread->ThreadListEntry.Flink;;//   +0x22c ThreadListEntry  : _LIST_ENTRY
		Entry = (PLIST_ENTRY)((ULONG)(Thread) + ThreadListEntry);
		Entry=Entry->Flink;
	}
	else
	{
		Entry = (PLIST_ENTRY)((ULONG)(Process) + ThreadListHead);//+0x190 ThreadListHead   : _LIST_ENTRY
		Entry = Entry->Flink; 
	}
	// ListHead = &Process->ThreadListHead;
	ListHead = (PLIST_ENTRY)((ULONG)Process + ThreadListHead);

	while (ListHead != Entry)
	{
		FoundThread = (PETHREAD)((ULONG)Entry - ThreadListEntry);

		//如果是结束进程的话，就ObReferenceObject掉线程的引数
		if (!bIsQueryThread){
			if (MmIsAddressValidEx(FoundThread)){
				if (ObReferenceObject(FoundThread))
					break;
			}
		}
		if (MmIsAddressValidEx(FoundThread))
			break;

		FoundThread = NULL;
		Entry = Entry->Flink;
	}
	if (!bIsQueryThread){
		if (Thread)
			ObDereferenceObject(Thread);
	}
	return FoundThread;
}

SIZE_T ListThreadOfProcess(IN PEPROCESS peprocess)
{
	PETHREAD          Thread;
	PEPROCESS         Process;
	int i=0;
	ULONG ulSysModuleBase,ulSysModuleSize;

	FreeThreadInfo();

	if (!MmIsAddressValid(peprocess) || IsExitProcess(peprocess))
	{
		KdPrint(("Eprocess failed\r\n"));
		return 0;
	}

	Process = peprocess;

	for(Thread = GetNextProcessThread(Process, NULL, TRUE);
		Thread != NULL;
		Thread = GetNextProcessThread(Process, Thread, TRUE))
	{
		if(AddThreadInfo(Thread))
		{
			KdPrint(("添加ThreadInfo实例成功\r\n"));
		}
		else
		{
			KdPrint(("添加ThreadInfo实例失败\r\n"));
		}
	}
	return SizeOfSListEntry(&g_sleThreadInfoHead);
}

BOOL AddThreadInfo(PETHREAD Thread)
{
	PTHREADINFO pti = (PTHREADINFO)ExAllocatePool(NonPagedPool,sizeof(THREADINFO));
	if(pti)
	{
		RtlZeroMemory(pti,sizeof(THREADINFO));
		if(MmIsAddressValid(Thread))
		{
			pti->pethread = ULONG_PTR(Thread);
		}
		ULONG offset = GetTeb_Offset();
		if(MmIsAddressValid(PVOID((ULONG)Thread + offset)))
		{
			pti->Teb = *PULONG_PTR((ULONG)Thread + offset);
		}
		offset = GetState_Offset();
		if(MmIsAddressValid(PVOID((ULONG)Thread + offset)))
		{
			pti->State = *PUCHAR((ULONG)Thread + offset);
		}
		offset = GetCid_Offset();
		if(MmIsAddressValid(PVOID((ULONG)Thread + offset)))
		{
			PCLIENT_ID cid = PCLIENT_ID((ULONG)Thread + offset);
			pti->UniqueThread = (ULONG_PTR)cid->UniqueThread;
		}
		offset = GetStartAddress_Offset();
		if(MmIsAddressValid(PVOID((ULONG)Thread + offset)))
		{
			pti->StartAddress = *PULONG_PTR((ULONG)Thread + offset);
		}
		offset = GetWin32StartAddress_Offset();
		if(MmIsAddressValid(PVOID((ULONG)Thread + offset)))
		{
			pti->Win32StartAddress = *PULONG_PTR((ULONG)Thread + offset);
		}
		offset = GetKernelTime_Offset();
		if(MmIsAddressValid(PVOID((ULONG)Thread + offset)))
		{
			pti->KernelTime = *PULONG_PTR((ULONG)Thread + offset);
		}
		offset = GetUserTime_Offset();
		if(MmIsAddressValid(PVOID((ULONG)Thread + offset)))
		{
			pti->UserTime = *PULONG_PTR((ULONG)Thread + offset);
		}
		PushSLISTENTRY(&g_sleThreadInfoHead,&pti->next);
		return TRUE;
	}
	return FALSE;
}

VOID FreeThreadInfo()
{
	ReleaseSListEntry(&g_sleThreadInfoHead);
}

VOID GetThreads(PVOID buff,SIZE_T size)
{
	PSLISTENTRY pSListEntry = &g_sleThreadInfoHead;
	PThreadInfo pti_tr = (PThreadInfo)buff;
	while(!IsEmptySListEntry(pSListEntry))
	{
		PTHREADINFO pti = (PTHREADINFO)NextSListEntry(pSListEntry);
		if(pti)
		{
			pti_tr->pethread = pti->pethread;
			pti_tr->Teb = pti->Teb;
			pti_tr->State = pti->State;
			pti_tr->StartAddress = pti->StartAddress;
			pti_tr->Win32StartAddress = pti->Win32StartAddress;
			pti_tr->UniqueThread = pti->UniqueThread;
			pti_tr->KernelTime = pti->KernelTime;
			pti_tr->UserTime = pti->UserTime;
		}
		pti_tr++;
	}
}

VOID KernelTerminateThreadRoutine( 
	IN PKAPC Apc, 
	IN OUT PKNORMAL_ROUTINE *NormalRoutine, 
	IN OUT PVOID *NormalContext, 
	IN OUT PVOID *SystemArgument1, 
	IN OUT PVOID *SystemArgument2 
	) 
{ 
	PULONG ThreadFlags;
	BOOL bInit = FALSE;

	if(Apc)
		ExFreePool(Apc);

	ULONG ulCrossThreadFlagsOffset = GetCrossThreadFlags_Offset();
	ULONG ulActiveExWorker = GetActiveExWorker_Offset();
	if (ulCrossThreadFlagsOffset && ulActiveExWorker) 
	{ 
		ThreadFlags = (ULONG *)((ULONG)(PsGetCurrentThread()) + ulCrossThreadFlagsOffset); 
		*ThreadFlags = (*ThreadFlags) | PS_CROSS_THREAD_FLAGS_SYSTEM; 

		*(PULONG)((ULONG)PsGetCurrentThread() + ulActiveExWorker) = FALSE; //要结束工作队列的线程，必须设置标志

		PsTerminateSystemThread(STATUS_SUCCESS); 
	}
}

BOOL KillSystemThread(PETHREAD Thread)
{
	PKAPC Apc = NULL; 
	BOOL blnSucceed = FALSE; 

	if (!MmIsAddressValidEx(Thread)) {
		return FALSE; 
	}

	Apc = (PKAPC)ExAllocatePool(NonPagedPool,sizeof(KAPC)); 
	if (!Apc){
		return blnSucceed; 
	}

	KeInitializeApc(Apc, 
		Thread, 
		0, 
		KernelTerminateThreadRoutine, 
		NULL, 
		NULL, 
		KernelMode, 
		NULL); 

	blnSucceed = KeInsertQueueApc(Apc, 
		NULL, 
		NULL, 
		0); 

	return blnSucceed; 
}

BOOL KillSystemThread_Win7(PETHREAD Thread)
{
	PKAPC Apc = NULL; 
	BOOL blnSucceed = FALSE; 

	if (!MmIsAddressValidEx(Thread)) {
		return FALSE; 
	}

	Apc = (PKAPC)ExAllocatePool(NonPagedPool,sizeof(KAPC)); 
	if (!Apc){
		return blnSucceed; 
	}

	KeInitializeApc(Apc, 
		Thread, 
		0, 
		KernelTerminateThreadRoutine, 
		NULL, 
		NULL, 
		UserMode, 
		NULL); 

	blnSucceed = KeInsertQueueApc(Apc, 
		NULL, 
		NULL, 
		0); 

	return blnSucceed; 
}

NTSTATUS KillThread(PTransferMsg msg)
{
	SYSTEM_VERSION version;
	version = GetSystemVersion();
	PThreadInfo pti = (PThreadInfo)msg->buff;
	switch(version)
	{
	case WINDOWSXP:
		{
			if(pti->pethread)
			{
				if(KillSystemThread((PETHREAD)pti->pethread))
				{
					return STATUS_SUCCESS;
				}
			}
		}
	case WINDOWS7:
		{
			if(g_PspTerminateThreadByPointer==NULL)
			{
				g_PspTerminateThreadByPointer = (PSPTERMINATETHREADBYPOINTER)GetAddress_PspTerminateThreadByPointer_WIN7();
			}
			if (pti->StartAddress>0x80000000)
			{
				if(KillSystemThread((PETHREAD)pti->pethread))
				{
					return STATUS_SUCCESS;
				}
			}
			else
			{
				if(g_PspTerminateThreadByPointer)
				{
					return g_PspTerminateThreadByPointer((PETHREAD)pti->pethread, 0, FALSE);
				}
			}
		}
	}

	return STATUS_UNSUCCESSFUL;
}

ULONG_PTR GetAddress_PspTerminateThreadByPointer_WIN7()
{
	BYTE KeyWords[] = {0x6a,0x01,0xff,0x75,0x0c,0x56,0xe8};	
	ULONG_PTR adr = 0;
	FARPROC ofunadr = *(FARPROC*)((ULONG_PTR)GetOriginalSSDT()->pvSSDTBase + 371 * 4); 
	if(ofunadr)
	{
		adr = FindMemoryAddress((ULONG_PTR)ofunadr, (ULONG_PTR)ofunadr + 0x100, KeyWords, 0x7, FALSE);
		if(adr)
		{
			ofunadr = FARPROC(adr + 0x7);
			adr = *PULONG_PTR(ofunadr);
			ofunadr = FARPROC((ULONG_PTR)ofunadr + 0x4 + adr);
			return (ULONG_PTR)ofunadr;
		}
	}
	return 0;
}

ULONG_PTR g_PsResumeProcess_adr = NULL; 

NTSTATUS __declspec(naked) PsResumeProcessJmp(			
		PEPROCESS Process
	)
{
	_asm
	{
		mov  edi,edi
		push ebp
		mov  ebp,esp
		push ebx
		push edi
		mov eax,g_PsResumeProcess_adr
		add eax,7
		jmp eax
	}
}

VOID ResumeAllThread(IN PEPROCESS peprocess)
{
	PETHREAD          Thread;
	PEPROCESS         Process;
	int i=0;
	ULONG ulSysModuleBase,ulSysModuleSize;
	

	if (!MmIsAddressValid(peprocess) || IsExitProcess(peprocess))
	{
		KdPrint(("ResumeAllThread Eprocess failed\r\n"));
		return;
	}

	SYSTEM_VERSION version = GetSystemVersion();

	Process = peprocess;

	switch(version)
	{
	case WINDOWSXP:
		{
			PSRESUMEPROCESS PsResumeProcess = (PSRESUMEPROCESS)GetRelocateFunction(PSRESUMEPROCESS_INDEX);
			if(PsResumeProcess)
			{
				g_PsResumeProcess_adr = (ULONG_PTR)PsResumeProcess;
				PsResumeProcessJmp(Process);
			}
		}
		break;
	case WINDOWS7:
		{
			for(Thread = GetNextProcessThread(Process, NULL, TRUE);
				Thread != NULL;
				Thread = GetNextProcessThread(Process, Thread, TRUE))
			{
				if(MmIsAddressValid(Thread))
				{
					KERESUMETHREAD RKeResumeThread = (KERESUMETHREAD)GetRelocateFunction(PSRESUMETHREAD_INDEX);
					if(RKeResumeThread)
					{
						_asm
						{
							mov eax,Thread
							call RKeResumeThread
						}
					}
				}
			}
		}
		break;
	}
}