#pragma 

#include "MyListEntry.h"
#define _CRD_
#include "Transfer.h"

typedef struct _IO_TIMER
{
	SHORT Type;
	SHORT TimerFlag;
	LIST_ENTRY TimerList;
	PVOID TimerRoutine;
	PVOID Context;
	PDEVICE_OBJECT DeviceObject;

} IO_TIMER, *PIO_TIMER;

typedef struct _IOTIMERINFO
{
	SLISTENTRY next;
	ULONG_PTR DeviceObject;
	ULONG_PTR IoTimerRoutineAddress;
	SHORT ulStatus;
}IOTIMERINFO, *PIOTIMERINFO;
ULONG_PTR GetIopTimerQueueHead();

VOID FreeIoTimerInfo();

BOOL AllocateIoTimerInfo(SIZE_T size);

VOID GetIoTimer(PVOID buff,SIZE_T size);

SIZE_T ListIoTimer();

NTSTATUS StartIoTimer(PTransferMsg msg);

NTSTATUS StopIoTimer(PTransferMsg msg);