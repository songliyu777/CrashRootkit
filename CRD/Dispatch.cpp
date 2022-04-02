#ifdef __cplusplus
extern "C" {
#endif
#include <ntddk.h>
#include <string.h>
#ifdef __cplusplus
}; // extern "C"
#endif

#include "Relocate.h"
#include "Dispatch.h"
#include "KeSysFun.h"
#include "PETools_R0.h"
#include "MyWinNT.h"
#include "CRD_KernelHook.h"
#include "CRD_Process.h"
#include "CRD_Thread.h"
#include "CRD_ProcessModule.h"
#include "CRD_Memory.h"
#include "CRD_KernelModule.h"
#include "CRD_SSDT.h"
#include "CRD_DpcTimer.h"
#include "CRD_IoTimer.h"
#include "CRD_Hook.h"
#include "CRD_HideKd.h"
#include "CRD_HookFunction.h"
#include "CRD_Object.h"
#include "CRD_ProtectTools.h"
#include "CRD_IoControl.h"
#include "CRD_SystemNotify.h"
#include "CRD_KeSpeed.h"
#include "CRD_KeException.h"
#include "CRD_Input.h"
#include "CRD_HideWindows.h"

#define _CRD_
#include "Transfer.h"

NTSTATUS DispatchMessage(PTransferMsg msg)
{
	switch(msg->dwMsgId)
	{
	case FREEALL:
		return FreeAll();
	case TERMINATE_PROCESS:
		return CRD_TerminateProcess(msg);
	case KILL_DPCTIMER:
		return KillDcpTimer(msg);
	case START_IOTIMER:
		return StartIoTimer(msg);
	case STOP_IOTIMER:
		return StopIoTimer(msg);
	case RECOVER_SSDT:
		return RecoverSSDT(msg);
	case RECOVER_SSDTSHADOW:
		return RecoverSSDTShadow(msg);
	case RECOVER_KERNELHOOK:
		return RecoverKernelHook(msg);
	case KILL_THREAD:
		return KillThread(msg);
	case ANTI_PROTECT_1:
		return DoAntiProtectHook_1();
	case ANTI_PROTECT_2:
		return DoAntiProtectHook_2();
	case KDENABLEDEBUGGER:
		return KdEnableDebugger();
	case KDDISABLEDEBUGGER:
		return KdDisableDebugger();
	case HIDEKERNELDEBUGGER:
		return HideKernelDebugger(msg);
	case MONITORDRIVERLOAD:
		return MonitorDriverLoad(msg);
	case ADDTARGETPROCESSID:
		return AddTargetProcessId(msg);
	case RECOVEROBJECT:
		return RecoverDbgkDebugObjectType();
	case ADDPROTECTPROCESS:
		return AddProtectProcess(msg);
	case ADDTARGETKERNELMODULE:
		return AddTargetKernelModuleInfo(msg);
	case ADDFILTERDRVIOINFO:
		return AddFilterDrvIoInfo(msg);
	case HIDEPROCESSMODULE:
		return HideProcessModule(msg);
	case HIDEPROCESSMEMORY:
		return HideProcessMemory(msg);
	case SETKESPEED:
		return SetKeSpeed(msg);
	case SETKEEXCEPTION:
		return SetKeException(msg);
	case ADDSEPARATEPROCESS:
		return AddSeparateProcess(msg);
	case SUSPENDTHREAD:
		return CRD_SuspendThread(msg);
	case RESUMETHREAD:
		return CRD_ResumeThread(msg);
	case SHUTDOWN:
		return CRD_ShutDown(msg);
	case GETPORT:
		return GetPort(msg);
	case SETPORT:
		return SetPort(msg);
	case INPUTSIMULATE:
		return SimulateInput(msg);
	case HIDEKERNELMODULE:
		return HideKernelModule(msg);
	case MODIFYKERNELMEMORY:
		return ModifyKernelMemory(msg);
	default:
		return STATUS_UNSUCCESSFUL;
	}
	return STATUS_SUCCESS;
}

NTSTATUS KernelHookCheck(SIZE_T & size)
{
	size = NtKrCheck();
	return STATUS_SUCCESS;
}

NTSTATUS KernelHookGet(PVOID & buff,SIZE_T size)
{
	NtKrGet(buff,size);
	if(buff!=NULL)
		return STATUS_SUCCESS;
	return STATUS_UNSUCCESSFUL;
}

NTSTATUS ProcessList(SIZE_T & size)
{
	size = ListProcess();
	return STATUS_SUCCESS;
}

NTSTATUS ProcessListGet(PVOID buff,SIZE_T size)
{
	GetProcesses(buff,size);
	return STATUS_SUCCESS;
}

NTSTATUS ModulesList(ULONG_PTR pid,SIZE_T & size)
{
	PEPROCESS eproc;
	NTSTATUS status = PsLookupProcessByProcessId((HANDLE)pid,&eproc);
	if(NT_SUCCESS(status))
	{
		size = ListModuleOfProcess(eproc);
		return STATUS_SUCCESS;
	}
	return STATUS_UNSUCCESSFUL;
}

NTSTATUS ModulesListGet(PVOID buff,SIZE_T size)
{
	GetModules(buff,size);
	return STATUS_SUCCESS;
}

NTSTATUS ThreadsList(ULONG_PTR pid,SIZE_T & size)
{
	PEPROCESS eproc;
	NTSTATUS status = PsLookupProcessByProcessId((HANDLE)pid,&eproc);
	if(NT_SUCCESS(status))
	{
		size = ListThreadOfProcess(eproc);
		return STATUS_SUCCESS;
	}
	return STATUS_UNSUCCESSFUL;
}

NTSTATUS ThreadsListGet(PVOID buff,SIZE_T size)
{
	GetThreads(buff,size);
	return STATUS_SUCCESS;
}

NTSTATUS ModulePeInfoGet(PVOID buff,SIZE_T & size)
{
	size = GetModulePeInfo(buff,size);
	if(size)
		return STATUS_SUCCESS;
	return STATUS_UNSUCCESSFUL;
}

NTSTATUS CR4Get(ULONG & cr4)
{
	ULONG uCR4 = 0;
	PAGEPROTECTOFF();
	// 获得CR4的值 mov eax,cr4 机器码
	__asm
	{
		_emit 0x0F 
		_emit 0x20
		_emit 0xE0
		mov uCR4, eax
	}
	PAGEPROTECTON();
	return STATUS_SUCCESS;
}

NTSTATUS ReadProcessMemory(PVOID buff,SIZE_T insize,SIZE_T & outsize)
{
	PReadMemoryInfo prmi = (PReadMemoryInfo)buff;
	return CRD_ReadProcessMemory((HANDLE)prmi->ProcessHandle,(PVOID)prmi->StartAddress,buff,prmi->nSize,&outsize);
}

NTSTATUS WriteProcessMemory(PVOID buff,SIZE_T insize,SIZE_T & outsize)
{
	PWriteMemoryInfo pwmi = (PWriteMemoryInfo)buff;
	buff = PVOID(pwmi + 1);
	return CRD_WriteProcessMemory((HANDLE)pwmi->ProcessHandle,(PVOID)pwmi->StartAddress,buff,pwmi->nSize,&outsize);
}

NTSTATUS NtOpenProcessEx(PVOID buff,OUT PHANDLE pHandle)
{
	ULONG ProcessId = *(ULONG*)buff;
	return CRD_OpenProcess(ProcessId,pHandle);
}

NTSTATUS KernelModuleList(SIZE_T & size)
{
	size = CRD_KernelModuleList();
	if(size)
		return STATUS_SUCCESS;
	return STATUS_UNSUCCESSFUL;
}

NTSTATUS KernelModuleListGet(PVOID buff,SIZE_T size)
{
	CRD_KernelModuleGet(buff,size);
	return STATUS_SUCCESS;
}

NTSTATUS SSDTList(SIZE_T & size)
{
	PSSDT ssdt = GetCurrentSSDT();
	if(!ssdt)
		return STATUS_UNSUCCESSFUL;
	size = ssdt->ulNumberOfServices;
	return STATUS_SUCCESS;
}

NTSTATUS SSDTGet(PVOID buff,SIZE_T size)
{
	if(!CRD_SSDTGet(buff,size))
		return STATUS_UNSUCCESSFUL;
	return STATUS_SUCCESS;
}

NTSTATUS SSDTShadowList(SIZE_T & size)
{
	PSSDTShadow ssdtshadow = GetCurrentSSDTShadow();
	if(!ssdtshadow)
		return STATUS_UNSUCCESSFUL;
	size = ssdtshadow->ulNumberOfServicesShadow;
	return STATUS_SUCCESS;
}

NTSTATUS SSDTShadowGet(PVOID buff,SIZE_T size)
{
	if(!CRD_SSDTShadowGet(buff,size))
		return STATUS_UNSUCCESSFUL;
	return STATUS_SUCCESS;
}

NTSTATUS DpcTimerList(SIZE_T & size)
{
	size = ListDpcTimer();
	if(!size)
		return STATUS_UNSUCCESSFUL;
	return STATUS_SUCCESS;
}

NTSTATUS DpcTimerGet(PVOID buff,SIZE_T size)
{
	GetDpcTimer(buff,size);
	return STATUS_SUCCESS;
}

NTSTATUS IoTimerList(SIZE_T & size)
{
	size = ListIoTimer();
	if(!size)
		return STATUS_UNSUCCESSFUL;
	return STATUS_SUCCESS;
}

NTSTATUS IoTimerGet(PVOID buff,SIZE_T size)
{
	GetIoTimer(buff,size);
	return STATUS_SUCCESS;
}

PKEVENT g_pEvent = NULL;
WCHAR g_swEventInfo[4096] = {0};

NTSTATUS SetSynchronizaEvent(HANDLE hEvent)
{
	NTSTATUS status;
	if(g_pEvent) 
		ObDereferenceObject(g_pEvent);
	status = ObReferenceObjectByHandle(hEvent, GENERIC_ALL, NULL, KernelMode, (PVOID *)&g_pEvent, NULL);
	return status;
}

NTSTATUS FreeSynchronizaEvent()
{
	if(g_pEvent) 
		ObDereferenceObject(g_pEvent);
	g_pEvent = NULL;
	return STATUS_SUCCESS;
}

NTSTATUS NotifyRing3()
{
	if(g_pEvent)
		return KeSetEvent(g_pEvent,0,FALSE);
	return STATUS_UNSUCCESSFUL;
}

NTSTATUS WaitForRing3Event()
{
	if(g_pEvent)
		return KeWaitForSingleObject((PKEVENT)g_pEvent,Executive,UserMode,0,0);
	return STATUS_UNSUCCESSFUL;
}

VOID SetSynchronizaInfo(PWCHAR pinfo,SIZE_T size)
{
	RtlCopyMemory(g_swEventInfo,pinfo,size);
}

NTSTATUS GetSynchronizaInfo(PVOID buff,SIZE_T size)
{
	RtlCopyMemory(buff,g_swEventInfo,size);
	return STATUS_SUCCESS;
}

NTSTATUS GetPort(PTransferMsg msg)
{
	NTSTATUS status = STATUS_SUCCESS;
	tagPort32Struct * pps = (tagPort32Struct *)msg->buff;
	unsigned short Port = pps->wPortAddr;
	char Val;
	__asm
	{
		mov dx,Port
		in al,dx
		mov Val,al
	}
	pps->dwPortVal = Val;
	return status;
}

NTSTATUS SetPort(PTransferMsg msg)
{
	NTSTATUS status = STATUS_SUCCESS;
	tagPort32Struct * pps = (tagPort32Struct *)msg->buff;
	unsigned short Port = pps->wPortAddr;
	char Val = pps->dwPortVal;
	__asm
	{
		mov dx,Port
		mov al,Val
		out dx,al
	}
	pps->dwPortVal = Val;
	return status;
}

NTSTATUS FreeAll()
{
	TerminalProtectThread();
	FreeKernelHook();
	FreeProcessInfo();
	FreeModuleInfo();
	FreeThreadInfo();
	FreeKernelModuleInfo();
	FreeSSDT();
	FreeDpcTimerInfo();
	FreeIoTimerInfo();
	FreeDynamicInLineHook();
	RecoverSSDTFunction();
	RecoverSSDTShadowFunction();
	FreeTargetKernelModuleInfo();
	UnMonitorDL();
	UnHideKD();
	FreeSynchronizaEvent();
	UnKeHook();
	KeSleep(3000);
	FreeKeHookInfo();
	FreeRelocate();
	FreeProtectProcessInfo();
	FreeTargetProcessInfo();
	FreeSeparateProcessInfo();
	FreeFilterDrvIoInfo();
	FreeHideMemoryInfo();
	ReleaseProcessMonitor();
	return STATUS_SUCCESS;
}