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
#include "CRD_ReplaceFunction.h"
#include "CRD_Hook.h"
#include "Log.h"
#include "CRD_ProtectTools.h"
#include "CRD_HideWindows.h"

extern NTUSERSENDINPUT Replace_NtUserSendInput;

UINT RNtUserSendInput_Proxy(IN UINT cInputs, IN CONST INPUT *pInputs, IN int cbSize)
{
	return Replace_NtUserSendInput(cInputs, pInputs, cbSize);
}

extern NTUSERFINDWINDOWEX Replace_NtUserFindWindowEx;

HWND RNtUserFindWindowEx_Proxy(IN HWND hwndParent, IN HWND hwndChild, IN PUNICODE_STRING pstrClassName OPTIONAL, IN PUNICODE_STRING pstrWindowName OPTIONAL, IN DWORD dwType)
{
	return Replace_NtUserFindWindowEx(hwndParent,hwndChild,pstrClassName,pstrWindowName,dwType);
}

