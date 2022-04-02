#ifdef __cplusplus
extern "C" {
#endif
#include <ntddk.h>
#include <string.h>
#ifdef __cplusplus
}; // extern "C"
#endif

#include "KeSysFun.h"
#include "CRD_Input.h"

KeyboardClassServiceCallback OrignalKeyboardClassServiceCallback = NULL;
MouseClassServiceCallback OrignalMouseClassServiceCallback = NULL;

PDEVICE_OBJECT g_kbDeviceObject = NULL;

//xp
//b8411192 8bff            mov     edi,edi
//b8411194 55              push    ebp
//b8411195 8bec            mov     ebp,esp
//b8411197 51              push    ecx
//b8411198 51              push    ecx
//b8411199 8b4508          mov     eax,dword ptr [ebp+8]
//b841119c 83650800        and     dword ptr [ebp+8],0
//b84111a0 53              push    ebx
//b84111a1 8b5d10          mov     ebx,dword ptr [ebp+10h]
//b84111a4 2b5d0c          sub     ebx,dword ptr [ebp+0Ch]
//b84111a7 56              push    esi
//b84111a8 8b7028          mov     esi,dword ptr [eax+28h]
//b84111ab 8b4514          mov     eax,dword ptr [ebp+14h]
//b84111ae 832000          and     dword ptr [eax],0
//b84111b1 57              push    edi
//b84111b2 6a04            push    4
//b84111b4 ff15a81e41b8    call    dword ptr [kbdclass!_imp__PoSetSystemState (b8411ea8)]
//b84111ba 8d4e6c          lea     ecx,[esi+6Ch]
//b84111bd ff15a41e41b8    call    dword ptr [kbdclass!_imp_KefAcquireSpinLockAtDpcLevel (b8411ea4)]

//win7
//900024a8 8bff            mov     edi,edi
//900024aa 55              push    ebp
//900024ab 8bec            mov     ebp,esp
//900024ad 83ec10          sub     esp,10h
//900024b0 53              push    ebx
//900024b1 56              push    esi
//900024b2 57              push    edi
//900024b3 a100500090      mov     eax,dword ptr [kbdclass!WPP_GLOBAL_Control (90005000)]
//900024b8 bfbc410090      mov     edi,offset kbdclass!WPP_ThisDir_CTLGUID_KbdClassTraceGuid+0x10 (900041bc)
//900024bd 57              push    edi
//900024be 6a2d            push    2Dh
//900024c0 6a03            push    3
//900024c2 33db            xor     ebx,ebx
//900024c4 53              push    ebx
//900024c5 ff7030          push    dword ptr [eax+30h]
//900024c8 e867ebffff      call    kbdclass!WPP_RECORDER_SF_ (90001034)



NTSTATUS KbdInit() 
{
	UNICODE_STRING ntUnicodeString;
	NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;
	PDEVICE_OBJECT	TargetDeviceObject = NULL;
	PFILE_OBJECT	TargetFileObject = NULL;

	RtlInitUnicodeString( &ntUnicodeString, L"\\Device\\KeyboardClass0" );
	ntStatus = IoGetDeviceObjectPointer(IN	&ntUnicodeString,
		IN	FILE_ALL_ACCESS,
		OUT	&TargetFileObject,   
		OUT	&g_kbDeviceObject
		);
	if( !NT_SUCCESS(ntStatus) )
	{
		DbgPrint(("Couldn't Get the Keyboard Device Object\n"));

		TargetFileObject	= NULL;
		g_kbDeviceObject = NULL;	
		return( ntStatus );
	}

	ULONG_PTR kcbase = GetKernelModuleAddressByName("kbdclass.sys");
	if(!kcbase)
	{
		return ntStatus;
	}


	return STATUS_SUCCESS;
}

NTSTATUS SimulateInput(PTransferMsg msg)
{
	return STATUS_SUCCESS;
}