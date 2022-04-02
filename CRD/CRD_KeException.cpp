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
#include "CRD_KeException.h"
#include "CRD_Hook.h"
#include "CRD_BsodHook.h"
#include "Log.h"
#include "CRD_ProtectTools.h"
#include "Platform.h"

ULONG_PTR OriginalKiUserExceptionDispatcher = 0; 
ULONG_PTR OrignalKiDispatchException_Jmp = 0;
ULONG_PTR KiUserExceptionDispatcherAddress_Proxy = 0;

BOOLEAN NeedChangeKiUserExceptionDispatcher(PKTRAP_FRAME pKtf)
{
	KPROCESSOR_MODE  mode;
	mode = ExGetPreviousMode();
	if (mode == UserMode)
	{
		if(IsTargetProcessId(PsGetCurrentProcessId()))
		{
			if(pKtf->Eip==pKtf->Dr0 || pKtf->Eip==pKtf->Dr1 || pKtf->Eip==pKtf->Dr2 || pKtf->Eip==pKtf->Dr3)
			{
				KdPrint(("Exception  ThreadId:[%d] Eax[0x%X] Ecx[0x%X] Edx[0x%X] Edi[0x%X] Esi[0x%X] Ebx[0x%X] Ebp[0x%X] Eip[0x%X] Dr0[0x%X] Dr1[0x%X] Dr2[0x%X] Dr3[0x%X] Dr6[0x%X] Dr7[0x%X]\r\n",
						 PsGetCurrentThreadId(),pKtf->Eax,pKtf->Ecx,pKtf->Edx,pKtf->Edi,pKtf->Esi,pKtf->Ebx,pKtf->Ebp,pKtf->Eip,pKtf->Dr0,pKtf->Dr1,pKtf->Dr2,pKtf->Dr3,pKtf->Dr6,pKtf->Dr7));
			}
			if(KiUserExceptionDispatcherAddress_Proxy)
				return TRUE;
		}
	}
	return FALSE;
}

VOID __declspec(naked) KiDispatchException_Proxy_XP()
{
	_asm
	{
		push ebx
		push ecx
		push edx
		push esi
		push edi
		push esp
		push ebp
		pushfd
		push ebx
		call NeedChangeKiUserExceptionDispatcher
		cmp al,0
		mov eax,OriginalKiUserExceptionDispatcher
		je NO_CHANGE_EXCEPTION_XP
		mov eax,KiUserExceptionDispatcherAddress_Proxy
NO_CHANGE_EXCEPTION_XP:
		popfd
		pop ebp
		pop esp
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		mov dword ptr [ebx+68h],eax //eax: ntdll!KiUserExceptionDispatcher
		or  dword ptr [ebp-4],0FFFFFFFFh
		jmp OrignalKiDispatchException_Jmp
	}
}
//xp
//804ff7a4 8ac8            mov     cl,al
//804ff7a6 f6d9            neg     cl
//804ff7a8 1bc9            sbb     ecx,ecx
//804ff7aa 83e103          and     ecx,3
//804ff7ad 83c120          add     ecx,20h
//804ff7b0 894b38          mov     dword ptr [ebx+38h],ecx
//804ff7b3 894b34          mov     dword ptr [ebx+34h],ecx
//804ff7b6 f6d8            neg     al
//804ff7b8 1bc0            sbb     eax,eax
//804ff7ba 83e003          and     eax,3
//804ff7bd 83c038          add     eax,38h
//804ff7c0 894350          mov     dword ptr [ebx+50h],eax
//804ff7c3 83633000        and     dword ptr [ebx+30h],0
//804ff7c7 a170d65580      mov     eax,dword ptr [nt!KeUserExceptionDispatcher (8055d670)]
//804ff7cc 894368          mov     dword ptr [ebx+68h],eax
//804ff7cf 834dfcff        or      dword ptr [ebp-4],0FFFFFFFFh

NTSTATUS DoKiDispatchExceptionHook_XP()
{
	ULONG_PTR maddress = 0,offset = 0;
	ULONG_PTR hookstartadr = 0;
	BYTE KeyWords[35] = {0x8a,0xc8,0xf6,0xd9,0x1b,0xc9,0x83,0xe1,0x03,0x83,
						 0xc1,0x20,0x89,0x4b,0x38,0x89,0x4b,0x34,0xf6,0xd8,
						 0x1b,0xc0,0x83,0xe0,0x03,0x83,0xc0,0x38,0x89,0x43,
						 0x50,0x83,0x63,0x30,0x0};
	ULONG_PTR BaseAddress = GetNtKrBase();
	__try
	{
		maddress = FindMemoryAddress((ULONG_PTR)GetMemoryModuleNtKr()->codeBase,(ULONG_PTR)GetMemoryModuleNtKr()->codeBase + GetMemoryModuleNtKr()->SizeOfImage,KeyWords,35,FALSE);
		if(maddress)
		{
			offset = maddress - (ULONG_PTR)GetMemoryModuleNtKr()->codeBase;
			hookstartadr = BaseAddress + offset + 40;
			if(RtlCompareMemory((PVOID)hookstartadr,(PVOID)(maddress + 40),5)!=5)
			{
				PrintLog(L"Find KiDispatchException Inline Hook \r\n");
				return STATUS_UNSUCCESSFUL;
			}
			OrignalKiDispatchException_Jmp = hookstartadr + 7;
			ULONG_PTR value = *PULONG_PTR(hookstartadr - 4);
			if(!MmIsAddressValid((PVOID)value))
			{
				return STATUS_UNSUCCESSFUL;
			}
			OriginalKiUserExceptionDispatcher = *PULONG_PTR(value);
			KdPrint(("KiDispatchExceptionHookAddress %p \r\n",hookstartadr));
			PrintLog(L"KiDispatchExceptionHookAddress %p \r\n",hookstartadr);
			if(!SetDynamicInLineHook("KiDispatchException",hookstartadr,5,(ULONG_PTR)KiDispatchException_Proxy_XP,INLINE_HOOK_TYPE_JMP))
			{
				return STATUS_UNSUCCESSFUL;
			}
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		KdPrint(("DoKiDispatchExceptionHook exception \r\n"));
		PrintLog(L"DoKiDispatchExceptionHook exception \r\n");
		return STATUS_UNSUCCESSFUL;
	}
	return STATUS_SUCCESS;
}

VOID __declspec(naked) KiDispatchException_Proxy_WIN7()
{
	_asm
	{
		push ebx
		push ecx
		push edx
		push esi
		push edi
		push esp
		push ebp
		pushfd
		push esi
		call NeedChangeKiUserExceptionDispatcher
		cmp al,0
		mov eax,OriginalKiUserExceptionDispatcher
		je NO_CHANGE_EXCEPTION_WIN7
		mov eax,KiUserExceptionDispatcherAddress_Proxy
NO_CHANGE_EXCEPTION_WIN7:
		popfd
		pop ebp
		pop esp
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		mov dword ptr [esi+68h],eax
		mov dword ptr [ebp-4],0FFFFFFFEh
		jmp OrignalKiDispatchException_Jmp
	}
}
//win7
//83d0128e 83e0fd          and     eax,0FFFFFFFDh
//83d01291 83c023          add     eax,23h
//83d01294 894638          mov     dword ptr [esi+38h],eax
//83d01297 894634          mov     dword ptr [esi+34h],eax
//83d0129a 33c0            xor     eax,eax
//83d0129c 384514          cmp     byte ptr [ebp+14h],al
//83d0129f 0f95c0          setne   al
//83d012a2 48              dec     eax
//83d012a3 83e0fd          and     eax,0FFFFFFFDh
//83d012a6 83c03b          add     eax,3Bh
//83d012a9 894650          mov     dword ptr [esi+50h],eax
//83d012ac 83663000        and     dword ptr [esi+30h],0
//83d012b0 a1e45adb83      mov     eax,dword ptr [nt!KeUserExceptionDispatcher (83db5ae4)]
//83d012b5 894668          mov     dword ptr [esi+68h],eax
//83d012b8 c745fcfeffffff  mov     dword ptr [ebp-4],0FFFFFFFEh

NTSTATUS DoKiDispatchExceptionHook_WIN7()
{
	ULONG_PTR maddress = 0,offset = 0;
	ULONG_PTR hookstartadr = 0;
	BYTE KeyWords[34] = {0x83,0xe0,0xfd,0x83,0xc0,0x23,0x89,0x46,0x38,0x89,
						 0x46,0x34,0x33,0xc0,0x38,0x45,0x14,0x0f,0x95,0xc0,
						 0x48,0x83,0xe0,0xfd,0x83,0xc0,0x3b,0x89,0x46,0x50,
						 0x83,0x66,0x30,0x00};
	ULONG_PTR BaseAddress = GetNtKrBase();
	__try
	{
		maddress = FindMemoryAddress((ULONG_PTR)GetMemoryModuleNtKr()->codeBase,(ULONG_PTR)GetMemoryModuleNtKr()->codeBase + GetMemoryModuleNtKr()->SizeOfImage,KeyWords,34,FALSE);
		if(maddress)
		{
			offset = maddress - (ULONG_PTR)GetMemoryModuleNtKr()->codeBase;
			hookstartadr = BaseAddress + offset + 39;
			if(RtlCompareMemory((PVOID)hookstartadr,(PVOID)(maddress + 39),5)!=5)
			{
				PrintLog(L"Find KiDispatchException Inline Hook \r\n");
				return STATUS_UNSUCCESSFUL;
			}
			OrignalKiDispatchException_Jmp = hookstartadr + 10;
			ULONG_PTR value = *PULONG_PTR(hookstartadr - 4);
			if(!MmIsAddressValid((PVOID)value))
			{
				return STATUS_UNSUCCESSFUL;
			}
			OriginalKiUserExceptionDispatcher = *PULONG_PTR(value);
			KdPrint(("KiDispatchExceptionHookAddress %p \r\n",hookstartadr));
			PrintLog(L"KiDispatchExceptionHookAddress %p \r\n",hookstartadr);
			if(!SetDynamicInLineHook("KiDispatchException",hookstartadr,5,(ULONG_PTR)KiDispatchException_Proxy_WIN7,INLINE_HOOK_TYPE_JMP))
			{
				return STATUS_UNSUCCESSFUL;
			}
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		KdPrint(("DoKiDispatchExceptionHook exception \r\n"));
		PrintLog(L"DoKiDispatchExceptionHook exception \r\n");
		return STATUS_UNSUCCESSFUL;
	}
	return STATUS_SUCCESS;
}

VOID SetKiUserExceptionDispatcherAddress(ULONG_PTR BaseAddress)
{
	KiUserExceptionDispatcherAddress_Proxy = BaseAddress;
}

ULONG_PTR GetKiUserExceptionDispatcherAddress()
{
	return KiUserExceptionDispatcherAddress_Proxy;
}

NTSTATUS SetKeException(PTransferMsg msg)
{
	ULONG_PTR ulBaseAddr = *PULONG_PTR(msg->buff);
	SetKiUserExceptionDispatcherAddress(ulBaseAddr);
	return STATUS_SUCCESS;
}

NTSTATUS DoKeExceptionHook()
{
	SYSTEM_VERSION version = GetSystemVersion();
	switch(version)
	{
	case WINDOWSXP:
		return DoKiDispatchExceptionHook_XP();
	case WINDOWS7:
		return DoKiDispatchExceptionHook_WIN7();
	}
	return STATUS_UNSUCCESSFUL;
}