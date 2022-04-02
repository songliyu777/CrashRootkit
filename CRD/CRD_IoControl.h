#pragma once

#include "MyWinNT.h"
#include "MyListEntry.h"
#define _CRD_
#include "Transfer.h"


typedef struct _FILTERDRVINFO
{
	SLISTENTRY next;
	ULONG_PTR ProcessId;
	WCHAR ImageFileName[16];
}FILTERDRVINFO,*PFILTERDRVINFO;

#define  NtDeviceIoControlFile_Index_WinXP 66
#define  NtDeviceIoControlFile_Index_Win7 107

typedef NTSTATUS (*NTDEVICEIOCONTROLFILE)(			
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
	);

NTSTATUS NtDeviceIoControlFile_Proxy(			
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
	);

VOID SetOriginalNtDeviceIoControlFile(NTDEVICEIOCONTROLFILE OriginalNtDeviceIoControlFile);

NTSTATUS AddFilterDrvIoInfo(PTransferMsg msg);

VOID FreeFilterDrvIoInfo();

PFILTERDRVINFO GetFilterDrvInfoByProcessId(HANDLE ProcessId);

NTSTATUS NTAPI NtDeviceIoControlFileJmp(			
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
	);
