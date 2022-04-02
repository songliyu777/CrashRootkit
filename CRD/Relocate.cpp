#ifdef __cplusplus
extern "C" {
#endif
#include <ntddk.h>
#include <string.h>
#ifdef __cplusplus
}; // extern "C"
#endif

#include "KeSysFun.h"
#include "Relocate.h"
#include "CRD_Object.h"
#include "Log.h"
#include "PETools_R0.h"
#include "CRD_SSDT.h"
#include "CRD_Hook.h"
#include "CRD_SystemNotify.h"
#include "CRD_HookFunction.h"
#include "CRD_Memory.h"
#include "Platform.h"

PMEMORYMODULE g_pMM_NtKr = NULL;
PMEMORYMODULE g_pMM_Win32k = NULL;

ULONG_PTR g_r_dwNtKrBase = 0;								//Nt核心基地址
ULONG_PTR g_r_dwWin32kBase = 0;							//Win32k核心基地址

KEATTACHPROCESS g_KeAttachProcess = NULL;
KEDETACHPROCESS g_KeDetachProcess = NULL;
KESTACKATTACHPROCESS g_KeStackAttachProcess = NULL;
KEUNSTACKDETACHPROCESS g_KeUnstackDetachProcess = NULL;
PSSUSPENDTHREAD g_PsSuspendThread = NULL;
PSRESUMETHREAD g_PsResumeThread = NULL;
PSRESUMEPROCESS g_PsResumeProcess = NULL;

BOOLEAN InitRelocate()
{
	if(!InitProcessMonitor())
	{
		PrintLog(L"InitProcessMonitor failed \r\n");
		return FALSE;
	}
	InitKeServiceDescriptorTableShadow();

	ULONG_PTR p = GetInfoTable(SystemModuleInformation);

	if(p==NULL)
		return FALSE;

	PSYSTEM_MODULE_INFORMATION pModule = PSYSTEM_MODULE_INFORMATION(p + sizeof(POINTER_TYPE));

	POINTER_TYPE count = *(POINTER_TYPE*)p;

	InitNtKrRelocate(pModule);
	InitWin32kRelocate(pModule,count);

	ExFreePool((PVOID)p); 

	if(!g_pMM_NtKr || !g_pMM_Win32k)
	{
		PrintLog(L"Relocate failed \r\n");
		return FALSE;
	}

	g_KeAttachProcess = (KEATTACHPROCESS)GetOrignalNtKrFunctionByName("KeAttachProcess");
	g_KeDetachProcess = (KEDETACHPROCESS)GetOrignalNtKrFunctionByName("KeDetachProcess");
	g_KeStackAttachProcess = (KESTACKATTACHPROCESS)GetOrignalNtKrFunctionByName("KeStackAttachProcess");
	g_KeUnstackDetachProcess = (KEUNSTACKDETACHPROCESS)GetOrignalNtKrFunctionByName("KeUnstackDetachProcess");
	g_PsSuspendThread = GetPsSuspendThreadAddress();
	g_PsResumeThread = GetPsResumeThreadAddress();
	g_PsResumeProcess = GetPsResumeProcessAddress();

	if(!InitDbgkDebugObjectType())
	{
		PrintLog(L"InitDbgkDebugObjectType failed \r\n");
		return FALSE;
	}

	if(!InitKeHookInfo())
		return FALSE;

	return TRUE;
}

//处理核心文件
VOID InitNtKrRelocate(PSYSTEM_MODULE_INFORMATION pModule)
{
	PVOID buff = NULL;
	SIZE_T size = 0;
	NTSTATUS status;

	CHAR szModulePath[MAX_PATH] = {0};
	ANSI_STRING aModulePath;
	ANSI_STRING aImageName,aKernelImageName;
	UNICODE_STRING wModulePath;
	strcat(szModulePath,"\\SystemRoot");

	g_r_dwNtKrBase = pModule->Base;

	for(SIZE_T i = 1;i < pModule->PathLength;i++)
	{
		if(pModule->ImageName[i]=='\\')
		{
			strcat(szModulePath, pModule->ImageName + i);
			break;
		}
	}

	RtlInitAnsiString(&aModulePath,szModulePath);
	RtlAnsiStringToUnicodeString(&wModulePath,&aModulePath,TRUE);

	status = ReadLocaleFile(&wModulePath,&buff,&size);
	if(NT_SUCCESS(status))
	{
		if(!g_pMM_NtKr)
		{
			g_pMM_NtKr = MemoryLoadLibrary(buff,pModule->Base);
			if(g_pMM_NtKr)
			{
				InitOriginalSSDT((ULONG_PTR)g_pMM_NtKr->codeBase,g_r_dwNtKrBase);
			}
		}
		ExFreePool(buff);
	}
	RtlFreeUnicodeString(&wModulePath);
}

//处理Win32k.sys
VOID InitWin32kRelocate(PSYSTEM_MODULE_INFORMATION pModule,POINTER_TYPE count)
{
	PVOID buff = NULL;
	SIZE_T size = 0,section_size1 = 0,section_size2 = 0;
	PBYTE section1,section2;
	NTSTATUS status;
	CHAR szModulePath[MAX_PATH] = {0};
	ANSI_STRING aModulePath;
	ANSI_STRING aImageName,aKernelImageName;
	UNICODE_STRING wModulePath;

	RtlInitAnsiString(&aImageName,"win32k.sys");

	for(ULONG i = 0;i < count;i++)
	{
		PCHAR pKernelName = (PCHAR)((POINTER_TYPE)pModule->ImageName + pModule->PathLength);//获取模块名称
		if(pKernelName!=NULL && strlen(pKernelName)>0)
		{
			RtlInitAnsiString(&aKernelImageName,pKernelName);
			if(!RtlCompareString(&aKernelImageName,&aImageName,TRUE))
			{
				g_r_dwWin32kBase = (DWORD)pModule->Base;
				break;
			}
		}
		pModule++;
	}

	if(!g_r_dwWin32kBase)
		return;

	RtlZeroMemory(szModulePath,MAX_PATH);
	strcat(szModulePath,"\\SystemRoot");
	for(SIZE_T i = 1;i < pModule->PathLength;i++)
	{
		if(pModule->ImageName[i]=='\\')
		{
			strcat(szModulePath, pModule->ImageName + i);
			break;
		}
	}

	RtlInitAnsiString(&aModulePath,szModulePath);
	RtlAnsiStringToUnicodeString(&wModulePath,&aModulePath,TRUE);

	status = ReadLocaleFile(&wModulePath,&buff,&size);
	if(NT_SUCCESS(status))
	{
		if(!g_pMM_Win32k)
		{
			g_pMM_Win32k = MemoryLoadLibrary(buff,pModule->Base);
			if(g_pMM_Win32k)
			{
				InitOriginalSSDTShadow((ULONG_PTR)g_pMM_Win32k->codeBase,g_r_dwWin32kBase);
			}
		}
		ExFreePool(buff);
	}

	RtlFreeUnicodeString(&wModulePath);
}

PMEMORYMODULE GetMemoryModuleNtKr()
{
	return g_pMM_NtKr;
}


PMEMORYMODULE GetMemoryModuleWin32k()
{
	return g_pMM_Win32k;
}

ULONG_PTR GetNtKrBase()
{
	return g_r_dwNtKrBase;
}
ULONG_PTR GetWin32kBase()
{
	return g_r_dwWin32kBase;
}

VOID FreeRelocate()
{
	MemoryFreeLibrary(g_pMM_NtKr);
	g_pMM_NtKr = NULL;
	MemoryFreeLibrary(g_pMM_Win32k);
	g_pMM_Win32k = NULL;
}

FARPROC GetNtKrFunctionByName(LPCSTR lpFunctionName)
{
	if(g_r_dwNtKrBase)
	{
		return GetProcAddress((HMODULE)g_r_dwNtKrBase,lpFunctionName);
	}
	return NULL;
}

FARPROC GetOrignalNtKrFunctionByName(LPCSTR lpFunctionName)
{
	if(g_pMM_NtKr)
	{
		return GetProcAddress((HMODULE)g_pMM_NtKr->codeBase,lpFunctionName);
	}
	PrintLog(L"GetOrignalNtKrFunctionByName %S failed \r\n",lpFunctionName);
	return NULL;
}

ULONG_PTR NtKrAddressToOrignalNtKrAddress(ULONG_PTR address)
{
	ULONG_PTR offset = 0;
	if(g_r_dwNtKrBase)
	{
		offset = address - g_r_dwNtKrBase;
		if(g_pMM_NtKr)
		{
			return (ULONG_PTR)g_pMM_NtKr->codeBase + offset;
		}
	}
	return NULL;
}

FARPROC GetRelocateFunction(ULONG_PTR INDEX)
{
	switch(INDEX)
	{
	case KEATTACHPROCESS_INDEX:
		return (FARPROC)g_KeAttachProcess;
	case KEDETACHPROCESS_INDEX:
		return (FARPROC)g_KeDetachProcess;
	case PSSUSPENDTHREAD_INDEX:
		return (FARPROC)g_PsSuspendThread;
	case PSRESUMETHREAD_INDEX:
		return (FARPROC)g_PsResumeThread;
	case KESTACKATTACHPROCESS_INDEX:
		return (FARPROC)g_KeStackAttachProcess;
	case KEUNSTACKDETACHPROCESS_INDEX:
		return (FARPROC)g_KeUnstackDetachProcess;
	case PSRESUMEPROCESS_INDEX:
		return (FARPROC)g_PsResumeProcess;
	}
	return NULL;
}

VOID RelocateSSDTByIndex(ULONG_PTR index)
{
	ULONG_PTR raddress = GetRelocateSSDTByIndex(index);
	PKeHookInfo KeHookInfo = GetKeHookInfo();
	*PULONG_PTR((ULONG_PTR)KeHookInfo->FakeSSDT->pvSSDTBase + index * 4) = raddress;
}

ULONG_PTR GetRelocateSSDTByIndex(ULONG_PTR index)
{
	ULONG_PTR address = *PULONG_PTR((ULONG_PTR)GetOriginalSSDT()->pvSSDTBase + index * 4);
	ULONG_PTR offset = address - g_r_dwNtKrBase;
	ULONG_PTR raddress = (ULONG_PTR)g_pMM_NtKr->codeBase + offset;
	return raddress;
}

VOID RelocateSSDTShadowByIndex(ULONG_PTR index)
{
	ULONG_PTR raddress = GetRelocateSSDTShadowByIndex(index);
	PKeHookInfo KeHookInfo = GetKeHookInfo();
	*PULONG_PTR((ULONG_PTR)KeHookInfo->FakeSSDTShadow->pvSSDTShadowBase + index * 4) = raddress;
}

ULONG_PTR GetRelocateSSDTShadowByIndex(ULONG_PTR index)
{
	ULONG_PTR address = *PULONG_PTR((ULONG_PTR)GetOriginalSSDTShadow()->pvSSDTShadowBase + index * 4);
	ULONG_PTR offset = address - g_r_dwWin32kBase;
	ULONG_PTR raddress = (ULONG_PTR)g_pMM_Win32k->codeBase + offset;
	return raddress;
}

PSSUSPENDTHREAD GetPsSuspendThreadAddress()
{
	BYTE KeyWords[] = {0xff,0x75,0xe4,0xe8};
	SYSTEM_VERSION version = GetSystemVersion();
	ULONG_PTR function = NULL;
	switch(version)
	{
	case WINDOWSXP:
		{
			function = *PULONG_PTR((ULONG_PTR)GetOriginalSSDT()->pvSSDTBase + NtSuspendThread_Index_WinXP * 4);
		}
		break;
	case WINDOWS7:
		{
			function = *PULONG_PTR((ULONG_PTR)GetOriginalSSDT()->pvSSDTBase + NtSuspendThread_Index_Win7 * 4);
		}
		break;
	}
	function = NtKrAddressToOrignalNtKrAddress(function);
	ULONG_PTR address = FindMemoryAddress(function,function+0x100,KeyWords,4,FALSE);
	if(address)
	{
		return (PSSUSPENDTHREAD)(address + 8 + *PULONG_PTR(address + 4));
	}
	return NULL;
}

PSRESUMETHREAD GetPsResumeThreadAddress()
{
	SYSTEM_VERSION version = GetSystemVersion();
	switch(version)
	{
	case WINDOWSXP:
		{
			BYTE KeyWords[] = {0xff,0x75,0xe4,0xe8};	
			ULONG_PTR function = *PULONG_PTR((ULONG_PTR)GetOriginalSSDT()->pvSSDTBase + NtResumeThread_Index_WinXP * 4);
			function = NtKrAddressToOrignalNtKrAddress(function);
			ULONG_PTR address = FindMemoryAddress(function,function+0x100,KeyWords,4,FALSE);
			if(address)
			{
				return (PSRESUMETHREAD)(address + 8 + *PULONG_PTR(address + 4));
			}
		}
		break;
	case WINDOWS7:
		{
			BYTE KeyWords[] = {0x8b,0x45,0xe4,0xe8};	
			ULONG_PTR function = *PULONG_PTR((ULONG_PTR)GetOriginalSSDT()->pvSSDTBase + NtResumeThread_Index_Win7 * 4);
			function = NtKrAddressToOrignalNtKrAddress(function);
			ULONG_PTR address = FindMemoryAddress(function,function+0x100,KeyWords,4,FALSE);
			if(address)
			{
				return (PSRESUMETHREAD)(address + 8 + *PULONG_PTR(address + 4));
			}
		}
		break;
	}
	return NULL;
}

PSRESUMEPROCESS GetPsResumeProcessAddress()
{
	SYSTEM_VERSION version = GetSystemVersion();
	switch(version)
	{
	case WINDOWSXP:
		{
			BYTE KeyWords[] = {0x7c,0x12,0xff,0x75,0x08,0xe8};	
			ULONG_PTR function = *PULONG_PTR((ULONG_PTR)GetOriginalSSDT()->pvSSDTBase + NtResumeProcess_Index_WinXP * 4);
			function = NtKrAddressToOrignalNtKrAddress(function);
			ULONG_PTR address = FindMemoryAddress(function,function+0x100,KeyWords,6,FALSE);
			if(address)
			{
				return (PSRESUMEPROCESS)(address + 10 + *PULONG_PTR(address + 6));
			}
		}
		break;
	case WINDOWS7:
		{
			BYTE KeyWords[] = {0x7c,0x12,0xff,0x75,0xfc,0xe8};
			ULONG_PTR function = *PULONG_PTR((ULONG_PTR)GetOriginalSSDT()->pvSSDTBase + NtResumeProcess_Index_Win7 * 4);
			function = NtKrAddressToOrignalNtKrAddress(function);
			ULONG_PTR address = FindMemoryAddress(function,function+0x100,KeyWords,6,FALSE);
			if(address)
			{
				return (PSRESUMEPROCESS)(address + 10 + *PULONG_PTR(address + 6));
			}
		}
		break;
	}
	return NULL;
}