#ifndef _HOOKTOOL_H_
#define _HOOKTOOL_H_
#include <map>
#include <string>

using namespace std;

typedef struct _DynamicInLineHook // 动态InlineHook
{
	_DynamicInLineHook()
	{
		original_address = NULL;
		myfunction_address = NULL;
		recover_code = NULL;
		code_length = 0;
		dynamic_code_enter = NULL;
	}
	ULONG_PTR original_address;
	ULONG_PTR myfunction_address;
	PBYTE recover_code;
	SIZE_T code_length;
	ULONG_PTR dynamic_code_enter; //动态代码入口
	string name;
} DInLineHook, *PDInLineHook;

typedef map<string,DInLineHook> DynamicInLineHookMap;

BOOL DoDynamicInLineHook(DInLineHook &dilh);

FARPROC GetRealFunctionAddress(string name);

void WINAPI ReleaseDynamicInLineHook();

BOOL HookLoadLibraryA();

BOOL HookLoadLibraryW();

BOOL HookLdrLoadDll();

#endif