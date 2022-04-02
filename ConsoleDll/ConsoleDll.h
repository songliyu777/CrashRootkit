#pragma once
// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� CONSOLEDLL_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// CONSOLEDLL_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef CONSOLEDLL_EXPORTS
#define CONSOLEDLL_API EXTERN_C __declspec(dllexport)
#else
#define CONSOLEDLL_API EXTERN_C __declspec(dllimport)
#endif

#include "Transfer.h"
#include "PETools_R3.h"


VOID InitDrvPath(OUT PWCHAR szFilePath,IN PWCHAR FileName);

CONSOLEDLL_API VOID InitFilePath(OUT PWCHAR szFilePath,IN PWCHAR FileName);

CONSOLEDLL_API VOID GetImageNameByPath(OUT PWCHAR szImageName,IN PWCHAR szPath);

CONSOLEDLL_API BOOL InitializeCRD();

CONSOLEDLL_API BOOL OpenCRD();

CONSOLEDLL_API BOOL ReleaseCRD();

CONSOLEDLL_API VOID PrintLog(const wchar_t * info, ...);

CONSOLEDLL_API VOID PrintDbgString(const wchar_t * info, ...);

CONSOLEDLL_API BOOL SetLogPath();

CONSOLEDLL_API BOOL TransferMessage(TransferMsg & msg);

CONSOLEDLL_API BOOL KernelHookCheck(OUT DWORD & size);

CONSOLEDLL_API BOOL KernelHookGet(IN OUT PVOID buff,IN DWORD size);

CONSOLEDLL_API BOOL ProcessesList(OUT DWORD & size);

CONSOLEDLL_API BOOL ProcessesGet(PVOID buff, DWORD size);

CONSOLEDLL_API BOOL ModulesList(IN ULONG_PTR pid, OUT DWORD & size);

CONSOLEDLL_API BOOL ModulesGet(PVOID buff, DWORD size);

CONSOLEDLL_API BOOL ThreadsList(IN ULONG_PTR pid, OUT DWORD & size);

CONSOLEDLL_API BOOL ThreadsGet(PVOID buff, DWORD size);

CONSOLEDLL_API BOOL ModulePeInfoGet(ModuleInfo Moduleinfo,OUT PVOID outputbuff,IN OUT DWORD & size);

CONSOLEDLL_API BOOL CR4Get(ULONG & cr4);

CONSOLEDLL_API HANDLE OpenProcessEx(__in DWORD dwProcessId);

CONSOLEDLL_API BOOL ReadProcessMemoryEx(__in      HANDLE hProcess,
										__in      LPCVOID lpBaseAddress,
										__out_bcount_part(nSize, *lpNumberOfBytesRead) LPVOID lpBuffer,
										__in      SIZE_T nSize,
										__out_opt SIZE_T * lpNumberOfBytesRead);

CONSOLEDLL_API BOOL WriteProcessMemoryEx(__in      HANDLE hProcess,
										 __in      LPVOID lpBaseAddress,
										 __in_bcount(nSize) LPCVOID lpBuffer,
										 __in      SIZE_T nSize,
										 __out_opt SIZE_T * lpNumberOfBytesWritten);

CONSOLEDLL_API BOOL KernelModulesList(OUT DWORD & size);

CONSOLEDLL_API BOOL KernelModulesGet(PVOID buff, DWORD size);

CONSOLEDLL_API BOOL SSDTList(OUT DWORD & size);

CONSOLEDLL_API BOOL SSDTGet(PVOID buff, DWORD size);

CONSOLEDLL_API BOOL SSDTShadowList(OUT DWORD & size);

CONSOLEDLL_API BOOL SSDTShadowGet(PVOID buff, DWORD size);

CONSOLEDLL_API BOOL DpcTimerList(OUT DWORD & size);

CONSOLEDLL_API BOOL DpcTimerGet(PVOID buff, DWORD size);

CONSOLEDLL_API BOOL IoTimerList(OUT DWORD & size);

CONSOLEDLL_API BOOL IoTimerGet(PVOID buff, DWORD size);

CONSOLEDLL_API BOOL SetSynchroniza(PVOID buff, DWORD size);

CONSOLEDLL_API BOOL GetSynchronizaInfo(PVOID buff, DWORD size);

typedef enum _SYSTEM_INFORMATION_CLASS     //    Q S
{
	SystemBasicInformation,                // 00 Y N
	SystemProcessorInformation,            // 01 Y N
	SystemPerformanceInformation,          // 02 Y N
	SystemTimeOfDayInformation,            // 03 Y N
	SystemNotImplemented1,                 // 04 Y N
	SystemProcessesAndThreadsInformation,  // 05 Y N
	SystemCallCounts,                      // 06 Y N
	SystemConfigurationInformation,        // 07 Y N
	SystemProcessorTimes,                  // 08 Y N
	SystemGlobalFlag,                      // 09 Y Y
	SystemNotImplemented2,                 // 10 Y N
	SystemModuleInformation,               // 11 Y N
	SystemLockInformation,                 // 12 Y N
	SystemNotImplemented3,                 // 13 Y N
	SystemNotImplemented4,                 // 14 Y N
	SystemNotImplemented5,                 // 15 Y N
	SystemHandleInformation_,               // 16 Y N
	SystemObjectInformation,               // 17 Y N
	SystemPagefileInformation,             // 18 Y N
	SystemInstructionEmulationCounts,      // 19 Y N
	SystemInvalidInfoClass1,               // 20
	SystemCacheInformation,                // 21 Y Y
	SystemPoolTagInformation,              // 22 Y N
	SystemProcessorStatistics,             // 23 Y N
	SystemDpcInformation,                  // 24 Y Y
	SystemNotImplemented6,                 // 25 Y N
	SystemLoadImage,                       // 26 N Y
	SystemUnloadImage,                     // 27 N Y
	SystemTimeAdjustment,                  // 28 Y Y
	SystemNotImplemented7,                 // 29 Y N
	SystemNotImplemented8,                 // 30 Y N
	SystemNotImplemented9,                 // 31 Y N
	SystemCrashDumpInformation,            // 32 Y N
	SystemExceptionInformation,            // 33 Y N
	SystemCrashDumpStateInformation,       // 34 Y Y/N
	SystemKernelDebuggerInformation,       // 35 Y N
	SystemContextSwitchInformation,        // 36 Y N
	SystemRegistryQuotaInformation,        // 37 Y Y
	SystemLoadAndCallImage,                // 38 N Y
	SystemPrioritySeparation,              // 39 N Y
	SystemNotImplemented10,                // 40 Y N
	SystemNotImplemented11,                // 41 Y N
	SystemInvalidInfoClass2,               // 42
	SystemInvalidInfoClass3,               // 43
	SystemTimeZoneInformation,             // 44 Y N
	SystemLookasideInformation,            // 45 Y N
	SystemSetTimeSlipEvent,                // 46 N Y
	SystemCreateSession,                   // 47 N Y
	SystemDeleteSession,                   // 48 N Y
	SystemInvalidInfoClass4,               // 49
	SystemRangeStartInformation,           // 50 Y N
	SystemVerifierInformation,             // 51 Y Y
	SystemAddVerifier,                     // 52 N Y
	SystemSessionProcessesInformation      // 53 Y N
} SYSTEM_INFORMATION_CLASS;

#define STATUS_INFO_LENGTH_MISMATCH      ((LONG)0xC0000004L)
#define STATUS_SUCCESS                   ((LONG)0x00000000L)

typedef LONG (WINAPI *ZWQUERYSYSTEMINFORMATION)(
	__in          SYSTEM_INFORMATION_CLASS SystemInformationClass,
	__inout       PVOID SystemInformation,
	__in          ULONG SystemInformationLength,
	__out_opt     PULONG ReturnLength
	);

typedef struct _CLIENT_ID
{
	HANDLE UniqueProcess;
	HANDLE UniqueThread;

} CLIENT_ID,*PCLIENT_ID;

typedef enum _THREAD_STATE
{
	StateInitialized,
	StateReady,
	StateRunning,
	StateStandby,
	StateTerminated,
	StateWait,
	StateTransition,
	StateUnknown

} THREAD_STATE;

typedef enum _KWAIT_REASON
{
	Executive,
	FreePage,
	PageIn,
	PoolAllocation,
	DelayExecution,
	Suspended,
	UserRequest,
	WrExecutive,
	WrFreePage,
	WrPageIn,
	WrPoolAllocation,
	WrDelayExecution,
	WrSuspended,
	WrUserRequest,
	WrEventPair,
	WrQueue,
	WrLpcReceive,
	WrLpcReply,
	WrVirtualMemory,
	WrPageOut,
	WrRendezvous,
	Spare2,
	Spare3,
	Spare4,
	Spare5,
	Spare6,
	WrKernel

} KWAIT_REASON;

typedef LONG KPRIORITY;

typedef struct _VM_COUNTERS
{
	ULONG PeakVirtualSize;				//����洢��ֵ��С
	ULONG VirtualSize;					//����洢��С
	ULONG PageFaultCount;				//ҳ������Ŀ
	ULONG PeakWorkingSetSize;			//��������ֵ��С
	ULONG WorkingSetSize;				//��������С
	ULONG QuotaPeakPagedPoolUsage;		//��ҳ��ʹ������ֵ
	ULONG QuotaPagedPoolUsage;			//��ҳ��ʹ�����
	ULONG QuotaPeakNonPagedPoolUsage;	//�Ƿ�ҳ��ʹ������ֵ
	ULONG QuotaNonPagedPoolUsage;		//�Ƿ�ҳ��ʹ�����
	ULONG PagefileUsage;				//ҳ�ļ�ʹ�����
	ULONG PeakPagefileUsage;			//ҳ�ļ�ʹ�÷�ֵ

} VM_COUNTERS, *PVM_COUNTERS;

typedef struct _SYSTEM_THREADS
{
	LARGE_INTEGER KernelTime;
	LARGE_INTEGER UserTime;
	LARGE_INTEGER CreateTime;
	ULONG WaitTime;
	PVOID StartAddress;
	CLIENT_ID ClientId;
	KPRIORITY Priority;
	KPRIORITY BasePriority;
	ULONG ContextSwitchCount;
	THREAD_STATE State;
	KWAIT_REASON WaitReason;

} SYSTEM_THREADS, *PSYSTEM_THREADS;

typedef struct _SYSTEM_PROCESSES
{
	ULONG NextEntryDelta;
	ULONG ThreadCount;
	ULONG Reserved1[6];
	LARGE_INTEGER CreateTime;
	LARGE_INTEGER UserTime;
	LARGE_INTEGER KernelTime;
	UNICODE_STRING ProcessName;
	KPRIORITY BasePriority;
	ULONG ProcessId;
	ULONG InheritedFromProcessId;
	ULONG HandleCount;
	ULONG Reserved2[2];
	VM_COUNTERS  VmCounters;
	IO_COUNTERS IoCounters;
	SYSTEM_THREADS Threads[1];

} SYSTEM_PROCESSES, *PSYSTEM_PROCESSES;

CONSOLEDLL_API PVOID GetInfoTable(SYSTEM_INFORMATION_CLASS SystemInformationClass);

CONSOLEDLL_API VOID FreeInfoTable(PVOID Ptr);

CONSOLEDLL_API UINT SendInputEx(UINT nInputs, LPINPUT pInputs, int cbSize);

CONSOLEDLL_API HWND FindWindowExtend(PWCHAR ClassName,PWCHAR WindowName);

CONSOLEDLL_API void KeyDownEx(long vKeyCoad);

CONSOLEDLL_API void KeyUpEx(long vKeyCoad);

CONSOLEDLL_API void KeyDown(long vKeyCoad);

CONSOLEDLL_API void KeyUp(long vKeyCoad);