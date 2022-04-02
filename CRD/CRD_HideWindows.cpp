#ifdef __cplusplus
extern "C" {
#endif
#include <ntddk.h>
#include <string.h>
#ifdef __cplusplus
}; // extern "C"
#endif

#include "CRD_HideWindows.h"
#include "CRD_ProtectTools.h"
#include "CRD_SSDT.h"
#include "CRD_Hook.h"
#include "Platform.h"
#include "Relocate.h"
#include "CRD_ReplaceFunction.h"

NTUSERQUERYWINDOW  Original_NtUserQueryWindow = NULL;

ULONG NtUserQueryWindow_Proxy(IN HWND WindowHandle,IN DWORD TypeInformation)
{
	ULONG WindowHandleProcessID = 0;
	if(Original_NtUserQueryWindow)
	{
		WindowHandleProcessID = Original_NtUserQueryWindow(WindowHandle,TypeInformation);
	}
	if (WindowHandleProcessID)
	{
		if(IsProtectProcessId((HANDLE)WindowHandleProcessID) && PsGetCurrentProcessId()!=(HANDLE)WindowHandleProcessID)
			return 0;
	}
	return WindowHandleProcessID;
}

NTUSERGETFOREGROUNDWINDOW  Original_NtUserGetForegroundWindow = NULL;

ULONG NtUserGetForegroundWindow_Proxy(VOID)
{
	ULONG WindowHandleProcessID = 0;
	ULONG result = 0;
	if(Original_NtUserGetForegroundWindow)
		result = Original_NtUserGetForegroundWindow();
	if(Original_NtUserQueryWindow)
		WindowHandleProcessID = Original_NtUserQueryWindow((HWND)result,0);
	if(IsProtectProcessId((HANDLE)WindowHandleProcessID) && PsGetCurrentProcessId()!=(HANDLE)WindowHandleProcessID)
		result = 0;
	return result;
}

NTUSERFINDWINDOWEX Original_NtUserFindWindowEx = NULL;

HWND NtUserFindWindowEx_Proxy(IN HWND hwndParent, IN HWND hwndChild, IN PUNICODE_STRING pstrClassName OPTIONAL, IN PUNICODE_STRING pstrWindowName OPTIONAL, IN DWORD dwType)
{
	HWND result;
	ULONG WindowHandleProcessID;
	if(Original_NtUserFindWindowEx)
		result=Original_NtUserFindWindowEx(hwndParent,hwndChild,pstrClassName,pstrWindowName,dwType);
	if (result)
	{
		if(Original_NtUserQueryWindow)
			WindowHandleProcessID = Original_NtUserQueryWindow(result,0);
		if (IsProtectProcessId((HANDLE)WindowHandleProcessID) && PsGetCurrentProcessId()!=(HANDLE)WindowHandleProcessID)
			result = (HWND)0;
	}
	return result;
}

NTUSERBUILDHWNDLIST Original_NtUserBuildHwndList = NULL;

NTSTATUS NtUserBuildHwndList_Proxy(IN ULONG hdesk, IN ULONG hwndNext, IN ULONG fEnumChildren, IN DWORD idThread, IN UINT cHwndMax, OUT HWND *phwndFirst, OUT ULONG* pcHwndNeeded)
{
	NTSTATUS result = STATUS_UNSUCCESSFUL;
	ULONG WindowHandleProcessID;
	BOOL bCheck = FALSE;
	if (fEnumChildren==1)
	{
		if(Original_NtUserQueryWindow)
			WindowHandleProcessID = Original_NtUserQueryWindow((HWND)hwndNext,0);
		if (IsProtectProcessId((HANDLE)WindowHandleProcessID))// && PsGetCurrentProcessId()!=(HANDLE)WindowHandleProcessID)
			return STATUS_UNSUCCESSFUL;
	}
	if(Original_NtUserBuildHwndList)
		result = Original_NtUserBuildHwndList(hdesk,hwndNext,fEnumChildren,idThread,cHwndMax,phwndFirst,pcHwndNeeded);
	if (NT_SUCCESS(result))
	{
		ULONG i=0;
		HWND phwnd = 0;
		while (i<*pcHwndNeeded)
		{
			if(Original_NtUserQueryWindow)
				WindowHandleProcessID = Original_NtUserQueryWindow((HWND)phwndFirst[i],0);
			if (IsProtectProcessId((HANDLE)WindowHandleProcessID) && PsGetCurrentProcessId()!=(HANDLE)WindowHandleProcessID)
			{
				phwndFirst[i]=phwnd;
			}
			i++;				
		}
	}
	return result;
}

NTUSERWINDOWFROMPOINT Original_NtUserWindowFromPoint = NULL;

HWND NtUserWindowFromPoint_Proxy(LONG x, LONG y)
{
	HWND hwnd = NULL;
	ULONG WindowHandleProcessID;
	if(Original_NtUserWindowFromPoint)
	{
		hwnd = Original_NtUserWindowFromPoint(x,y);
		if(Original_NtUserQueryWindow)
		{
			WindowHandleProcessID = Original_NtUserQueryWindow((HWND)hwnd,0);
			if (IsProtectProcessId((HANDLE)WindowHandleProcessID) && PsGetCurrentProcessId()!=(HANDLE)WindowHandleProcessID)
				return (HWND)0;
		}
	}
	return hwnd;
}

NTUSERSETWINEVENTHOOK Original_NtUserSetWinEventHook = NULL;

HWINEVENTHOOK NtUserSetWinEventHook_Proxy(
	IN DWORD eventMin,
	IN DWORD eventMax,
	IN HINSTANCE hmodWinEventProc,
	IN PUNICODE_STRING pstrLib OPTIONAL,
	IN WINEVENTPROC pfnWinEventProc,
	IN DWORD idEventProcess,
	IN DWORD idEventThread,
	IN DWORD dwFlags)
{
	DbgPrint("NtUserSetWinEventHook_Proxy Successfull!\r\n");
	return 0;
}

#define WH_GETMESSAGE       3

NTUSERSETWINDOWSHOOKEX Original_NtUserSetWindowsHookEx = NULL;

HHOOK NtUserSetWindowsHookEx_Proxy(
								   HINSTANCE Mod,
								   PUNICODE_STRING ModuleName,
								   DWORD ThreadId,
								   int HookId,
								   HOOKPROC HookProc,
								   DWORD dwFlags)
{
	if(HookId==WH_GETMESSAGE)
		return NULL;
	if(Original_NtUserSetWindowsHookEx)
		return Original_NtUserSetWindowsHookEx(Mod,ModuleName,ThreadId,HookId,HookProc,dwFlags);
	return NULL;
}

NTUSERSENDINPUT Replace_NtUserSendInput = NULL;
NTUSERFINDWINDOWEX Replace_NtUserFindWindowEx = NULL;

VOID ReplaceFakeSSDTShadowFunction()
{

	PKeHookInfo pKeHookInfo = GetKeHookInfo();
	SYSTEM_VERSION version = GetSystemVersion();
	switch(version)
	{
	case WINDOWSXP:
		{
			Original_NtUserQueryWindow = (NTUSERQUERYWINDOW)ReplaceSSDTApi(pKeHookInfo->FakeSSDTShadow->pvSSDTShadowBase,NtUserQueryWindow_Index_WinXP,(ULONG_PTR)NtUserQueryWindow_Proxy);
			Original_NtUserGetForegroundWindow = (NTUSERGETFOREGROUNDWINDOW)ReplaceSSDTApi(pKeHookInfo->FakeSSDTShadow->pvSSDTShadowBase,NtUserGetForegroundWindow_Index_WinXP,(ULONG_PTR)NtUserGetForegroundWindow_Proxy);
			Original_NtUserFindWindowEx = (NTUSERFINDWINDOWEX)ReplaceSSDTApi(pKeHookInfo->FakeSSDTShadow->pvSSDTShadowBase,NtUserFindWindowEx_Index_WinXP,(ULONG_PTR)NtUserFindWindowEx_Proxy);
			Original_NtUserBuildHwndList = (NTUSERBUILDHWNDLIST)ReplaceSSDTApi(pKeHookInfo->FakeSSDTShadow->pvSSDTShadowBase,NtUserBuildHwndList_Index_WinXP,(ULONG_PTR)NtUserBuildHwndList_Proxy);
			Original_NtUserWindowFromPoint = (NTUSERWINDOWFROMPOINT)ReplaceSSDTApi(pKeHookInfo->FakeSSDTShadow->pvSSDTShadowBase,NtUserWindowFromPoint_Index_WinXP,(ULONG_PTR)NtUserWindowFromPoint_Proxy);
			//Original_NtUserSetWindowsHookEx = (NTUSERSETWINDOWSHOOKEX)ReplaceSSDTApi(pKeHookInfo->FakeSSDTShadow->pvSSDTShadowBase,NtUserSetWindowsHookEx_Index_WinXP,(ULONG_PTR)NtUserSetWindowsHookEx_Proxy);
			Replace_NtUserSendInput = (NTUSERSENDINPUT)GetRelocateSSDTShadowByIndex(NtUserSendInput_Index_WinXP);
			Replace_NtUserFindWindowEx = (NTUSERFINDWINDOWEX)GetRelocateSSDTShadowByIndex(NtUserFindWindowEx_Index_WinXP);
			Original_NtUserSetWinEventHook = (NTUSERSETWINEVENTHOOK)ReplaceSSDTApi(pKeHookInfo->FakeSSDTShadow->pvSSDTShadowBase, NtUserSetWinEventHook_Index_WinXP, (ULONG_PTR)NtUserSetWinEventHook_Proxy);
		}
		break;
	case WINDOWS7:
		{
			Original_NtUserQueryWindow = (NTUSERQUERYWINDOW)ReplaceSSDTApi(pKeHookInfo->FakeSSDTShadow->pvSSDTShadowBase,NtUserQueryWindow_Index_Win7,(ULONG_PTR)NtUserQueryWindow_Proxy);
			Original_NtUserGetForegroundWindow = (NTUSERGETFOREGROUNDWINDOW)ReplaceSSDTApi(pKeHookInfo->FakeSSDTShadow->pvSSDTShadowBase,NtUserGetForegroundWindow_Index_Win7,(ULONG_PTR)NtUserGetForegroundWindow_Proxy);
			Original_NtUserFindWindowEx = (NTUSERFINDWINDOWEX)ReplaceSSDTApi(pKeHookInfo->FakeSSDTShadow->pvSSDTShadowBase,NtUserFindWindowEx_Index_Win7,(ULONG_PTR)NtUserFindWindowEx_Proxy);
			Original_NtUserBuildHwndList = (NTUSERBUILDHWNDLIST)ReplaceSSDTApi(pKeHookInfo->FakeSSDTShadow->pvSSDTShadowBase,NtUserBuildHwndList_Index_Win7,(ULONG_PTR)NtUserBuildHwndList_Proxy);
			Original_NtUserWindowFromPoint = (NTUSERWINDOWFROMPOINT)ReplaceSSDTApi(pKeHookInfo->FakeSSDTShadow->pvSSDTShadowBase,NtUserWindowFromPoint_Index_Win7,(ULONG_PTR)NtUserWindowFromPoint_Proxy);
			//Original_NtUserSetWindowsHookEx = (NTUSERSETWINDOWSHOOKEX)ReplaceSSDTApi(pKeHookInfo->FakeSSDTShadow->pvSSDTShadowBase,NtUserSetWindowsHookEx_Index_Win7,(ULONG_PTR)NtUserSetWindowsHookEx_Proxy);
			Replace_NtUserSendInput = (NTUSERSENDINPUT)GetRelocateSSDTShadowByIndex(NtUserSendInput_Index_Win7);
			Replace_NtUserFindWindowEx = (NTUSERFINDWINDOWEX)GetRelocateSSDTShadowByIndex(NtUserFindWindowEx_Index_Win7);
		}
		break;
	}
}

VOID ReplaceSSDTShadowFunction()
{
	SYSTEM_VERSION version = GetSystemVersion();
	if(!GetCsrssEprocess())
		return;
	KeAttachProcess(GetCsrssEprocess());
	switch(version)
	{
	case WINDOWSXP:
		{
			Original_NtUserSetWindowsHookEx = (NTUSERSETWINDOWSHOOKEX)ReplaceSSDTApi(GetCurrentSSDTShadow()->pvSSDTShadowBase,NtUserSetWindowsHookEx_Index_WinXP,(ULONG_PTR)NtUserSetWindowsHookEx_Proxy);
		}
		break;
	case WINDOWS7:
		{
			Original_NtUserSetWindowsHookEx = (NTUSERSETWINDOWSHOOKEX)ReplaceSSDTApi(GetCurrentSSDTShadow()->pvSSDTShadowBase,NtUserSetWindowsHookEx_Index_Win7,(ULONG_PTR)NtUserSetWindowsHookEx_Proxy);
		}
		break;
	}
	KeDetachProcess();
}
VOID RecoverSSDTShadowFunction()
{
	SYSTEM_VERSION version = GetSystemVersion();
	if(!GetCsrssEprocess())
		return;
	KeAttachProcess(GetCsrssEprocess());
	switch(version)
	{
	case WINDOWSXP:
		{
			if(Original_NtUserSetWindowsHookEx){ReplaceSSDTApi(GetCurrentSSDTShadow()->pvSSDTShadowBase,NtUserSetWindowsHookEx_Index_WinXP,(ULONG_PTR)Original_NtUserSetWindowsHookEx);}
		}
		break;
	case WINDOWS7:
		{
			if(Original_NtUserSetWindowsHookEx){ReplaceSSDTApi(GetCurrentSSDTShadow()->pvSSDTShadowBase,NtUserSetWindowsHookEx_Index_Win7,(ULONG_PTR)Original_NtUserSetWindowsHookEx);}
		}
		break;
	}
	KeDetachProcess();
}