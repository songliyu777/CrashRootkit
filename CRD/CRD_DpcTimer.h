#pragma once
#include "MyWinNT.h"
#include "MyListEntry.h"
#define _CRD_
#include "Transfer.h"

//********************************************************
typedef struct _KTIMER_TABLE_ENTRY_WIN7{
	ULONG Lock;
	LIST_ENTRY Entry;
	ULARGE_INTEGER Time;
}KTIMER_TABLE_ENTRY_WIN7,*PKTIMER_TABLE_ENTRY_WIN7;

//kpcr中KdVersionBlock的数据结构
typedef struct _DBGKD_GET_VERSION64 {
	USHORT  MajorVersion;
	USHORT  MinorVersion;
	USHORT  ProtocolVersion;
	USHORT  Flags;
	USHORT  MachineType;
	UCHAR   MaxPacketType;
	UCHAR   MaxStateChange;
	UCHAR   MaxManipulate;
	UCHAR   Simulation;
	USHORT  Unused[1];
	ULONG64 KernBase;
	ULONG64 PsLoadedModuleList;
	ULONG64 DebuggerDataList;//高位是0xffffffff
} DBGKD_GET_VERSION64, *PDBGKD_GET_VERSION64;

typedef struct _DBGKD_DEBUG_DATA_HEADER64 {
	LIST_ENTRY64 List;
	ULONG           OwnerTag;//"KDBG"
	ULONG           Size;
} DBGKD_DEBUG_DATA_HEADER64, *PDBGKD_DEBUG_DATA_HEADER64;

//KdVersionBlock的DebuggerDataList指向的结构
typedef struct _KDDEBUGGER_DATA64 
{
	DBGKD_DEBUG_DATA_HEADER64 Header;
	ULONG64   KernBase;//指向ntoskrnl.exe
	ULONG64   BreakpointWithStatus;       // address of breakpoint
	ULONG64   SavedContext;
	USHORT  ThCallbackStack;            // offset in thread data
	USHORT  NextCallback;               // saved pointer to next callback frame
	USHORT  FramePointer;               // saved frame pointer
	USHORT  PaeEnabled:1;
	ULONG64   KiCallUserMode;             // kernel routine
	ULONG64   KeUserCallbackDispatcher;   // address in ntdll
	ULONG64   PsLoadedModuleList;
	ULONG64   PsActiveProcessHead;
	ULONG64   PspCidTable;
	ULONG64   ExpSystemResourcesList;
	ULONG64   ExpPagedPoolDescriptor;
	ULONG64   ExpNumberOfPagedPools;
	ULONG64   KeTimeIncrement;
	ULONG64   KeBugCheckCallbackListHead;
	ULONG64   KiBugcheckData;
	ULONG64   IopErrorLogListHead;
	ULONG64   ObpRootDirectoryObject;
	ULONG64   ObpTypeObjectType;
	ULONG64   MmSystemCacheStart;
	ULONG64   MmSystemCacheEnd;
	ULONG64   MmSystemCacheWs;
	ULONG64   MmPfnDatabase;
	ULONG64   MmSystemPtesStart;
	ULONG64   MmSystemPtesEnd;
	ULONG64   MmSubsectionBase;
	ULONG64   MmNumberOfPagingFiles;
	ULONG64   MmLowestPhysicalPage;
	ULONG64   MmHighestPhysicalPage;
	ULONG64   MmNumberOfPhysicalPages;
	ULONG64   MmMaximumNonPagedPoolInBytes;
	ULONG64   MmNonPagedSystemStart;
	ULONG64   MmNonPagedPoolStart;
	ULONG64   MmNonPagedPoolEnd;
	ULONG64   MmPagedPoolStart;
	ULONG64   MmPagedPoolEnd;
	ULONG64   MmPagedPoolInformation;
	ULONG64   MmPageSize;
	ULONG64   MmSizeOfPagedPoolInBytes;
	ULONG64   MmTotalCommitLimit;
	ULONG64   MmTotalCommittedPages;
	ULONG64   MmSharedCommit;
	ULONG64   MmDriverCommit;
	ULONG64   MmProcessCommit;
	ULONG64   MmPagedPoolCommit;
	ULONG64   MmExtendedCommit;
	ULONG64   MmZeroedPageListHead;
	ULONG64   MmFreePageListHead;
	ULONG64   MmStandbyPageListHead;
	ULONG64   MmModifiedPageListHead;
	ULONG64   MmModifiedNoWritePageListHead;
	ULONG64   MmAvailablePages;
	ULONG64   MmResidentAvailablePages;
	ULONG64   PoolTrackTable;
	ULONG64   NonPagedPoolDescriptor;
	ULONG64   MmHighestUserAddress;
	ULONG64   MmSystemRangeStart;
	ULONG64   MmUserProbeAddress;
	ULONG64   KdPrintCircularBuffer;
	ULONG64   KdPrintCircularBufferEnd;
	ULONG64   KdPrintWritePointer;
	ULONG64   KdPrintRolloverCount;
	ULONG64   MmLoadedUserImageList;
	// NT 5.1 Addition
	ULONG64   NtBuildLab;
	ULONG64   KiNormalSystemCall;
	// NT 5.0 hotfix addition
	ULONG64   KiProcessorBlock;
	ULONG64   MmUnloadedDrivers;
	ULONG64   MmLastUnloadedDriver;
	ULONG64   MmTriageActionTaken;
	ULONG64   MmSpecialPoolTag;
	ULONG64   KernelVerifier;
	ULONG64   MmVerifierData;
	ULONG64   MmAllocatedNonPagedPool;
	ULONG64   MmPeakCommitment;
	ULONG64   MmTotalCommitLimitMaximum;
	ULONG64   CmNtCSDVersion;
	// NT 5.1 Addition
	ULONG64   MmPhysicalMemoryBlock;
	ULONG64   MmSessionBase;
	ULONG64   MmSessionSize;
	ULONG64   MmSystemParentTablePage;
	// Server 2003 addition
	ULONG64   MmVirtualTranslationBase;
	USHORT    OffsetKThreadNextProcessor;
	USHORT    OffsetKThreadTeb;
	USHORT    OffsetKThreadKernelStack;
	USHORT    OffsetKThreadInitialStack;
	USHORT    OffsetKThreadApcProcess;
	USHORT    OffsetKThreadState;
	USHORT    OffsetKThreadBStore;
	USHORT    OffsetKThreadBStoreLimit;
	USHORT    SizeEProcess;
	USHORT    OffsetEprocessPeb;
	USHORT    OffsetEprocessParentCID;
	USHORT    OffsetEprocessDirectoryTableBase;
	USHORT    SizePrcb;
	USHORT    OffsetPrcbDpcRoutine;
	USHORT    OffsetPrcbCurrentThread;
	USHORT    OffsetPrcbMhz;
	USHORT    OffsetPrcbCpuType;
	USHORT    OffsetPrcbVendorString;
	USHORT    OffsetPrcbProcStateContext;
	USHORT    OffsetPrcbNumber;
	USHORT    SizeEThread;
	ULONG64   KdPrintCircularBufferPtr;
	ULONG64   KdPrintBufferSize;
	ULONG64   KeLoaderBlock;
	USHORT    SizePcr;
	USHORT    OffsetPcrSelfPcr;
	USHORT    OffsetPcrCurrentPrcb;
	USHORT    OffsetPcrContainedPrcb;
	USHORT    OffsetPcrInitialBStore;
	USHORT    OffsetPcrBStoreLimit;
	USHORT    OffsetPcrInitialStack;
	USHORT    OffsetPcrStackLimit;
	USHORT    OffsetPrcbPcrPage;
	USHORT    OffsetPrcbProcStateSpecialReg;
	USHORT    GdtR0Code;
	USHORT    GdtR0Data;
	USHORT    GdtR0Pcr;
	USHORT    GdtR3Code;
	USHORT    GdtR3Data;
	USHORT    GdtR3Teb;
	USHORT    GdtLdt;
	USHORT    GdtTss;
	USHORT    Gdt64R3CmCode;
	USHORT    Gdt64R3CmTeb;
	ULONG64   IopNumTriageDumpDataBlocks;
	ULONG64   IopTriageDumpDataBlocks;
	ULONG64   VfCrashDataBlock;
	ULONG64   MmBadPagesDetected;
	ULONG64   MmZeroedPageSingleBitErrorsDetected;
} KDDEBUGGER_DATA64, *PKDDEBUGGER_DATA64;

typedef struct _DPCTIMERINFO{
	SLISTENTRY next;
	ULONG_PTR TimerAddress;
	ULONG_PTR DpcAddress;
	ULONG_PTR DpcRoutineAddress;
	ULONG_PTR Period;
}DPCTIMERINFO,*PDPCTIMERINFO;

SIZE_T GetDpcTimerInformation_WIN7600_UP();

SIZE_T GetDpcTimerInformation_XP_2K3();

VOID FreeDpcTimerInfo();

SIZE_T ListDpcTimer();

VOID GetDpcTimer(PVOID buff,SIZE_T size);

BOOL AllocateDpcTimerInfo(SIZE_T size);

NTSTATUS KillDcpTimer(PTransferMsg msg);