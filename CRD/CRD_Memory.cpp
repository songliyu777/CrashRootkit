#ifdef __cplusplus
extern "C" {
#endif
#include <ntddk.h>
#include <string.h>
#ifdef __cplusplus
}; // extern "C"
#endif

#include "Log.h"
#include "Platform.h"
#include "KeSysFun.h"
#include "CRD_HookFunction.h"
#include "CRD_Memory.h"
#include "CRD_KernelModule.h"
#include "Relocate.h"
#include "CRD_SSDT.h"
#define _CRD_
#include "Transfer.h"

MMCOPYVIRTUALMEMORY g_MmCopyVirtualMemory = NULL;

NTSTATUS CRD_ReadProcessMemory(HANDLE hProcess,PVOID lpBaseAddress,PVOID lpBuffer,SIZE_T BufferSize, SIZE_T * lpNumberOfBytesRead)
{
	if(hProcess==NULL)
	{
		KdPrint(("ReadProcessMemory ß∞‹:1\n"));
		return STATUS_UNSUCCESSFUL;
	}

	NTSTATUS status=STATUS_UNSUCCESSFUL;

	PEPROCESS EProcess;
	KAPC_STATE ApcState;
	PVOID readbuffer=NULL;


	status = ObReferenceObjectByHandle(
		hProcess,
		PROCESS_VM_WRITE|PROCESS_VM_READ,
		NULL,
		KernelMode,
		(VOID**)&EProcess,
		NULL
		);


	if(!NT_SUCCESS(status))
	{
		ObDereferenceObject(EProcess);
		KdPrint(("ReadProcessMemory ß∞‹:2\n"));
		return STATUS_UNSUCCESSFUL;
	}

	readbuffer = ExAllocatePoolWithTag (NonPagedPool, BufferSize, 'Sys');

	if(readbuffer==NULL)
	{
		ObDereferenceObject(EProcess);
		KdPrint(("ReadProcessMemory ß∞‹:3\n"));
		return STATUS_UNSUCCESSFUL;
	}
	*(ULONG*)readbuffer=(ULONG)0x1;

	KESTACKATTACHPROCESS m_KeStackAttachProcess = (KESTACKATTACHPROCESS)GetRelocateFunction(KESTACKATTACHPROCESS_INDEX);
	m_KeStackAttachProcess (EProcess, &ApcState);
	__try
	{
		ProbeForRead ((CONST PVOID)lpBaseAddress, BufferSize, sizeof(CHAR));
		RtlCopyMemory (readbuffer, lpBaseAddress, BufferSize);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		status = STATUS_UNSUCCESSFUL;
		KdPrint(("ReadProcessMemory ß∞‹:4\n"));
	}

	KEUNSTACKDETACHPROCESS m_KeUnstackDetachProcess = (KEUNSTACKDETACHPROCESS)GetRelocateFunction(KEUNSTACKDETACHPROCESS_INDEX);
	m_KeUnstackDetachProcess (&ApcState);

	if(NT_SUCCESS(status))
	{
		if (MmIsAddressValid(lpBuffer))
		{
			__try
			{
				RtlCopyMemory (lpBuffer, readbuffer, BufferSize);
			}
			__except(EXCEPTION_EXECUTE_HANDLER)
			{
				status = STATUS_UNSUCCESSFUL;
				KdPrint(("ReadProcessMemory ß∞‹:5\n"));
			}
		}
		else
		{
			status = STATUS_UNSUCCESSFUL;
			KdPrint(("ReadProcessMemory ß∞‹:6\n"));
		}
	}

	ObDereferenceObject(EProcess);
	ExFreePool (readbuffer);
	return status;
}

//BOOL CRD_ReadProcessMemory(HANDLE hProcess,PVOID lpBaseAddress,PVOID lpBuffer,SIZE_T nSize, SIZE_T * lpNumberOfBytesRead)
//{
//	if(!lpBaseAddress)
//	{
//		return FALSE;
//	}
//	if(KeGetCurrentIrql()!=PASSIVE_LEVEL)
//	{
//		return FALSE;
//	}
//	PEPROCESS eproc;
//	SIZE_T i = 0;
//	NTSTATUS status = PsLookupProcessByProcessId((HANDLE)hProcess,&eproc);
//	if(!NT_SUCCESS(status))
//	{
//		return FALSE;
//	}
//	if(MmIsAddressValid(PVOID((ULONG_PTR)eproc+GetDirectoryTableBase_Offset())))
//	{
//		//KIRQL Irql;
//		//KeRaiseIrql(DISPATCH_LEVEL,&Irql);
//		ULONG uOldCr3 = 0;
//		__try
//		{ 
//			ULONG uCurrentCr3 = *(PULONG)((ULONG_PTR)eproc + GetDirectoryTableBase_Offset());
//			__asm
//			{
//				mov eax, cr3
//				mov uOldCr3, eax
//				mov eax, uCurrentCr3
//				mov cr3, eax
//			}
//			for(i=0;i<nSize;i++)
//			{
//				BYTE b = *((PBYTE)lpBaseAddress + i);
//				*((PBYTE)lpBuffer + i) = b ;
//			}
//		}
//		__except (EXCEPTION_EXECUTE_HANDLER)
//		{
//			PrintLog(L"This is an exception on ReadProcessMemory()...\r\n");
//			KdPrint(("This is an exception on ReadProcessMemory()...\r\n"));
//		}
//		*lpNumberOfBytesRead = i;
//		__asm
//		{
//			mov eax, uOldCr3
//			mov cr3, eax
//		}
//		//KeLowerIrql(Irql);
//	}
//	else
//	{
//		return FALSE;
//	}
//	return TRUE;
//}

NTSTATUS CRD_WriteProcessMemory(HANDLE ProcessHandle,PVOID lpBaseAddress,PVOID lpBuffer,SIZE_T BufferSize, SIZE_T * lpNumberOfBytesWritten)
{
	if(ProcessHandle==NULL)
	{
		KdPrint(("WriteProcessMemory ß∞‹:1\n"));
		return STATUS_UNSUCCESSFUL;
	}
	NTSTATUS status=STATUS_UNSUCCESSFUL;

	PEPROCESS EProcess; 
	KAPC_STATE ApcState;
	PVOID writebuffer=NULL;

	status = ObReferenceObjectByHandle(
		ProcessHandle,
		PROCESS_VM_WRITE|PROCESS_VM_READ,
		NULL,
		KernelMode,
		(VOID**)&EProcess,
		NULL
		);

	if(!NT_SUCCESS(status))
	{
		ObDereferenceObject(EProcess);
		KdPrint(("WriteProcessMemory ß∞‹:2\n"));
		return STATUS_UNSUCCESSFUL;
	}
	writebuffer = ExAllocatePoolWithTag (NonPagedPool, BufferSize, 'Sys');

	if(writebuffer==NULL)
	{
		ObDereferenceObject(EProcess);
		KdPrint(("WriteProcessMemory ß∞‹:3\n"));
		return STATUS_UNSUCCESSFUL;
	}

	*(ULONG*)writebuffer=(ULONG)0x1;

	if (MmIsAddressValid(lpBuffer))
	{
		__try
		{
			RtlCopyMemory (writebuffer, lpBuffer, BufferSize);
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			KdPrint(("WriteProcessMemory ß∞‹:4\n"));
			status = STATUS_UNSUCCESSFUL;
		}
	}
	else
	{
		KdPrint(("WriteProcessMemory ß∞‹:5\n"));
		status = STATUS_UNSUCCESSFUL;
	}

	if (NT_SUCCESS(status))
	{
		KESTACKATTACHPROCESS m_KeStackAttachProcess = (KESTACKATTACHPROCESS)GetRelocateFunction(KESTACKATTACHPROCESS_INDEX);
		m_KeStackAttachProcess (EProcess, &ApcState);
		__try
		{
			ProbeForWrite ((CONST PVOID)lpBaseAddress, BufferSize, sizeof(CHAR));
			RtlCopyMemory (lpBaseAddress,writebuffer, BufferSize);
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			KdPrint(("WriteProcessMemory ß∞‹:6\n"));
			status = STATUS_UNSUCCESSFUL;
		}
		KEUNSTACKDETACHPROCESS m_KeUnstackDetachProcess = (KEUNSTACKDETACHPROCESS)GetRelocateFunction(KEUNSTACKDETACHPROCESS_INDEX);
		m_KeUnstackDetachProcess (&ApcState);
	}

	ObDereferenceObject(EProcess);
	ExFreePool (writebuffer);
	return status;
}
//BOOL CRD_WriteProcessMemory(HANDLE hProcess,PVOID lpBaseAddress,PVOID lpBuffer,SIZE_T nSize, SIZE_T * lpNumberOfBytesWritten)
//{
//	if(!lpBaseAddress)
//	{
//		return FALSE;
//	}
//	if(KeGetCurrentIrql()!=PASSIVE_LEVEL)
//	{
//		return FALSE;
//	}
//	PEPROCESS eproc;
//	SIZE_T i = 0;
//	NTSTATUS status = PsLookupProcessByProcessId((HANDLE)hProcess,&eproc);
//	if(!NT_SUCCESS(status))
//	{
//		return FALSE;
//	}
//	if(MmIsAddressValid(PVOID((ULONG_PTR)eproc+GetDirectoryTableBase_Offset())))
//	{
//		//KIRQL Irql;
//		//KeRaiseIrql(DISPATCH_LEVEL,&Irql);
//		ULONG uOldCr3 = 0;
//		__try
//		{ 
//			ULONG uCurrentCr3 = *(PULONG)((ULONG_PTR)eproc + GetDirectoryTableBase_Offset());
//			__asm
//			{
//				mov eax, cr3
//					mov uOldCr3, eax
//					mov eax, uCurrentCr3
//					mov cr3, eax
//			}
//			PAGEPROTECTOFF();
//			for(i=0;i<nSize;i++)
//			{
//				BYTE b = *((PBYTE)lpBuffer + i);
//				*((PBYTE)lpBaseAddress + i) = b;
//			}
//		}
//		__except (EXCEPTION_EXECUTE_HANDLER)
//		{
//			PrintLog(L"This is an exception on WriteProcessMemory()...\r\n");
//			KdPrint(("This is an exception on WriteProcessMemory()...\r\n"));
//		}
//		PAGEPROTECTON();
//		*lpNumberOfBytesWritten = i;
//		__asm
//		{
//			mov eax, uOldCr3
//			mov cr3, eax
//		}
//		//KeLowerIrql(Irql);
//	}
//	else
//	{
//		return FALSE;
//	}
//	return TRUE;
//}

NTSTATUS CRD_OpenProcess(IN ULONG ProcessId, OUT PHANDLE pHandle)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PEPROCESS EProcess = NULL;
	HANDLE handle = NULL;
	char AuxData[0xc8];   
	ACCESS_STATE AccessState;
	ULONG PGENERIC_MAPPING_OFFSET = 0x34;
	SYSTEM_VERSION version = GetSystemVersion();
	switch(version)
	{
	case WINDOWSXP:
		PGENERIC_MAPPING_OFFSET = 0x68;
		break;
	case WINDOWS7:
		PGENERIC_MAPPING_OFFSET = 0x34;
		break;
	}
	status = SeCreateAccessState(
		&AccessState,
		&AuxData,
		PROCESS_ALL_ACCESS,
		(PGENERIC_MAPPING)((PCHAR)*PsProcessType + PGENERIC_MAPPING_OFFSET)  
		);

	if (!NT_SUCCESS(status) ) {
		return status;
	}
	
	AccessState.PreviouslyGrantedAccess |= AccessState.RemainingDesiredAccess;  
	AccessState.RemainingDesiredAccess = 0; 

	status = PsLookupProcessByProcessId((HANDLE)ProcessId, &EProcess);
	if (!NT_SUCCESS(status) ) {
		SeDeleteAccessState(&AccessState);  
		return status;
	}

	if (NT_SUCCESS(status))
	{
		status = ObOpenObjectByPointer(EProcess, 0, &AccessState, PROCESS_ALL_ACCESS, *PsProcessType, UserMode, &handle);
		if (NT_SUCCESS(status))
		{
			*pHandle = handle;
		}
		SeDeleteAccessState(&AccessState);  
		ObfDereferenceObject(EProcess);
	} 
	return status;
}

SLISTENTRY g_sleHideMemoryInfoHead = {0};

NTSTATUS HideProcessMemory(PTransferMsg msg)
{
	if(AddHideMemoryInfo(PModuleInfo(msg->buff)))
	{
		return STATUS_SUCCESS;
	}
	return STATUS_UNSUCCESSFUL;
}

BOOL AddHideMemoryInfo(PModuleInfo pmi)
{
	PMODULEINFO new_pmi = (PMODULEINFO)ExAllocatePool(NonPagedPool,sizeof(MODULEINFO)); 
	if(new_pmi)
	{
		RtlZeroMemory(new_pmi,sizeof(MODULEINFO));
		new_pmi->BaseAddress = pmi->BaseAddress;
		new_pmi->SizeOfImage = pmi->SizeOfImage;
		PushSLISTENTRY(&g_sleHideMemoryInfoHead,&new_pmi->next);
		return TRUE;
	}
	return FALSE;
}

VOID FreeHideMemoryInfo()
{
	ReleaseSListEntry(&g_sleHideMemoryInfoHead);
}

BOOL IsHideMemory(ULONG_PTR address)
{
	PSLISTENTRY pSListEntry = &g_sleHideMemoryInfoHead;
	while(!IsEmptySListEntry(pSListEntry))
	{
		PMODULEINFO pmi = (PMODULEINFO)NextSListEntry(pSListEntry);
		if(pmi)
		{
			if(pmi->BaseAddress <= address && address < pmi->BaseAddress + pmi->SizeOfImage)
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

NTSTATUS ModifyKernelMemory(PTransferMsg msg)
{
	ULONG type = *PULONG(msg->buff);
	if(type)
	{
		//–¥ƒ⁄¥Ê
		PWriteMemoryInfo pwi = PWriteMemoryInfo(msg->buff);
		PVOID buff = PVOID(pwi+1);
		if(MmIsAddressValid((PVOID)pwi->StartAddress))
		{
			PAGEPROTECTOFF();
			KIRQL Irql;
			KeRaiseIrql(DISPATCH_LEVEL,&Irql);
			RtlCopyMemory((PVOID)pwi->StartAddress,buff,pwi->nSize);
			KeLowerIrql(Irql);
			PAGEPROTECTON();
			return STATUS_SUCCESS;
		}
	}
	else
	{
		//∂¡ƒ⁄¥Ê
	}
	return STATUS_UNSUCCESSFUL;
}
//xp
//805b548c 8d45d8          lea     eax,[ebp-28h]
//805b548f 50              push    eax
//805b5490 ff75e0          push    dword ptr [ebp-20h]
//805b5493 56              push    esi
//805b5494 ff750c          push    dword ptr [ebp+0Ch]
//805b5497 ff75dc          push    dword ptr [ebp-24h]
//805b549a ff7510          push    dword ptr [ebp+10h]
//805b549d ff7744          push    dword ptr [edi+44h]
//805b54a0 e887fdffff      call    nt!MmCopyVirtualMemory (805b522c)


//win7
//83eac9a5 8b4750          mov     eax,dword ptr [edi+50h]
//83eac9a8 8d4ddc          lea     ecx,[ebp-24h]
//83eac9ab 51              push    ecx
//83eac9ac ff75e4          push    dword ptr [ebp-1Ch]
//83eac9af 56              push    esi
//83eac9b0 ff750c          push    dword ptr [ebp+0Ch]
//83eac9b3 ff75e0          push    dword ptr [ebp-20h]
//83eac9b6 ff7510          push    dword ptr [ebp+10h]
//83eac9b9 50              push    eax
//83eac9ba e831fbffff      call    nt!MmCopyVirtualMemory (83eac4f0)

//
//BOOL InitMmCopyVirtualMemory()
//{
//	ULONG_PTR ntwvmadr = NULL,maddress = NULL;
//	SYSTEM_VERSION version = GetSystemVersion();
//	switch(version)
//	{
//	case WINDOWSXP:
//		ntwvmadr = GetSSDTFunction(NtWriteVirtualMemory_Index_WinXP);
//		break;
//	case WINDOWS7:
//		ntwvmadr = GetSSDTFunction(NtWriteVirtualMemory_Index_Win7);
//		break;
//	}
//	if(!ntwvmadr)
//		return FALSE;
//	BYTE KeyWords[4] = {0x56,0xff,0x75,0x0c};
//	BYTE KeyWord = 0xe8;
//	maddress = FindMemoryAddress((ULONG_PTR)ntwvmadr,(ULONG_PTR)ntwvmadr+0x100,KeyWords,4,FALSE);
//	if(maddress)
//	{
//		ntwvmadr = FindMemoryAddress((ULONG_PTR)maddress,(ULONG_PTR)maddress+0x10,&KeyWord,1,FALSE);
//		if(ntwvmadr)
//		{
//			g_MmCopyVirtualMemory = MMCOPYVIRTUALMEMORY(ntwvmadr + *PULONG_PTR(ntwvmadr+1) + 5);
//			KdPrint(("ªÒ»°MmCopyVirtualMemoryµÿ÷∑[%p]≥…π¶\r\n",g_MmCopyVirtualMemory));
//			return TRUE;
//		}
//	}
//	return FALSE;
//}
