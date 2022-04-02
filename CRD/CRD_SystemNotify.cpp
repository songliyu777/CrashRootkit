#ifdef __cplusplus
extern "C" {
#endif
#include <ntddk.h>
#include <string.h>
#ifdef __cplusplus
}; // extern "C"
#endif

#include "CRD_SystemNotify.h"
#include "CRD_Process.h"
#include "KeSysFun.h"
#include "CRD_ProtectTools.h"

VOID ProcessNotify(
				   IN HANDLE ParentId,
				   IN HANDLE ProcessId,
				   IN BOOLEAN Create
				   )
{
	if(Create)
	{
		PEPROCESS peprocess;
		NTSTATUS status = PsLookupProcessByProcessId(ProcessId,&peprocess);
		if(NT_SUCCESS(status))
		{
			WCHAR ProcessPath[MAX_PATH];
			GetProcessPath((ULONG)peprocess,ProcessPath);
			if (NULL != wcsstr(ProcessPath, L"DNF.exe"))
			{
				AddTargetProcessIdEx(ProcessId);
				//AddProtectProcessIdEx(ProcessId);
			}
		}
	}
	KdPrint(("ParentId[%d] ProcessId[%d] Create[%d]\r\n",ParentId,ProcessId,Create));
}


BOOLEAN InitProcessMonitor()
{
	//NTSTATUS status = PsSetCreateProcessNotifyRoutine(ProcessNotify,FALSE);
	//if(NT_SUCCESS(status))
	//{
	//	return TRUE;
	//}
	//return FALSE;
	return TRUE;
}

VOID ReleaseProcessMonitor()
{
	//PsSetCreateProcessNotifyRoutine(ProcessNotify,TRUE);
}