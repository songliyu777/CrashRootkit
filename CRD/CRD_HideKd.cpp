#ifdef __cplusplus
extern "C" {
#endif
#include <ntddk.h>
#include <string.h>
#ifdef __cplusplus
}; // extern "C"
#endif

#include "KeSysFun.h"
#include "Dispatch.h"
#include "PETools_R0.h"
#include "CRD_HideKd.h"
#include "CRD_Hook.h"
#include "CRD_HookFunction.h"

VOID MonitorImage(IN PUNICODE_STRING  FullImageName, IN HANDLE  ProcessId, IN PIMAGE_INFO  ImageInfo)
{
	if (!ProcessId)
	{
		NTSTATUS status;
		KdPrint(("Load kernel module:%ws.\r\nImageBase:%08X.\r\n", FullImageName->Buffer,(DWORD)ImageInfo->ImageBase));
		SetSynchronizaInfo(FullImageName->Buffer,FullImageName->Length);
		status = NotifyRing3();
		KdPrint(("NotifyRing3 %X \r\n",status));
		status = WaitForRing3Event();
		KdPrint(("WaitForRing3Event %X \r\n",status));
	}
}

NTSTATUS MonitorDriverLoad(PTransferMsg msg)
{
	if(*PBOOLEAN(msg->buff))
	{
		NTSTATUS status = PsSetLoadImageNotifyRoutine(MonitorImage);
		if (!NT_SUCCESS( status ))
		{
			KdPrint(("MonitorDriverLoad() failed\n"));
			return status;
		}
	}
	else
	{
		return UnMonitorDL();
	}
	
	return STATUS_SUCCESS;
}

NTSTATUS UnMonitorDL()
{
	return PsRemoveLoadImageNotifyRoutine(MonitorImage);
}

NTSTATUS HideKernelDebugger(PTransferMsg msg)
{
	if(*PBOOLEAN(msg->buff))
	{
		NTSTATUS status = PsSetLoadImageNotifyRoutine(ImageCreateMon);
		if (!NT_SUCCESS( status ))
		{
			KdPrint(("HideKernelDebugger() failed\n"));
			return status;
		}
		if(!DoKeHook())
		{
			KdPrint(("DoKeHook() failed\n"));
			return STATUS_UNSUCCESSFUL;
		}
	}
	else
	{
		return UnHideKD();
	}

	return STATUS_SUCCESS;
}

VOID ImageCreateMon (IN PUNICODE_STRING  FullImageName, IN HANDLE  ProcessId, IN PIMAGE_INFO  ImageInfo)
{
	if ( !ProcessId )
	{
		KdPrint(("Load kernel module:%ws.\r\nImageBase:%08X.\r\n", 
			FullImageName->Buffer,(DWORD)ImageInfo->ImageBase));
		unsigned char *codeBase = (unsigned char *)ImageInfo->ImageBase;
		PIMAGE_DATA_DIRECTORY directory;
		directory = GetDataDirectory((HMODULE)ImageInfo->ImageBase,IMAGE_DIRECTORY_ENTRY_IMPORT);
		if(directory && directory->Size)
		{
			PIMAGE_IMPORT_DESCRIPTOR importDesc = (PIMAGE_IMPORT_DESCRIPTOR) (codeBase + directory->VirtualAddress);
			for (;(ULONG_PTR)importDesc < (ULONG_PTR)codeBase + directory->VirtualAddress + directory->Size && MmIsAddressValid(importDesc) && importDesc->Name; importDesc++)
			{
				POINTER_TYPE *thunkRef;
				FARPROC *funcRef;
				HMODULE handle,*modules;
				LPCSTR moduleName = (LPCSTR) (codeBase + importDesc->Name);
				if (importDesc->OriginalFirstThunk) {
					thunkRef = (POINTER_TYPE *) (codeBase + importDesc->OriginalFirstThunk);
					funcRef = (FARPROC *) (codeBase + importDesc->FirstThunk);
				} else {
					// no hint table
					thunkRef = (POINTER_TYPE *) (codeBase + importDesc->FirstThunk);
					funcRef = (FARPROC *) (codeBase + importDesc->FirstThunk);
				}
				for (; *thunkRef; thunkRef++, funcRef++) 
				{
					if (IMAGE_SNAP_BY_ORDINAL(*thunkRef)) {
						PIMAGE_IMPORT_BY_NAME thunkData = (PIMAGE_IMPORT_BY_NAME)*thunkRef;
						//KdPrint(("获的函数名 1 %s \n",(LPCSTR)&thunkData->Name));
						if(!strcmp((LPCSTR)&thunkData->Name,"ZwQuerySystemInformation"))
						{
							PAGEPROTECTOFF();
							*funcRef = (FARPROC)ZwQuerySystemInformation_Proxy;
							PAGEPROTECTON();
						}
						if(!strcmp((LPCSTR)&thunkData->Name,"MmGetSystemRoutineAddress"))
						{
							PAGEPROTECTOFF();
							*funcRef = (FARPROC)MmGetSystemRoutineAddress_Proxy;
							PAGEPROTECTON();
						}
						if(!strcmp((LPCSTR)&thunkData->Name,"IoGetCurrentProcess"))
						{
							PAGEPROTECTOFF();
							*funcRef = (FARPROC)IoGetCurrentProcess_Proxy;
							PAGEPROTECTON();
						}
						if(!strcmp((LPCSTR)&thunkData->Name,"PsGetCurrentProcessId"))
						{
							PAGEPROTECTOFF();
							*funcRef = (FARPROC)PsGetCurrentProcessId_Proxy;
							PAGEPROTECTON();
						}
					} else {
						PIMAGE_IMPORT_BY_NAME thunkData = (PIMAGE_IMPORT_BY_NAME) (codeBase + (*thunkRef));
						//KdPrint(("获的函数名 2 %s \n",(LPCSTR)&thunkData->Name));
						*funcRef;
					}
					if (*funcRef == 0) {
						break;
					} else {
						//KdPrint(("获的函数地址 %p \n",*funcRef));
					}
				}
			}
		}
	}
}


NTSTATUS UnHideKD()
{
	UnKeHook();
	//FreeDynamicInLineHookByFunctionName("MmGetSystemRoutineAddress");
	return PsRemoveLoadImageNotifyRoutine(ImageCreateMon);
}

