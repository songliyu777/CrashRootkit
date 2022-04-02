#ifdef __cplusplus
extern "C" {
#endif
#include <ntddk.h>
#include <string.h>
#ifdef __cplusplus
}; // extern "C"
#endif

#include "CRD_IoControl.h"
#include "CRD_ProtectTools.h"
#include "KeSysFun.h"

SLISTENTRY g_sleFilterDrvHead = {0};

NTDEVICEIOCONTROLFILE Original_NtDeviceIoControlFile = NULL;

VOID SetOriginalNtDeviceIoControlFile(NTDEVICEIOCONTROLFILE OriginalNtDeviceIoControlFile)
{
	Original_NtDeviceIoControlFile = OriginalNtDeviceIoControlFile;
}

NTSTATUS NTAPI NtDeviceIoControlFile_Proxy(			
	IN HANDLE FileHandle,
	IN HANDLE Event OPTIONAL,
	IN PIO_APC_ROUTINE ApcRoutine /*OPTIONAL*/,
	IN PVOID ApcContext /*OPTIONAL*/,
	OUT PIO_STATUS_BLOCK IoStatusBlock,
	IN ULONG IoControlCode,
	IN PVOID InputBuffer /*OPTIONAL*/,
	IN ULONG InputBufferLength,
	OUT PVOID OutputBuffer /*OPTIONAL*/,
	IN ULONG OutputBufferLength
	)
{
	NTSTATUS status = STATUS_DOWNGRADE_DETECTED;
	WCHAR FileName[MAX_PATH];
	if (Original_NtDeviceIoControlFile)
	{
		HANDLE ProcessId = PsGetCurrentProcessId();
		PFILTERDRVINFO pfi = GetFilterDrvInfoByProcessId(ProcessId);
		if(pfi)
		{
			PUNICODE_STRING pDriverName;
			if (GetDeviceName(FileHandle,&pDriverName))
			{
				int len1 = wcslen(pDriverName->Buffer);
				int len2 = wcslen(pfi->ImageFileName);
				if(len1>=len2)
				{
					if(!wcscmp(pDriverName->Buffer + len1 - len2, pfi->ImageFileName))
					{
						KdPrint(("IoControlCode %X Thread %d\r\n",IoControlCode,PsGetCurrentThreadId()));
						//return STATUS_UNSUCCESSFUL;
					}
				}
			}
		}
		if(IsProtectProcessId(ProcessId))
		{
			status = NtDeviceIoControlFileJmp(FileHandle,
											  Event,
											  ApcRoutine,
											  ApcContext,
											  IoStatusBlock,
											  IoControlCode,
											  InputBuffer,
											  InputBufferLength,
											  OutputBuffer,
											  OutputBufferLength);
		}
		else
		{
			status = Original_NtDeviceIoControlFile(FileHandle,
													Event,
													ApcRoutine,
													ApcContext,
													IoStatusBlock,
													IoControlCode,
													InputBuffer,
													InputBufferLength,
													OutputBuffer,
													OutputBufferLength);
		}
	
	}
	return status;
}

NTSTATUS __declspec(naked) NtDeviceIoControlFileJmp(			
										IN HANDLE FileHandle,
										IN HANDLE Event OPTIONAL,
										IN PIO_APC_ROUTINE ApcRoutine /*OPTIONAL*/,
										IN PVOID ApcContext /*OPTIONAL*/,
										OUT PIO_STATUS_BLOCK IoStatusBlock,
										IN ULONG IoControlCode,
										IN PVOID InputBuffer /*OPTIONAL*/,
										IN ULONG InputBufferLength,
										OUT PVOID OutputBuffer /*OPTIONAL*/,
										IN ULONG OutputBufferLength
										)
{
	_asm
	{
		mov   edi,edi
		push  ebp
		mov   ebp,esp
		push  1
		mov eax,Original_NtDeviceIoControlFile
		add eax,7
		jmp eax
	}
}

VOID FreeFilterDrvIoInfo()
{
	ReleaseSListEntry(&g_sleFilterDrvHead);
}

NTSTATUS AddFilterDrvIoInfo(PTransferMsg msg)
{
	PFilterDrvInfo pFilerDrvInfo  = (PFilterDrvInfo)msg->buff;
	PFILTERDRVINFO pfi = (PFILTERDRVINFO)ExAllocatePool(NonPagedPool,sizeof(FILTERDRVINFO));
	if(pfi)
	{
		pfi->ProcessId = pFilerDrvInfo->ProcessId;
		wcscpy(pfi->ImageFileName,pFilerDrvInfo->ImageFileName);
		PushSLISTENTRY(&g_sleFilterDrvHead,&pfi->next);
		return STATUS_SUCCESS;
	}
	return STATUS_UNSUCCESSFUL;
}

PFILTERDRVINFO GetFilterDrvInfoByProcessId(HANDLE ProcessId)
{
	PSLISTENTRY pSListEntry = &g_sleFilterDrvHead;
	while(!IsEmptySListEntry(pSListEntry))
	{
		PFILTERDRVINFO pfi = (PFILTERDRVINFO)NextSListEntry(pSListEntry);
		if(pfi)
		{
			if((ULONG_PTR)ProcessId==pfi->ProcessId)
			{
				return pfi;
			}
		}
	}
	return NULL;
}