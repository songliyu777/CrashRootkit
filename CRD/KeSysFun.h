#pragma once

#include "MyWinNT.h"
#pragma warning(disable:4996)
#pragma warning(disable:4995)

extern "C" NTSYSAPI NTSTATUS NTAPI ZwQuerySystemInformation(
	__in          SYSTEM_INFORMATION_CLASS SystemInformationClass,
	__inout       PVOID SystemInformation,
	__in          ULONG SystemInformationLength,
	__out_opt     PULONG ReturnLength
	);


POINTER_TYPE GetInfoTable(SYSTEM_INFORMATION_CLASS SystemInformationClass);

NTSTATUS ReadLocaleFile(PUNICODE_STRING pFilePath,OUT PVOID * buff,OUT SIZE_T * size);

extern "C" NTKERNELAPI NTSTATUS PsLookupProcessByProcessId(
	__in HANDLE ProcessId,
	__deref_out PEPROCESS *Process
	);

extern "C" NTKERNELAPI NTSTATUS PsLookupThreadByThreadId(
						 __in HANDLE ThreadId,
						 __deref_out PETHREAD *Thread
						 );

extern "C" NTKERNELAPI VOID KeAttachProcess(__inout PEPROCESS Process);

extern "C" NTKERNELAPI VOID KeDetachProcess (VOID);

extern "C" NTKERNELAPI NTSTATUS ObReferenceObjectByHandle (
						   __in HANDLE Handle,
						   __in ACCESS_MASK DesiredAccess,
						   __in_opt POBJECT_TYPE ObjectType,
						   __in KPROCESSOR_MODE AccessMode,
						   __out PVOID *Object,
						   __out_opt POBJECT_HANDLE_INFORMATION HandleInformation
						   );

extern "C" NTKERNELAPI NTSTATUS SeCreateAccessState(
					__out PACCESS_STATE AccessState,
					__out PVOID AuxData,
					__in ACCESS_MASK DesiredAccess,
					__in_opt PGENERIC_MAPPING GenericMapping
					);

extern "C" NTKERNELAPI VOID NTAPI SeDeleteAccessState(PACCESS_STATE AccessState); 



typedef struct _KAPC_STATE {
	LIST_ENTRY ApcListHead[MaximumMode];
	struct _KPROCESS *Process;
	BOOLEAN KernelApcInProgress;
	BOOLEAN KernelApcPending;
	BOOLEAN UserApcPending;
} KAPC_STATE, *PKAPC_STATE, *RESTRICTED_POINTER PRKAPC_STATE;

extern "C" NTKERNELAPI VOID KeStackAttachProcess (IN PKPROCESS  Process, OUT PRKAPC_STATE  ApcState);

extern "C" NTKERNELAPI VOID KeUnstackDetachProcess(IN PRKAPC_STATE  ApcState);

extern "C" NTKERNELAPI NTSTATUS KeAddSystemServiceTable(DWORD p1, DWORD p2, DWORD p3,DWORD p4,DWORD p5);

extern "C" NTKERNELAPI NTSTATUS KeUpdateSystemTime();

extern "C" NTKERNELAPI LARGE_INTEGER KeQueryPerformanceCounter(OUT PLARGE_INTEGER PerformanceFrequency);

extern "C" NTKERNELAPI VOID KeSetSystemAffinityThread (KAFFINITY Affinity);

extern "C" NTKERNELAPI VOID KeRevertToUserAffinityThread (VOID);

extern "C" NTKERNELAPI NTSTATUS ObOpenObjectByPointer(
	IN PVOID  Object,
	IN ULONG  HandleAttributes,
	IN PACCESS_STATE  PassedAccessState OPTIONAL,
	IN ACCESS_MASK  DesiredAccess,
	IN POBJECT_TYPE  ObjectType,
	IN KPROCESSOR_MODE  AccessMode,
	OUT PHANDLE  Handle
	);

extern "C" NTKERNELAPI BOOLEAN KeCancelTimer(IN PKTIMER Timer);

extern "C" NTKERNELAPI VOID IoStartTimer(IN PDEVICE_OBJECT  DeviceObject);

extern "C" NTKERNELAPI VOID IoStopTimer(IN PDEVICE_OBJECT  DeviceObject);

extern "C" NTKERNELAPI VOID KeInitializeApc(PKAPC Apc,
											PETHREAD Thread,
											ULONG Environment,
											PKKERNEL_ROUTINE KernelRoutine,
											PKRUNDOWN_ROUTINE RundownRoutine,
											PKNORMAL_ROUTINE NormalRoutine,
											KPROCESSOR_MODE ProcessorMode,
											PVOID NormalContext
											);

extern "C" NTKERNELAPI BOOLEAN KeInsertQueueApc(PKAPC Apc,
												PVOID SystemArg1,
												PVOID SystemArg2,
												KPRIORITY Increment
												);

extern "C" NTKERNELAPI NTSTATUS  ExRaiseHardError(IN NTSTATUS ErrorStatus, 
												  IN ULONG NumberOfParameters, 
												  IN ULONG UnicodeStringParameterMask, 
												  IN PVOID Parameters, 
												  IN ULONG ResponseOption, 
												  OUT PULONG Response
												  );



typedef POBJECT_TYPE (*OBGETOBJECTTYPE)(IN PVOID Object);

POBJECT_TYPE  CRD_ObGetObjectType(IN PVOID Object);

typedef struct _OBJECT_HEADER {
	union {
		struct {
			LONG PointerCount;
			LONG HandleCount;
		};
		LIST_ENTRY Entry;
	};
	POBJECT_TYPE Type;
	UCHAR NameInfoOffset;
	UCHAR HandleInfoOffset;
	UCHAR QuotaInfoOffset;
	UCHAR Flags;

	union {
		//POBJECT_CREATE_INFORMATION ObjectCreateInfo;
		PVOID QuotaBlockCharged;
	};

	PSECURITY_DESCRIPTOR SecurityDescriptor;

	QUAD Body;
} OBJECT_HEADER, *POBJECT_HEADER;

//winxp
//nt!_OBJECT_HEADER
//+0x000 PointerCount     : Int4B
//+0x004 HandleCount      : Int4B
//+0x004 NextToFree       : Ptr32 Void
//+0x008 Type             : Ptr32 _OBJECT_TYPE
//+0x00c NameInfoOffset   : UChar
//+0x00d HandleInfoOffset : UChar
//+0x00e QuotaInfoOffset  : UChar
//+0x00f Flags            : UChar
//+0x010 ObjectCreateInfo : Ptr32 _OBJECT_CREATE_INFORMATION
//+0x010 QuotaBlockCharged : Ptr32 Void
//+0x014 SecurityDescriptor : Ptr32 Void
//+0x018 Body             : _QUAD
//kd> dt _OBJECT_TYPE
//nt!_OBJECT_TYPE
//+0x000 Mutex            : _ERESOURCE
//+0x038 TypeList         : _LIST_ENTRY
//+0x040 Name             : _UNICODE_STRING
//+0x048 DefaultObject    : Ptr32 Void
//+0x04c Index            : Uint4B
//+0x050 TotalNumberOfObjects : Uint4B
//+0x054 TotalNumberOfHandles : Uint4B
//+0x058 HighWaterNumberOfObjects : Uint4B
//+0x05c HighWaterNumberOfHandles : Uint4B
//+0x060 TypeInfo         : _OBJECT_TYPE_INITIALIZER
//+0x0ac Key              : Uint4B
//+0x0b0 ObjectLocks      : [4] _ERESOURCE


//win7
//nt!_OBJECT_HEADER
//+0x000 PointerCount     : Int4B
//+0x004 HandleCount      : Int4B
//+0x004 NextToFree       : Ptr32 Void
//+0x008 Lock             : _EX_PUSH_LOCK
//+0x00c TypeIndex        : UChar
//+0x00d TraceFlags       : UChar
//+0x00e InfoMask         : UChar
//+0x00f Flags            : UChar
//+0x010 ObjectCreateInfo : Ptr32 _OBJECT_CREATE_INFORMATION
//+0x010 QuotaBlockCharged : Ptr32 Void
//+0x014 SecurityDescriptor : Ptr32 Void
//+0x018 Body             : _QUAD


#define OBJECT_TO_OBJECT_HEADER(obj)  CONTAINING_RECORD((obj), OBJECT_HEADER, Body);

//关内存保护
VOID PAGEPROTECTOFF();
//开内存保护
VOID PAGEPROTECTON();
//内核休眠一段时间
#define DELAY_ONE_MICROSECOND 	(-10)
#define DELAY_ONE_MILLISECOND	(DELAY_ONE_MICROSECOND*1000)
VOID KeSleep(LONG msec);
//安全重启电脑
VOID RestartSystem();
//获取TickCount
ULONG SDGetTickCount();
//查找内存
ULONG_PTR FindMemoryAddress(ULONG_PTR start, ULONG_PTR end, PBYTE keywords, SIZE_T length ,BOOL IsValid);

#define MB_OK                       0x00000000L // answer was 6
#define MB_OKCANCEL                 0x00000001L // answer ok-6  Отмена -3
#define MB_ABORTRETRYIGNORE         0x00000002L // answer Прервать-2 Повторить-7 Пропустить -4
#define MB_YESNOCANCEL              0x00000003L // answer да-8 Нет-5 Отмена -3
#define MB_YESNO                    0x00000004L // answer да-8 Нет-5 
#define MB_RETRYCANCEL              0x00000005L // answer Повторить-7 Отмена-3
#define MB_CANCELTRYCONTINUE        0x00000006L // answer Отмена-3 Попторить поп -9 Продолжить-10
#define MB_ICONHAND                 0x00000010L 
#define MB_ICONQUESTION             0x00000020L
#define MB_ICONEXCLAMATION          0x00000030L
#define MB_ICONASTERISK             0x00000040L
#define MB_USERICON                 0x00000080L
#define MB_ICONWARNING              MB_ICONEXCLAMATION
#define MB_ICONERROR                MB_ICONHAND
#define MB_ICONINFORMATION          MB_ICONASTERISK
#define MB_ICONSTOP                 MB_ICONHAND
#define MB_DEFBUTTON1               0x00000000L
#define MB_DEFBUTTON2               0x00000100L
#define MB_DEFBUTTON3               0x00000200L
#define MB_DEFBUTTON4               0x00000300L
#define MB_APPLMODAL                0x00000000L
#define MB_SYSTEMMODAL              0x00001000L
#define MB_TASKMODAL                0x00002000L
#define MB_HELP                     0x00004000L 
#define MB_NOFOCUS                  0x00008000L
#define MB_SETFOREGROUND            0x00010000L
#define MB_DEFAULT_DESKTOP_ONLY     0x00020000L
#define MB_TOPMOST                  0x00040000L
#define MB_RIGHT                    0x00080000L
#define MB_RTLREADING               0x00100000L

ULONG KernelMessageBox(IN PWSTR pwText, IN PWSTR pwCaption, IN int uType);

BOOL GetDeviceName(IN HANDLE hHandleFile,OUT PUNICODE_STRING *lpOutUniString);

ULONG_PTR GetKernelModuleAddressByName(PCHAR ModuleName);
