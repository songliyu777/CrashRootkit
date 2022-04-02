#ifdef __cplusplus
extern "C" {
#endif
#include <ntddk.h>
#include <string.h>
#ifdef __cplusplus
}; // extern "C"
#endif

#include "Relocate.h"
#include "KeSysFun.h"
#include "CRD_BsodHook.h"
#include "CRD_Hook.h"
#include "Log.h"

VOID KeBugCheck2_InlineProxy( IN ULONG BugCheckCode,
						IN ULONG_PTR BugCheckParameter1,
						IN ULONG_PTR BugCheckParameter2,
						IN ULONG_PTR BugCheckParameter3,
						IN ULONG_PTR BugCheckParameter4,
						IN PKTRAP_FRAME TrapFrame)
{
	if( BugCheckCode==IRQL_NOT_LESS_OR_EQUAL||
		BugCheckCode==DRIVER_IRQL_NOT_LESS_OR_EQUAL||
		BugCheckCode==IRQL_GT_ZERO_AT_SYSTEM_SERVICE || 
		BugCheckCode==WINLOGON_FATAL_ERROR)
	{
		ExRaiseStatus(STATUS_UNSUCCESSFUL);
		return;
	}
	KEBUGCHECK2 function = (KEBUGCHECK2)GetRealFunctionAddress("KeBugCheck2");
	if(function)
	{
		function(BugCheckCode,BugCheckParameter1,BugCheckParameter2,BugCheckParameter3,BugCheckParameter4,TrapFrame);
	}
	else
	{
		ExRaiseStatus(STATUS_UNSUCCESSFUL);
	}
}

ULONG_PTR GetHookAddress_KeBugCheck2()
{
	BYTE KeyWords[] = {0xe8};	
	ULONG_PTR adr = 0;
	FARPROC funadr = GetNtKrFunctionByName("KeBugCheck"); 
	FARPROC ofunadr = GetOrignalNtKrFunctionByName("KeBugCheck");
	if(funadr && ofunadr)
	{
		adr = FindMemoryAddress((ULONG_PTR)ofunadr, (ULONG_PTR)ofunadr + 0x100, KeyWords, 0x1, FALSE);
		if(adr)
		{
			funadr = FARPROC((ULONG_PTR)funadr + (adr + 0x1 - (ULONG_PTR)ofunadr));
			ofunadr = FARPROC(adr + 0x1);
			ULONG_PTR cKeBugCheck2 =  (ULONG_PTR)funadr + *PULONG_PTR(funadr) + 4;
			ULONG_PTR oKeBugCheck2 =  (ULONG_PTR)ofunadr + *PULONG_PTR(ofunadr) + 4;
			if(RtlCompareMemory((PVOID)cKeBugCheck2,(PVOID)oKeBugCheck2,5)!=5)
			{
				PrintLog(L"Find KeBugCheck2 Inline Hook\r\n");
				return 0;
			}
			return (ULONG_PTR)cKeBugCheck2;
		}
	}
	return 0;
}

BOOLEAN DoBsodHook()
{
	ULONG_PTR KeBugCheck2 = GetHookAddress_KeBugCheck2();
	if(KeBugCheck2)
	{
		if(SetDynamicInLineHook("KeBugCheck2",KeBugCheck2,5,(ULONG_PTR)KeBugCheck2_InlineProxy,INLINE_HOOK_TYPE_JMP))
		{
			return TRUE;
		}
	}
	return FALSE;
}