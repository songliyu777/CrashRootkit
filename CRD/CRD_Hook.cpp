#ifdef __cplusplus
extern "C" {
#endif
#include <ntddk.h>
#include <string.h>
#ifdef __cplusplus
}; // extern "C"
#endif

#include "CRD_Hook.h"
#include "CRD_HideKd.h"
#include "CRD_HookFunction.h"
#include "CRD_HideWindows.h"
#include "CRD_Process.h"
#include "CRD_ProtectTools.h"
#include "CRD_BsodHook.h"
#include "CRD_KernelModule.h"
#include "CRD_KeException.h"
#include "Log.h"
#include "KeSysFun.h"
#include "Relocate.h"
#include "Platform.h"

SLISTENTRY g_sleDynamicInLineHookHead = {0};

BOOL AddDInLineHook(PDInLineHook pDlh)
{
	PDInLineHook pDlhnew = (PDInLineHook)ExAllocatePool(NonPagedPool,sizeof(DInLineHook));
	if(pDlhnew)
	{
		RtlZeroMemory(pDlhnew,sizeof(DInLineHook));
		pDlhnew->code_length = pDlh->code_length;
		pDlhnew->dynamic_code_enter = pDlh->dynamic_code_enter;
		pDlhnew->myfunction_address = pDlh->myfunction_address;
		pDlhnew->original_address = pDlh->original_address;
		pDlhnew->recover_code = pDlh->recover_code;
		pDlhnew->hook_code = pDlh->hook_code;
		strcpy(pDlhnew->name, pDlh->name);
		PushSLISTENTRY(&g_sleDynamicInLineHookHead,&pDlhnew->next);
		return TRUE;
	}
	return FALSE;
}

BOOL DoDynamicInLineHook(PDInLineHook pDilh)
{
	//64位下 代码长度最少要16 32位下最少要4
	if(pDilh->code_length == 0)
	{
		KdPrint(("需要制定动态HOOK代码长度 \r\n"));
		PrintLog(L"Hook code_length is not 0\r\n");
		return FALSE;
	}
#ifndef _WIN64
	if(pDilh->code_length < 4)
	{
		KdPrint(("JMP代码长度不够 %d \r\n",pDilh->code_length));
		PrintLog(L"JMP code is not enough %d\r\n",pDilh->code_length);
		return FALSE;
	}
#else
	if(pDilh->code_length < 16)
	{
		KdPrint(("JMP代码长度不够 %d \r\n",pDilh->code_length));
		PrintLog(L"JMP code is not enough %d\r\n",pDilh->code_length);
		return FALSE;
	}
#endif
	if(pDilh->myfunction_address==NULL || pDilh->original_address==NULL)
	{
		KdPrint(("原函数和跳转函数地址不能为空 \r\n"));
		PrintLog(L"Original and Jmp address are not NULL\r\n");
		return FALSE;
	}
	if(strlen(pDilh->name)==0)
	{
		KdPrint(("需要一个名字标示 \r\n"));
		PrintLog(L"hook need a name\r\n");
		return FALSE;
	}
	if(pDilh->recover_code == NULL)
	{
		pDilh->recover_code = (PBYTE)ExAllocatePool(NonPagedPool, pDilh->code_length);
		if(!pDilh->recover_code)
		{
			KdPrint(("DynamicInLineHook分配恢复代码存储空间失败 \r\n"));
			PrintLog(L"Allocate DynamicInLineHook recover code failed \r\n");
			return FALSE;
		}
	}
	if(pDilh->hook_code == NULL)
	{
		pDilh->hook_code = (PBYTE)ExAllocatePool(NonPagedPool, pDilh->code_length);
		if(!pDilh->hook_code)
		{
			KdPrint(("分配DynamicInLineHook代码存储空间失败 \r\n"));
			PrintLog(L"Allocate DynamicInLineHook hook code failed \r\n");
			ExFreePool(pDilh->recover_code);
			return FALSE;
		}
	}
#ifndef _WIN64
	pDilh->dynamic_code_enter = (ULONG_PTR)ExAllocatePool(NonPagedPool, pDilh->code_length + 5);
#else
	pDilh->dynamic_code_enter = (ULONG_PTR)ExAllocatePool(NonPagedPool, pDilh->code_length + 15);
#endif
	if(!pDilh->dynamic_code_enter)
	{
		KdPrint(("分配动态代码空间失败 \r\n"));
		PrintLog(L"Allocate dynamic code enter failed \r\n");
		ExFreePool(pDilh->recover_code);
		ExFreePool(pDilh->hook_code);
		return FALSE;
	}

	//构造动态入口代码
	ULONG_PTR dynamic_code_enter = pDilh->dynamic_code_enter;
	ULONG_PTR originFunctionAddress = pDilh->dynamic_code_enter + pDilh->code_length;
	ULONG_PTR mapFunctionAddress = pDilh->original_address + pDilh->code_length;
	SIZE_T code_length = pDilh->code_length;
	ULONG_PTR value =  0;
	if(pDilh->type==0)
	{
		value =  mapFunctionAddress - originFunctionAddress - 5;
	}
	if(pDilh->type==1)
	{
		value =  mapFunctionAddress - originFunctionAddress - 4;
	}
#ifndef _WIN64
	if(pDilh->type==0)
	{
		_asm
		{
			mov eax,dynamic_code_enter
				mov ebx,code_length
				add eax,ebx
				mov [eax],0xe9
				add eax,1
				mov ebx,value
				mov dword ptr [eax],ebx 
		}
	}
#else
	mapFunctionAddress-=2;//为了返回到原函数中多运行一个pop r11 5b41H
	ASM_struct_return_code_D(originFunctionAddress, mapFunctionAddress); // push r11 5341H,mov r11,push r11,ret
#endif
	//Hook函数
	originFunctionAddress = pDilh->original_address;
	RtlCopyMemory(pDilh->recover_code, (PVOID)originFunctionAddress, pDilh->code_length);
	RtlCopyMemory((PVOID)pDilh->dynamic_code_enter, pDilh->recover_code, pDilh->code_length);
	mapFunctionAddress = pDilh->myfunction_address;
	if(pDilh->type==0)
	{
		value =  mapFunctionAddress - originFunctionAddress - 5;
	}
	if(pDilh->type==1)
	{
		value =  mapFunctionAddress - originFunctionAddress - 4;
	}
	PAGEPROTECTOFF();
	KIRQL Irql;
	KeRaiseIrql(DISPATCH_LEVEL,&Irql);
#ifndef _WIN64
	if(pDilh->type==0)
	{
		_asm
		{
			mov eax,originFunctionAddress
				mov [eax],0xe9
				add eax,1
				mov ebx,value
				mov dword ptr [eax],ebx
		}
	}
	if(pDilh->type==1)
	{
		_asm
		{
			mov eax,originFunctionAddress
			mov ebx,value
			mov dword ptr [eax],ebx
		}
	}
#else
	UINT64 popadr = originFunctionAddress + pDilh->code_length - 2;//多构建一个pop r11 5b41H还原寄存器
	ASM_struct_return_code_O(originFunctionAddress, mapFunctionAddress,popadr);
#endif
	RtlCopyMemory(pDilh->hook_code, (PVOID)originFunctionAddress, pDilh->code_length);
	AddDInLineHook(pDilh);
	KeLowerIrql(Irql);
	PAGEPROTECTON();
	PrintLog(L"[%S] length(%d) dynamic_code_enter[%p] HookCode[%p] \r\n",pDilh->name,pDilh->code_length,pDilh->dynamic_code_enter,pDilh->hook_code);
	return TRUE;
}

BOOL UnDynamicInLineHook()
{
	BOOL bSuccess = TRUE;
	PSLISTENTRY pSListEntry = &g_sleDynamicInLineHookHead;
	if(!GetCsrssEprocess())
		return TRUE;
	KeAttachProcess(GetCsrssEprocess());
	PAGEPROTECTOFF();
	KIRQL Irql;
	KeRaiseIrql(DISPATCH_LEVEL,&Irql);
	while(!IsEmptySListEntry(pSListEntry))
	{
		PDInLineHook pDilh = (PDInLineHook)NextSListEntry(pSListEntry);
		if(pDilh)
		{
			RtlCopyMemory((PVOID)pDilh->original_address,pDilh->recover_code,pDilh->code_length);
			if(RtlCompareMemory((PVOID)pDilh->original_address,pDilh->recover_code,pDilh->code_length)!=pDilh->code_length)//判断是否恢复成功
			{
				bSuccess = FALSE;
				break;
			}
		}
	}
	KeLowerIrql(Irql);
	PAGEPROTECTON();
	KeDetachProcess();
	return bSuccess; 
}

VOID FreeDynamicInLineHook()
{
	while(!UnDynamicInLineHook())
	{
		PrintLog(L"UnDynamicInLineHook Failed \r\n");
		KeSleep(100);
	}
	KeSleep(500);
	PSLISTENTRY pSListEntry = &g_sleDynamicInLineHookHead;
	while(!IsEmptySListEntry(pSListEntry))
	{
		PDInLineHook pDilh = (PDInLineHook)NextSListEntry(pSListEntry);
		if(pDilh)
		{
			if(pDilh->recover_code)
				ExFreePool(pDilh->recover_code);
			if(pDilh->hook_code)
				ExFreePool(pDilh->hook_code);
			if(pDilh->dynamic_code_enter)
				ExFreePool((PVOID)pDilh->dynamic_code_enter);
		}
	}
	ReleaseSListEntry(&g_sleDynamicInLineHookHead);
}

BOOL FreeDynamicInLineHookByFunctionName(LPCSTR lpFunctionName)
{
	BOOL bSuccess = TRUE;
	PSLISTENTRY pSListEntry = &g_sleDynamicInLineHookHead;
	if(!GetCsrssEprocess())
		return TRUE;
	KeAttachProcess(GetCsrssEprocess());
	PAGEPROTECTOFF();
	KIRQL Irql;
	KeRaiseIrql(DISPATCH_LEVEL,&Irql);
	while(!IsEmptySListEntry(pSListEntry))
	{
		PDInLineHook pDilh = (PDInLineHook)NextSListEntry(pSListEntry);
		if(pDilh)
		{
			if(!strcmp(pDilh->name,lpFunctionName))
			{
				RtlCopyMemory((PVOID)pDilh->original_address,pDilh->recover_code,pDilh->code_length);
				if(RtlCompareMemory((PVOID)pDilh->original_address,pDilh->recover_code,pDilh->code_length)!=pDilh->code_length)//判断是否恢复成功
				{
					bSuccess = FALSE;
					break;
				}
			}

		}
	}
	KeLowerIrql(Irql);
	PAGEPROTECTON();
	KeDetachProcess();
	if(bSuccess)
	{
		KeSleep(500);
		pSListEntry = &g_sleDynamicInLineHookHead;
		while(!IsEmptySListEntry(pSListEntry))
		{
			PSLISTENTRY pPreSLE = pSListEntry;
			PDInLineHook pDilh = (PDInLineHook)NextSListEntry(pSListEntry);
			if(pDilh)
			{
				if(!strcmp(pDilh->name,lpFunctionName))
				{
					if(pDilh->recover_code)
						ExFreePool(pDilh->recover_code);
					if(pDilh->hook_code)
						ExFreePool(pDilh->hook_code);
					if(pDilh->dynamic_code_enter)
						ExFreePool((PVOID)pDilh->dynamic_code_enter);
					pSListEntry = RemoveSListEntry(pPreSLE,(PSLISTENTRY)pDilh);
				}
			}
		}
	}
	return bSuccess;
}

BOOL SetDynamicInLineHook(LPCSTR lpFunctionName,ULONG_PTR functionaddress,SIZE_T hooklenght,ULONG_PTR functionproxy,ULONG type)
{
	if(type==INLINE_HOOK_TYPE_JMP)
	{	
		if(functionaddress)
		{
			ULONG_PTR address = (ULONG_PTR)functionaddress;
			if(!address){return FALSE;}
			DInLineHook Dilh;
			RtlZeroMemory(&Dilh,sizeof(DInLineHook));
			strcpy(Dilh.name,lpFunctionName);
			Dilh.myfunction_address = (ULONG_PTR)functionproxy;
			Dilh.original_address = address;
			Dilh.code_length = hooklenght;
			Dilh.type = type;
			return DoDynamicInLineHook(&Dilh);
		}
		else
		{
			FARPROC funadr =  GetNtKrFunctionByName(lpFunctionName);
			if(funadr)
			{
	/*			FARPROC ofunadr = GetOrignalNtKrFunctionByName(lpFunctionName);
				if(ofunadr && RtlCompareMemory(funadr,ofunadr,hooklenght)!=hooklenght)
				{
					PrintLog(L"Find Inlinehook [%S] \r\n",lpFunctionName);
					return FALSE;
				}*/
				ULONG_PTR address = (ULONG_PTR)funadr;
				if(!address){return FALSE;}
				DInLineHook Dilh;
				RtlZeroMemory(&Dilh,sizeof(DInLineHook));
				strcpy(Dilh.name,lpFunctionName);
				Dilh.myfunction_address = (ULONG_PTR)functionproxy;
				Dilh.original_address = address;
				Dilh.code_length = hooklenght;
				Dilh.type = type;
				return DoDynamicInLineHook(&Dilh);
			}
			PrintLog(L"Unsupport [%S] \r\n",lpFunctionName);
		}
	}
	if(type==INLINE_HOOK_TYPE_CALL)
	{
		FARPROC funadr =  (FARPROC)functionaddress;
		if(funadr)
		{
			FARPROC ofunadr = (FARPROC)NtKrAddressToOrignalNtKrAddress((ULONG_PTR)funadr);
			if(ofunadr && RtlCompareMemory(funadr,ofunadr,hooklenght)!=hooklenght)
			{
				PrintLog(L"Find Inlinehook [%p] \r\n",functionaddress);
				return FALSE;
			}
			ULONG_PTR address = (ULONG_PTR)funadr;
			if(!address){return FALSE;}
			DInLineHook Dilh;
			RtlZeroMemory(&Dilh,sizeof(DInLineHook));
			if(lpFunctionName){
				strcpy(Dilh.name,lpFunctionName);
			}else{
				strcpy(Dilh.name,"Inline_Hook");
			}
			Dilh.myfunction_address = (ULONG_PTR)functionproxy;
			Dilh.original_address = address;
			Dilh.code_length = hooklenght;
			Dilh.type = type;
			return DoDynamicInLineHook(&Dilh);
		}
	}
	return FALSE;
}

ULONG_PTR GetRealFunctionAddress(const CHAR * FunctionName)
{
	PSLISTENTRY pSListEntry = &g_sleDynamicInLineHookHead;
	while(!IsEmptySListEntry(pSListEntry))
	{
		PDInLineHook pDilh = (PDInLineHook)NextSListEntry(pSListEntry);
		if(pDilh)
		{
			if(!strcmp(pDilh->name,FunctionName))
				return pDilh->dynamic_code_enter;
		}
	}
	return NULL;
}

NTSTATUS ObOpenObjectByPointer_Proxy(
	IN PVOID  Object,
	IN ULONG  HandleAttributes,
	IN PACCESS_STATE  PassedAccessState OPTIONAL,
	IN ACCESS_MASK  DesiredAccess,
	IN POBJECT_TYPE  ObjectType,
	IN KPROCESSOR_MODE  AccessMode,
	OUT PHANDLE  Handle
	)
{
	HANDLE ProcessId = PsGetCurrentProcessId();
	if(IsSeparateProcessId(ProcessId) && IsTargetProcess((PEPROCESS)Object))
	{
		KdPrint(("禁止访问\r\n"));
		return STATUS_ACCESS_DENIED;
	}
	if(!IsProtectProcessId(ProcessId) && IsProtectProcess((PEPROCESS)Object))
	{
		return STATUS_ACCESS_DENIED;
	}
	return ObOpenObjectByPointer(Object,HandleAttributes,PassedAccessState,DesiredAccess,ObjectType,AccessMode,Handle);
}

ULONG_PTR GetHookAddress_ObOpenObjectByPointerInNtOpenProcess_XP()
{
	BYTE KeyWords[] = {0x50,0xff,0x75,0xc8,0xff,0x75,0xdc,0xe8};	
	ULONG_PTR adr = 0;
	FARPROC funadr = GetNtKrFunctionByName("NtOpenProcess"); 
	FARPROC ofunadr = GetOrignalNtKrFunctionByName("NtOpenProcess");
	if(funadr && ofunadr)
	{
		adr = FindMemoryAddress((ULONG_PTR)ofunadr + 0x100, (ULONG_PTR)ofunadr + 0x250, KeyWords, 0x8, FALSE);
		if(adr)
		{
			funadr = FARPROC((ULONG_PTR)funadr + (adr + 0x8 - (ULONG_PTR)ofunadr));
			return (ULONG_PTR)funadr;
		}
	}
	return 0;
}

ULONG_PTR GetHookAddress_ObOpenObjectByPointerInNtOpenThread_XP()
{
	BYTE KeyWords[] = {0x50,0xff,0x75,0xcc,0xff,0x75,0xe0,0xe8};	
	ULONG_PTR adr = 0;
	FARPROC funadr = GetNtKrFunctionByName("NtOpenThread"); 
	FARPROC ofunadr = GetOrignalNtKrFunctionByName("NtOpenThread");
	if(funadr && ofunadr)
	{
		adr = FindMemoryAddress((ULONG_PTR)ofunadr + 0x100, (ULONG_PTR)ofunadr + 0x250, KeyWords, 0x8, FALSE);
		if(adr)
		{
			funadr = FARPROC((ULONG_PTR)funadr + (adr + 0x8 - (ULONG_PTR)ofunadr));
			return (ULONG_PTR)funadr;
		}
	}
	return 0;
}

//83e8625c 50              push    eax
//83e8625d ffb50cffffff    push    dword ptr [ebp-0F4h]
//83e86263 ffb510ffffff    push    dword ptr [ebp-0F0h]
//83e86269 e83455fdff      call    nt!ObOpenObjectByPointer (83e5b7a2)


ULONG_PTR GetHookAddress_ObOpenObjectByPointerInNtOpenProcess_WIN7()
{
	BYTE KeyWords[] = {0xc9,0xc2,0x10,0x00};
	BYTE KeyWordsInPs[] = {0x50,0xff,0xb5,0x0c,0xff,0xff,0xff,0xff,0xb5,0x10,0xff,0xff,0xff,0xe8};
	ULONG_PTR adr = 0;
	FARPROC funadr = GetNtKrFunctionByName("NtOpenProcess"); 
	FARPROC ofunadr = GetOrignalNtKrFunctionByName("NtOpenProcess");
	if(funadr && ofunadr)
	{
		adr = FindMemoryAddress((ULONG_PTR)ofunadr, (ULONG_PTR)ofunadr + 0x100, KeyWords, 0x4, FALSE);
		if(adr)
		{
			funadr = FARPROC((ULONG_PTR)funadr + (adr - 0x4 - (ULONG_PTR)ofunadr));
			if(MmIsAddressValid(funadr))
			{
				ULONG_PTR psaddress =  *(PULONG_PTR)funadr;
				psaddress = (ULONG_PTR)funadr + psaddress;
				if(MmIsAddressValid((PVOID)psaddress))
				{
					adr = FindMemoryAddress(psaddress + 0x100, psaddress + 0x250, KeyWordsInPs, 0xE, FALSE);
					if(adr)
					{
						return (adr + 0xE);
					}
				}
			}
		}
	}
	return NULL;
}

//83e6deda 50              push    eax
//83e6dedb ffb510ffffff    push    dword ptr [ebp-0F0h]
//83e6dee1 ffb514ffffff    push    dword ptr [ebp-0ECh]
//83e6dee7 e8b6d8feff      call    nt!ObOpenObjectByPointer (83e5b7a2)

ULONG_PTR GetHookAddress_ObOpenObjectByPointerInNtOpenThread_WIN7()
{
	BYTE KeyWords[] = {0xc9,0xc2,0x10,0x00};
	BYTE KeyWordsInPs[] = {0x50,0xff,0xb5,0x10,0xff,0xff,0xff,0xff,0xb5,0x14,0xff,0xff,0xff,0xe8};
	ULONG_PTR adr = 0;
	FARPROC funadr = GetNtKrFunctionByName("NtOpenThread"); 
	FARPROC ofunadr = GetOrignalNtKrFunctionByName("NtOpenThread");
	if(funadr && ofunadr)
	{
		adr = FindMemoryAddress((ULONG_PTR)ofunadr, (ULONG_PTR)ofunadr + 0x100, KeyWords, 0x4, FALSE);
		if(adr)
		{
			funadr = FARPROC((ULONG_PTR)funadr + (adr - 0x4 - (ULONG_PTR)ofunadr));
			if(MmIsAddressValid(funadr))
			{
				ULONG_PTR psaddress =  *(PULONG_PTR)funadr;
				psaddress = (ULONG_PTR)funadr + psaddress;
				if(MmIsAddressValid((PVOID)psaddress))
				{
					adr = FindMemoryAddress(psaddress + 0x100, psaddress + 0x250, KeyWordsInPs, 0xE, FALSE);
					if(adr)
					{
						return (adr + 0xE);
					}
				}
			}
		}
	}
	return NULL;
}

typedef NTSTATUS (*IOGETDEVICEOBJECTPOINTER)(
							   IN PUNICODE_STRING  ObjectName,
							   IN ACCESS_MASK  DesiredAccess,
							   OUT PFILE_OBJECT  *FileObject,
							   OUT PDEVICE_OBJECT  *DeviceObject
							   );

NTSTATUS IoGetDeviceObjectPointer_InlineProxy(
						 IN PUNICODE_STRING  ObjectName,
						 IN ACCESS_MASK  DesiredAccess,
						 OUT PFILE_OBJECT  *FileObject,
						 OUT PDEVICE_OBJECT  *DeviceObject
						 )
{
	IOGETDEVICEOBJECTPOINTER function = (IOGETDEVICEOBJECTPOINTER)GetRealFunctionAddress("IoGetDeviceObjectPointer");
	if(function)
	{
		if(ObjectName && ObjectName->Length && ObjectName->Buffer)
		{
			KdPrint(("ObjectName[%s]\r\n",ObjectName->Buffer));	
		}
		return function(ObjectName,DesiredAccess,FileObject,DeviceObject);
	}
	return STATUS_UNSUCCESSFUL;
}


NTSTATUS DoAntiProtectHook_1()
{
	SYSTEM_VERSION version = GetSystemVersion();
	//if(version == WINDOWSXP)
	//{
	//	ULONG_PTR address = GetHookAddress_ObOpenObjectByPointerInNtOpenProcess_XP();
	//	if(address)
	//	{
	//		SetDynamicInLineHook(NULL,address,4,(ULONG_PTR)ObOpenObjectByPointer_Proxy,INLINE_HOOK_TYPE_CALL);
	//	}
	//	address = GetHookAddress_ObOpenObjectByPointerInNtOpenThread_XP();
	//	if(address)
	//	{
	//		SetDynamicInLineHook(NULL,address,4,(ULONG_PTR)ObOpenObjectByPointer_Proxy,INLINE_HOOK_TYPE_CALL);
	//	}
	//}
	//else if(version==WINDOWS7)
	//{
	//	ULONG_PTR address = GetHookAddress_ObOpenObjectByPointerInNtOpenProcess_WIN7();
	//	if(address)
	//	{
	//		SetDynamicInLineHook(NULL,address,4,(ULONG_PTR)ObOpenObjectByPointer_Proxy,INLINE_HOOK_TYPE_CALL);
	//	}
	//	address = GetHookAddress_ObOpenObjectByPointerInNtOpenThread_WIN7();
	//	if(address)
	//	{
	//		SetDynamicInLineHook(NULL,address,4,(ULONG_PTR)ObOpenObjectByPointer_Proxy,INLINE_HOOK_TYPE_CALL);
	//	}
	//}
	//SetDynamicInLineHook("IoGetDeviceObjectPointer",(ULONG_PTR)IoGetDeviceObjectPointer,5,(ULONG_PTR)IoGetDeviceObjectPointer_InlineProxy,INLINE_HOOK_TYPE_JMP);
	//SetDynamicInLineHook("PsCreateSystemThread",NULL,5,(ULONG_PTR)PsCreateSystemThread_InlineProxy,INLINE_HOOK_TYPE_JMP);
	if(!DoKeHook())
	{
		KdPrint(("DoKeHook() failed\n"));
		return STATUS_UNSUCCESSFUL;
	}
	if(!DoBsodHook())
	{
		KdPrint(("DoBsodHook() failed\n"));
		return STATUS_UNSUCCESSFUL;
	}
	if(!NT_SUCCESS(DoKeExceptionHook()))
	{
		KdPrint(("DoKeExceptionHook() failed\n"));
		return STATUS_UNSUCCESSFUL;
	}
	//ReplaceSSDTFunction();
	//ReplaceSSDTShadowFunction();
	return STATUS_SUCCESS;
}

NTSTATUS DoAntiProtectHook_2()
{
	SYSTEM_VERSION version = GetSystemVersion();
	switch(version)
	{
		case WINDOWS7:
			{
				RelocateSSDTByIndex(NtProtectVirtualMemory_Index_Win7);
				RelocateSSDTByIndex(NtReadVirtualMemory_Index_Win7);
				RelocateSSDTByIndex(NtWriteVirtualMemory_Index_Win7);
				RelocateSSDTByIndex(NtDebugActiveProcess_Index_Win7);
				RelocateSSDTByIndex(NtOpenProcess_Index_Win7);
				RelocateSSDTByIndex(NtDebugContinue_Index_Win7);
				RelocateSSDTByIndex(NtWaitForDebugEvent_Index_Win7);
				RelocateSSDTByIndex(NtSetInformationDebugObject_Index_Win7);
				RelocateSSDTByIndex(NtSuspendThread_Index_Win7);
				RelocateSSDTByIndex(NtSetContextThread_Index_Win7);
				RelocateSSDTByIndex(NtGetContextThread_Index_Win7);
			}	
			break;
		case WINDOWSXP:
			{
				RelocateSSDTByIndex(NtProtectVirtualMemory_Index_WinXP);
				RelocateSSDTByIndex(NtReadVirtualMemory_Index_WinXP);
				RelocateSSDTByIndex(NtWriteVirtualMemory_Index_WinXP);
				RelocateSSDTByIndex(NtDebugActiveProcess_Index_WinXP);
				RelocateSSDTByIndex(NtOpenProcess_Index_WinXP);
				RelocateSSDTByIndex(NtDebugContinue_Index_WinXP);
				RelocateSSDTByIndex(NtWaitForDebugEvent_Index_WinXP);
				RelocateSSDTByIndex(NtSetInformationDebugObject_Index_WinXP);
				RelocateSSDTByIndex(NtSuspendThread_Index_WinXP);
				RelocateSSDTByIndex(NtSetContextThread_Index_WinXP);
				RelocateSSDTByIndex(NtGetContextThread_Index_WinXP);
			}
			break;
	}
	if(!DoBsodHook())
	{
		KdPrint(("DoBsodHook() failed\n"));
		return STATUS_UNSUCCESSFUL;
	}
	if(!DoKeHook())
	{
		KdPrint(("DoKeHook() failed\n"));
		return STATUS_UNSUCCESSFUL;
	}
	return STATUS_SUCCESS;
}

//817c6e31 83b9c033678100  cmp     dword ptr nt!KeServiceDescriptorTableShadow (816733c0)[ecx],0

//nt!NtOpenProcess+0x209:
//805cc66c 56              push    esi
//805cc66d 8d8548ffffff    lea     eax,[ebp-0B8h]
//805cc673 50              push    eax
//805cc674 ff75c8          push    dword ptr [ebp-38h]
//805cc677 ff75dc          push    dword ptr [ebp-24h]
//805cc67a e83906ffff      call    nt!ObOpenObjectByPointer (805bccb8)

ULONG_PTR FindFakeSSDTOffset(OUT PULONG_PTR adr)
{
	ULONG_PTR offset = 0;
	ULONG_PTR ssdt,ssdtshadow;
	if(GetCurrentSSDT() == NULL || GetCurrentSSDTShadow() == NULL)
	{
		return 0;
	}
	ssdt = (ULONG_PTR)GetCurrentSSDT();
	ssdtshadow = (ULONG_PTR)GetCurrentSSDTShadow();
	SIZE_T area = 0;
	if(ssdt>ssdtshadow)
	{
		area = ssdt - ssdtshadow + sizeof(SSDT);
		*adr = (ULONG_PTR)ExAllocatePool(NonPagedPool,area);
		if(*adr == NULL)
		{
			return 0;
		}
		offset =  *adr - ssdtshadow;
		RtlCopyMemory((void *)*adr,(void *)ssdtshadow,area);
	}
	else
	{
		area = ssdtshadow - ssdt + sizeof(SSDTShadow);
		*adr = (ULONG_PTR)ExAllocatePool(NonPagedPool,area);
		if(*adr == NULL)
		{
			return 0;
		}
		offset = *adr - ssdt;
		RtlCopyMemory((void *)*adr,(void *)ssdt,area);
	}
	KdPrint(("Fake Table %p size %d \r\n",*adr,area));
	PrintLog(L"Fake Table %p size %d \r\n",*adr,area);
	return offset;
}
//核心HOOK,走自己通道用
PKeHookInfo g_pKeHookInfo = NULL;

PKeHookInfo GetKeHookInfo()
{
	return g_pKeHookInfo;
}

BOOL InitKeHookInfo()
{
	if(!g_pKeHookInfo)
	{
		g_pKeHookInfo = (PKeHookInfo)ExAllocatePool(NonPagedPool,sizeof(KeHookInfo));
		if(g_pKeHookInfo == NULL)
		{
			KdPrint(("Allocate KeHookInfo Failed \r\n"));
			PrintLog(L"Allocate KeHookInfo Failed \r\n");
			return FALSE;
		}
	}
	RtlZeroMemory(g_pKeHookInfo, sizeof(KeHookInfo));
	//////////////////////////////////
	//分配假表连续存储空间
	//FAKE SSDT
	ULONG_PTR faketablearea;
	INT_PTR offset = FindFakeSSDTOffset(&faketablearea); //查找并分配SSDT和SSDTSHADOW连续内存
	if(!offset || !faketablearea)
	{
		KdPrint(("Get FakeSSDTOffset Failed \r\n"));
		PrintLog(L"Get FakeSSDTOffset Failed \r\n");
		FreeKeHookInfo();
		return FALSE;
	}
	g_pKeHookInfo->pFakeTableArea = faketablearea;
	g_pKeHookInfo->FakeSSDT = PSSDT((ULONG_PTR)GetCurrentSSDT() + offset);
	RtlCopyMemory(g_pKeHookInfo->FakeSSDT,GetCurrentSSDT(),sizeof(SSDT));
	if(RtlCompareMemory(g_pKeHookInfo->FakeSSDT,GetCurrentSSDT(),sizeof(SSDT))!=sizeof(SSDT))
	{
		KdPrint(("Construct Fake SSDT Table Failed \r\n"));
		PrintLog(L"Construct Fake SSDT Table Failed \r\n");
		FreeKeHookInfo();
		return FALSE;
	}
	PVOID fakessdttable = ExAllocatePool(NonPagedPool,g_pKeHookInfo->FakeSSDT->ulNumberOfServices * sizeof(ULONG_PTR));//分配假表空间
	if(fakessdttable == NULL)
	{
		KdPrint(("Allocate Fake SSDT Table Failed \r\n"));
		PrintLog(L"Allocate Fake SSDT Table Failed \r\n");
		FreeKeHookInfo();
		return FALSE;
	}
	RtlCopyMemory(fakessdttable, GetOriginalSSDT()->pvSSDTBase,GetOriginalSSDT()->ulNumberOfServices * sizeof(ULONG_PTR));//假表复制干净的原表
	g_pKeHookInfo->FakeSSDT->pvSSDTBase = fakessdttable;
	//FAKE SSDT SHADOW
	g_pKeHookInfo->FakeSSDTShadow = PSSDTShadow((ULONG_PTR)GetCurrentSSDTShadow() + offset);
	RtlCopyMemory(g_pKeHookInfo->FakeSSDTShadow,GetCurrentSSDTShadow(),sizeof(SSDTShadow));
	if(RtlCompareMemory(g_pKeHookInfo->FakeSSDTShadow,GetCurrentSSDTShadow(),sizeof(SSDTShadow))!=sizeof(SSDTShadow))
	{
		KdPrint(("Construct Fake SSDT Shadow Table Failed \r\n"));
		PrintLog(L"Construct Fake SSDT Shadow Table Failed \r\n");
		FreeKeHookInfo();
		return FALSE;
	}
	g_pKeHookInfo->FakeSSDTShadow->pvSSDTBase = g_pKeHookInfo->FakeSSDT->pvSSDTBase;
	PVOID fakessdttableshadow = ExAllocatePool(NonPagedPool,g_pKeHookInfo->FakeSSDTShadow->ulNumberOfServicesShadow * sizeof(ULONG_PTR));//分配假表空间
	if(fakessdttableshadow == NULL)
	{
		KdPrint(("Allocate Fake SSDT Shadow Table Failed \r\n"));
		PrintLog(L"Allocate Fake SSDT Shadow Table Failed \r\n");
		FreeKeHookInfo();
		return FALSE;
	}
	RtlCopyMemory(fakessdttableshadow,GetOriginalSSDTShadow()->pvSSDTShadowBase,GetOriginalSSDTShadow()->ulNumberOfServicesShadow * sizeof(ULONG_PTR));
	g_pKeHookInfo->FakeSSDTShadow->pvSSDTShadowBase = fakessdttableshadow;

	//钩挂位置和长度获取
	DWORD hooklength = 0;
	BOOLEAN ThirdParty = FALSE; //是否挂钩第三方模块
	ULONG_PTR hookadr = GetKiFastCallEntryHookAddress(&hooklength, &ThirdParty);//查找关键钩挂位置
	g_pKeHookInfo->IsThirdParty = ThirdParty;
	if(!hookadr || !hooklength)
	{
		KdPrint(("Get KiFastCallEntryHookAddress Failed \r\n"));
		PrintLog(L"Get KiFastCallEntryHookAddress Failed \r\n");
		FreeKeHookInfo();
		return FALSE;
	}

	g_pKeHookInfo->pvHookAddress = (PVOID)hookadr;
	g_pKeHookInfo->szHooklength = hooklength;

	//恢复代码计算构造
	PBYTE recovercode = (PBYTE)ExAllocatePool(NonPagedPool,hooklength);
	if(!recovercode)
	{
		KdPrint(("Allocate Hook Recover Code Failed \r\n"));
		PrintLog(L"Allocate Hook Recover Code Failed \r\n");
		FreeKeHookInfo();
		return FALSE;
	}
	//HOOK的恢复代码
	ULONG_PTR rc_adr;
	if (!ThirdParty)
	{
		rc_adr = ULONG_PTR(GetMemoryModuleNtKr()->codeBase) + (ULONG_PTR(g_pKeHookInfo->pvHookAddress) - GetNtKrBase());
	}
	else
	{
		rc_adr = ULONG_PTR(g_pKeHookInfo->pvHookAddress); //ULONG_PTR(GetMemoryModuleNtKr()->codeBase) + (ULONG_PTR(g_pKeHookInfo->pvHookAddress) - GetNtKrBase());
	}
	
	g_pKeHookInfo->pbRecoverCode = recovercode;
	RtlCopyMemory(g_pKeHookInfo->pbRecoverCode,PVOID(rc_adr),g_pKeHookInfo->szHooklength);
	
	//核心代码构造
	PBYTE kecodebuff = (PBYTE)ExAllocatePool(NonPagedPool,hooklength + 0x31);
	if(!kecodebuff)
	{
		KdPrint(("Allocate Ke Code buffer Failed \r\n"));
		PrintLog(L"Allocate Ke Code buffer Failed \r\n");
		FreeKeHookInfo();
		return FALSE;
	}
	/////////////////////////////////////
	////构建核心代码
	CreateKeCode(offset,kecodebuff, ThirdParty);
	g_pKeHookInfo->pbKeCode = kecodebuff;
	PBYTE hookcodebuff = (PBYTE)ExAllocatePool(NonPagedPool,hooklength);
	if(!hookcodebuff)
	{
		KdPrint(("Allocate Hook Code buffer Failed \r\n"));
		PrintLog(L"Allocate Hook Code buffer Failed \r\n");
		FreeKeHookInfo();
		return FALSE;
	}

	///////////////////////////////////////
	//////构建钩挂代码
	if(!CreateHookCode(hookcodebuff,hooklength))
	{
		KdPrint(("Create Hook Code Failed \r\n"));
		PrintLog(L"Create Hook Code Failed \r\n");
		FreeKeHookInfo();
		return FALSE;
	}

	g_pKeHookInfo->pbHookCode = hookcodebuff;
	//替换假表中的函数
	ReplaceFakeSSDTFunction();
	ReplaceFakeSSDTShadowFunction();
	return TRUE;
}

VOID FreeKeHookInfo()
{
	if(g_pKeHookInfo)
	{
		if(g_pKeHookInfo->FakeSSDT->pvSSDTBase && g_pKeHookInfo->FakeSSDT->pvSSDTBase!=GetCurrentSSDT()->pvSSDTBase)
		{
			ExFreePool(g_pKeHookInfo->FakeSSDT->pvSSDTBase);
		}
		if(g_pKeHookInfo->FakeSSDTShadow->pvSSDTShadowBase && g_pKeHookInfo->FakeSSDTShadow->pvSSDTShadowBase!=GetCurrentSSDTShadow()->pvSSDTShadowBase)
		{
			ExFreePool(g_pKeHookInfo->FakeSSDTShadow->pvSSDTShadowBase);
		}
		if(g_pKeHookInfo->pbHookCode)
		{
			ExFreePool(g_pKeHookInfo->pbHookCode);
		}
		if(g_pKeHookInfo->pbRecoverCode)
		{
			ExFreePool(g_pKeHookInfo->pbRecoverCode);
		}
		if(g_pKeHookInfo->pbKeCode)
		{
			ExFreePool(g_pKeHookInfo->pbKeCode);
		}
		if(g_pKeHookInfo->pFakeTableArea)
		{
			ExFreePool((PVOID)g_pKeHookInfo->pFakeTableArea);
		}
		ExFreePool(g_pKeHookInfo);
		g_pKeHookInfo = NULL;
	}
}

ULONG_PTR GetKiFastCallEntryHookAddress(OUT SIZE_T* length, OUT BOOLEAN* bThirdParty)
{
	ULONG_PTR address = 0;
	ULONG_PTR hookstartadr = 0,hookendadr = 0;
	BYTE KeyWords[7] = { 0x2b,0xe1,				//80889775 2be1            sub     esp,ecx
		0xc1,0xe9,0x2,			//80889777 c1e902          shr     ecx,2
		0x8b,0xfc};				//8088977a 8bfc            mov     edi,esp

	ULONG_PTR BaseAddress = (ULONG_PTR)GetMemoryModuleNtKr()->codeBase;
	ULONG_PTR start;
	ULONG_PTR end;

	__try
	{
		// 获取360安全卫士hookport.sys模块加载基地址
		MODULEINFO buff;
		NTSTATUS status = CRD_KernelModuleTargetGet(&buff, L"Hookport.sys");
		status = STATUS_UNSUCCESSFUL;
		if (NT_SUCCESS(status))
		{
			start = buff.BaseAddress;
			end = start + buff.SizeOfImage;
			BYTE KeyWords360[13] = {0x8b,0x4,0xb0,0xeb,0x3,0x8b,0x45,0xc,0x5e,0x5d,0xc2,0xc,0x0};
			address = FindMemoryAddress(start,end,KeyWords360,13,FALSE);
			if (address)
			{
				hookstartadr = address + 8;
				*length = 5;
				*bThirdParty = TRUE;
				KdPrint(("KiFastCallEntryHookAddress 360: %p \r\n",hookstartadr));
				PrintLog(L"KiFastCallEntryHookAddress 360: %p \r\n",hookstartadr);
			}
		}
		else
		{
			start = BaseAddress;
			end = start + GetMemoryModuleNtKr()->SizeOfImage;
			address = FindMemoryAddress(start,end,KeyWords,7,FALSE);
			if(address)
			{
				BYTE KeyWords2[2] = {0x8b,0xf2};
				address = FindMemoryAddress(address,address - 0x30,KeyWords2,2,FALSE);
				if(address)
				{
					hookendadr = address;
					BYTE KeyWords3[2] = {0x58,0x5a};
					address = FindMemoryAddress(address,address - 0x30,KeyWords3,2,FALSE);
					if(address)
					{
						hookstartadr = address + 2;
						address = hookstartadr;
						*length = hookendadr - hookstartadr;
						hookstartadr = hookstartadr - (ULONG_PTR)BaseAddress + GetNtKrBase(); // 获取内核中真实的位置
						KdPrint(("KiFastCallEntryHookAddress %p \r\n",hookstartadr));
						PrintLog(L"KiFastCallEntryHookAddress %p \r\n",hookstartadr);
						if(MmIsAddressValid((PVOID)hookstartadr) && RtlCompareMemory((PVOID)hookstartadr,(PVOID)address,*length)!=*length)
						{
							KdPrint(("KiFastCallEntryHookAddress has been hacked \r\n"));
							PrintLog(L"KiFastCallEntryHookAddress has been hacked \r\n");
						}
					}
				}
			}
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		KdPrint(("GetKiFastCallEntryHookAddress exception \r\n"));
		PrintLog(L"GetKiFastCallEntryHookAddress exception \r\n");
	}

	return hookstartadr;
}

//850 edfa3a58 57              push    edi
//851 edfa3a59 56              push    esi
//852 edfa3a5a 55              push    ebp
//853 edfa3a5b 53              push    ebx
//854 edfa3a5c 52              push    edx
//855 edfa3a5d 50              push    eax
//856 edfa3a5e 9c              pushfd
//			  50			   push eax
//			57				   push    edi	
//857 edfa3a5f b85049faed      mov     eax,offset CRD!IsProtectTools (edfa4950) | 0xff
//00B11005    35 FF000000     xor eax,0FF
//858 edfa3a64 ffd0            call    eax
//859 edfa3a66 8bc8            mov     ecx,eax
//860 edfa3a68 9d              popfd
//861 edfa3a69 58              pop     eax
//862 edfa3a6a 5a              pop     edx
//863 edfa3a6b 5b              pop     ebx
//864 edfa3a6c 5d              pop     ebp
//865 edfa3a6d 5e              pop     esi
//866 edfa3a6e 5f              pop     edi
//867 edfa3a6f 83f900          cmp     ecx,0
//868 edfa3a72 7406            je      CRD!CreateHookCode+0x2a (edfa3a7a)
//869 edfa3a74 81c789670500    add     edi,56789h
//871 edfa3a7a b9c0fbcc04      mov     ecx,4CCFBC0h
//872 edfa3a7f ffe1            jmp     ecx

/***************************************** 360 构造函数 **************************************/
//1001 b0ab57f0 53              push    ebx
//1005 b0ab57f1 9c              pushfd
//1006 b0ab57f2 ff7510          push    dword ptr [ebp+10h]
//1007 b0ab57f5 ff750c          push    dword ptr [ebp+0Ch]
//1008 b0ab57f8 ff7508          push    dword ptr [ebp+8]
//1009 b0ab57fb bbb057abb0      mov     ebx,offset CRD!FilterKiFastCallEntry360 (b0ab57b0)
//1010 b0ab5800 ffd3            call    ebx
//1011 b0ab5802 9d              popfd
//1012 b0ab5803 5b              pop     ebx
//1013 b0ab5804 5e              pop     esi
//1014 b0ab5805 5d              pop     ebp
//1015 b0ab5806 c20c00          ret     0Ch
VOID CreateKeCode(INT_PTR offset, PBYTE buff, BOOLEAN bThreadParty)
{
	PBYTE address = buff;
	if (!bThreadParty)
	{
		// HOOK 普通钩子
		RtlCopyMemory(address,g_pKeHookInfo->pbRecoverCode,g_pKeHookInfo->szHooklength);
		address+=g_pKeHookInfo->szHooklength;
		BYTE keywords1[10] = {0x57,0x56,0x55,0x53,0x52,0x50,0x9c,0x50,0x57,0xb8};
		RtlCopyMemory(address,keywords1,10);
		address+=10;
		*PULONG_PTR(address)=(ULONG_PTR)IsProtectTools ^ 0xFF;
		address+=sizeof(ULONG_PTR);
		BYTE keywords2[5] = {0x35,0xFF,00,00,00};
		RtlCopyMemory(address,keywords2,5);
		address+=5;
		BYTE keywords3[16]={0xff,0xd0,0x8b,0xc8,0x9d,0x58,0x5a,0x5b,0x5d,0x5e,0x5f,0x83,0xf9,0x00,0x74,0x06};
		RtlCopyMemory(address,keywords3,16);
		address+=16;
		address+=CreateOffsetCode(offset,address);
		*PBYTE(address)=0xb9;
		address++;
		*PULONG_PTR(address)=(ULONG_PTR)g_pKeHookInfo->pvHookAddress+g_pKeHookInfo->szHooklength;
		address+=sizeof(ULONG_PTR);
		*PWORD(address)=0xe1ff;
	} 
	else
	{
		// HOOK 360钩子
		BYTE keywords4[12] = {0x53,0x9c,0xff,0x75,0x10,0xff,0x75,0xc,0xff,0x75,0x8,0xbb};
		RtlCopyMemory(address, keywords4, 12);
		address+=12;
		*PULONG_PTR(address)=(ULONG_PTR)FilterKiFastCallEntry360;
		address+=sizeof(ULONG_PTR);
		BYTE keywords5[9] = {0xff,0xd3,0x9d,0x5b,0x5e,0x5d,0xc2,0xc,0x0};
		RtlCopyMemory(address, keywords5, 9);
	}
}

ULONG FilterKiFastCallEntry360(ULONG FuncIndex, ULONG OrigFuncAddress, ULONG_PTR ServiceTableBase)
{
	if (IsProtectTools(ServiceTableBase, FuncIndex))
	{
		if(ServiceTableBase==(ULONG_PTR)GetCurrentSSDTShadow()->pvSSDTShadowBase)
		{
			 OrigFuncAddress = GetFakeSSDTShadowFunction(FuncIndex);
		}
		else
		{
			OrigFuncAddress = GetFakeSSDTFunction(FuncIndex);
		}
	}
	return OrigFuncAddress;
}

SIZE_T CreateOffsetCode(INT_PTR offset, PBYTE buff)
{
	if(offset > 0)
	{
		if(offset <= 0x7f)
		{
			buff[0] = 0x83;
			buff[1] = 0xc7;
			buff[2] = (UCHAR)offset;
			return 3;
		}
		else
		{
			buff[0] = 0x81;
			buff[1] = 0xc7;
			RtlCopyMemory((buff + 2), &offset, 4);
			return 6;
		}
	}
	else
	{
		if(offset >= -0x7f)
		{
			buff[0] = 0x83;
			buff[1] = 0xc7;
			buff[2] = -(~(offset & 0xff | 0x80) + 1);//符号 int 转 符号 byte
			return 3;
		}
		else
		{
			buff[0] = 0x81;
			buff[1] = 0xc7;
			RtlCopyMemory((buff + 2), &offset, 4);
			return 6;
		}
	}
	return 0;
}

BOOL CreateHookCode(PBYTE buff,SIZE_T lenght)
{
	if(lenght>=5)
	{
		*buff = 0xe9;
		buff++;
		ULONG_PTR jmpoffset = (ULONG_PTR)g_pKeHookInfo->pbKeCode - (ULONG_PTR)g_pKeHookInfo->pvHookAddress - 5;
		*PULONG_PTR(buff) = jmpoffset;
		buff+=sizeof(ULONG_PTR);
		for(SIZE_T i=5;i<lenght;i++)
		{
			*PBYTE(buff) = 0x90;
			buff++;
		}
		return TRUE;
	}
	return FALSE;
}

BOOL DoKeHook()
{
	if(g_pKeHookInfo && g_pKeHookInfo->pvHookAddress && g_pKeHookInfo->pbHookCode && g_pKeHookInfo->szHooklength>0)
	{
		PAGEPROTECTOFF();
		KIRQL Irql;
		KeRaiseIrql(DISPATCH_LEVEL,&Irql);
		RtlCopyMemory(g_pKeHookInfo->pvHookAddress,g_pKeHookInfo->pbHookCode,g_pKeHookInfo->szHooklength);
		KeLowerIrql(Irql);
		PAGEPROTECTON();
		if(RtlCompareMemory(g_pKeHookInfo->pvHookAddress,g_pKeHookInfo->pbHookCode,g_pKeHookInfo->szHooklength)==g_pKeHookInfo->szHooklength)
		{
			return TRUE;
		}
	}
	return FALSE;
}

VOID UnKeHook()
{
	if(g_pKeHookInfo && g_pKeHookInfo->pvHookAddress && g_pKeHookInfo->pbRecoverCode && g_pKeHookInfo->szHooklength>0)
	{
		if(RtlCompareMemory(g_pKeHookInfo->pvHookAddress,g_pKeHookInfo->pbRecoverCode,g_pKeHookInfo->szHooklength)!=g_pKeHookInfo->szHooklength)
		{
			PAGEPROTECTOFF();
			KIRQL Irql;
			KeRaiseIrql(DISPATCH_LEVEL,&Irql);
			RtlCopyMemory(g_pKeHookInfo->pvHookAddress,g_pKeHookInfo->pbRecoverCode,g_pKeHookInfo->szHooklength);//恢复关键HOOK
			KeLowerIrql(Irql);
			PAGEPROTECTON();
		}
	}
}

ULONG_PTR ReplaceSSDTApi(PVOID pvSSDTBase, ULONG index,ULONG_PTR proxyadr)
{
	PAGEPROTECTOFF();
	KIRQL Irql;
	KeRaiseIrql(DISPATCH_LEVEL,&Irql);
	ULONG_PTR oldadr = *PULONG_PTR((ULONG_PTR)pvSSDTBase + index * 4);
	RtlCopyMemory(PVOID((ULONG_PTR)pvSSDTBase + index * 4),&proxyadr,sizeof(ULONG_PTR));
	KeLowerIrql(Irql);
	PAGEPROTECTON();
	return oldadr;
}

ULONG_PTR GetFakeSSDTFunction(ULONG FuncIndex)
{
	return *PULONG_PTR((ULONG_PTR)(g_pKeHookInfo->FakeSSDT->pvSSDTBase) + FuncIndex * 4);
}

ULONG_PTR GetFakeSSDTShadowFunction(ULONG FuncIndex)
{
	return *PULONG_PTR((ULONG_PTR)(g_pKeHookInfo->FakeSSDTShadow->pvSSDTShadowBase) + FuncIndex * 4);
}