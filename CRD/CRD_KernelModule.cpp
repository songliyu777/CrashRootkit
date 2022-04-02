#ifdef __cplusplus
extern "C" {
#endif
#include <ntddk.h>
#include <string.h>
#ifdef __cplusplus
}; // extern "C"
#endif

#include "CRD_KernelModule.h"
#include "CRD_ProcessModule.h"
#include "FuncAddrValid.h"
#include "Log.h"
#include "KeSysFun.h"

extern "C" extern PDRIVER_OBJECT pdoGlobalDrvObj;

SLISTENTRY g_sleKernelModuleInfoHead = {0};

VOID EnumKernelModules(PDRIVER_OBJECT DriverObject)
{
	PLDR_DATA_TABLE_ENTRY LdrDataTable,HideLdrDataTable;
	BOOL bRetOK = FALSE;
	__try
	{
		LdrDataTable=(PLDR_DATA_TABLE_ENTRY)DriverObject->DriverSection;
		do 
		{
			if (LdrDataTable->BaseDllName.Length>0 && LdrDataTable->BaseDllName.Buffer!=NULL && MmIsAddressValid(LdrDataTable->BaseDllName.Buffer))
			{
				MODULEINFO mi;
				mi.BaseAddress = (ULONG_PTR)LdrDataTable->DllBase;
				mi.EntryPoint = (ULONG_PTR)LdrDataTable->EntryPoint;
				mi.SizeOfImage = (ULONG_PTR)LdrDataTable->SizeOfImage;
				RtlZeroMemory(mi.FullDllName,MAX_PATH * sizeof(WCHAR));
				if(ValidateUnicodeString(&LdrDataTable->FullDllName))
				{
					if(LdrDataTable->FullDllName.Length >= MAX_PATH * sizeof(WCHAR))
					{
						RtlCopyMemory(mi.FullDllName,LdrDataTable->FullDllName.Buffer,MAX_PATH * sizeof(WCHAR));
						mi.FullDllName[MAX_PATH - 1] = 0;
					}
					else
					{
						RtlCopyMemory(mi.FullDllName,LdrDataTable->FullDllName.Buffer,LdrDataTable->FullDllName.Length);
					}
				}
				if(AddKernelModuleInfo(mi))
				{
					KdPrint(("添加内核ModuleInfo实例成功\r\n"));
				}
				else
				{
					KdPrint(("添加内核ModuleInfo实例失败\r\n"));
				}
			}
			LdrDataTable=(PLDR_DATA_TABLE_ENTRY)LdrDataTable->InLoadOrderLinks.Flink;

		} while ((PLDR_DATA_TABLE_ENTRY)DriverObject->DriverSection!=LdrDataTable && LdrDataTable!=NULL);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		PrintLog(L"This is an exception on EnumKernelModules()...\r\n");
		KdPrint(("This is an exception on EnumKernelModules()...\r\n"));
	}
}

DWORD CRD_KernelModuleList()
{
	FreeKernelModuleInfo();
	EnumKernelModules(pdoGlobalDrvObj);
	return SizeOfSListEntry(&g_sleKernelModuleInfoHead);
}

VOID CRD_KernelModuleGet(PVOID buff,SIZE_T size)
{
	PSLISTENTRY pSListEntry = &g_sleKernelModuleInfoHead;
	PModuleInfo pmi_tr = (PModuleInfo)buff;
	while(!IsEmptySListEntry(pSListEntry))
	{
		PMODULEINFO pmi = (PMODULEINFO)NextSListEntry(pSListEntry);
		if(pmi)
		{
			pmi_tr->BaseAddress = pmi->BaseAddress;
			pmi_tr->EntryPoint = pmi->EntryPoint;
			pmi_tr->SizeOfImage = pmi->SizeOfImage;
			wcscpy(pmi_tr->FullDllName,pmi->FullDllName);
		}
		pmi_tr++;
	}
}

/************************************************************************/
/* 根据驱动模块名称获取加载驱动信息                                     */
/************************************************************************/
NTSTATUS CRD_KernelModuleTargetGet(PMODULEINFO buff, const wchar_t* TargetSys)
{
	CRD_KernelModuleList();
	PSLISTENTRY pSListEntry = &g_sleKernelModuleInfoHead;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	while(!IsEmptySListEntry(pSListEntry))
	{
		PMODULEINFO pmi = (PMODULEINFO)NextSListEntry(pSListEntry);
		if (NULL != wcsstr(pmi->FullDllName, TargetSys))
		{
			buff->BaseAddress = pmi->BaseAddress;
			buff->EntryPoint = pmi->EntryPoint;
			buff->SizeOfImage = pmi->SizeOfImage;
			status = STATUS_SUCCESS;
			break;
		}
	}
	return status;
}

BOOL AddKernelModuleInfo(MODULEINFO mi)
{
	PMODULEINFO new_pmi = (PMODULEINFO)ExAllocatePool(NonPagedPool,sizeof(MODULEINFO)); 
	if(new_pmi)
	{
		RtlZeroMemory(new_pmi,sizeof(MODULEINFO));
		new_pmi->BaseAddress = mi.BaseAddress;
		new_pmi->SizeOfImage = mi.SizeOfImage;
		new_pmi->EntryPoint = mi.EntryPoint;
		RtlCopyMemory(new_pmi->FullDllName,mi.FullDllName,MAX_PATH * sizeof(WCHAR));
		PushSLISTENTRY(&g_sleKernelModuleInfoHead,&new_pmi->next);
		return TRUE;
	}
	return FALSE;
}

VOID FreeKernelModuleInfo()
{
	ReleaseSListEntry(&g_sleKernelModuleInfoHead);
}

SLISTENTRY g_sleTargetKernelModuleInfoHead = {0};

NTSTATUS AddTargetKernelModuleInfo(PTransferMsg msg)
{
	PModuleInfo pmi_tr  = (PModuleInfo)msg->buff;
	PMODULEINFO new_pmi = (PMODULEINFO)ExAllocatePool(NonPagedPool,sizeof(MODULEINFO)); 
	if(new_pmi)
	{
		RtlZeroMemory(new_pmi,sizeof(MODULEINFO));
		new_pmi->BaseAddress = pmi_tr->BaseAddress;
		new_pmi->SizeOfImage = pmi_tr->SizeOfImage;
		new_pmi->EntryPoint = pmi_tr->EntryPoint;
		RtlCopyMemory(new_pmi->FullDllName,pmi_tr->FullDllName,MAX_PATH * sizeof(WCHAR));
		PushSLISTENTRY(&g_sleTargetKernelModuleInfoHead,&new_pmi->next);
		return STATUS_SUCCESS;
	}
	return STATUS_UNSUCCESSFUL;
}

VOID FreeTargetKernelModuleInfo()
{
	ReleaseSListEntry(&g_sleTargetKernelModuleInfoHead);
}

BOOL IsInTargetKernelModule(ULONG_PTR address)
{
	PSLISTENTRY pSListEntry = &g_sleTargetKernelModuleInfoHead;
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

NTSTATUS HideKernelModule(PTransferMsg msg)
{
	PDRIVER_OBJECT DriverObject = pdoGlobalDrvObj;
	PWCHAR pHKName = (PWCHAR)msg->buff; 
	PLDR_DATA_TABLE_ENTRY LdrDataTable,HideLdrDataTable;
	UNICODE_STRING str1,str2;
	BOOL bRetOK = FALSE;
	__try
	{
		LdrDataTable=(PLDR_DATA_TABLE_ENTRY)DriverObject->DriverSection;
		do 
		{
			if (LdrDataTable->BaseDllName.Length>0 && LdrDataTable->BaseDllName.Buffer!=NULL && MmIsAddressValid(LdrDataTable->BaseDllName.Buffer))
			{
				MODULEINFO mi;
				mi.BaseAddress = (ULONG_PTR)LdrDataTable->DllBase;
				mi.EntryPoint = (ULONG_PTR)LdrDataTable->EntryPoint;
				mi.SizeOfImage = (ULONG_PTR)LdrDataTable->SizeOfImage;
				RtlZeroMemory(mi.FullDllName,MAX_PATH * sizeof(WCHAR));
				if(ValidateUnicodeString(&LdrDataTable->FullDllName))
				{
					if(LdrDataTable->FullDllName.Length >= MAX_PATH * sizeof(WCHAR))
					{
						RtlCopyMemory(mi.FullDllName,LdrDataTable->FullDllName.Buffer,MAX_PATH * sizeof(WCHAR));
						mi.FullDllName[MAX_PATH - 1] = 0;
					}
					else
					{
						RtlCopyMemory(mi.FullDllName,LdrDataTable->FullDllName.Buffer,LdrDataTable->FullDllName.Length);
					}
					if(wcslen(mi.FullDllName)>wcslen(pHKName))
					{
						PWCHAR pImageName = mi.FullDllName + wcslen(mi.FullDllName) - wcslen(pHKName);
						RtlInitUnicodeString(&str1,pImageName);
						RtlInitUnicodeString(&str2,pHKName);
						if(!RtlCompareUnicodeString(&str1,&str2,TRUE))
						{
							wcscpy(LdrDataTable->FullDllName.Buffer,L"skbdclassfix.sys");
							//LdrDataTable->InLoadOrderLinks.Flink = LdrDataTable->InLoadOrderLinks.Blink;
							return STATUS_SUCCESS;
						}
					}
				}
			}
			LdrDataTable=(PLDR_DATA_TABLE_ENTRY)LdrDataTable->InLoadOrderLinks.Flink;

		} while ((PLDR_DATA_TABLE_ENTRY)DriverObject->DriverSection!=LdrDataTable && LdrDataTable!=NULL);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		PrintLog(L"This is an exception on HideKernelModule()...\r\n");
		KdPrint(("This is an exception on HideKernelModule()...\r\n"));
	}
	return STATUS_UNSUCCESSFUL;
}