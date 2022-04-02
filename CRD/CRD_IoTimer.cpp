#ifdef __cplusplus
extern "C" {
#endif
#include <ntddk.h>
#include <string.h>
#ifdef __cplusplus
}; // extern "C"
#endif

#include "CRD_IOTimer.h"
#include "FuncAddrValid.h"
#include "KeSysFun.h"
#define _CRD_
#include "Transfer.h"

SLISTENTRY g_sleIoTimerInfoHead = {0};

ULONG_PTR GetIopTimerQueueHead()
{
	UNICODE_STRING UnicodeFuncName;
	ULONG ulFunction;
	ULONG ulCodeSize;
	ULONG IopTimerQueueHead = 0;
	PUCHAR i=0;

	RtlInitUnicodeString(&UnicodeFuncName,L"IoInitializeTimer");
	ulFunction = (ULONG)MmGetSystemRoutineAddress(&UnicodeFuncName);
	if (ulFunction)
	{
		ulCodeSize = 0x100;
		if (ulCodeSize)
		{
			for (i=(PUCHAR)ulFunction;i<(PUCHAR)ulFunction+ulCodeSize;i++)
			{
				if (*i == 0xb9)
				{
					IopTimerQueueHead = *(PULONG)(i+1);
					if (MmIsAddressValidEx((PVOID)IopTimerQueueHead))
					{
						break;
					}
				}
			}
		}
	}
	return IopTimerQueueHead;
}

SIZE_T ListIoTimer()
{
	PLIST_ENTRY  pList = NULL;
	PLIST_ENTRY pNextList = NULL;
	PIO_TIMER  pTimer = NULL;
	ULONG ulModuleBase;
	ULONG ulModuleSize;
	ULONG uCount = 0;

	ULONG MAX_IO_TIMER_COUNT = 0x100;

	FreeIoTimerInfo();

	pList = (PLIST_ENTRY)GetIopTimerQueueHead();
	if (!pList)
		return 0;

	if(!AllocateIoTimerInfo(MAX_IO_TIMER_COUNT))
		return 0;
	
	PSLISTENTRY pSListEntry = &g_sleIoTimerInfoHead;
	KIRQL Irql;
	KeRaiseIrql(DISPATCH_LEVEL,&Irql);
	for ( pNextList = pList->Blink; pNextList != pList; pNextList = pNextList->Blink )    //遍历blink链
	{
		pTimer = CONTAINING_RECORD(pNextList,IO_TIMER,TimerList);            //得到结构首

		if (MmIsAddressValidEx(pTimer) &&
			MmIsAddressValidEx(pTimer->DeviceObject) &&
			MmIsAddressValidEx(pTimer->TimerRoutine) &&
			MmIsAddressValidEx(&pTimer->TimerFlag))                    //过滤
		{
			if(uCount>=MAX_IO_TIMER_COUNT)
				break;
			PIOTIMERINFO piti = (PIOTIMERINFO)NextSListEntry(pSListEntry);
			if(piti)
			{
				if(!MmIsAddressValid((PVOID)pTimer))break;
				if(!MmIsAddressValid((PVOID)&pTimer->DeviceObject))break;
					piti->DeviceObject = (ULONG_PTR)pTimer->DeviceObject;
				if(!MmIsAddressValid((PVOID)&pTimer->TimerRoutine))break;
					piti->IoTimerRoutineAddress = (ULONG_PTR)pTimer->TimerRoutine;
				if(!MmIsAddressValid((PVOID)&pTimer->TimerFlag))break;
					piti->ulStatus = pTimer->TimerFlag;
			}
			uCount++;
		}
		if (!MmIsAddressValidEx(pNextList->Blink))
		{
			break;                  //过滤
		}
	}
	KeLowerIrql(Irql);
	return uCount;
}

BOOL AllocateIoTimerInfo(SIZE_T size)
{
	for(SIZE_T i=0;i<size;i++)
	{
		PIOTIMERINFO piti = (PIOTIMERINFO)ExAllocatePool(NonPagedPool,sizeof(IOTIMERINFO));
		if(!piti)
		{
			FreeIoTimerInfo();
			return FALSE;
		}
		PushSLISTENTRY(&g_sleIoTimerInfoHead,&piti->next);
	}
	return TRUE;
}

VOID FreeIoTimerInfo()
{
	ReleaseSListEntry(&g_sleIoTimerInfoHead);
}

VOID GetIoTimer(PVOID buff,SIZE_T size)
{
	size = size/sizeof(IoTimerInfo);
	PSLISTENTRY pSListEntry = &g_sleIoTimerInfoHead;
	PIoTimerInfo piti_tr = (PIoTimerInfo)buff;
	while(!IsEmptySListEntry(pSListEntry))
	{
		PIOTIMERINFO piti = (PIOTIMERINFO)NextSListEntry(pSListEntry);
		if(piti)
		{
			piti_tr->DeviceObject = piti->DeviceObject;
			piti_tr->IoTimerRoutineAddress = piti->IoTimerRoutineAddress;
			piti_tr->ulStatus = piti->ulStatus;
		}
		piti_tr++;
		size--;
		if(size==0)
		{
			break;
		}
	}
}

NTSTATUS StartIoTimer(PTransferMsg msg)
{
	PDEVICE_OBJECT DeviceObject = PDEVICE_OBJECT(*PULONG_PTR(msg->buff));
	if(MmIsAddressValid(DeviceObject))
	{
		IoStartTimer(DeviceObject);
		return STATUS_SUCCESS;
	}
	return STATUS_UNSUCCESSFUL;
}

NTSTATUS StopIoTimer(PTransferMsg msg)
{
	PDEVICE_OBJECT DeviceObject = PDEVICE_OBJECT(*PULONG_PTR(msg->buff));
	if(MmIsAddressValid(DeviceObject))
	{
		IoStopTimer(DeviceObject);
		return STATUS_SUCCESS;
	}
	return STATUS_UNSUCCESSFUL;
}
