#ifdef __cplusplus
extern "C" {
#endif
#include <ntddk.h>
#include <string.h>
#ifdef __cplusplus
}; // extern "C"
#endif

#include "CRD_ProtectTools.h"
#include "CRD_HideWindows.h"
#include "CRD_SSDT.h"
#include "CRD_IoControl.h"
#include "Platform.h"
#include "CRD_HookFunction.h"
#include "KeSysFun.h"
#include "SSDTResource.h"
#include "CRD_Hook.h"
#include "CRD_Thread.h"
#include "Log.h"

SLISTENTRY g_sleTargetProcessInfoHead = {0};

BOOLEAN IsTargetProcessId(HANDLE ProcessId)
{
	PSLISTENTRY pSListEntry = &g_sleTargetProcessInfoHead;
	while(!IsEmptySListEntry(pSListEntry))
	{
		PPROCESSINFO ppi = (PPROCESSINFO)NextSListEntry(pSListEntry);
		if(ppi)
		{
			if(ppi->UniqueProcessId==(ULONG_PTR)ProcessId)
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

BOOLEAN IsTargetProcess(PEPROCESS peprocess)
{
	PSLISTENTRY pSListEntry = &g_sleTargetProcessInfoHead;
	while(!IsEmptySListEntry(pSListEntry))
	{
		PPROCESSINFO ppi = (PPROCESSINFO)NextSListEntry(pSListEntry);
		if(ppi)
		{
			if(ppi->perocess==peprocess)
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

NTSTATUS AddTargetProcessId(PTransferMsg msg)
{
	HANDLE ProcessId = *(PHANDLE)msg->buff;
	if(AddTargetProcessIdEx(ProcessId))
	{
		return STATUS_SUCCESS;
	}
	return STATUS_UNSUCCESSFUL;
}

BOOLEAN AddTargetProcessIdEx(HANDLE ProcessId)
{
	PEPROCESS peprocess;
	NTSTATUS status;
	if(ProcessId)
	{
		status = PsLookupProcessByProcessId(ProcessId,&peprocess);
		if(NT_SUCCESS(status))
		{
			PPROCESSINFO pi = (PPROCESSINFO)ExAllocatePool(NonPagedPool,sizeof(PROCESSINFO));
			if(pi)
			{
				pi->UniqueProcessId = (ULONG_PTR)ProcessId;
				pi->perocess = peprocess;
				PushSLISTENTRY(&g_sleTargetProcessInfoHead,&pi->next);
				return TRUE;
			}
		}
	}
	return FALSE;
}

VOID FreeTargetProcessInfo()
{
	ReleaseSListEntry(&g_sleTargetProcessInfoHead);
}

SLISTENTRY g_sleProtectProcessInfoHead = {0};

BOOLEAN IsProtectTools(ULONG_PTR tableadr, ULONG index)
{
	HANDLE ProcessId = PsGetCurrentProcessId();
	SYSTEM_VERSION version = GetSystemVersion();
	BOOLEAN IsSSDTShadow = FALSE;
	if(GetKeHookInfo()->IsThirdParty)
	{
		IsSSDTShadow = (tableadr==(ULONG_PTR)GetCurrentSSDTShadow()->pvSSDTShadowBase);
	}
	else
	{
		IsSSDTShadow = (tableadr==(ULONG_PTR)&GetCurrentSSDTShadow()->pvSSDTShadowBase);
	}
	switch (version)
	{
	case WINDOWSXP:
		{
			if(IsSSDTShadow)
			{
				if(IsTargetProcessId(ProcessId))
				{
					KdPrint(("SSDT Shadow : %s Thread %d \r\n",SSDTShadow_Name_XP[index],PsGetCurrentThreadId()));
					//PrintLog(L"SSDT Shadow : %s Thread %d \r\n",SSDTShadow_Name_XP[index],PsGetCurrentThreadId());
				}
				//if( index == NtUserQueryWindow_Index_WinXP || 
				//	index == NtUserGetForegroundWindow_Index_WinXP ||
				//	index == NtUserFindWindowEx_Index_WinXP ||
				//	index == NtUserBuildHwndList_Index_WinXP ||
				//	index == NtUserWindowFromPoint_Index_WinXP)/* ||
				//	index == NtUserSetWindowsHookEx_Index_WinXP)*/
				//{
				//	return TRUE;
				//}

			}
			else
			{
				if(IsTargetProcessId(ProcessId))
				{
					KdPrint(("SSDT : %s Thread %d \r\n",SSDT_Name_XP[index],PsGetCurrentThreadId()));
					//PrintLog(L"SSDT : %s Thread %d \r\n",SSDT_Name_XP[index],PsGetCurrentThreadId());
				}
				if( //index == NtQuerySystemInformation_Index_WinXP || 
					//index == NtQueryInformationProcess_Index_WinXP ||
					//index == NtGetContextThread_Index_WinXP ||
					////index == NtSetInformationThread_Index_WinXP ||
					//index == NtProtectVirtualMemory_Index_WinXP ||
					////index == NtTerminateProcess_Index_WinXP ||
					//index == NtDeviceIoControlFile_Index_WinXP ||
					////index == NtQueryInformationThread_Index_WinXP ||
					////index == NtCreateThread_Index_WinXP || 
					//index == NtQueryVirtualMemory_Index_WinXP ||
					//index == NtContinue_Index_WinXP ||
					index == NtRaiseHardError_Index_WinXP 
					//index == NtResumeThread_Index_WinXP ||
					//index == NtCreateSemaphore_Index_WinXP || 
					//index == NtCreateMutant_Index_WinXP ||
					//index == NtCreateFile_Index_WinXP
					)
				{
					return TRUE;
				}
			}
		}
		break;
	case WINDOWS7:
		{
			if(IsSSDTShadow)
			{
				if(IsTargetProcessId(ProcessId))
				{
					//KdPrint(("SSDT Shadow : %s \r\n",SSDTShadow_Name_Win7[index]));
				}
				if( index == NtUserQueryWindow_Index_Win7 || 
					index == NtUserGetForegroundWindow_Index_Win7 ||
					index == NtUserFindWindowEx_Index_Win7 ||
					index == NtUserBuildHwndList_Index_Win7 || 
					index == NtUserWindowFromPoint_Index_Win7)/*||
					index == NtUserSetWindowsHookEx_Index_Win7)*/
				{
					return TRUE;
				}
			}
			else
			{
				if(IsTargetProcessId(ProcessId))
				{
					//KdPrint(("SSDT : %s \r\n",SSDT_Name_Win7[index]));
				}
				if( //index == NtQuerySystemInformation_Index_Win7 || 
					//index == NtQueryInformationProcess_Index_Win7||
					index == NtGetContextThread_Index_Win7 ||
					//index == NtSetInformationThread_Index_Win7 ||
					index == NtProtectVirtualMemory_Index_Win7 ||
					//index == NtTerminateProcess_Index_Win7||
					index == NtDeviceIoControlFile_Index_Win7 ||
					//index == NtQueryInformationThread_Index_Win7 ||
					//index == NtCreateThread_Index_Win7 || 
					index == NtQueryVirtualMemory_Index_Win7 ||
					index == NtContinue_Index_Win7
					//index == NtResumeThread_Index_Win7 ||
					//index == NtCreateFile_Index_Win7
					)
				{
					return TRUE;
				}
			}
		}
		break;
	}
	return IsProtectProcessId(ProcessId);
}

NTSTATUS AddProtectProcess(PTransferMsg msg)
{
	HANDLE ProcessId = *(PHANDLE)msg->buff;
	if(AddProtectProcessIdEx(ProcessId))
	{
		return STATUS_SUCCESS;
	}
	return STATUS_UNSUCCESSFUL;
}

BOOLEAN AddProtectProcessIdEx(HANDLE ProcessId)
{
	PEPROCESS peprocess;
	NTSTATUS status;
	if(ProcessId)
	{
		status = PsLookupProcessByProcessId(ProcessId,&peprocess);
		if(NT_SUCCESS(status))
		{
			PPROCESSINFO pi = (PPROCESSINFO)ExAllocatePool(NonPagedPool,sizeof(PROCESSINFO));
			if(pi)
			{
				pi->UniqueProcessId = (ULONG_PTR)ProcessId;
				pi->perocess = peprocess;
				PushSLISTENTRY(&g_sleProtectProcessInfoHead,&pi->next);
				return TRUE;
			}
		}
	}
	return FALSE;
}

BOOLEAN IsProtectProcessId(HANDLE ProcessId)
{
	PSLISTENTRY pSListEntry = &g_sleProtectProcessInfoHead;
	while(!IsEmptySListEntry(pSListEntry))
	{
		PPROCESSINFO ppi = (PPROCESSINFO)NextSListEntry(pSListEntry);
		if(ppi)
		{
			if(ppi->UniqueProcessId==(ULONG_PTR)ProcessId)
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

BOOLEAN IsProtectProcess(PEPROCESS peprocess)
{
	PSLISTENTRY pSListEntry = &g_sleProtectProcessInfoHead;
	while(!IsEmptySListEntry(pSListEntry))
	{
		PPROCESSINFO ppi = (PPROCESSINFO)NextSListEntry(pSListEntry);
		if(ppi)
		{
			if(ppi->perocess==peprocess)
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

VOID FreeProtectProcessInfo()
{
	ReleaseSListEntry(&g_sleProtectProcessInfoHead);
}

SLISTENTRY g_sleSeparateProcessInfoHead = {0};

NTSTATUS AddSeparateProcess(PTransferMsg msg)
{
	HANDLE ProcessId = *(PHANDLE)msg->buff;
	if(AddSeparateProcessIdEx(ProcessId))
	{
		return STATUS_SUCCESS;
	}
	return STATUS_UNSUCCESSFUL;
}

BOOLEAN AddSeparateProcessIdEx(HANDLE ProcessId)
{
	PEPROCESS peprocess;
	NTSTATUS status;
	if(ProcessId)
	{
		status = PsLookupProcessByProcessId(ProcessId,&peprocess);
		if(NT_SUCCESS(status))
		{
			PPROCESSINFO pi = (PPROCESSINFO)ExAllocatePool(NonPagedPool,sizeof(PROCESSINFO));
			if(pi)
			{
				pi->UniqueProcessId = (ULONG_PTR)ProcessId;
				pi->perocess = peprocess;
				PushSLISTENTRY(&g_sleSeparateProcessInfoHead,&pi->next);
				return TRUE;
			}
		}
	}
	return FALSE;
}

BOOLEAN IsSeparateProcessId(HANDLE ProcessId)
{
	PSLISTENTRY pSListEntry = &g_sleSeparateProcessInfoHead;
	while(!IsEmptySListEntry(pSListEntry))
	{
		PPROCESSINFO ppi = (PPROCESSINFO)NextSListEntry(pSListEntry);
		if(ppi)
		{
			if(ppi->UniqueProcessId==(ULONG_PTR)ProcessId)
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

BOOLEAN IsSeparateProcess(PEPROCESS peprocess)
{
	PSLISTENTRY pSListEntry = &g_sleSeparateProcessInfoHead;
	while(!IsEmptySListEntry(pSListEntry))
	{
		PPROCESSINFO ppi = (PPROCESSINFO)NextSListEntry(pSListEntry);
		if(ppi)
		{
			if(ppi->perocess==peprocess)
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

VOID FreeSeparateProcessInfo()
{
	ReleaseSListEntry(&g_sleSeparateProcessInfoHead);
}

PVOID CreateThreadFunctionHead(ULONG_PTR funadr)
{
	PBYTE kecodebuff = (PBYTE)ExAllocatePool(NonPagedPool,0x6);
	if(kecodebuff)
	{
		PBYTE address = kecodebuff;
		BYTE keywords1 = 0x68;
		RtlCopyMemory(address,&keywords1,1);
		address += 1;
		RtlCopyMemory(address,&funadr,sizeof(ULONG_PTR));
		address += 4;
		BYTE keywords2 = 0xC3;
		RtlCopyMemory(address,&keywords2,1); 
	}
	return kecodebuff;
}

BOOL g_ThreadFlag = FALSE;
PKSTART_ROUTINE g_ThreadHeadBuff = NULL;

VOID ProtectThreadFunc(IN PVOID context)
{
	if(g_ThreadHeadBuff)
	{
		ExFreePool(g_ThreadHeadBuff);
	}
	while(g_ThreadFlag)
	{
		PSLISTENTRY pSListEntry = &g_sleProtectProcessInfoHead;
		while(!IsEmptySListEntry(pSListEntry))
		{
			PPROCESSINFO ppi = (PPROCESSINFO)NextSListEntry(pSListEntry);
			if(ppi)
			{
				ResumeAllThread(ppi->perocess);
			}
		}
		KdPrint(("ProtectThreadFunc\r\n"));
		KeSleep(500);
	}
	// 结束线程
	g_ThreadFlag = TRUE;
	PsTerminateSystemThread(STATUS_SUCCESS);
}

BOOLEAN CreateProtectThread()
{
	HANDLE hThread;
	NTSTATUS status;
	g_ThreadFlag = TRUE;
	g_ThreadHeadBuff = (PKSTART_ROUTINE)CreateThreadFunctionHead((ULONG_PTR)ProtectThreadFunc);
	if(!g_ThreadHeadBuff)
	{
		g_ThreadFlag = FALSE;
		return FALSE;
	}
	// 创建系统线程
	status = PsCreateSystemThread(&hThread, 0, NULL, NULL, NULL, g_ThreadHeadBuff,NULL);
	if (!NT_SUCCESS(status))
	{
		g_ThreadFlag = FALSE;
		return FALSE;
	}
	return TRUE;
}

VOID TerminalProtectThread()
{
	if(g_ThreadFlag)
	{
		g_ThreadFlag = FALSE;
		while(!g_ThreadFlag)
		{
			KeSleep(500);
		}
		g_ThreadFlag = FALSE;
	}
}