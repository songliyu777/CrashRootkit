#ifdef __cplusplus
extern "C" {
#endif
#include <ntddk.h>
#include <string.h>
#ifdef __cplusplus
}; // extern "C"
#endif

#include "Log.h"

#pragma warning(disable:4995)
#pragma warning(disable:4996)

WCHAR g_wlogPath[512] = L"\\??\\C:\\CRDLog.txt";			//���ڴ洢Logλ��

NTSTATUS SetLogPath(PWCHAR path)
{
	if(path)
	{
		wcscpy(g_wlogPath, path);
		return STATUS_SUCCESS;
	}
	return STATUS_UNSUCCESSFUL;
}

PWCHAR GetLogPath()
{
	return g_wlogPath;
}

VOID SaveToLog(IN PWCHAR logstr)
{
	NTSTATUS status;
	HANDLE hFile ,hSection;
	SIZE_T size = 0;
	PVOID BaseAddress = NULL;
	UNICODE_STRING LogPath;
	RtlInitUnicodeString(&LogPath,g_wlogPath);
	IO_STATUS_BLOCK iostatus;
	OBJECT_ATTRIBUTES objattr;
	InitializeObjectAttributes(&objattr, &LogPath, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
	status =  ZwCreateFile(	&hFile,
		GENERIC_WRITE | GENERIC_READ,
		&objattr,
		&iostatus,
		0,
		FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
		FILE_OPEN_IF,
		FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
		NULL,
		0);

	if (NT_SUCCESS(status))
	{
		FILE_STANDARD_INFORMATION fsi;
		//��ѯ�ļ�������Ϣ
		status = ZwQueryInformationFile(hFile, &iostatus, &fsi, sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation);
		if (NT_SUCCESS(status))
		{
			status  = ZwWriteFile(hFile, NULL, NULL, NULL, &iostatus, logstr, wcslen(logstr)*sizeof(WCHAR), &fsi.EndOfFile, NULL);
			if (!NT_SUCCESS(status))
			{
				KdPrint(("д��LOGʧ�ܣ�\r\n"));
			}
		}
		ZwClose(hFile);
	}
	else
	{
		KdPrint(("��LOGʧ�ܣ�\r\n"));
	}
}

VOID PrintLog( __in __format_string NTSTRSAFE_PCWSTR pszFormat,...)
{
	//����¼LOG
	if(KeGetCurrentIrql() > PASSIVE_LEVEL)
		return;
	PWCHAR strData = NULL;	
	strData = (PWCHAR)ExAllocatePool(NonPagedPool,LogDataSize);
	if(strData) 
	{	
		RtlZeroMemory(strData, LogDataSize);
		NTSTATUS status;
		size_t cchDest = LogDataSize / sizeof(wchar_t);

		status = RtlStringValidateDestW(strData, cchDest, NTSTRSAFE_MAX_CCH);

		if (NT_SUCCESS(status))
		{
			va_list argList;

			va_start(argList, pszFormat);

			status = RtlStringVPrintfWorkerW(strData,
				cchDest,
				NULL,
				pszFormat,
				argList);

			va_end(argList);
			PWCHAR timestr = CurTimeStr();
			PWCHAR tmpstr = NULL;
			tmpstr = (PWCHAR)ExAllocatePool(NonPagedPool,LogDataSize);
			if(tmpstr)
			{
				RtlStringCchPrintfW(tmpstr,LogDataSize,L"[%ws]%ws",timestr,strData);
				SaveToLog(tmpstr);
				ExFreePool(tmpstr);
			}
		}else
		{
			KdPrint(("Log ��ʽ��ʧ��\r\n"));
		}

		ExFreePool(strData);
	}
	else
	{
		KdPrint(("Log �ڴ����ʧ��\r\n"));
	}
}

PWCHAR CurTimeStr()
{ 
	LARGE_INTEGER snow,now; 
	TIME_FIELDS now_fields; 
	static WCHAR time_str[32] = { 0 }; 
	// ��ñ�׼ʱ�� 
	KeQuerySystemTime(&snow); 
	// ת��Ϊ����ʱ�� 
	ExSystemTimeToLocalTime(&snow,&now); 
	// ת��Ϊ�����������ʱ��Ҫ�� 
	RtlTimeToTimeFields(&now,&now_fields); 
	// ��ӡ���ַ����� 
	RtlStringCchPrintfW( 
		time_str, 
		32*2, 
		L"%4d-%02d-%02d %02d:%02d:%02d", 
		now_fields.Year,now_fields.Month,now_fields.Day, 
		now_fields.Hour,now_fields.Minute,now_fields.Second); 
	return time_str; 
}