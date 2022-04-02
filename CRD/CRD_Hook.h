#pragma once
#include "MyListEntry.h"
#include "MyWinNT.h"
#include "CRD_SSDT.h"

typedef struct _DynamicInLineHook // 动态InlineHook
{
	SLISTENTRY next;
	ULONG_PTR original_address;
	ULONG_PTR myfunction_address;
	PBYTE recover_code;
	PBYTE hook_code;
	SIZE_T code_length;
	ULONG_PTR dynamic_code_enter;					//动态代码入口
	ULONG type;
	CHAR name[64];
} DInLineHook, *PDInLineHook;

VOID FreeDynamicInLineHook();

ULONG_PTR GetRealFunctionAddress(const CHAR * FunctionName);

#define INLINE_HOOK_TYPE_JMP 0
#define INLINE_HOOK_TYPE_CALL 1

BOOL SetDynamicInLineHook(LPCSTR lpFunctionName,ULONG_PTR functionaddress,SIZE_T hooklenght,ULONG_PTR functionproxy,ULONG type);

BOOL FreeDynamicInLineHookByFunctionName(LPCSTR lpFunctionName);

ULONG_PTR GetHookAddress_ObOpenObjectByPointerInNtOpenProcess();

typedef struct _KeHookInfo
{
	PBYTE pbRecoverCode;
	PBYTE pbHookCode;
	PBYTE pbKeCode;
	SIZE_T szHooklength;
	PVOID pvHookAddress;
	PSSDT FakeSSDT;
	PSSDTShadow FakeSSDTShadow;
	ULONG_PTR pFakeTableArea;
	BOOLEAN IsThirdParty;
}KeHookInfo,*PKeHookInfo;

ULONG_PTR FindFakeSSDTOffset(OUT PULONG_PTR adr);

BOOL InitKeHookInfo();

VOID FreeKeHookInfo();

PKeHookInfo GetKeHookInfo();

ULONG_PTR GetKiFastCallEntryHookAddress(OUT SIZE_T* length, OUT BOOLEAN* bThirdParty);

VOID CreateKeCode(INT_PTR offset, PBYTE buff, BOOLEAN bThreadParty);

BOOL CreateHookCode(PBYTE buff,SIZE_T lenght);

SIZE_T CreateOffsetCode(INT_PTR offset, PBYTE buff);

BOOL DoKeHook();

VOID UnKeHook();

ULONG_PTR ReplaceSSDTApi(PVOID pvSSDTBase, ULONG index,ULONG_PTR proxyadr);

NTSTATUS DoAntiProtectHook_1();

NTSTATUS DoAntiProtectHook_2();

ULONG FilterKiFastCallEntry360(ULONG FuncIndex, ULONG OrigFuncAddress, ULONG_PTR ServiceTableBase);

ULONG_PTR GetFakeSSDTFunction(ULONG FuncIndex);

ULONG_PTR GetFakeSSDTShadowFunction(ULONG FuncIndex);