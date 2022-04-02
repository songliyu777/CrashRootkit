#pragma once

#include "MyWinNT.h"
#define _CRD_
#include "Transfer.h"

#define  NtQuerySystemInformation_Index_WinXP 173
#define  NtQuerySystemInformation_Index_Win7 261

typedef NTSTATUS (*NTQUERYSYSTEMINFORMATION)(
	__in          SYSTEM_INFORMATION_CLASS SystemInformationClass,
	__inout       PVOID SystemInformation,
	__in          ULONG SystemInformationLength,
	__out_opt     PULONG ReturnLength
	);

NTSTATUS NtQuerySystemInformation_Proxy(
									   __in          SYSTEM_INFORMATION_CLASS SystemInformationClass,
									   __inout       PVOID SystemInformation,
									   __in          ULONG SystemInformationLength,
									   __out_opt     PULONG ReturnLength
									   );

#define NtQueryInformationProcess_Index_WinXP 154
#define NtQueryInformationProcess_Index_Win7 251

typedef NTSTATUS (*NTQUERYINFORMATIONPROCESS)(
	__in          HANDLE ProcessHandle,
	__in          PROCESSINFOCLASS ProcessInformationClass,
	__out         PVOID ProcessInformation,
	__in          ULONG ProcessInformationLength,
	__out_opt     PULONG ReturnLength
	);

NTSTATUS NtQueryInformationProcess_Proxy(
	__in          HANDLE ProcessHandle,
	__in          PROCESSINFOCLASS ProcessInformationClass,
	__out         PVOID ProcessInformation,
	__in          ULONG ProcessInformationLength,
	__out_opt     PULONG ReturnLength
	);


#define NtCreateFile_Index_WinXP 37
#define NtCreateFile_Index_Win7	66

typedef NTSTATUS (*NTCREATEFILE)(
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
								 );
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
							);

#define NtResumeThread_Index_WinXP 206
#define NtResumeThread_Index_Win7  304

typedef NTSTATUS (*NTRESUMETHREAD)(
						   __in HANDLE ThreadHandle,
						   __out_opt PULONG PreviousSuspendCount
						   );

NTSTATUS NtResumeThread_Proxy(
			   __in HANDLE ThreadHandle,
			   __out_opt PULONG PreviousSuspendCount
			   );


#define NtSetInformationThread_Index_WinXP 229
#define NtSetInformationThread_Index_Win7  335

typedef NTSTATUS (*NTSETINFORMATIONTHREAD)(
	HANDLE ThreadHandle,
	THREADINFOCLASS ThreadInformationClass,
	PVOID ThreadInformation,
	ULONG ThreadInformationLength
	);

NTSTATUS NtSetInformationThread_Proxy(
	HANDLE ThreadHandle,
	THREADINFOCLASS ThreadInformationClass,
	PVOID ThreadInformation,
	ULONG ThreadInformationLength
	);

#define NtQueryInformationThread_Index_WinXP 155
#define NtQueryInformationThread_Index_Win7 236

typedef NTSTATUS (*NTQUERYINFORMATIONTHREAD)(
	__in HANDLE ThreadHandle,
	__in THREADINFOCLASS ThreadInformationClass,
	__out_bcount(ThreadInformationLength) PVOID ThreadInformation,
	__in ULONG ThreadInformationLength,
	__out_opt PULONG ReturnLength
	);

NTSTATUS NtQueryInformationThread_Proxy(
						 __in HANDLE ThreadHandle,
						 __in THREADINFOCLASS ThreadInformationClass,
						 __out_bcount(ThreadInformationLength) PVOID ThreadInformation,
						 __in ULONG ThreadInformationLength,
						 __out_opt PULONG ReturnLength
						 );

#define NtOpenProcess_Index_WinXP 122
#define NtOpenProcess_Index_Win7  190

typedef NTSTATUS (*NTOPENPROCESS)(
								  __out PHANDLE  ProcessHandle,
								  __in ACCESS_MASK  DesiredAccess,
								  __in POBJECT_ATTRIBUTES  ObjectAttributes,
								  __in_opt PCLIENT_ID  ClientId
								  );

NTSTATUS NtOpenProcess_Proxy(
							__out PHANDLE  ProcessHandle,
							__in ACCESS_MASK  DesiredAccess,
							__in POBJECT_ATTRIBUTES  ObjectAttributes,
							__in_opt PCLIENT_ID  ClientId
							);

NTSTATUS Relocate_NtOpenProcess(
							  __out PHANDLE  ProcessHandle,
							  __in ACCESS_MASK  DesiredAccess,
							  __in POBJECT_ATTRIBUTES  ObjectAttributes,
							  __in_opt PCLIENT_ID  ClientId
							  );

#define NtOpenThread_Index_WinXP 128

typedef NTSTATUS (*NTOPENTHREAD) (
			  __out PHANDLE ThreadHandle,
			  __in ACCESS_MASK DesiredAccess,
			  __in POBJECT_ATTRIBUTES ObjectAttributes,
			  __in_opt PCLIENT_ID ClientId
			  );

NTSTATUS NtOpenThread_Proxy(
							  __out PHANDLE ThreadHandle,
							  __in ACCESS_MASK DesiredAccess,
							  __in POBJECT_ATTRIBUTES ObjectAttributes,
							  __in_opt PCLIENT_ID ClientId
							  );

#define NtDebugContinue_Index_WinXP 58
#define NtDebugContinue_Index_Win7 97

#define NtDebugActiveProcess_Index_WinXP 57
#define NtDebugActiveProcess_Index_Win7 96

typedef NTSTATUS (*NTDEBUGACTIVEPROCESS)(
	IN HANDLE ProcessHandle,
	IN HANDLE DebugObjectHandle
	);

#define NtReadVirtualMemory_Index_WinXP 186
#define NtReadVirtualMemory_Index_Win7  277

#define NtProtectVirtualMemory_Index_WinXP 137
#define NtProtectVirtualMemory_Index_Win7  215

typedef NTSTATUS (*NTPROTECTVIRTUALMEMORY)(
	__in HANDLE ProcessHandle,
	__inout PVOID *BaseAddress,
	__inout PSIZE_T RegionSize,
	__in ULONG NewProtectWin32,
	__out PULONG OldProtect
	);

NTSTATUS
NtProtectVirtualMemory_Proxy(
					   __in HANDLE ProcessHandle,
					   __inout PVOID *BaseAddress,
					   __inout PSIZE_T RegionSize,
					   __in ULONG NewProtectWin32,
					   __out PULONG OldProtect
					   );

#define NtWriteVirtualMemory_Index_WinXP 277
#define NtWriteVirtualMemory_Index_Win7  399

#define NtWaitForDebugEvent_Index_WinXP 269
#define NtWaitForDebugEvent_Index_Win7 387

#define NtSuspendThread_Index_WinXP 254
#define NtSuspendThread_Index_Win7 367

typedef NTSTATUS (*NTSUSPENDTHREAD)(
					  __in HANDLE ThreadHandle,
					  __out_opt PULONG PreviousSuspendCount
					  );

NTSTATUS
NtSuspendThread_Proxy(
				__in HANDLE ThreadHandle,
				__out_opt PULONG PreviousSuspendCount
				);

#define NtSetInformationDebugObject_Index_WinXP 223
#define NtSetInformationDebugObject_Index_Win7 327

#define NtSetContextThread_Index_WinXP 213
#define NtSetContextThread_Index_Win7 316

typedef NTSTATUS (*NTSETCONTEXTTHREAD)(
									   __in HANDLE ThreadHandle,
									   __in PCONTEXT ThreadContext
									   );	

NTSTATUS 
NtSetContextThread_Proxy(
						  __in HANDLE ThreadHandle,
						  __in PCONTEXT ThreadContext
						  );

#define NtGetContextThread_Index_WinXP 85
#define NtGetContextThread_Index_Win7 135

typedef NTSTATUS (*NTGETCONTEXTTHREAD)(
										IN HANDLE ThreadHandle,
										OUT PCONTEXT Context
									   );	

NTSTATUS NtGetContextThread_Proxy(
								  IN HANDLE ThreadHandle,
								  OUT PCONTEXT Context
								  );

#define NtTerminateProcess_Index_WinXP 257
#define NtTerminateProcess_Index_Win7 370

typedef NTSTATUS (*NTTERMINATEPROCESS)(
									   IN HANDLE  ProcessHandle,
									   IN NTSTATUS  ExitStatus
									   );

NTSTATUS NtTerminateProcess_Proxy(
				   IN HANDLE  ProcessHandle,
				   IN NTSTATUS  ExitStatus
				   );

#define NtCreateThread_Index_WinXP 53
#define NtCreateThread_Index_Win7 87

typedef struct _INITIAL_TEB {
	struct {
		PVOID OldStackBase;
		PVOID OldStackLimit;
	} OldInitialTeb;
	PVOID StackBase;
	PVOID StackLimit;
	PVOID StackAllocationBase;
} INITIAL_TEB, *PINITIAL_TEB;

typedef NTSTATUS (*NTCREATETHREAD)(
									  __out PHANDLE ThreadHandle,
									  __in ACCESS_MASK DesiredAccess,
									  __in_opt POBJECT_ATTRIBUTES ObjectAttributes,
									  __in HANDLE ProcessHandle,
									  __out PCLIENT_ID ClientId,
									  __in PCONTEXT ThreadContext,
									  __in PINITIAL_TEB InitialTeb,
									  __in BOOLEAN CreateSuspended
									  );

NTSTATUS NtCreateThread_Proxy(
			   __out PHANDLE ThreadHandle,
			   __in ACCESS_MASK DesiredAccess,
			   __in_opt POBJECT_ATTRIBUTES ObjectAttributes,
			   __in HANDLE ProcessHandle,
			   __out PCLIENT_ID ClientId,
			   __in PCONTEXT ThreadContext,
			   __in PINITIAL_TEB InitialTeb,
			   __in BOOLEAN CreateSuspended
			   );

#define NtContinue_Index_WinXP 32
#define NtContinue_Index_Win7  60

typedef NTSTATUS (*NTCONTINUE)(
							  IN PCONTEXT ContextRecord,
							  IN BOOLEAN TestAlert);

void NtContinue_Proxy();


#define NtQueryVirtualMemory_Index_WinXP 178
#define NtQueryVirtualMemory_Index_Win7 267

typedef NTSTATUS (*NTQUERYVIRTUALMEMORY)(
	__in HANDLE ProcessHandle,
	__in PVOID BaseAddress,
	__in MEMORY_INFORMATION_CLASS MemoryInformationClass,
	__out_bcount(MemoryInformationLength) PVOID MemoryInformation,
	__in SIZE_T MemoryInformationLength,
	__out_opt PSIZE_T ReturnLength
	);

NTSTATUS NtQueryVirtualMemory_Proxy(
					 __in HANDLE ProcessHandle,
					 __in PVOID BaseAddress,
					 __in MEMORY_INFORMATION_CLASS MemoryInformationClass,
					 __out_bcount(MemoryInformationLength) PVOID MemoryInformation,
					 __in SIZE_T MemoryInformationLength,
					 __out_opt PSIZE_T ReturnLength
					 );

#define NtCreateMutant_Index_WinXP 43

typedef NTSTATUS (*NTCREATEMUTANT)(
							   __out PHANDLE MutantHandle,
							   __in ACCESS_MASK DesiredAccess,
							   __in_opt POBJECT_ATTRIBUTES ObjectAttributes,
							   __in BOOLEAN InitialOwner
							   );

NTSTATUS NtCreateMutant_Proxy (
				__out PHANDLE MutantHandle,
				__in ACCESS_MASK DesiredAccess,
				__in_opt POBJECT_ATTRIBUTES ObjectAttributes,
				__in BOOLEAN InitialOwner
				);

#define NtCreateSemaphore_Index_WinXP 51

typedef NTSTATUS (*NTCREATESEMAPHORE)(
				   __out PHANDLE SemaphoreHandle,
				   __in ACCESS_MASK DesiredAccess,
				   __in_opt POBJECT_ATTRIBUTES ObjectAttributes,
				   __in LONG InitialCount,
				   __in LONG MaximumCount
				   );

NTSTATUS NtCreateSemaphore_Proxy (
								  __out PHANDLE SemaphoreHandle,
								  __in ACCESS_MASK DesiredAccess,
								  __in_opt POBJECT_ATTRIBUTES ObjectAttributes,
								  __in LONG InitialCount,
								  __in LONG MaximumCount
								  );

#define NtRaiseHardError_Index_WinXP 182

typedef NTSTATUS (*NTRAISEHARDERROR)(
						__in NTSTATUS ErrorStatus,
						__in ULONG NumberOfParameters,
						__in ULONG UnicodeStringParameterMask,
						__in_ecount(NumberOfParameters) PULONG_PTR Parameters,
						__in ULONG ValidResponseOptions,
						__out PULONG Response
				  );

NTSTATUS
NtRaiseHardError_Proxy (
				  __in NTSTATUS ErrorStatus,
				  __in ULONG NumberOfParameters,
				  __in ULONG UnicodeStringParameterMask,
				  __in_ecount(NumberOfParameters) PULONG_PTR Parameters,
				  __in ULONG ValidResponseOptions,
				  __out PULONG Response
				  );

typedef PVOID (*MMGETSYSTEMROUTINEADDRESS)(__in PUNICODE_STRING SystemRoutineName);

PVOID MmGetSystemRoutineAddress_InlineProxy(__in PUNICODE_STRING SystemRoutineName);

PVOID MmGetSystemRoutineAddress_Proxy(__in PUNICODE_STRING SystemRoutineName);

typedef NTSTATUS (*PSCREATESYSTEMTHREAD)(
									  OUT PHANDLE  ThreadHandle,
									  IN ULONG  DesiredAccess,
									  IN POBJECT_ATTRIBUTES  ObjectAttributes  OPTIONAL,
									  IN HANDLE  ProcessHandle  OPTIONAL,
									  OUT PCLIENT_ID  ClientId  OPTIONAL,
									  IN PKSTART_ROUTINE  StartRoutine,
									  IN PVOID  StartContext
									  );

NTSTATUS PsCreateSystemThread_InlineProxy(
					 OUT PHANDLE  ThreadHandle,
					 IN ULONG  DesiredAccess,
					 IN POBJECT_ATTRIBUTES  ObjectAttributes  OPTIONAL,
					 IN HANDLE  ProcessHandle  OPTIONAL,
					 OUT PCLIENT_ID  ClientId  OPTIONAL,
					 IN PKSTART_ROUTINE  StartRoutine,
					 IN PVOID  StartContext
					 );

PEPROCESS IoGetCurrentProcess_Proxy();

HANDLE PsGetCurrentProcessId_Proxy();

NTSTATUS ReplaceFakeSSDTFunction();

NTSTATUS ZwQuerySystemInformation_Proxy(
	__in          SYSTEM_INFORMATION_CLASS SystemInformationClass,
	__inout       PVOID SystemInformation,
	__in          ULONG SystemInformationLength,
	__out_opt     PULONG ReturnLength
	);

VOID ReplaceSSDTFunction();
VOID RecoverSSDTFunction();
