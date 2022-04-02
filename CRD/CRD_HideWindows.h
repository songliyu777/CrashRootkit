#pragma once

#include "MyWinNT.h"

typedef ULONG HWND,UINT;

#if !defined(CONST)
#define CONST const
#endif

#define NtUserQueryWindow_Index_WinXP 483
#define NtUserQueryWindow_Index_Win7 515

typedef ULONG (*NTUSERQUERYWINDOW)(IN HWND WindowHandle,IN DWORD TypeInformation);

ULONG NtUserQueryWindow_Proxy(IN HWND WindowHandle,IN DWORD TypeInformation);

#define NtUserGetForegroundWindow_Index_WinXP 404
#define NtUserGetForegroundWindow_Index_Win7 423

typedef ULONG (*NTUSERGETFOREGROUNDWINDOW)(VOID);

ULONG NtUserGetForegroundWindow_Proxy(VOID);

#define NtUserFindWindowEx_Index_WinXP 378
#define NtUserFindWindowEx_Index_Win7 396

typedef HWND (*NTUSERFINDWINDOWEX)(IN HWND hwndParent, IN HWND hwndChild, IN PUNICODE_STRING pstrClassName OPTIONAL, IN PUNICODE_STRING pstrWindowName OPTIONAL, IN DWORD dwType);

HWND NtUserFindWindowEx_Proxy(IN HWND hwndParent, IN HWND hwndChild, IN PUNICODE_STRING pstrClassName OPTIONAL, IN PUNICODE_STRING pstrWindowName OPTIONAL, IN DWORD dwType);

#define NtUserBuildHwndList_Index_WinXP 312
#define NtUserBuildHwndList_Index_Win7 323

typedef NTSTATUS (*NTUSERBUILDHWNDLIST)(IN ULONG hdesk, IN ULONG hwndNext, IN ULONG fEnumChildren, IN DWORD idThread, IN UINT cHwndMax, OUT HWND *phwndFirst, OUT ULONG* pcHwndNeeded);

NTSTATUS NtUserBuildHwndList_Proxy(IN ULONG hdesk, IN ULONG hwndNext, IN ULONG fEnumChildren, IN DWORD idThread, IN UINT cHwndMax, OUT HWND *phwndFirst, OUT ULONG* pcHwndNeeded);

#define NtUserWindowFromPoint_Index_WinXP 592
#define NtUserWindowFromPoint_Index_Win7 629

typedef HWND (*NTUSERWINDOWFROMPOINT)(LONG x, LONG y);

HWND NtUserWindowFromPoint_Proxy(LONG x, LONG y);


#define NtUserSendInput_Index_WinXP 502
#define NtUserSendInput_Index_Win7 536

typedef HANDLE HHOOK,HINSTANCE;
typedef ULONG_PTR HOOKPROC;


#define NtUserSetWindowsHookEx_Index_WinXP 549
#define NtUserSetWindowsHookEx_Index_Win7 585

typedef HHOOK (*NTUSERSETWINDOWSHOOKEX)(
										HINSTANCE Mod,
										PUNICODE_STRING ModuleName,
										DWORD ThreadId,
										int HookId,
										HOOKPROC HookProc,
										DWORD dwFlags);

HHOOK NtUserSetWindowsHookEx_Proxy(
					   HINSTANCE Mod,
					   PUNICODE_STRING ModuleName,
					   DWORD ThreadId,
					   int HookId,
					   HOOKPROC HookProc,
					   DWORD dwFlags);


#define NtUserSetWinEventHook_Index_WinXP 552

typedef PVOID HANDLE;
typedef HANDLE HINSTANCE;
typedef DWORD HWINEVENTHOOK;

typedef VOID (__stdcall *WINEVENTPROC)(HWINEVENTHOOK hEvent, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD idEventThread, DWORD dwmsEventTime);

typedef HWINEVENTHOOK (*NTUSERSETWINEVENTHOOK)(
	IN DWORD eventMin,
	IN DWORD eventMax,
	IN HINSTANCE hmodWinEventProc,
	IN PUNICODE_STRING pstrLib OPTIONAL,
	IN WINEVENTPROC pfnWinEventProc,
	IN DWORD idEventProcess,
	IN DWORD idEventThread,
	IN DWORD dwFlags);

HWINEVENTHOOK NtUserSetWinEventHook_Proxy(
	IN DWORD eventMin,
	IN DWORD eventMax,
	IN HINSTANCE hmodWinEventProc,
	IN PUNICODE_STRING pstrLib OPTIONAL,
	IN WINEVENTPROC pfnWinEventProc,
	IN DWORD idEventProcess,
	IN DWORD idEventThread,
	IN DWORD dwFlags);

VOID ReplaceFakeSSDTShadowFunction();

VOID ReplaceSSDTShadowFunction();
VOID RecoverSSDTShadowFunction();