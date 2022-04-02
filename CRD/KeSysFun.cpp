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
#include "FuncAddrValid.h"
#include "Log.h"

POINTER_TYPE GetInfoTable(SYSTEM_INFORMATION_CLASS SystemInformationClass)
{
	ULONG mSize = 0x4000;
	PVOID mPtr = NULL;
	NTSTATUS St;
	do
	{
		mPtr = ExAllocatePool(NonPagedPool, mSize);
		memset(mPtr, 0, mSize);
		if (mPtr)
		{
			St = ZwQuerySystemInformation(SystemInformationClass, mPtr, mSize, NULL);
		} else return NULL;
		if (St == STATUS_INFO_LENGTH_MISMATCH)
		{
			ExFreePool(mPtr);
			mSize = mSize * 2;
		}
	} while (St == STATUS_INFO_LENGTH_MISMATCH);
	if (St == STATUS_SUCCESS) return (POINTER_TYPE)mPtr;
	ExFreePool(mPtr);
	return NULL;
}

NTSTATUS ReadLocaleFile(PUNICODE_STRING pFilePath,OUT PVOID * buff,OUT SIZE_T * size)
{
	NTSTATUS status;
	HANDLE hFile ,hSection;
	PVOID BaseAddress = NULL;
	IO_STATUS_BLOCK iostatus;
	OBJECT_ATTRIBUTES objattr;
	FILE_STANDARD_INFORMATION fsi;

	InitializeObjectAttributes(&objattr, pFilePath, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
	status = ZwOpenFile(&hFile, FILE_EXECUTE | SYNCHRONIZE, &objattr, &iostatus, FILE_SHARE_READ, FILE_NON_DIRECTORY_FILE|FILE_RANDOM_ACCESS|FILE_SYNCHRONOUS_IO_NONALERT);

	if(!NT_SUCCESS(status))
	{
		KdPrint(("打开内核文件%s失败 \r\n",pFilePath->Buffer));
		PrintLog(L"Open Kernel File %S Failed \r\n",pFilePath->Buffer);
		return STATUS_UNSUCCESSFUL;
	}

	
	//查询文件基本信息
	status = ZwQueryInformationFile(hFile, &iostatus, &fsi, sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation);

	if (NT_SUCCESS(status))
	{
		KdPrint(("查询文件属性成功！\n"));
		KdPrint(("AllocationSize %d\n",fsi.AllocationSize));
		KdPrint(("EndOfFile %d\n",fsi.EndOfFile));
		*size = (SIZE_T)fsi.EndOfFile.QuadPart;
		*buff = (PVOID)ExAllocatePool(NonPagedPool,*size);
		if(*buff)
		{
			status = ZwReadFile(hFile, NULL, NULL, NULL, &iostatus, *buff, *size, NULL, NULL);
			if(NT_SUCCESS(status))
			{
				KdPrint(("读取长度 %d bytes \r\n", *size));
				ZwClose(hFile);
				return STATUS_SUCCESS;
			}
		}
	}
	if(*buff)
	{
		ExFreePool(*buff);
		*buff = NULL;
		*size = 0;
	}
	ZwClose(hFile);
	return STATUS_UNSUCCESSFUL;
}

VOID PAGEPROTECTOFF()
{
	__asm					
	{						
		cli					
			mov eax, cr0		
			and eax, ~0x10000	
			mov cr0, eax		
	}
}

VOID PAGEPROTECTON()
{
	__asm					
	{						
		mov eax, cr0		
			or eax, 0x10000		
			mov cr0, eax		
			sti					
	}
}

VOID KeSleep(LONG msec)
{
	LARGE_INTEGER my_interval;
	my_interval.QuadPart = DELAY_ONE_MILLISECOND;
	my_interval.QuadPart *= msec;
	KeDelayExecutionThread(KernelMode,0,&my_interval);
}

//重启电脑
VOID RestartSystem()
{
	//KeBugCheck(POWER_FAILURE_SIMULATE);
	__asm
	{
		mov al,0xFE
		out 64h,al
	}
}

ULONG SDGetTickCount()
{
	LARGE_INTEGER tick_count; 
	ULONG myinc = KeQueryTimeIncrement(); 
	KeQueryTickCount(&tick_count); 
	tick_count.QuadPart *= myinc; 
	tick_count.QuadPart /=  10000;
	return tick_count.LowPart;
}

ULONG_PTR FindMemoryAddress(ULONG_PTR start, ULONG_PTR end, PBYTE keywords, SIZE_T length ,BOOL IsValid)
{
	ULONG_PTR i;
	PCHAR ptr;
	ULONG_PTR dwordatbyte;
	ptr = (PCHAR) start;
	if(start<end)
	{
		for(i = start; i <= end - length; i++, ptr++)
		{
			__try
			{
				dwordatbyte = (ULONG_PTR)ptr;
			}
			__except(EXCEPTION_EXECUTE_HANDLER)
			{
				return 0;
			}

			if(IsValid)
			{
				if(MmIsAddressValid((PVOID)dwordatbyte))
				{
					if(RtlCompareMemory((PVOID)dwordatbyte, (PVOID)keywords, length) == length)
					{
						return dwordatbyte;
						break;
					}
				}
			}
			else
			{
				if(RtlCompareMemory((PVOID)dwordatbyte, (PVOID)keywords, length) == length)
				{
					return dwordatbyte;
					break;
				}
			}
		}
	}
	else
	{
		for(i = start - length; i >= end ; i--, ptr--)
		{
			__try
			{
				dwordatbyte = (ULONG_PTR)ptr;
			}
			__except(EXCEPTION_EXECUTE_HANDLER)
			{
				return 0;
			}

			if(MmIsAddressValid((PVOID)dwordatbyte))
			{
				if(RtlCompareMemory((PVOID)dwordatbyte, (PVOID)keywords, length) == length)
				{
					return dwordatbyte;
					break;
				}
			}
		}
	}
	return (ULONG_PTR)0;
}

OBGETOBJECTTYPE gfun_ObGetObjectType = NULL;

POBJECT_TYPE  CRD_ObGetObjectType(IN PVOID Object)
{
	if(!gfun_ObGetObjectType)
	{
		gfun_ObGetObjectType = (OBGETOBJECTTYPE)GetNtKrFunctionByName("ObGetObjectType");
	}
	if(!gfun_ObGetObjectType)
		return NULL;
	return gfun_ObGetObjectType(Object);
}

ULONG KernelMessageBox(IN PWSTR pwText, IN PWSTR pwCaption, IN int uType) 
{ 
	NTSTATUS Status; 
	UNICODE_STRING Mes,Title; 
	ULONG Response = 0; 
	RtlInitUnicodeString (&Mes, pwText); 
	RtlInitUnicodeString (&Title,pwCaption); 
	ULONG_PTR Param[3];
	Param[0]=(ULONG_PTR)&Mes;
	Param[1]=(ULONG_PTR)&Title;
	Param[2]=uType;
	Status = ExRaiseHardError ( 
		STATUS_SERVICE_NOTIFICATION , 
		3, 
		3, 
		&Param, 
		NULL, 
		&Response 
		); 
	return Response; 
}

BOOL GetDeviceName(IN HANDLE hHandleFile,OUT PUNICODE_STRING *lpOutUniString)
{
	BOOL bResult = FALSE;
	PFILE_OBJECT pFileObject = NULL;
	PVOID pObject = NULL;
	PLDR_DATA_TABLE_ENTRY pModuleEntry;
	// get device object by handle
	NTSTATUS status = ObReferenceObjectByHandle(hHandleFile,
												0,
												*IoFileObjectType, 
												KernelMode,
												(PVOID *)&pFileObject, 
												NULL);

	if (NT_SUCCESS(status))
	{
		// validate pointer to device object
		if (MmIsAddressValid(pFileObject->DeviceObject))
		{
			pObject = pFileObject->DeviceObject;
		}
		else
		{
			goto end1;
		}
		// validate pointer to driver object
		if (!MmIsAddressValid(pFileObject->DeviceObject->DriverObject))
		{
			goto end1;
		}
		// get loader information entry for the driver module
		pModuleEntry = (PLDR_DATA_TABLE_ENTRY)pFileObject->DeviceObject->DriverObject->DriverSection;
		if (pModuleEntry == NULL)
		{
			goto end1;
		}
		// validate pointer to loader's table and data from it
		if (!MmIsAddressValid(pModuleEntry) ||!ValidateUnicodeString(&pModuleEntry->FullDllName))
		{
			goto end1;
		}
		*lpOutUniString = &pModuleEntry->FullDllName;
		bResult=TRUE;
	}
	else
	{
		goto end2;
	}
end1:
	ObDereferenceObject(pFileObject);
end2:
	return bResult;
}

ULONG_PTR GetKernelModuleAddressByName(PCHAR ModuleName)
{
	CHAR szModulePath[MAX_PATH] = {0};
	ANSI_STRING cImageName,cKernelImageName;
	UNICODE_STRING wModulePath;
	ULONG_PTR BaseAddress = NULL;

	ULONG_PTR p = GetInfoTable(SystemModuleInformation);
	if(p==NULL)
		return STATUS_UNSUCCESSFUL;

	POINTER_TYPE count = *(POINTER_TYPE*)p;
	RtlInitAnsiString(&cImageName,ModuleName);
	PSYSTEM_MODULE_INFORMATION pModule = PSYSTEM_MODULE_INFORMATION(p + sizeof(POINTER_TYPE));
	for(ULONG i = 0;i < count;i++)
	{
		PCHAR pKernelName = (PCHAR)((POINTER_TYPE)pModule->ImageName + pModule->PathLength);//获取模块名称
		if(pKernelName!=NULL && strlen(pKernelName)>0)
		{
			RtlInitAnsiString(&cKernelImageName,pKernelName);
			if(!RtlCompareString(&cKernelImageName,&cImageName,TRUE))
			{
				BaseAddress = (DWORD)pModule->Base;
				break;
			}
		}
		pModule++;
	}
	RtlFreeUnicodeString(&wModulePath);
	return BaseAddress;
}