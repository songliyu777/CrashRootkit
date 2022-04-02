#include "stdafx.h"

#include "HookTool.h"
#include "Log.h"

DynamicInLineHookMap g_dilhmap;

BOOL DoDynamicInLineHook(DInLineHook &dilh)
{
	if(dilh.code_length == 0)
	{
		PrintDbgString(L"需要制定动态HOOK代码长度 \r\n");
		return FALSE;
	}
	if(dilh.myfunction_address==NULL || dilh.original_address==NULL)
	{
		PrintDbgString(L"原函数和跳转函数地址不能为空 \n");
		return FALSE;
	}
	if(dilh.recover_code == NULL)
	{
		dilh.recover_code = (PBYTE)malloc(dilh.code_length);
		if(!dilh.recover_code)
		{
			PrintDbgString(L"分配存储空间失败 \r\n");
			return FALSE;
		}
	}
	if(dilh.name == "")
	{
		PrintDbgString(L"需要一个名字标示 \r\n");
		return FALSE;
	}
	DWORD oldprotect;
	if(VirtualProtect((LPVOID)dilh.original_address,dilh.code_length,PAGE_EXECUTE_READWRITE,&oldprotect))
	{
		PrintDbgString(L"VirtualProtect 空间成功[%p]\r\n",dilh.original_address);
		DWORD originFunctionAddress = (DWORD)dilh.original_address;
		memcpy(dilh.recover_code, (PVOID)originFunctionAddress, dilh.code_length);
		DWORD mapFunctionAddress = (DWORD)dilh.myfunction_address;
		DWORD value = mapFunctionAddress - originFunctionAddress - 5;
		//Jmp构造
		_asm
		{
			mov eax,originFunctionAddress
				mov [eax],0xe9
				add eax,1
				mov ebx,value
				mov dword ptr [eax],ebx
		}
		//VirtualProtect((LPVOID)dilh.original_address,dilh.code_lenght,oldprotect,&oldprotect);
		//动态调用头构造
		LPVOID lpParameter = VirtualAlloc(NULL, dilh.code_length + 5 ,MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		if(lpParameter)
		{
			PrintDbgString(L"VirtualAlloc 空间成功[%p] \r\n",lpParameter);
			dilh.dynamic_code_enter = (ULONG_PTR)lpParameter;
			originFunctionAddress = (DWORD)lpParameter + dilh.code_length;
			mapFunctionAddress = (DWORD)dilh.original_address + dilh.code_length;
			DWORD code_length = dilh.code_length;
			value =  mapFunctionAddress - originFunctionAddress - 5;;
			memcpy(lpParameter,dilh.recover_code,dilh.code_length);
			_asm
			{
				mov eax,lpParameter
					mov ebx,code_length
					add eax,ebx
					mov [eax],0xe9
					add eax,1
					mov ebx,value
					mov dword ptr [eax],ebx 
			}
			g_dilhmap.insert(DynamicInLineHookMap::value_type(dilh.name,dilh));
			return TRUE;
		}
	}
	return FALSE;
}

FARPROC GetRealFunctionAddress(string name)
{
	DynamicInLineHookMap::iterator iter;
	iter = g_dilhmap.find(name);
	if(iter != g_dilhmap.end())
	{
		return (FARPROC)(*iter).second.dynamic_code_enter;
	}
	return NULL;
}

void WINAPI ReleaseDynamicInLineHook()
{
	DynamicInLineHookMap::iterator iter;
	for(iter = g_dilhmap.begin();iter != g_dilhmap.end();iter++)
	{

		DWORD oldprotect;
		PDInLineHook pdilh = &(*iter).second;
		if(VirtualProtect((LPVOID)pdilh->original_address,pdilh->code_length,PAGE_EXECUTE_READWRITE,&oldprotect))
		{
			memcpy((LPVOID)pdilh->original_address,pdilh->recover_code,pdilh->code_length);
			VirtualProtect((LPVOID)pdilh->original_address,pdilh->code_length,oldprotect,&oldprotect);
		}
		if(pdilh->recover_code)
			free(pdilh->recover_code);
		VirtualFree((LPVOID)pdilh->dynamic_code_enter,pdilh->code_length + 5,MEM_DECOMMIT | MEM_RELEASE);
	}
	g_dilhmap.clear();
}

typedef HMODULE (WINAPI * LOADLIBRARYA)(__in LPCSTR lpLibFileName);

HMODULE WINAPI LoadLibraryAProxy(
			 __in LPCSTR lpLibFileName
			 )
{
	LOADLIBRARYA fun = (LOADLIBRARYA)GetRealFunctionAddress("LoadLibraryA");
	if(fun)
		return fun(lpLibFileName);
	return NULL;
}

typedef HMODULE (WINAPI * LOADLIBRARYW)(__in LPCWSTR lpLibFileName);

HMODULE WINAPI LoadLibraryWProxy(
			 __in LPCWSTR lpLibFileName
			 )
{
	LOADLIBRARYW fun = (LOADLIBRARYW)GetRealFunctionAddress("LoadLibraryW");
	if(fun)
		return fun(lpLibFileName);
	return NULL;
}

BOOL HookLoadLibraryA()
{
	ULONG_PTR address = (ULONG_PTR)LoadLibraryA;
	DInLineHook dilh;
	dilh.name = "LoadLibraryA";
	dilh.code_length = 5;
	dilh.myfunction_address = (ULONG_PTR)LoadLibraryAProxy;
	dilh.original_address = (ULONG_PTR)address;
	return DoDynamicInLineHook(dilh);
}

BOOL HookLoadLibraryW()
{
	ULONG_PTR address = (ULONG_PTR)LoadLibraryW;
	DInLineHook dilh;
	dilh.name = "LoadLibraryW";
	dilh.code_length = 5;
	dilh.myfunction_address = (ULONG_PTR)LoadLibraryWProxy;
	dilh.original_address = (ULONG_PTR)address;
	return DoDynamicInLineHook(dilh);
}

typedef long (NTAPI * LDRLOADDLL)(IN PWCHAR	pszSearchPath OPTIONAL,IN PULONG puDllCharacteristics OPTIONAL,IN PUNICODE_STRING pusDllName,OUT PHANDLE phDllHandle);

long NTAPI FakeLdrLoadDll(IN PWCHAR	pszSearchPath OPTIONAL,IN PULONG puDllCharacteristics OPTIONAL,IN PUNICODE_STRING pusDllName,OUT PHANDLE phDllHandle)
{
	LDRLOADDLL fun = (LDRLOADDLL)GetRealFunctionAddress("LdrLoadDll");
	if(pusDllName)
	{
		WCHAR szImageName[MAX_PATH];
		GetImageNameByPath(szImageName,pusDllName->Buffer);
		if(!wcscmp(szImageName,L"TASAntiPhishing_1.dll"))
		{
			return STATUS_SUCCESS;
		}
	}
	if(fun)
		return fun(pszSearchPath,puDllCharacteristics,pusDllName,phDllHandle);
	return NULL;
}

BOOL HookLdrLoadDll()
{
	HMODULE module = GetModuleHandle(L"ntdll.dll");
	FARPROC adr = GetProcAddress(module,"LdrLoadDll");
	ULONG_PTR address = (ULONG_PTR)adr;
	DInLineHook dilh;
	dilh.name = "LdrLoadDll";
	dilh.code_length = 5;
	dilh.myfunction_address = (ULONG_PTR)FakeLdrLoadDll;
	dilh.original_address = (ULONG_PTR)address;
	return DoDynamicInLineHook(dilh);
}
