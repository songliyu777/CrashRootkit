#ifdef __cplusplus
extern "C" {
#endif
#include <ntddk.h>
#include <string.h>
#ifdef __cplusplus
}; // extern "C"
#endif

#include "CRD_DpcTimer.h"
#include "KeSysFun.h"
#include "Platform.h"
#include "FuncAddrValid.h"
#define _CRD_
#include "Transfer.h"

#define KDBG_TAG 'GBDK'

#define MAX_PROCESSOR_COUNT 32  //当前机器处理器最大数量

SLISTENTRY g_sleDpcTimerInfoHead = {0};

/************************************************************************
* 函数名称:KdGetDebuggerDataBlock
* 功能描述:获取CPU域控制块KPCR中的KDDEBUGGER_DATA64
* 参数列表:
* 返回 值:PKDDEBUGGER_DATA64
*************************************************************************/
PKDDEBUGGER_DATA64 KdGetDebuggerDataBlock()
{
	PKDDEBUGGER_DATA64 pKDDebuggerData;
	PDBGKD_GET_VERSION64 KdVersionBlock;

	KeSetSystemAffinityThread(1);  //使当前线程运行在第一个处理器上，因为只有第一个处理器的值才有效
	_asm
	{
		mov eax,fs:[0x1c] //SelfPcr就是指向fs指向的_kpcr
		mov eax,[eax+0x34]//kpcr+0x34->KdVersionBlock
		mov KdVersionBlock,eax
	}
	KeRevertToUserAffinityThread();//恢复线程运行的处理器

	if (!MmIsAddressValidEx(KdVersionBlock))
	{
		return NULL;
	}
	pKDDebuggerData =(PKDDEBUGGER_DATA64)(*(PULONG)KdVersionBlock->DebuggerDataList);
	if (!MmIsAddressValidEx(pKDDebuggerData))
	{
		return NULL;
	}
	if (pKDDebuggerData->Header.OwnerTag != KDBG_TAG)
	{
		KdPrint(("(KdGetDebuggerDataBlock) KDDebuggerData failed to get"));
		return NULL;
	}
	return pKDDebuggerData;
}

////////////////////////////////////////////////////////////////////////////////////
ULONG64 GetKiProcessorBlock()
{
	PKDDEBUGGER_DATA64 KdData64;

	KdData64 = KdGetDebuggerDataBlock();
	if (MmIsAddressValidEx(KdData64))
	{
		return KdData64->KiProcessorBlock;
	}
	return 0;
}

SIZE_T GetDpcTimerInformation_WIN7600_UP()
{
	ULONG           TimerTableOffsetInKprcb;
	ULONG           NumberOfTimerTable;
	ULONG           NumberOfProcessor;
	ULONG           i,j;
	ULONG           ulTemp;
	ULONG			uCount =0;
	PULONG          pKiProcessorBlock = NULL;
	PKTIMER_TABLE_ENTRY_WIN7  pTimerTableEntryWin7 = NULL;
	PLIST_ENTRY          pNextList = NULL;
	PKTIMER          pTimer = NULL;
	ULONG ulModuleBase;
	ULONG ulModuleSize;

	TimerTableOffsetInKprcb = 0x1960+0x40;                        //首个_KTIMER_TABLE_ENTRY在PRCB中的偏移
	NumberOfTimerTable = 0x100;                                  //_KTIMER_TABLE_ENTRY数量

	NumberOfProcessor = (ULONG)KeNumberProcessors;                        //当前机器处理器数量
	if (NumberOfProcessor > MAX_PROCESSOR_COUNT){
		return 0;
	}

	pKiProcessorBlock = (PULONG)GetKiProcessorBlock();                      //取得KiProcessorBlock,包含了NumberOfProcessor个KPRCB
	if (!pKiProcessorBlock){
		return 0;
	}

	if(!AllocateDpcTimerInfo(NumberOfTimerTable))
	{
		return 0;
	}
	PSLISTENTRY pSListEntry = &g_sleDpcTimerInfoHead;
	KIRQL Irql;
	KeRaiseIrql(DISPATCH_LEVEL,&Irql);
	for ( i = 0; i < NumberOfProcessor; i++, pKiProcessorBlock++ )                //DPC timer在每个cpu中都有一个队列,所以枚举每一个KPRCB
	{
		//检测一下当前的KPRCB地址是否可访问
		if (!MmIsAddressValidEx((PVOID)pKiProcessorBlock)){
			break;
		}

		//取得当前CPU的KPRCB中首个KTIMER_TABLE_ENTRY地址
		ulTemp = *pKiProcessorBlock + TimerTableOffsetInKprcb;
		if (!MmIsAddressValidEx((PVOID)ulTemp)){
			break;
		}
		//此时ulTemp是 timer table entry地址
		pTimerTableEntryWin7 = (PKTIMER_TABLE_ENTRY_WIN7)ulTemp;                

		//准备遍历timer table表
		for ( j = 0; j < NumberOfTimerTable; j++, pTimerTableEntryWin7++ )
		{
			if (!MmIsAddressValidEx((PVOID)pTimerTableEntryWin7)){
				break;
			}
			//为空的数组高位双字为FFFFFFFF
			if ( pTimerTableEntryWin7->Time.HighPart == 0xFFFFFFFF ){
				continue;
			}

			//链表是否可以访问
			if (!MmIsAddressValidEx((PVOID)pTimerTableEntryWin7->Entry.Blink) ||
				!MmIsAddressValidEx((PVOID)pTimerTableEntryWin7->Entry.Flink)){
					continue;
			}

			for (pNextList   = (PLIST_ENTRY)pTimerTableEntryWin7->Entry.Blink;
				pNextList  != (PLIST_ENTRY)&pTimerTableEntryWin7->Entry;
				pNextList   = pNextList->Blink)
			{

				pTimer = CONTAINING_RECORD(pNextList,KTIMER,TimerListEntry);          //取得timer对象

				//检查pTimer以及各成员
				if (MmIsAddressValidEx((PVOID)pTimer) &&
					MmIsAddressValidEx((PVOID)pTimer->Dpc) &&
					MmIsAddressValidEx((PVOID)pTimer->Dpc->DeferredRoutine) &&
					MmIsAddressValidEx((PVOID)&pTimer->Period))//过滤
				{             
					if(uCount>=NumberOfTimerTable)
						break;
					PDPCTIMERINFO pdti = (PDPCTIMERINFO)NextSListEntry(pSListEntry);
					if(pdti)
					{
						if(!MmIsAddressValid((PVOID)pTimer))break;
						pdti->TimerAddress = (ULONG_PTR)pTimer;
						if(!MmIsAddressValid((PVOID)pTimer->Dpc))break;
						pdti->DpcAddress = (ULONG_PTR)pTimer->Dpc;
						if(!MmIsAddressValid((PVOID)&pTimer->Dpc->DeferredRoutine))break;
						pdti->DpcRoutineAddress = (ULONG_PTR)pTimer->Dpc->DeferredRoutine;
						if(!MmIsAddressValid((PVOID)&pTimer->Period))break;
						pdti->Period = (ULONG_PTR)pTimer->Period;
					}
					uCount++;
				}

				if (!MmIsAddressValidEx((PVOID)pNextList->Blink)){
					break;
				}
			}
		}
	}
	KeLowerIrql(Irql);
	return uCount;
}

ULONG GetKiTimerTableListHead()
{
	UNICODE_STRING UnicodeTimerHead;
	ULONG ulTimerTable;
	PUCHAR i;
	ULONG_PTR ulTimerTableListHead;
	SYSTEM_VERSION version;

	version = GetSystemVersion();
	switch (version)
	{
	case WINDOWSXP:
		ulTimerTable = (ULONG_PTR)KeUpdateSystemTime;
		if (ulTimerTable == 0) return 0;
		break;
	case WINDOWS2003:
		RtlInitUnicodeString(&UnicodeTimerHead,(PWCHAR)L"KeSetTimerEx");
		ulTimerTable = (ULONG_PTR)MmGetSystemRoutineAddress(&UnicodeTimerHead);
		if (ulTimerTable == 0) return 0;
		break;
	default:
		return 0;
	}
	for (i = (PUCHAR)ulTimerTable;i<(PUCHAR)ulTimerTable + 0x100;i++)
	{
		switch (version)
		{
		case WINDOWSXP:
			//80541fcd 8d0cc520405580  lea     ecx,nt!KiTimerTableListHead (80554020)[eax*8]
			if (*i == 0x8d)
			{
				ulTimerTableListHead = *(PULONG)(i+3);
				if (MmIsAddressValidEx((PVOID)ulTimerTableListHead))
				{
					return ulTimerTableListHead;
				}
			}
			break;
		case WINDOWS2003:
			//80826ea8 81c240738980    add     edx,offset nt!KiTimerTableListHead (80897340)
			if (*i == 0x81)
			{
				ulTimerTableListHead = *(PULONG)(i+2);
				if (MmIsAddressValidEx((PVOID)ulTimerTableListHead))
				{
					return ulTimerTableListHead;
				}
			}
			break;
		}
	}
	return NULL;
}

SIZE_T GetDpcTimerInformation_XP_2K3()
{
	ULONG  NumberOfTimerTable;
	ULONG  i;
	PLIST_ENTRY  pList = NULL;
	PLIST_ENTRY pNextList = NULL;
	PKTIMER  pTimer = NULL;
	ULONG ulModuleBase;
	ULONG ulModuleSize;
	ULONG uCount = 0;
	SYSTEM_VERSION version;

	version = GetSystemVersion();
	switch (version)
	{
	case WINDOWSXP:
		NumberOfTimerTable = 0x100;
		break;
	case WINDOWS2003:
		NumberOfTimerTable = 0x200;                                  //_KTIMER_TABLE_ENTRY数量
		break;
	default:
		return 0;
	}
	pList = (PLIST_ENTRY)GetKiTimerTableListHead();                    //取得链表头
	if (pList == NULL) return 0;
	if(!AllocateDpcTimerInfo(NumberOfTimerTable))
		 return 0;
	PSLISTENTRY pSListEntry = &g_sleDpcTimerInfoHead;
	KIRQL Irql;
	KeRaiseIrql(DISPATCH_LEVEL,&Irql);
	for ( i = 0; i < NumberOfTimerTable; i++, pList++ )                      //NumberOfTimerTable 个list
	{
		if (!MmIsAddressValidEx((PVOID)&pList))
		{
			break;
		}
		if (!MmIsAddressValidEx((PVOID)pList->Blink) ||
			!MmIsAddressValidEx((PVOID)pList->Flink))
		{
			continue;          //如果listentry域地址无效,continue
		}
		for ( pNextList = pList->Flink; pNextList != pList; pNextList = pNextList->Flink )    //遍历blink链
		{
			pTimer = CONTAINING_RECORD(pNextList,KTIMER,TimerListEntry);            //得到结构首

			//检查pTimer以及各成员
			if (MmIsAddressValidEx((PVOID)pTimer) &&
				MmIsAddressValidEx((PVOID)pTimer->Dpc) &&
				MmIsAddressValidEx((PVOID)pTimer->Dpc->DeferredRoutine) &&
				MmIsAddressValidEx((PVOID)&pTimer->Period))//过滤
			{             
				if(uCount>=NumberOfTimerTable)
					break;
				PDPCTIMERINFO pdti = (PDPCTIMERINFO)NextSListEntry(pSListEntry);
				if(pdti)
				{
					if(!MmIsAddressValid((PVOID)pTimer))break;
						pdti->TimerAddress = (ULONG_PTR)pTimer;
					if(!MmIsAddressValid((PVOID)pTimer->Dpc))break;
						pdti->DpcAddress = (ULONG_PTR)pTimer->Dpc;
					if(!MmIsAddressValid((PVOID)&pTimer->Dpc->DeferredRoutine))break;
						pdti->DpcRoutineAddress = (ULONG_PTR)pTimer->Dpc->DeferredRoutine;
					if(!MmIsAddressValid((PVOID)&pTimer->Period))break;
						pdti->Period = (ULONG_PTR)pTimer->Period;
				}
				uCount++;
			}
			if (!MmIsAddressValidEx(pNextList->Blink))
			{
				break;                  //过滤
			}
		}
	}
	KeLowerIrql(Irql);
	return uCount;
}

BOOL InsertDpcTimerInfo(PDPCTIMERINFO pdti,ULONG index)
{
	PSLISTENTRY pSListEntry = &g_sleDpcTimerInfoHead;
	ULONG nItem = 0;
	while(!IsEmptySListEntry(pSListEntry))
	{
		PDPCTIMERINFO newpdti = (PDPCTIMERINFO)NextSListEntry(pSListEntry);
		if(pdti)
		{
			if(nItem == index)
			{
				newpdti->TimerAddress = pdti->TimerAddress;
				newpdti->DpcAddress = pdti->DpcAddress;
				newpdti->DpcRoutineAddress = pdti->DpcRoutineAddress;
				newpdti->Period = pdti->Period;
				return TRUE;
			}
		}
		nItem++;
	}
	return FALSE;
}

VOID FreeDpcTimerInfo()
{
	ReleaseSListEntry(&g_sleDpcTimerInfoHead);
}

SIZE_T ListDpcTimer()
{
	FreeDpcTimerInfo();
	SYSTEM_VERSION version = GetSystemVersion();
	switch(version)
	{
	case WINDOWSXP:
		return GetDpcTimerInformation_XP_2K3();
	case WINDOWS7:
		return GetDpcTimerInformation_WIN7600_UP();
	}
	return 0;
}

VOID GetDpcTimer(PVOID buff,SIZE_T size)
{
	size = size/sizeof(DpcTimerInfo);
	PSLISTENTRY pSListEntry = &g_sleDpcTimerInfoHead;
	PDpcTimerInfo pdti_tr = (PDpcTimerInfo)buff;
	while(!IsEmptySListEntry(pSListEntry))
	{
		PDPCTIMERINFO pdti = (PDPCTIMERINFO)NextSListEntry(pSListEntry);
		if(pdti)
		{
			pdti_tr->TimerAddress = pdti->TimerAddress;
			pdti_tr->DpcAddress = pdti->DpcAddress;
			pdti_tr->DpcRoutineAddress = pdti->DpcRoutineAddress;
			pdti_tr->Period = pdti->Period;
		}
		pdti_tr++;
		size--;
		if(size==0)
		{
			break;
		}
	}
}

BOOL AllocateDpcTimerInfo(SIZE_T size)
{
	for(SIZE_T i=0;i<size;i++)
	{
		PDPCTIMERINFO pdti = (PDPCTIMERINFO)ExAllocatePool(NonPagedPool,sizeof(DPCTIMERINFO));
		if(!pdti)
		{
			FreeDpcTimerInfo();
			return FALSE;
		}
		PushSLISTENTRY(&g_sleDpcTimerInfoHead,&pdti->next);
	}
	return TRUE;
}

NTSTATUS KillDcpTimer(PTransferMsg msg)
{
	NTSTATUS status = STATUS_SUCCESS;
	PKTIMER Timer = PKTIMER(*PULONG_PTR(msg->buff));
	if(MmIsAddressValid(Timer))
	{
		if(!KeCancelTimer(Timer))
			status = STATUS_UNSUCCESSFUL;
	}
	else
	{
		status = STATUS_UNSUCCESSFUL;
	}
	return status;
}