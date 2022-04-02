#pragma once
#include "MyWinNT.h"
#include "MyListEntry.h"
#define _CRD_
#include "Transfer.h"

ULONG_PTR GetPspCidTable();

#pragma pack(push, 1)
typedef struct _EX_PUSH_LOCK
{
	unsigned Waiting : 1;      //1 Bit
	unsigned Exclusive  : 1;  //1 Bit
	unsigned Shared  : 30;   //30 Bits
	ULONG Value;			//: Uint4B
	PVOID Ptr;              //: Ptr32 Void

}EX_PUSH_LOCK,*PEX_PUSH_LOCK;


typedef struct _HANDLE_TRACE_DB_ENTRY
{
	_CLIENT_ID ClientId;
	PVOID Handle;
	ULONG Type;
	PVOID StackTrace[16];
}HANDLE_TRACE_DB_ENTRY,*PHANDLE_TRACE_DB_ENTRY;

typedef struct _HANDLE_TRACE_DEBUG_INFO
{
	ULONG CurrentStackIndex;
	_HANDLE_TRACE_DB_ENTRY TraceDb[4096];
}HANDLE_TRACE_DEBUG_INFO,*PHANDLE_TRACE_DEBUG_INFO;

typedef struct _HANDLE_TABLE
{
	ULONG TableCode;
	PEPROCESS QuotaProcess;
	PVOID UniqueProcessId;
	PEX_PUSH_LOCK HandleTableLock[4];
	_LIST_ENTRY HandleTableList;
	PEX_PUSH_LOCK HandleContentionEvent;
	PHANDLE_TRACE_DEBUG_INFO DebugInfo;
	ULONG ExtraInfoPages;
	ULONG FirstFree;
	ULONG LastFree;
	ULONG NextHandleNeedingPool;
	int HandleCount;
	ULONG Flags;
	unsigned StrictFIFO : 1;
}HANDLE_TABLE,*PHANDLE_TABLE;

//win7
//nt!_HANDLE_TABLE
//+0x000 TableCode        : 0x960d0001
//+0x004 QuotaProcess     : (null) 
//+0x008 UniqueProcessId  : (null) 
//+0x00c HandleLock       : _EX_PUSH_LOCK
//+0x010 HandleTableList  : _LIST_ENTRY [ 0x8b401110 - 0x8b401110 ]
//+0x018 HandleContentionEvent : _EX_PUSH_LOCK
//+0x01c DebugInfo        : (null) 
//+0x020 ExtraInfoPages   : 0n0
//+0x024 Flags            : 1
//+0x024 StrictFIFO       : 0y1
//+0x028 FirstFreeHandle  : 0xb68
//+0x02c LastFreeHandleEntry : 0x960ce6c0 _HANDLE_TABLE_ENTRY
//+0x030 HandleCount      : 0x275
//+0x034 NextHandleNeedingPool : 0x1000
//+0x038 HandleCountHighWatermark : 0x276


#pragma pack(pop)

SIZE_T ListProcess();
VOID GetProcesses(PVOID buff,SIZE_T size);
BOOL AddProcessInfo(PEPROCESS peprocess);
VOID FreeProcessInfo();
VOID GetProcessPath(ULONG eprocess,OUT PWCHAR ProcessPath);
BOOL IsExitProcess(IN PEPROCESS peprocess);

typedef struct _PROCESSINFO
{
	SLISTENTRY next;
	PEPROCESS perocess;
	ULONG_PTR UniqueProcessId;
	ULONG_PTR InheritedFromUniqueProcessId;
	UCHAR ImageFileName[16];
	WCHAR ImagePath[MAX_PATH];
}PROCESSINFO,*PPROCESSINFO;

typedef struct _SEGMENT
{
	PVOID ControlArea;
}SEGMENT,*PSEGMENT;

typedef struct _EVENT_COUNTER
{

}EVENT_COUNTER,PEVENT_COUNTER;

typedef struct _CONTROL_AREA
{
	PSEGMENT Segment;
	_LIST_ENTRY DereferenceList;
	ULONG NumberOfSectionReferences;
	ULONG NumberOfPfnReferences;
	ULONG NumberOfMappedViews;
	USHORT NumberOfSubsections;
	USHORT FlushInProgressCount;
	ULONG NumberOfUserReferences;
	ULONG  __unnamed;
	PFILE_OBJECT FilePointer;
	PEVENT_COUNTER WaitingForDeletion;
	USHORT ModifiedWriteCount;
	USHORT NumberOfSystemCacheViews;
}CONTROL_AREA,*PCONTROL_AREA;

typedef struct _SUBSECTION
{

}SUBSECTION,*PSUBSECTION;

typedef struct _LARGE_CONTROL_AREA
{

}LARGE_CONTROL_AREA,*PLARGE_CONTROL_AREA;

typedef struct _MMSECTION_FLAGS
{

}MMSECTION_FLAGS,*PMMSECTION_FLAGS;

typedef struct _MMSUBSECTION_FLAGS
{

}MMSUBSECTION_FLAGS,*PMMSUBSECTION_FLAGS;

typedef struct _SEGMENT_OBJECT
{
	PVOID BaseAddress;
	ULONG TotalNumberOfPtes;
	_LARGE_INTEGER SizeOfSegment;
	ULONG NonExtendedPtes;
	ULONG ImageCommitment;
	PCONTROL_AREA ControlArea;
	PSUBSECTION Subsection;
	PLARGE_CONTROL_AREA LargeControlArea;
	PLARGE_CONTROL_AREA MmSectionFlags;
	PMMSUBSECTION_FLAGS MmSubSectionFlags;
}SEGMENT_OBJECT,*PSEGMENT_OBJECT;

typedef struct _SECTION_OBJECT
{
	PVOID StartingVa;
	PVOID EndingVa;
	PVOID Parent;
	PVOID LeftChild;
	PVOID RightChild;
	PSEGMENT_OBJECT Segment;
}SECTION_OBJECT,*PSECTION_OBJECT;

typedef struct _EX_FAST_REF
{
	union{
		PVOID Object;
		unsigned RefCnt : 4; 
		ULONG Value;
	};

}EX_FAST_REF,*PEX_FAST_REF;

ULONG GetObjectType_XP(PVOID objectheader);
ULONG GetObjectType_WIN7(PVOID objectheader);

PEPROCESS GetCsrssEprocess();
NTSTATUS ZwKillProcess(PEPROCESS Process);
NTSTATUS CRD_TerminateProcess(PTransferMsg msg);
NTSTATUS CRD_ResumeThread(PTransferMsg msg);
NTSTATUS CRD_SuspendThread(PTransferMsg msg);
NTSTATUS CRD_ShutDown(PTransferMsg msg);

