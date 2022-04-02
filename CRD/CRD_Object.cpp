#ifdef __cplusplus
extern "C" {
#endif
#include <ntddk.h>
#include <string.h>
#ifdef __cplusplus
}; // extern "C"
#endif

#include "CRD_Object.h"
#include "CRD_HookFunction.h"
#include "CRD_SSDT.h"
#include "KeSysFun.h"
#include "Platform.h"

POBJECT_TYPE DbgkDebugObjectType = NULL;

//xp
//80644bb1 6a00            push    0
//80644bb3 8d4508          lea     eax,[ebp+8]
//80644bb6 50              push    eax
//80644bb7 ff75fc          push    dword ptr [ebp-4]
//80644bba ff3540a55580    push    dword ptr [nt!DbgkDebugObjectType (8055a540)]
//80644bc0 6a02            push    2
BOOLEAN InitDbgkDebugObjectType_XP()
{
	BYTE KeyWords[] = {0x6a,0x00,0x8d,0x45,0x08,0x50,0xff,0x75,0xfc,0xff,0x35};	
	ULONG_PTR function = *PULONG_PTR((ULONG_PTR)GetOriginalSSDT()->pvSSDTBase + NtDebugActiveProcess_Index_WinXP * 4);
	ULONG_PTR adr = 0;
	adr = FindMemoryAddress(function,function+0x100,KeyWords,11,false);
	if(adr)
	{
		adr = *PULONG_PTR(adr + 11);
		DbgkDebugObjectType = POBJECT_TYPE(*PULONG_PTR(adr));
		return TRUE;
	}
	return FALSE;
}
//win7
//83ebdd9f 6a00            push    0
//83ebdda1 8d45f8          lea     eax,[ebp-8]
//83ebdda4 50              push    eax
//83ebdda5 ff75f4          push    dword ptr [ebp-0Ch]
//83ebdda8 ff35346bd483    push    dword ptr [nt!DbgkDebugObjectType (83d46b34)]
//83ebddae 6a02            push    2
BOOLEAN InitDbgkDebugObjectType_WIN7()
{
	BYTE KeyWords[] = {0x6a,0x00,0x8d,0x45,0xf8,0x50,0xff,0x75,0xf4,0xff,0x35};	
	ULONG_PTR function = *PULONG_PTR((ULONG_PTR)GetOriginalSSDT()->pvSSDTBase + NtDebugActiveProcess_Index_Win7 * 4);
	ULONG_PTR adr = 0;
	adr = FindMemoryAddress(function,function+0x100,KeyWords,11,false);
	if(adr)
	{
		adr = *PULONG_PTR(adr + 11);
		DbgkDebugObjectType = POBJECT_TYPE(*PULONG_PTR(adr));
		return TRUE;
	}
	return FALSE;
}

BOOLEAN InitDbgkDebugObjectType()
{
	SYSTEM_VERSION version = GetSystemVersion();
	switch(version)
	{
	case WINDOWSXP:
		return InitDbgkDebugObjectType_XP();
	case WINDOWS7:
		return InitDbgkDebugObjectType_WIN7();
	}
	return FALSE;
}

NTSTATUS RecoverDbgkDebugObjectType_XP()
{
	if(DbgkDebugObjectType)
	{
		PULONG pValidAccessMask = PULONG((ULONG_PTR)DbgkDebugObjectType + 0x60 + 0x18);
		PAGEPROTECTOFF();
		KIRQL Irql;
		KeRaiseIrql(DISPATCH_LEVEL,&Irql);
		*pValidAccessMask = 0x1f000f;
		KeLowerIrql(Irql);
		PAGEPROTECTON();
		return STATUS_SUCCESS;
	}
	return STATUS_UNSUCCESSFUL;
}

NTSTATUS RecoverDbgkDebugObjectType_WIN7()
{
	if(DbgkDebugObjectType)
	{
		PULONG pValidAccessMask = PULONG((ULONG_PTR)DbgkDebugObjectType + 0x28 + 0x1c);
		PAGEPROTECTOFF();
		KIRQL Irql;
		KeRaiseIrql(DISPATCH_LEVEL,&Irql);
		*pValidAccessMask = 0x1f000f;
		KeLowerIrql(Irql);
		PAGEPROTECTON();
		return STATUS_SUCCESS;
	}
	return STATUS_UNSUCCESSFUL;
}

NTSTATUS RecoverDbgkDebugObjectType()
{
	SYSTEM_VERSION version = GetSystemVersion();
	switch(version)
	{
	case WINDOWSXP:
		return RecoverDbgkDebugObjectType_XP();
	case WINDOWS7:
		return RecoverDbgkDebugObjectType_WIN7();
	}
	return STATUS_UNSUCCESSFUL;
}