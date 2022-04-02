#pragma once

#define _CRD_
#include "Transfer.h"
#include "CRD_HideWindows.h"

typedef UINT (*NTUSERSENDINPUT)(IN UINT cInputs, IN CONST INPUT *pInputs, IN int cbSize);

UINT RNtUserSendInput_Proxy(IN UINT cInputs, IN CONST INPUT *pInputs, IN int cbSize);

HWND RNtUserFindWindowEx_Proxy(IN HWND hwndParent, IN HWND hwndChild, IN PUNICODE_STRING pstrClassName OPTIONAL, IN PUNICODE_STRING pstrWindowName OPTIONAL, IN DWORD dwType);