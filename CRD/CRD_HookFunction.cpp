#ifdef __cplusplus
extern "C" {
#endif
#include <ntddk.h>
#include <string.h>
#ifdef __cplusplus
}; // extern "C"
#endif

#include "CRD_HookFunction.h"
#include "CRD_Hook.h"
#include "CRD_KernelModule.h"
#include "CRD_ProtectTools.h"
#include "CRD_IoControl.h"
#include "CRD_BsodHook.h"
#include "Relocate.h"
#include "Platform.h"
#include "KeSysFun.h"
#include "CRD_Memory.h"
#include "Log.h"

NTQUERYSYSTEMINFORMATION Original_NtQuerySystemInformation = NULL;
LARGE_INTEGER UserTime = {0};
LARGE_INTEGER KernelTime = {0};
NTSTATUS NtQuerySystemInformation_Proxy(
										__in          SYSTEM_INFORMATION_CLASS SystemInformationClass,
										__inout       PVOID SystemInformation,
										__in          ULONG SystemInformationLength,
										__out_opt     PULONG ReturnLength
										)
{
	NTSTATUS status = STATUS_SUCCESS;
	KPROCESSOR_MODE  mode;
	mode = ExGetPreviousMode();
	if (mode == UserMode)
	{
		//KdPrint(("ProcessId %d SYSTEM_INFORMATION_CLASS %d \r\n",PsGetCurrentProcessId(),SystemInformationClass));
		if(Original_NtQuerySystemInformation)
		{
			status = Original_NtQuerySystemInformation(SystemInformationClass,SystemInformation,SystemInformationLength,ReturnLength);
			if(NT_SUCCESS(status))
			{
				if(SystemInformationClass==SystemKernelDebuggerInformation)
				{
					__try
					{
						ProbeForWrite(SystemInformation,SystemInformationLength,1);
						RtlZeroMemory(SystemInformation,SystemInformationLength);
					}
					__except (EXCEPTION_EXECUTE_HANDLER)
					{
						return STATUS_UNSUCCESSFUL;
					}
				}
				if(IsTargetProcessId(PsGetCurrentProcessId()))
				{
					KdPrint(("NtQuerySystemInformation %d ThreadID %d \r\n",SystemInformationClass,PsGetCurrentThreadId()));
				}
				if(SystemInformationClass == SystemProcessesAndThreadsInformation)
				{
					HANDLE ProcessID;
					ProcessID = PsGetCurrentProcessId();
					if(IsTargetProcessId(ProcessID))
					{
						KdPrint(("SystemProcessesAndThreadsInformation ThreadID %d \r\n",PsGetCurrentThreadId()));
						__try
						{
							PSYSTEM_PROCESSES pSysProcInfo;
							ProbeForRead(SystemInformation,SystemInformationLength,1);
							ProbeForWrite(SystemInformation,SystemInformationLength,1);
							pSysProcInfo=(PSYSTEM_PROCESSES)SystemInformation;
							while(pSysProcInfo)
							{
								if(pSysProcInfo->ProcessName.Buffer!=NULL)
								{
									if(IsTargetProcessId((HANDLE)pSysProcInfo->ProcessId))//目标的进程
									{
										PSYSTEM_THREADS Threads = pSysProcInfo->Threads;
										for(ULONG i=0;i<pSysProcInfo->ThreadCount;i++)
										{
											Threads->ThreadState = 5;
											Threads++;
										}
									}
								}
	
								if(pSysProcInfo->NextEntryDelta)
								{
									pSysProcInfo = (PSYSTEM_PROCESSES)((UCHAR*)pSysProcInfo + pSysProcInfo->NextEntryDelta);//pSysProcInfo指向下一个节点
								}
								else
								{
									pSysProcInfo = NULL;//最后一个
								}
							}
						}
						__except (EXCEPTION_EXECUTE_HANDLER)
						{
							return STATUS_UNSUCCESSFUL;
						}
					}
					if(ProcessID && (!IsProtectProcessId(ProcessID) || IsTargetProcessId(ProcessID))) //不是保护进程或者目标进程
					{
						__try
						{
							PSYSTEM_PROCESSES pSysProcInfo,pPrev;
							ProbeForRead(SystemInformation,SystemInformationLength,1);
							ProbeForWrite(SystemInformation,SystemInformationLength,1);
							pSysProcInfo=(PSYSTEM_PROCESSES)SystemInformation;
							pPrev = NULL;
							while(pSysProcInfo)
							{
								if(pPrev == NULL)
								{
									pPrev = pSysProcInfo;
								}
								if(pSysProcInfo->ProcessName.Buffer!=NULL)
								{
									if(IsProtectProcessId((HANDLE)pSysProcInfo->ProcessId) && pSysProcInfo->ProcessId != (ULONG)ProcessID)//是保护进程，不是自身进程就隐藏
									{
										if(pSysProcInfo->NextEntryDelta)//中间节点
										{
											pPrev->NextEntryDelta+=pSysProcInfo->NextEntryDelta;
										}
										else//最后一个节点
										{
											pPrev->NextEntryDelta=NULL;
										}
									}
									else
									{
										pPrev = pSysProcInfo;//保存上一个节点地址
									}
								}
								else //系统空闲进程,把我们进程的CPU时间加给空闲进程
								{
									pSysProcInfo->UserTime.QuadPart += UserTime.QuadPart;
									pSysProcInfo->KernelTime.QuadPart += KernelTime.QuadPart;
									UserTime.QuadPart = KernelTime.QuadPart = 0;
									pPrev = pSysProcInfo;//保存上一个节点地址
								}
								if(pSysProcInfo->NextEntryDelta)
								{
									pSysProcInfo = (PSYSTEM_PROCESSES)((UCHAR*)pSysProcInfo + pSysProcInfo->NextEntryDelta);//pSysProcInfo指向下一个节点
								}
								else
								{
									pSysProcInfo = NULL;//最后一个
								}
							}
						}
						__except (EXCEPTION_EXECUTE_HANDLER)
						{
							return STATUS_UNSUCCESSFUL;
						}
					}
				}
			}
			return status;
		}
	}
	else
	{
		//KdPrint(("ThreadId %d SYSTEM_INFORMATION_CLASS %d \r\n",PsGetCurrentThreadId(),SystemInformationClass));
		if(Original_NtQuerySystemInformation)
		{
			status = Original_NtQuerySystemInformation(SystemInformationClass,SystemInformation,SystemInformationLength,ReturnLength);
			if(NT_SUCCESS(status))
			{
				if(SystemInformationClass==SystemKernelDebuggerInformation)
				{
					if(MmIsAddressValid(SystemInformation))
					{
						RtlZeroMemory(SystemInformation,SystemInformationLength);
					}
				}
				if(SystemInformationClass == SystemProcessesAndThreadsInformation)
				{
					PSYSTEM_PROCESSES pSysProcInfo,pPrev;
					if(MmIsAddressValid(SystemInformation))
					{
						pSysProcInfo=(PSYSTEM_PROCESSES)SystemInformation;
						pPrev = NULL;
						while(pSysProcInfo)
						{
							if(pPrev == NULL)
							{
								pPrev = pSysProcInfo;
							}
							if(pSysProcInfo->ProcessName.Buffer!=NULL)
							{
								if(IsProtectProcessId((HANDLE)pSysProcInfo->ProcessId))//是不是我们要隐藏的进程
								{
									if(pSysProcInfo->NextEntryDelta)//中间节点
									{
										pPrev->NextEntryDelta+=pSysProcInfo->NextEntryDelta;
									}
									else//最后一个节点
									{
										pPrev->NextEntryDelta=NULL;
									}
								}
								else
								{
									pPrev = pSysProcInfo;//保存上一个节点地址
								}
							}
							else //系统空闲进程,把我们进程的CPU时间加给空闲进程
							{
								pSysProcInfo->UserTime.QuadPart += UserTime.QuadPart;
								pSysProcInfo->KernelTime.QuadPart += KernelTime.QuadPart;
								UserTime.QuadPart = KernelTime.QuadPart = 0;
								pPrev = pSysProcInfo;//保存上一个节点地址
							}
							if(pSysProcInfo->NextEntryDelta)
							{
								pSysProcInfo = (PSYSTEM_PROCESSES)((UCHAR*)pSysProcInfo + pSysProcInfo->NextEntryDelta);//pSysProcInfo指向下一个节点
							}
							else
							{
								pSysProcInfo = NULL;//最后一个
							}
						}
					}
					else
					{
						return STATUS_UNSUCCESSFUL;
					}
				}
			}
			return status;
		}
	}
	return status;
}

NTQUERYINFORMATIONPROCESS Original_NtQueryInformationProcess = NULL;

NTSTATUS NtQueryInformationProcess_Proxy(
	__in          HANDLE ProcessHandle,
	__in          PROCESSINFOCLASS ProcessInformationClass,
	__out         PVOID ProcessInformation,
	__in          ULONG ProcessInformationLength,
	__out_opt     PULONG ReturnLength
	)
{
	NTSTATUS status = STATUS_SUCCESS;
	KPROCESSOR_MODE  mode;
	mode = ExGetPreviousMode();
	if (mode == UserMode)
	{
		if(Original_NtQueryInformationProcess)
		{
			status = Original_NtQueryInformationProcess(ProcessHandle,ProcessInformationClass,ProcessInformation,ProcessInformationLength,ReturnLength);
			if(ProcessInformationClass==ProcessDebugPort && NT_SUCCESS(status))
			{
				__try
				{
					ProbeForWrite(ProcessInformation,ProcessInformationLength,1);
					RtlZeroMemory(ProcessInformation,ProcessInformationLength);
				}
				__except (EXCEPTION_EXECUTE_HANDLER)
				{
					return STATUS_UNSUCCESSFUL;
				}
			}
			return status;
		}
	}
	else
	{
		if(Original_NtQuerySystemInformation)
		{
			status = Original_NtQueryInformationProcess(ProcessHandle,ProcessInformationClass,ProcessInformation,ProcessInformationLength,ReturnLength);
			if(ProcessInformationClass==ProcessDebugPort && NT_SUCCESS(status))
			{
				if(MmIsAddressValid(ProcessInformation))
				{
					RtlZeroMemory(ProcessInformation,ProcessInformationLength);
				}
			}
			return status;
		}
	}
	return status;
}

NTSETINFORMATIONTHREAD Original_NtSetInformationThread = NULL;

NTSTATUS NtSetInformationThread_Proxy(
									  HANDLE ThreadHandle,
									  THREADINFOCLASS ThreadInformationClass,
									  PVOID ThreadInformation,
									  ULONG ThreadInformationLength
									  )
{
	HANDLE ProcessID = PsGetCurrentProcessId();
	NTSTATUS status = STATUS_SUCCESS;
	if(ThreadInformationClass == ThreadHideFromDebugger)
	{
		KdPrint(("NtSetInformationThread_Proxy ProcessID[%d] \r\n",ProcessID));
		PrintLog(L"NtSetInformationThread_Proxy ProcessID[%d] \r\n",ProcessID);
		return status;
	}
	if(Original_NtSetInformationThread)
		status = Original_NtSetInformationThread(ThreadHandle,ThreadInformationClass,ThreadInformation,ThreadInformationLength);
	return status;
}

NTGETCONTEXTTHREAD Original_NtGetContextThread = NULL;

NTSTATUS NtGetContextThread_Proxy(
								  IN HANDLE ThreadHandle,
								  OUT PCONTEXT Context
								  )
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	if(Original_NtGetContextThread)
	{
		status = Original_NtGetContextThread(ThreadHandle,Context);
		if(NT_SUCCESS(status))
		{
			HANDLE ProcessId = PsGetCurrentProcessId();
			if(IsProtectProcessId(ProcessId))
			{
				return status;
			}
			if(IsTargetProcessId(ProcessId))
			{
				Context->Dr0 = 0;
				Context->Dr1 = 0;
				Context->Dr2 = 0;
				Context->Dr3 = 0;
				Context->Dr6 = 0;
				Context->Dr7 = 0;
				Context->EFlags &= ~0x100;//保护单步~
			}
		}
	}
	return status;
}

NTSETCONTEXTTHREAD Original_NtSetContextThread = NULL;

NTSTATUS NtSetContextThread_Proxy(
				   __in HANDLE ThreadHandle,
				   __in PCONTEXT ThreadContext
				   )
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	if(Original_NtSetContextThread)
	{
		HANDLE ProcessId = PsGetCurrentProcessId();
		if(IsProtectProcessId(ProcessId))
		{
			return Original_NtSetContextThread(ThreadHandle,ThreadContext);	
		}
	}
	return status;
}

NTTERMINATEPROCESS Original_NtTerminateProcess = NULL;

NTSTATUS NtTerminateProcess_Proxy(
								  IN HANDLE  ProcessHandle,
								  IN NTSTATUS  ExitStatus
								  )
{
	PEPROCESS peprocess;
	NTSTATUS status = STATUS_SUCCESS;
	status = ObReferenceObjectByHandle(ProcessHandle,0,NULL,KernelMode,(PVOID *)&peprocess,NULL);
	if(NT_SUCCESS(status))
	{
		if(IsTargetProcess(peprocess) && IsTargetProcessId(PsGetCurrentProcessId()))
		{
			KdPrint(("NtTerminateProcess_Proxy ThreadId[%d]\r\n",PsGetCurrentThreadId()));
			return status;
		}
	}
	if(Original_NtTerminateProcess)
		status = Original_NtTerminateProcess(ProcessHandle,ExitStatus);
	return status;
}

BOOL FindString(PWCHAR source,PWCHAR dst)
{
	int slen = wcslen(source);
	int dlen = wcslen(dst);
	if(slen<dlen)
	{
		return FALSE;
	}
	for (int i=0;i<slen-dlen+1;i++)
	{
		for(int j=0;j<dlen;j++)
		{
			WCHAR s = source[i+j];
			WCHAR d = dst[j];
			if(s!=d)
			{
				goto NOCMP;
			}
		}
		return TRUE;
NOCMP:
		;
	}
	return FALSE;
}

NTCREATEFILE Original_NtCreateFile = NULL;

NTSTATUS NtCreateFile_Proxy(
						   PHANDLE FileHandle,
						   ACCESS_MASK DesiredAccess,
						   POBJECT_ATTRIBUTES ObjectAttributes,
						   PIO_STATUS_BLOCK IoStatusBlock,
						   PLARGE_INTEGER AllocationSize,
						   ULONG FileAttributes,
						   ULONG ShareAccess,
						   ULONG CreateDisposition,
						   ULONG CreateOptions,
						   PVOID EaBuffer,
						   ULONG EaLength
						   )
{
	if(IsTargetProcessId(PsGetCurrentProcessId()))
	{
		if(ObjectAttributes && ObjectAttributes->ObjectName && ObjectAttributes->ObjectName->Buffer && ObjectAttributes->ObjectName->Length)
		{
			KdPrint(("NtCreateFile_Proxy[%ws] ThreadId[%d] DesiredAccess[%X] ShareAccess[%X]\r\n",ObjectAttributes->ObjectName->Buffer,PsGetCurrentThreadId(),DesiredAccess,ShareAccess));
			if(FindString(ObjectAttributes->ObjectName->Buffer,L".cmp"))
			{
				return STATUS_ACCESS_DENIED;
			}
			if(FindString(ObjectAttributes->ObjectName->Buffer,L".dmp"))
			{
				return STATUS_ACCESS_DENIED;
			}
			if(FindString(ObjectAttributes->ObjectName->Buffer,L".cra"))
			{
				return STATUS_ACCESS_DENIED;
			}
			/*if(FindString(ObjectAttributes->ObjectName->Buffer,L".zip"))
			{
				return STATUS_ACCESS_DENIED;
			}*/
		}
	}
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	if(Original_NtCreateFile)
	{
		status = Original_NtCreateFile(FileHandle,DesiredAccess,ObjectAttributes,IoStatusBlock,AllocationSize,FileAttributes,ShareAccess,CreateDisposition,CreateOptions,EaBuffer,EaLength);
	}
	return status;
}

NTRESUMETHREAD Original_NtResumeThread = NULL;

NTSTATUS NtResumeThread_Proxy(
							  __in HANDLE ThreadHandle,
							  __out_opt PULONG PreviousSuspendCount
							  )
{
	/*if(IsTargetProcessId(PsGetCurrentProcessId()))
	{
		return STATUS_SUCCESS;
	}*/
	if(Original_NtResumeThread)
	{
		return Original_NtResumeThread(ThreadHandle,PreviousSuspendCount);
	}
	return STATUS_UNSUCCESSFUL;
}

NTSUSPENDTHREAD Original_NtSuspendThread = NULL;

NTSTATUS NtSuspendThread_Proxy(
					  __in HANDLE ThreadHandle,
					  __out_opt PULONG PreviousSuspendCount
					  )
{
	if(Original_NtSuspendThread)
	{
		return Original_NtSuspendThread(ThreadHandle,PreviousSuspendCount);
	}
	return STATUS_UNSUCCESSFUL;
}

NTPROTECTVIRTUALMEMORY Original_NtProtectVirtualMemory = NULL;

NTSTATUS NtProtectVirtualMemory_Proxy(
							 __in HANDLE ProcessHandle,
							 __inout PVOID *BaseAddress,
							 __inout PSIZE_T RegionSize,
							 __in ULONG NewProtectWin32,
							 __out PULONG OldProtect
					   )
{
	NTSTATUS status = STATUS_SUCCESS;
	if(Original_NtProtectVirtualMemory)
	{
		HANDLE ProcessId = PsGetCurrentProcessId();
		if(IsTargetProcessId(ProcessId))
		{
			KdPrint(("NtProtectVirtualMemory BaseAddress[%p] RegionSize[%X] NewProtectWin32[%X] ThreadId[%d] \r\n",*BaseAddress,*RegionSize,NewProtectWin32,PsGetCurrentThreadId()));
			if(IsHideMemory((ULONG_PTR)(*BaseAddress)) || IsHideMemory((ULONG_PTR)(*BaseAddress)+ULONG_PTR(*RegionSize)))
			{
				KdPrint(("NtProtectVirtualMemory HideMemory %p \r\n",*BaseAddress));
				return STATUS_UNSUCCESSFUL;
			}
		}
		status =  Original_NtProtectVirtualMemory(ProcessHandle,BaseAddress,RegionSize,NewProtectWin32,OldProtect);
	}
	return status;
}

NTQUERYINFORMATIONTHREAD Original_NtQueryInformationThread = NULL;

NTSTATUS NtQueryInformationThread_Proxy(
										__in HANDLE ThreadHandle,
										__in THREADINFOCLASS ThreadInformationClass,
										__out_bcount(ThreadInformationLength) PVOID ThreadInformation,
										__in ULONG ThreadInformationLength,
										__out_opt PULONG ReturnLength
										)
{
	if(Original_NtQueryInformationThread)
	{
		HANDLE ProcessId = PsGetCurrentThreadId();
		if(IsTargetProcessId(ProcessId))
		{
			KdPrint(("NtQueryInformationThread %d THREADINFOCLASS %d \r\n",PsGetCurrentThreadId(),ThreadInformationClass));
		}
		return Original_NtQueryInformationThread(ThreadHandle,ThreadInformationClass,ThreadInformation,ThreadInformationLength,ReturnLength);
	}
	return STATUS_UNSUCCESSFUL;
}

NTOPENTHREAD Original_NtOpenThread = NULL;

NTSTATUS NtOpenThread_Proxy(
							__out PHANDLE ThreadHandle,
							__in ACCESS_MASK DesiredAccess,
							__in POBJECT_ATTRIBUTES ObjectAttributes,
							__in_opt PCLIENT_ID ClientId
							)
{
	if(Original_NtOpenThread)
	{
		return Original_NtOpenThread(ThreadHandle,DesiredAccess,ObjectAttributes,ClientId);
	}
	return FALSE;
}


NTCREATETHREAD Original_NtCreateThread = NULL;

NTSTATUS NtCreateThread_Proxy(
							  __out PHANDLE ThreadHandle,
							  __in ACCESS_MASK DesiredAccess,
							  __in_opt POBJECT_ATTRIBUTES ObjectAttributes,
							  __in HANDLE ProcessHandle,
							  __out PCLIENT_ID ClientId,
							  __in PCONTEXT ThreadContext,
							  __in PINITIAL_TEB InitialTeb,
							  __in BOOLEAN CreateSuspended
							  )
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	if(IsTargetProcessId(PsGetCurrentProcessId()))
	{
		KdPrint(("NtCreateThread_Proxy %d \r\n",PsGetCurrentThreadId()));
		if(Original_NtCreateThread)
		{
			status = Original_NtCreateThread(ThreadHandle,DesiredAccess,ObjectAttributes,ProcessHandle,ClientId,ThreadContext,InitialTeb,TRUE);
			if(NT_SUCCESS(status))
			{
				PETHREAD pethread;
				status = PsLookupThreadByThreadId(ClientId->UniqueThread,&pethread);
				if(NT_SUCCESS(status))
				{
					ULONG offset = GetWin32StartAddress_Offset();
					if(MmIsAddressValid(PVOID((ULONG)pethread + offset)))
					{
						ULONG Win32StartAddress = *PULONG_PTR((ULONG)pethread + offset);
						KdPrint(("Win32StartAddress %p \r\n",Win32StartAddress));
					}
				}
				if(!CreateSuspended)
				{
					if(Original_NtResumeThread)
					{
						ULONG PreviousSuspendCount;
						status = Original_NtResumeThread(*ThreadHandle,&PreviousSuspendCount);
					}
				}
			}
		}
	}
	else
	{
		if(Original_NtCreateThread)
		{
			status = Original_NtCreateThread(ThreadHandle,DesiredAccess,ObjectAttributes,ProcessHandle,ClientId,ThreadContext,InitialTeb,CreateSuspended);
		}
	}
	return status;
}

NTQUERYVIRTUALMEMORY Original_NtQueryVirtualMemory = NULL;

NTSTATUS NtQueryVirtualMemory_Proxy(
									__in HANDLE ProcessHandle,
									__in PVOID BaseAddress,
									__in MEMORY_INFORMATION_CLASS MemoryInformationClass,
									__out_bcount(MemoryInformationLength) PVOID MemoryInformation,
									__in SIZE_T MemoryInformationLength,
									__out_opt PSIZE_T ReturnLength
									)
{
	if(IsTargetProcessId(PsGetCurrentProcessId()))
	{
		KdPrint(("NtQueryVirtualMemory_Proxy ThreadId[%d] BaseAddress[%X] MemoryInformationClass[%d] \r\n",PsGetCurrentThreadId(),BaseAddress,MemoryInformationClass));
		if(IsHideMemory((ULONG_PTR)BaseAddress))
		{
			KdPrint(("NtQueryVirtualMemory_Proxy HideMemory %p\r\n",BaseAddress));
			return STATUS_UNSUCCESSFUL;
		}
	}
	if(Original_NtQueryVirtualMemory)
	{
		return Original_NtQueryVirtualMemory(ProcessHandle,BaseAddress,MemoryInformationClass,MemoryInformation,MemoryInformationLength,ReturnLength);
	}
	return STATUS_UNSUCCESSFUL;
}

NTCREATEMUTANT Original_NtCreateMutant = NULL;

NTSTATUS NtCreateMutant_Proxy (
							   __out PHANDLE MutantHandle,
							   __in ACCESS_MASK DesiredAccess,
							   __in_opt POBJECT_ATTRIBUTES ObjectAttributes,
							   __in BOOLEAN InitialOwner
							   )
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	if(Original_NtCreateMutant)
	{
		if(IsTargetProcessId(PsGetCurrentProcessId()))
		{
			if(ObjectAttributes && ObjectAttributes->ObjectName && ObjectAttributes->ObjectName->Buffer && ObjectAttributes->ObjectName->Length)
			{
				KdPrint(("NtCreateMutant_Proxy[%ws] ThreadId[%d] DesiredAccess[%X] \r\n",ObjectAttributes->ObjectName->Buffer,PsGetCurrentThreadId(),DesiredAccess));
				if(!wcscmp(ObjectAttributes->ObjectName->Buffer,L"dbefeuate_ccen_khxfor_lcar_blr"))
				{
					status =  Original_NtCreateMutant(MutantHandle,DesiredAccess,ObjectAttributes,InitialOwner);
					return STATUS_SUCCESS;
				}
			}
		}
		status =  Original_NtCreateMutant(MutantHandle,DesiredAccess,ObjectAttributes,InitialOwner);
	}
	return status;
}

NTCREATESEMAPHORE Original_NtCreateSemaphore = NULL;

NTSTATUS NtCreateSemaphore_Proxy (
								  __out PHANDLE SemaphoreHandle,
								  __in ACCESS_MASK DesiredAccess,
								  __in_opt POBJECT_ATTRIBUTES ObjectAttributes,
								  __in LONG InitialCount,
								  __in LONG MaximumCount
								  )
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	if(Original_NtCreateSemaphore)
	{
		if(IsTargetProcessId(PsGetCurrentProcessId()))
		{
			if(ObjectAttributes && ObjectAttributes->ObjectName && ObjectAttributes->ObjectName->Buffer && ObjectAttributes->ObjectName->Length)
			{
				KdPrint(("NtCreateSemaphore_Proxy[%ws] ThreadId[%d] DesiredAccess[%X] \r\n",ObjectAttributes->ObjectName->Buffer,PsGetCurrentThreadId(),DesiredAccess));
			}
		}
		status =  Original_NtCreateSemaphore(SemaphoreHandle,DesiredAccess,ObjectAttributes,InitialCount,MaximumCount);
	}
	return status;
}

NTCONTINUE Original_NtContinue = NULL;

void MyNtContinue(PCONTEXT ContextRecord,PKTRAP_FRAME pKtf)
{
	KPROCESSOR_MODE  mode;
	mode = ExGetPreviousMode();
	if (mode == UserMode)
	{
		if(IsTargetProcessId(PsGetCurrentProcessId()))
		{
			__try
			{
				ProbeForRead(ContextRecord,sizeof(PCONTEXT),1);
				KdPrint(("NtContinue ThreadId:[%d] Eax[0x%X] Ecx[0x%X] Edx[0x%X] Edi[0x%X] Esi[0x%X] Ebx[0x%X] Ebp[0x%X] Eip[0x%X] Dr0[0x%X] Dr1[0x%X] Dr2[0x%X] Dr3[0x%X] Dr6[0x%X] Dr7[0x%X]\r\n",
					     PsGetCurrentThreadId(),ContextRecord->Eax,ContextRecord->Ecx,ContextRecord->Edx,ContextRecord->Edi,ContextRecord->Esi,ContextRecord->Ebx,ContextRecord->Ebp,ContextRecord->Eip,ContextRecord->Dr0,ContextRecord->Dr1,ContextRecord->Dr2,ContextRecord->Dr3,ContextRecord->Dr6,ContextRecord->Dr7));
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
			}
		}
	}
}

void __declspec(naked) NtContinue_Proxy()
{
	_asm
	{
		push ebp
		mov  ebp,esp
		pushfd
		mov  eax,dword ptr [ebp]
		mov  ecx,dword ptr [ebp+8]
		push eax
		push ecx
		call MyNtContinue
		popfd
		pop ebp
		jmp Original_NtContinue
	}
}

NTSTATUS KdDisableDebuggerProxy()
{
	return STATUS_SUCCESS;
}

BOOLEAN KdDebuggerEnabledProxy = FALSE;

PVOID MmGetSystemRoutineAddress_InlineProxy(__in PUNICODE_STRING SystemRoutineName)
{
	
	if(SystemRoutineName && SystemRoutineName->Buffer)
	{
		KdPrint(("SystemRoutineName %ws \r\n",SystemRoutineName->Buffer));
		PVOID address = NULL;
		UNICODE_STRING FunctionName;
		RtlInitUnicodeString(&FunctionName,L"KdDisableDebugger");
		if(!RtlCompareUnicodeString(SystemRoutineName,&FunctionName,false))
		{
			return KdDisableDebuggerProxy;
		}
		RtlInitUnicodeString(&FunctionName,L"KdDebuggerEnabled");
		if(!RtlCompareUnicodeString(SystemRoutineName,(PUNICODE_STRING)&FunctionName,false))
		{
			return &KdDebuggerEnabledProxy;
		}
	}
	MMGETSYSTEMROUTINEADDRESS function = (MMGETSYSTEMROUTINEADDRESS)GetRealFunctionAddress("MmGetSystemRoutineAddress");
	if(function)
	{
		return function(SystemRoutineName);
	}
	return NULL;
}

PVOID MmGetSystemRoutineAddress_Proxy(__in PUNICODE_STRING SystemRoutineName)
{
	if(SystemRoutineName && SystemRoutineName->Buffer)
	{
		KdPrint(("SystemRoutineName %ws \r\n",SystemRoutineName->Buffer));
		PVOID address = NULL;
		UNICODE_STRING FunctionName;
		RtlInitUnicodeString(&FunctionName,L"KdDisableDebugger");
		if(!RtlCompareUnicodeString(SystemRoutineName,&FunctionName,false))
		{
			return KdDisableDebuggerProxy;
		}
		RtlInitUnicodeString(&FunctionName,L"KdDebuggerEnabled");
		if(!RtlCompareUnicodeString(SystemRoutineName,(PUNICODE_STRING)&FunctionName,false))
		{
			return &KdDebuggerEnabledProxy;
		}
		RtlInitUnicodeString(&FunctionName,L"PsGetCurrentProcessId");
		if(!RtlCompareUnicodeString(SystemRoutineName,(PUNICODE_STRING)&FunctionName,false))
		{
			return PsGetCurrentProcessId_Proxy;
		}
	}
	return MmGetSystemRoutineAddress(SystemRoutineName);
}

NTSTATUS PsCreateSystemThread_InlineProxy(
	OUT PHANDLE  ThreadHandle,
	IN ULONG  DesiredAccess,
	IN POBJECT_ATTRIBUTES  ObjectAttributes  OPTIONAL,
	IN HANDLE  ProcessHandle  OPTIONAL,
	OUT PCLIENT_ID  ClientId  OPTIONAL,
	IN PKSTART_ROUTINE  StartRoutine,
	IN PVOID  StartContext
	)
{
	if(IsInTargetKernelModule((ULONG_PTR)StartRoutine))
	{
		KdPrint(("ThreadId %d \r\n",PsGetCurrentThreadId()));
		return STATUS_SUCCESS;
	}
	KdPrint(("StartRoutine %p \r\n",StartRoutine));
	PSCREATESYSTEMTHREAD function = (PSCREATESYSTEMTHREAD)GetRealFunctionAddress("PsCreateSystemThread");
	if(function)
	{
		return function(ThreadHandle,DesiredAccess,ObjectAttributes,ProcessHandle,ClientId,StartRoutine,StartContext);
	}
	return STATUS_UNSUCCESSFUL;
}

NTRAISEHARDERROR Original_NtRaiseHardError = NULL;

NTSTATUS NtRaiseHardError_Proxy (
						__in NTSTATUS ErrorStatus,
						__in ULONG NumberOfParameters,
						__in ULONG UnicodeStringParameterMask,
						__in_ecount(NumberOfParameters) PULONG_PTR Parameters,
						__in ULONG ValidResponseOptions,
						__out PULONG Response
				  )
{
	while(true)
	{
		KdPrint(("TheadId:%d\r\n",PsGetCurrentThreadId()));
		KeSleep(1000);
	}
	return STATUS_SUCCESS;
}

PEPROCESS IoGetCurrentProcess_Proxy()
{
	return IoGetCurrentProcess();
}

HANDLE PsGetCurrentProcessId_Proxy()
{
	return PsGetCurrentProcessId();
}

NTSTATUS ZwQuerySystemInformation_Proxy(
										__in          SYSTEM_INFORMATION_CLASS SystemInformationClass,
										__inout       PVOID SystemInformation,
										__in          ULONG SystemInformationLength,
										__out_opt     PULONG ReturnLength
										)
{
	NTSTATUS status = STATUS_SUCCESS;
	KPROCESSOR_MODE  mode;
	mode = ExGetPreviousMode();
	if (mode == UserMode)
	{
		KdPrint(("ZwQuerySystemInformation_Proxy ProcessId %d SYSTEM_INFORMATION_CLASS %d \r\n",PsGetCurrentProcessId(),SystemInformationClass));
		status = ZwQuerySystemInformation(SystemInformationClass,SystemInformation,SystemInformationLength,ReturnLength);
		return status;
	}
	else
	{
		KdPrint(("ZwQuerySystemInformation_Proxy ThreadId %d SYSTEM_INFORMATION_CLASS %d \r\n",PsGetCurrentThreadId(),SystemInformationClass));
		status = ZwQuerySystemInformation(SystemInformationClass,SystemInformation,SystemInformationLength,ReturnLength);
		return status;
	}
	return status;
}

NTSTATUS ReplaceFakeSSDTFunction()
{
	PKeHookInfo pKeHookInfo = GetKeHookInfo();
	SYSTEM_VERSION version = GetSystemVersion();
	switch(version)
	{
	case WINDOWSXP:
		{
			//Original_NtQuerySystemInformation = (NTQUERYSYSTEMINFORMATION)ReplaceSSDTApi(pKeHookInfo->FakeSSDT->pvSSDTBase,NtQuerySystemInformation_Index_WinXP,(ULONG_PTR)NtQuerySystemInformation_Proxy);
			//Original_NtQueryInformationProcess = (NTQUERYINFORMATIONPROCESS)ReplaceSSDTApi(pKeHookInfo->FakeSSDT->pvSSDTBase,NtQueryInformationProcess_Index_WinXP,(ULONG_PTR)NtQueryInformationProcess_Proxy);
			//Original_NtSetInformationThread = (NTSETINFORMATIONTHREAD)ReplaceSSDTApi(pKeHookInfo->FakeSSDT->pvSSDTBase,NtSetInformationThread_Index_WinXP,(ULONG_PTR)NtSetInformationThread_Proxy);
			//Original_NtGetContextThread = (NTGETCONTEXTTHREAD)ReplaceSSDTApi(pKeHookInfo->FakeSSDT->pvSSDTBase,NtGetContextThread_Index_WinXP,(ULONG_PTR)NtGetContextThread_Proxy);
			//Original_NtTerminateProcess = (NTTERMINATEPROCESS)ReplaceSSDTApi(pKeHookInfo->FakeSSDT->pvSSDTBase,NtTerminateProcess_Index_WinXP,(ULONG_PTR)NtTerminateProcess_Proxy);
			//Original_NtProtectVirtualMemory = (NTPROTECTVIRTUALMEMORY)ReplaceSSDTApi(pKeHookInfo->FakeSSDT->pvSSDTBase,NtProtectVirtualMemory_Index_WinXP,(ULONG_PTR)NtProtectVirtualMemory_Proxy);
			//Original_NtQueryInformationThread = (NTQUERYINFORMATIONTHREAD)ReplaceSSDTApi(pKeHookInfo->FakeSSDT->pvSSDTBase,NtQueryInformationThread_Index_WinXP,(ULONG_PTR)NtQueryInformationThread_Proxy);
			//Original_NtCreateThread = (NTCREATETHREAD)ReplaceSSDTApi(pKeHookInfo->FakeSSDT->pvSSDTBase,NtCreateThread_Index_WinXP,(ULONG_PTR)NtCreateThread_Proxy);
			//Original_NtQueryVirtualMemory = (NTQUERYVIRTUALMEMORY)ReplaceSSDTApi(pKeHookInfo->FakeSSDT->pvSSDTBase,NtQueryVirtualMemory_Index_WinXP,(ULONG_PTR)NtQueryVirtualMemory_Proxy);
			//Original_NtContinue = (NTCONTINUE)ReplaceSSDTApi(pKeHookInfo->FakeSSDT->pvSSDTBase,NtContinue_Index_WinXP,(ULONG_PTR)NtContinue_Proxy);
			Original_NtRaiseHardError = (NTRAISEHARDERROR)ReplaceSSDTApi(pKeHookInfo->FakeSSDT->pvSSDTBase,NtRaiseHardError_Index_WinXP,(ULONG_PTR)NtRaiseHardError_Proxy);
			//Original_NtResumeThread = (NTRESUMETHREAD)ReplaceSSDTApi(pKeHookInfo->FakeSSDT->pvSSDTBase,NtResumeThread_Index_WinXP,(ULONG_PTR)NtResumeThread_Proxy);
			//Original_NtCreateFile = (NTCREATEFILE)ReplaceSSDTApi(pKeHookInfo->FakeSSDT->pvSSDTBase,NtCreateFile_Index_WinXP,(ULONG_PTR)NtCreateFile_Proxy);
			//Original_NtOpenProcess = (NTOPENPROCESS)ReplaceSSDTApi(pKeHookInfo->FakeSSDT->pvSSDTBase,NtOpenProcess_Index_WinXP,(ULONG_PTR)NtOpenProcess_Proxy);
			//Original_NtCreateMutant = (NTCREATEMUTANT)ReplaceSSDTApi(pKeHookInfo->FakeSSDT->pvSSDTBase,NtCreateMutant_Index_WinXP,(ULONG_PTR)NtCreateMutant_Proxy);
			//Original_NtCreateSemaphore = (NTCREATESEMAPHORE)ReplaceSSDTApi(pKeHookInfo->FakeSSDT->pvSSDTBase,NtCreateSemaphore_Index_WinXP,(ULONG_PTR)NtCreateSemaphore_Proxy);
			//SetOriginalNtDeviceIoControlFile((NTDEVICEIOCONTROLFILE)ReplaceSSDTApi(pKeHookInfo->FakeSSDT->pvSSDTBase,NtDeviceIoControlFile_Index_WinXP,(ULONG_PTR)NtDeviceIoControlFile_Proxy));
			//Replace_NtOpenProcess = (NTOPENPROCESS)GetRelocateSSDTByIndex(NtOpenProcess_Index_WinXP);
		}
		break;
	case WINDOWS7:
		{
			//Original_NtQuerySystemInformation = (NTQUERYSYSTEMINFORMATION)ReplaceSSDTApi(pKeHookInfo->FakeSSDT->pvSSDTBase,NtQuerySystemInformation_Index_Win7,(ULONG_PTR)NtQuerySystemInformation_Proxy);
			//Original_NtQueryInformationProcess = (NTQUERYINFORMATIONPROCESS)ReplaceSSDTApi(pKeHookInfo->FakeSSDT->pvSSDTBase,NtQueryInformationProcess_Index_Win7,(ULONG_PTR)NtQueryInformationProcess_Proxy);
			//Original_NtSetInformationThread = (NTSETINFORMATIONTHREAD)ReplaceSSDTApi(pKeHookInfo->FakeSSDT->pvSSDTBase,NtSetInformationThread_Index_Win7,(ULONG_PTR)NtSetInformationThread_Proxy);
			Original_NtGetContextThread = (NTGETCONTEXTTHREAD)ReplaceSSDTApi(pKeHookInfo->FakeSSDT->pvSSDTBase,NtGetContextThread_Index_Win7,(ULONG_PTR)NtGetContextThread_Proxy);
			//Original_NtTerminateProcess = (NTTERMINATEPROCESS)ReplaceSSDTApi(pKeHookInfo->FakeSSDT->pvSSDTBase,NtTerminateProcess_Index_Win7,(ULONG_PTR)NtTerminateProcess_Proxy);
			Original_NtProtectVirtualMemory = (NTPROTECTVIRTUALMEMORY)ReplaceSSDTApi(pKeHookInfo->FakeSSDT->pvSSDTBase,NtProtectVirtualMemory_Index_Win7,(ULONG_PTR)NtProtectVirtualMemory_Proxy);
			//Original_NtQueryInformationThread = (NTQUERYINFORMATIONTHREAD)ReplaceSSDTApi(pKeHookInfo->FakeSSDT->pvSSDTBase,NtQueryInformationThread_Index_Win7,(ULONG_PTR)NtQueryInformationThread_Proxy);
			//Original_NtCreateThread = (NTCREATETHREAD)ReplaceSSDTApi(pKeHookInfo->FakeSSDT->pvSSDTBase,NtCreateThread_Index_Win7,(ULONG_PTR)NtCreateThread_Proxy);
			Original_NtQueryVirtualMemory = (NTQUERYVIRTUALMEMORY)ReplaceSSDTApi(pKeHookInfo->FakeSSDT->pvSSDTBase,NtQueryVirtualMemory_Index_Win7,(ULONG_PTR)NtQueryVirtualMemory_Proxy);
			Original_NtContinue = (NTCONTINUE)ReplaceSSDTApi(pKeHookInfo->FakeSSDT->pvSSDTBase,NtContinue_Index_Win7,(ULONG_PTR)NtContinue_Proxy);
			//Original_NtResumeThread = (NTRESUMETHREAD)ReplaceSSDTApi(pKeHookInfo->FakeSSDT->pvSSDTBase,NtResumeThread_Index_Win7,(ULONG_PTR)NtResumeThread_Proxy);
			//Original_NtCreateFile = (NTCREATEFILE)ReplaceSSDTApi(pKeHookInfo->FakeSSDT->pvSSDTBase,NtCreateFile_Index_Win7,(ULONG_PTR)NtCreateFile_Proxy);
			SetOriginalNtDeviceIoControlFile((NTDEVICEIOCONTROLFILE)ReplaceSSDTApi(pKeHookInfo->FakeSSDT->pvSSDTBase,NtDeviceIoControlFile_Index_Win7,(ULONG_PTR)NtDeviceIoControlFile_Proxy));
			//Replace_NtOpenProcess = (NTOPENPROCESS)GetRelocateSSDTByIndex(NtOpenProcess_Index_Win7);
		}
		break;
	}
	return STATUS_SUCCESS;
}

VOID ReplaceSSDTFunction()
{
	SYSTEM_VERSION version = GetSystemVersion();
	switch(version)
	{
	case WINDOWSXP:
		{
			//Original_NtGetContextThread = (NTGETCONTEXTTHREAD)ReplaceSSDTApi(GetCurrentSSDT()->pvSSDTBase,NtGetContextThread_Index_WinXP,(ULONG_PTR)NtGetContextThread_Proxy);
			//Original_NtSetContextThread = (NTSETCONTEXTTHREAD)ReplaceSSDTApi(GetCurrentSSDT()->pvSSDTBase,NtSetContextThread_Index_WinXP,(ULONG_PTR)NtSetContextThread_Proxy);
			Original_NtProtectVirtualMemory = (NTPROTECTVIRTUALMEMORY)ReplaceSSDTApi(GetCurrentSSDT()->pvSSDTBase,NtProtectVirtualMemory_Index_WinXP,(ULONG_PTR)NtProtectVirtualMemory_Proxy);
			Original_NtQueryVirtualMemory = (NTQUERYVIRTUALMEMORY)ReplaceSSDTApi(GetCurrentSSDT()->pvSSDTBase,NtQueryVirtualMemory_Index_WinXP,(ULONG_PTR)NtQueryVirtualMemory_Proxy);
			//Original_NtTerminateProcess = (NTTERMINATEPROCESS)ReplaceSSDTApi(GetCurrentSSDT()->pvSSDTBase,NtTerminateProcess_Index_WinXP,(ULONG_PTR)NtTerminateProcess_Proxy);
			Original_NtContinue = (NTCONTINUE)ReplaceSSDTApi(GetCurrentSSDT()->pvSSDTBase,NtContinue_Index_WinXP,(ULONG_PTR)NtContinue_Proxy);
		}
		break;
	case WINDOWS7:
		{
			//Original_NtGetContextThread = (NTGETCONTEXTTHREAD)ReplaceSSDTApi(GetCurrentSSDT()->pvSSDTBase,NtGetContextThread_Index_Win7,(ULONG_PTR)NtGetContextThread_Proxy);
			//Original_NtSetContextThread = (NTSETCONTEXTTHREAD)ReplaceSSDTApi(GetCurrentSSDT()->pvSSDTBase,NtSetContextThread_Index_Win7,(ULONG_PTR)NtSetContextThread_Proxy);
			Original_NtProtectVirtualMemory = (NTPROTECTVIRTUALMEMORY)ReplaceSSDTApi(GetCurrentSSDT()->pvSSDTBase,NtProtectVirtualMemory_Index_Win7,(ULONG_PTR)NtProtectVirtualMemory_Proxy);
			Original_NtQueryVirtualMemory = (NTQUERYVIRTUALMEMORY)ReplaceSSDTApi(GetCurrentSSDT()->pvSSDTBase,NtQueryVirtualMemory_Index_Win7,(ULONG_PTR)NtQueryVirtualMemory_Proxy);
			//Original_NtTerminateProcess = (NTTERMINATEPROCESS)ReplaceSSDTApi(GetCurrentSSDT()->pvSSDTBase,NtTerminateProcess_Index_Win7,(ULONG_PTR)NtTerminateProcess_Proxy);
			Original_NtContinue = (NTCONTINUE)ReplaceSSDTApi(GetCurrentSSDT()->pvSSDTBase,NtContinue_Index_Win7,(ULONG_PTR)NtContinue_Proxy);
		}
		break;
	}
}

VOID RecoverSSDTFunction()
{
	SYSTEM_VERSION version = GetSystemVersion();
	switch(version)
	{
	case WINDOWSXP:
		{
			//if(Original_NtSetContextThread){ReplaceSSDTApi(GetCurrentSSDT()->pvSSDTBase,NtSetContextThread_Index_WinXP,(ULONG_PTR)Original_NtSetContextThread);}
			//if(Original_NtGetContextThread){ReplaceSSDTApi(GetCurrentSSDT()->pvSSDTBase,NtGetContextThread_Index_WinXP,(ULONG_PTR)Original_NtGetContextThread);}
			if(Original_NtProtectVirtualMemory){ReplaceSSDTApi(GetCurrentSSDT()->pvSSDTBase,NtProtectVirtualMemory_Index_WinXP,(ULONG_PTR)Original_NtProtectVirtualMemory);}
			if(Original_NtQueryVirtualMemory){ReplaceSSDTApi(GetCurrentSSDT()->pvSSDTBase,NtQueryVirtualMemory_Index_WinXP,(ULONG_PTR)Original_NtQueryVirtualMemory);}
			//if(Original_NtTerminateProcess){ReplaceSSDTApi(GetCurrentSSDT()->pvSSDTBase,NtQueryVirtualMemory_Index_WinXP,(ULONG_PTR)Original_NtTerminateProcess);}
			if(Original_NtContinue){ReplaceSSDTApi(GetCurrentSSDT()->pvSSDTBase,NtContinue_Index_WinXP,(ULONG_PTR)Original_NtContinue);}
		}
		break;
	case WINDOWS7:
		{
			//if(Original_NtSetContextThread){ReplaceSSDTApi(GetCurrentSSDT()->pvSSDTBase,NtSetContextThread_Index_Win7,(ULONG_PTR)Original_NtSetContextThread);}
			//if(Original_NtGetContextThread){ReplaceSSDTApi(GetCurrentSSDT()->pvSSDTBase,NtGetContextThread_Index_Win7,(ULONG_PTR)Original_NtGetContextThread);}
			if(Original_NtProtectVirtualMemory){ReplaceSSDTApi(GetCurrentSSDT()->pvSSDTBase,NtProtectVirtualMemory_Index_Win7,(ULONG_PTR)Original_NtProtectVirtualMemory);}
			if(Original_NtQueryVirtualMemory){ReplaceSSDTApi(GetCurrentSSDT()->pvSSDTBase,NtQueryVirtualMemory_Index_Win7,(ULONG_PTR)Original_NtQueryVirtualMemory);}
			//if(Original_NtTerminateProcess){ReplaceSSDTApi(GetCurrentSSDT()->pvSSDTBase,NtQueryVirtualMemory_Index_Win7,(ULONG_PTR)Original_NtTerminateProcess);}
			if(Original_NtContinue){ReplaceSSDTApi(GetCurrentSSDT()->pvSSDTBase,NtContinue_Index_Win7,(ULONG_PTR)Original_NtContinue);}
		}
		break;
	}
}

