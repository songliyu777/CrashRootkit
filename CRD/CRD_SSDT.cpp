#ifdef __cplusplus
extern "C" {
#endif
#include <ntddk.h>
#include <string.h>
#ifdef __cplusplus
}; // extern "C"
#endif

#include "CRD_SSDT.h"
#include "CRD_Process.h"
#include "KeSysFun.h"
#include "Log.h"
#include "Platform.h"
#define _CRD_
#include "Transfer.h"

extern "C" extern PSSDT KeServiceDescriptorTable;
PSSDT g_OriginalSSDT = NULL;
PSSDTShadow KeServiceDescriptorTableShadow = NULL;
PSSDTShadow g_OriginalSSDTShadow = NULL;

ULONG_PTR GetSSDTShadow()
{
	SYSTEM_VERSION version = GetSystemVersion();
	if(version==WINDOWS8)
	{
		//win8x32
		//817c6e2f 7541            jne     nt!KeAddSystemServiceTable+0x5b (817c6e72)
		//	nt!KeAddSystemServiceTable+0x1a:
		//817c6e31 83b9c033678100  cmp     dword ptr nt!KeServiceDescriptorTableShadow (816733c0)[ecx],0
		BYTE KeyWords[] = {0x75, 0x41, 0x83, 0xb9};	
		ULONG_PTR adr = 0;
		if(&KeAddSystemServiceTable)
		{
			adr = FindMemoryAddress((ULONG_PTR)KeAddSystemServiceTable, (ULONG_PTR)KeAddSystemServiceTable + 0x100, KeyWords, 0x4, FALSE);
			if(adr)
			{
				return *PULONG_PTR(adr + 4);
			}
		}
	}
	else
	{
		BYTE KeyWords[] = {0x8d, 0x88};			/*8d88401b9381    lea     ecx,nt!KeServiceDescriptorTableShadow (81931b40)[eax]*/	
		ULONG_PTR adr = 0;
		if(&KeAddSystemServiceTable)
		{
			adr = FindMemoryAddress((ULONG_PTR)KeAddSystemServiceTable, (ULONG_PTR)KeAddSystemServiceTable + 0x100, KeyWords, 0x2, FALSE);
			if(adr)
			{
				return *PULONG_PTR(adr + 2);
			}
		}
	}
	return (ULONG_PTR)0;
}

VOID InitKeServiceDescriptorTableShadow()
{
	if(!KeServiceDescriptorTableShadow)
	{
		KeServiceDescriptorTableShadow = (PSSDTShadow)GetSSDTShadow();
		if(!KeServiceDescriptorTableShadow)
		{
			KdPrint(("获取KeServiceDescriptorTableShadow失败 \r\n"));
			PrintLog(L"获取KeServiceDescriptorTableShadow失败 \r\n");
		}
	}
}

VOID InitOriginalSSDT(ULONG_PTR pPe_NtKr,ULONG_PTR NtKrBase)
{
	DWORD offset =	(DWORD)KeServiceDescriptorTable->pvSSDTBase - (DWORD)NtKrBase;
	g_OriginalSSDT = (PSSDT)ExAllocatePool(NonPagedPool,sizeof(SSDT));
	if(!g_OriginalSSDT)
		return;
	RtlCopyMemory(g_OriginalSSDT,KeServiceDescriptorTable,sizeof(SSDT));
	PVOID originalssdtbase = PVOID(pPe_NtKr + offset);
	SIZE_T sizeofssdt = KeServiceDescriptorTable->ulNumberOfServices * sizeof(ULONG_PTR);
	g_OriginalSSDT->pvSSDTBase = ExAllocatePool(NonPagedPool,sizeofssdt);
	if(!g_OriginalSSDT->pvSSDTBase)
	{
		ExFreePool(g_OriginalSSDT);
		g_OriginalSSDT = NULL;
	}
	RtlCopyMemory(g_OriginalSSDT->pvSSDTBase,originalssdtbase,sizeofssdt);
}

VOID InitOriginalSSDTShadow(ULONG_PTR pPe_Win32kBase,ULONG_PTR Win32kBase)
{
	DWORD offset =	(DWORD)KeServiceDescriptorTableShadow->pvSSDTShadowBase - (DWORD)Win32kBase;
	g_OriginalSSDTShadow = (PSSDTShadow)ExAllocatePool(NonPagedPool,sizeof(SSDTShadow));
	if(!g_OriginalSSDTShadow)
		return;
	RtlCopyMemory(g_OriginalSSDTShadow,KeServiceDescriptorTableShadow,sizeof(SSDTShadow));
	PVOID originalssdtshadowbase = PVOID(pPe_Win32kBase + offset);
	SIZE_T sizeofssdtshadow = KeServiceDescriptorTableShadow->ulNumberOfServicesShadow * sizeof(ULONG_PTR);
	g_OriginalSSDTShadow->pvSSDTShadowBase = ExAllocatePool(NonPagedPool,sizeofssdtshadow);
	if(!g_OriginalSSDTShadow->pvSSDTShadowBase)
	{
		ExFreePool(g_OriginalSSDTShadow);
		g_OriginalSSDTShadow = NULL;
	}
	RtlCopyMemory(g_OriginalSSDTShadow->pvSSDTShadowBase,originalssdtshadowbase,sizeofssdtshadow);
}

PSSDT GetCurrentSSDT()
{
	return KeServiceDescriptorTable;
}

PSSDTShadow GetCurrentSSDTShadow()
{
	return KeServiceDescriptorTableShadow;
}

BOOL CRD_SSDTGet(PVOID buff,SIZE_T size)
{
	if(!KeServiceDescriptorTable)
		return FALSE;
	PSSDTInfo pSSDTInfo = (PSSDTInfo)buff;
	for(DWORD i = 0;i < KeServiceDescriptorTable->ulNumberOfServices;i++)
	{
		ULONG_PTR CurrentAddress = *PULONG_PTR((ULONG)KeServiceDescriptorTable->pvSSDTBase + i * 4);
		pSSDTInfo->CurrentAddress = CurrentAddress;
		if(g_OriginalSSDT)
		{
			ULONG_PTR OriginalAddress = *PULONG_PTR((ULONG)g_OriginalSSDT->pvSSDTBase + i * 4);
			pSSDTInfo->OriginalAddress = OriginalAddress;
		}
		else
		{
			pSSDTInfo->OriginalAddress = CurrentAddress;
		}
		pSSDTInfo->Index = i;
		pSSDTInfo++;
	}
	return TRUE;
}

BOOL CRD_SSDTShadowGet(PVOID buff,SIZE_T size)
{
	if(!KeServiceDescriptorTableShadow)
		return FALSE;
	if(!GetCsrssEprocess())
		return FALSE;
	KeAttachProcess(GetCsrssEprocess());
	PSSDTInfo pSSDTInfo = (PSSDTInfo)buff;
	for(DWORD i = 0;i < KeServiceDescriptorTableShadow->ulNumberOfServicesShadow;i++)
	{
		ULONG_PTR CurrentAddress = *PULONG_PTR((ULONG)KeServiceDescriptorTableShadow->pvSSDTShadowBase + i * 4);
		pSSDTInfo->CurrentAddress = CurrentAddress;
		if(g_OriginalSSDTShadow)
		{
			ULONG_PTR OriginalAddress = *PULONG_PTR((ULONG)g_OriginalSSDTShadow->pvSSDTShadowBase + i * 4);
			pSSDTInfo->OriginalAddress = OriginalAddress;
		}
		else
		{
			pSSDTInfo->OriginalAddress = CurrentAddress;
		}
		pSSDTInfo->Index = i;
		pSSDTInfo++;
	}
	KeDetachProcess();
	return TRUE;
}

VOID FreeSSDT()
{
	if(g_OriginalSSDT)
	{
		if(g_OriginalSSDT->pvSSDTBase)
		{
			ExFreePool(g_OriginalSSDT->pvSSDTBase);
		}
		ExFreePool(g_OriginalSSDT);
		g_OriginalSSDT = NULL;
	}
	if(g_OriginalSSDTShadow)
	{
		if(g_OriginalSSDTShadow->pvSSDTShadowBase)
		{
			ExFreePool(g_OriginalSSDTShadow->pvSSDTShadowBase);
		}
		ExFreePool(g_OriginalSSDTShadow);
		g_OriginalSSDTShadow = NULL;
	}
}

NTSTATUS RecoverSSDT(PTransferMsg msg)
{
	PSSDTInfo pSI = (PSSDTInfo)msg->buff;
	if(KeServiceDescriptorTable && pSI->CurrentAddress != pSI->OriginalAddress && pSI->Index < KeServiceDescriptorTable->ulNumberOfServices)
	{
		PVOID dist = PVOID((ULONG_PTR)KeServiceDescriptorTable->pvSSDTBase +  pSI->Index * 4);
		PAGEPROTECTOFF();
		KIRQL Irql;
		KeRaiseIrql(DISPATCH_LEVEL,&Irql);
		RtlCopyMemory(dist,&pSI->OriginalAddress,sizeof(ULONG));
		KeLowerIrql(Irql);
		PAGEPROTECTON();
		return STATUS_SUCCESS;
	}
	return STATUS_UNSUCCESSFUL;
}

NTSTATUS RecoverSSDTShadow(PTransferMsg msg)
{
	if(!GetCsrssEprocess())
		return STATUS_UNSUCCESSFUL;
	PSSDTInfo pSI = (PSSDTInfo)msg->buff;
	KeAttachProcess(GetCsrssEprocess());
	if(KeServiceDescriptorTableShadow && pSI->CurrentAddress != pSI->OriginalAddress && pSI->Index < KeServiceDescriptorTableShadow->ulNumberOfServicesShadow)
	{
		PVOID dist = PVOID((ULONG_PTR)KeServiceDescriptorTableShadow->pvSSDTShadowBase +  pSI->Index * 4);
		PAGEPROTECTOFF();
		KIRQL Irql;
		KeRaiseIrql(DISPATCH_LEVEL,&Irql);
		RtlCopyMemory(dist,&pSI->OriginalAddress,sizeof(ULONG));
		KeLowerIrql(Irql);
		PAGEPROTECTON();
		KeDetachProcess();
		return STATUS_SUCCESS;
	}
	KeDetachProcess();
	return STATUS_UNSUCCESSFUL;
}

PSSDT GetOriginalSSDT()
{
	return g_OriginalSSDT;
}

PSSDTShadow GetOriginalSSDTShadow()
{
	return g_OriginalSSDTShadow;
}

ULONG_PTR GetSSDTFunction(ULONG FuncIndex)
{
	return *PULONG_PTR((ULONG_PTR)(KeServiceDescriptorTable->pvSSDTBase) + FuncIndex * 4);
}

ULONG_PTR GetSSDTShadowFunction(ULONG FuncIndex)
{
	return *PULONG_PTR((ULONG_PTR)(KeServiceDescriptorTableShadow->pvSSDTShadowBase) + FuncIndex * 4);
}