#ifdef __cplusplus
extern "C" {
#endif
#include <ntddk.h>
#include <string.h>
#ifdef __cplusplus
}; // extern "C"
#endif

#include "CRD_VtAssembly.h"
#include "CRD_VT.h"

__declspec(naked) ULONG_PTR GetCr0()
{
	__asm
	{
		_emit 0x0F // mov eax,cr0
		_emit 0x20
		_emit 0xC0
		retn
	}
}

__declspec(naked) ULONG_PTR GetCr3()
{
	__asm
	{
		_emit 0x0F // mov eax,cr3
		_emit 0x20
		_emit 0xD8
		retn
	}
}

__declspec(naked) ULONG_PTR GetCr4()
{
	__asm
	{
		_emit 0x0F // mov eax,cr4
		_emit 0x20
		_emit 0xE0
		retn
	}
}

VOID __stdcall SetCr4(ULONG_PTR ulValue)
{
	__asm
	{
		push eax
		mov eax,ulValue
		_emit 0x0F // mov cr4,eax
		_emit 0x22
		_emit 0xE0
		pop eax
	}
}

__declspec(naked) ULONG GetEflags()
{
	__asm
	{
		pushfd
		pop eax
		retn
	}
}

ULONG64 __stdcall ReadMsr(ULONG Index)
{
	__asm
	{
		mov ecx,Index
		_emit 0x5D //pop ebp

		_emit 0x0F // rdmsr
		_emit 0x32
		retn 0x4
	}
}

ULONG64 __stdcall WriteMsr(ULONG MSRIndex,ULONG LowPart,ULONG HighPart)
{
	__asm
	{
		mov	ecx, MSRIndex
		mov	eax, LowPart
		mov	edx, HighPart
		_emit 0x0F // 0F 30 wrmsr
		_emit 0x30
		retn 0xC
	}
}

ULONG GetGdtBase()
{
	char gdtr[6];
	__asm
	{
		sgdt gdtr
		mov eax,dword ptr gdtr[2]
	}
}

USHORT GetGdtLimit()
{
	char gdtr[6];
	__asm
	{
		sgdt gdtr
		xor eax,eax
		mov ax, word ptr gdtr[0]
	}
}

//===========================================================================
//	获取idt表基址
//===========================================================================
ULONG GetIdtBase()
{
	char idtr[6];
	__asm
	{
		sidt idtr
		mov eax,dword ptr idtr[2]
	}
}

USHORT GetIdtLimit()
{
	char idtr[6];
	__asm
	{
		sidt idtr
		xor eax,eax
		mov ax,word ptr idtr[0]
	}
}

VOID __stdcall SetIdtr(ULONG Base, ULONG Limit)
{
	__asm
	{
		push Base
		shl	Limit, 16
		push Limit
		lidt fword ptr [esp+2]
		pop	eax
		pop	eax
	}
}

__declspec(naked) USHORT GetCsSelector()
{
	__asm
	{
		xor eax,eax
		mov ax,cs
		retn
	}
}

__declspec(naked) USHORT GetDsSelector()
{
	__asm
	{
		xor eax,eax
		mov ax,ds
		retn
	}
}

__declspec(naked) USHORT GetEsSelector()
{
	__asm
	{
		xor eax,eax
		mov ax,es
		retn
	}
}

__declspec(naked) USHORT GetFsSelector()
{
	__asm
	{
		xor eax,eax
		mov ax,fs
		retn
	}
}

__declspec(naked) USHORT GetGsSelector()
{
	__asm
	{
		xor eax,eax
		mov ax,gs
		retn
	}
}

__declspec(naked) USHORT GetSsSelector()
{
	__asm
	{
		xor eax,eax
		mov ax,ss
		retn
	}
}

__declspec(naked) USHORT GetLdtrSelector()
{
	__asm
	{
		xor	eax,eax
		sldt ax
		ret
	}
}

__declspec(naked) USHORT GetTrSelector()
{
	__asm
	{
		xor	eax,eax
		str	ax
		ret
	}
}

VOID __stdcall ExecuteVmxOn(ULONG PtrLowPart, ULONG PtrHighPart)
{
	__asm
	{
		push PtrHighPart
		push PtrLowPart
		_emit 0xF3 // vmxon qword ptr [esp]
		_emit 0x0F
		_emit 0xC7
		_emit 0x34
		_emit 0x24
		add esp,0x8
	}
}

VOID __stdcall ExecuteVmxOff( ULONG_PTR RegEsp, ULONG_PTR RegEip )
{
	__asm
	{
		push GUEST_IDTR_BASE
		call ReadVMCS
		push eax
		push GUEST_IDTR_LIMIT
		call ReadVMCS
		shl	eax,16
		push eax
		lidt fword ptr [esp+2]
		pop	eax
		pop	eax

		push GUEST_GDTR_BASE
		call ReadVMCS
		push	eax
		push	GUEST_GDTR_LIMIT
		call	ReadVMCS
		shl	eax,16
		push eax
		lgdt fword ptr [esp+2]
		pop	eax
		pop	eax

		push	GUEST_CR0
		call	ReadVMCS
		mov	cr0,eax

		push GUEST_CR3
		call ReadVMCS
		mov	cr3,eax

		push GUEST_CR4
		call ReadVMCS
		_emit 0x0F // 0F 22 E0 mov	cr4,eax
		_emit 0x22
		_emit 0xE0

		mov	eax,RegEip
		mov	ecx,RegEsp

		_emit 0x0F // 0F 01 C4  vmxoff
		_emit 0x01
		_emit 0xC4

		leave

		mov	esp,ecx
		jmp	eax
	}
}

//===========================================================================
//	开启VT成功以后, 这就是Guest入口. 
//===========================================================================
__declspec(naked) VOID __stdcall GuestEntryPoint( PVOID _GuestEsp )
{
	__asm
	{
		pop ebp
		popad
		retn
	}
}

//===========================================================================
//	这是关闭VT以后的Guest退出函数的入口.
//===========================================================================
__declspec(naked) NTSTATUS NTAPI GuestExitPoint()
{
	__asm
	{
		_emit 0x5D // 5D pop ebp
		_emit 0x9D // 9D popfd
		_emit 0x61 // 61 popad
		mov eax, 0
		retn
	}
}

__declspec(naked) ULONG VmFailInvalid()
{
	__asm
	{
		pushfd
		pop eax

		xor ecx,ecx
		bt eax,0 // EFLAGS.CF
		adc cl,cl
		mov eax,ecx
		retn
	}
}

//===========================================================================
//	关闭VT技术, 这个函数也是不会返回的, 如果真正关闭成功接下来的函数是
//===========================================================================
NTSTATUS MakeHyperExitCall()
{
	__asm
	{
		pushad
		pushfd
		push ebp

		mov eax,'Joen'
		mov ecx,'Ddvp'
		mov edx,'Exit'
		mov ebx,'Quit'
		_emit 0x0F // vmcall
		_emit 0x01
		_emit 0xC1
		nop
		popfd
		popad
	}
	return STATUS_UNSUCCESSFUL;
}

/*===========================================================================
 这个函数看着有点奇怪, 如果开启VT成功. 其实这个函数不会再返回
 这个函数和GuestEntryPoint要注意, 这两个函数, 配合实现堆栈平衡.
=========================================================================== */
NTSTATUS NTAPI CmSubvert(PVOID pValue)
{
	NTSTATUS Status;

	Status = SubvertCpu(pValue);

	return Status;
}

VOID __stdcall ExecuteVmPtrLd(ULONG PtrLowPart, ULONG PtrHighPart)
{
	__asm
	{
		push PtrHighPart
		push PtrLowPart
		_emit 0x0F // 0F C7 34 24   vmptrld	qword ptr [esp]
		_emit 0xC7
		_emit 0x34
		_emit 0x24
		add	esp,0x8
	}
}

VOID __stdcall ExecuteVmClear(ULONG PtrLowPart, ULONG PtrHighPart)
{
	__asm
	{
		push PtrHighPart
		push PtrLowPart
		_emit 0x66 // 66| 0F C7 34 24  vmclear	qword ptr [esp]
		_emit 0x0F
		_emit 0xC7
		_emit 0x34
		_emit 0x24
		add esp,0x8
	}
}

VOID WriteVMCS(ULONG Field, ULONG Value)
{
	__asm
	{
		mov eax,Field
		mov ecx,Value
		_emit 0x0F // 0F 79 C1 vmwrite	eax,ecx
		_emit 0x79
		_emit 0xC1
		//retn 0x8
	}
}

ULONG __stdcall ReadVMCS(ULONG Field)
{
	__asm
	{
		mov	eax,Field
		_emit 0x0F // 0F 78 C1 vmread	ecx,eax
		_emit 0x78
		_emit 0xC1
		mov	eax,ecx
		//retn 0x4
	}
}

//===========================================================================
//	这个函数不会返回.会进入到VMCB中设置的Guest Eip中.  
//===========================================================================
__declspec(naked) ULONG ExecuteVmLaunch()
{
	__asm
	{
		_emit 0x0F // 0F 01 C2 vmlaunch
		_emit 0x01
		_emit 0xC2
		retn
	}
}

__declspec(naked) ULONG VmLaunchFailValid()
{
	__asm
	{
		pushfd
		pop	eax
		xor	ecx,ecx
		bt	eax,6	// RFLAGS.ZF
		adc	cl,cl
		mov	eax,ecx
		ret
	}
}

VOID __stdcall ExecuteCpuId( ULONG fn, OUT PULONG ret_eax, OUT PULONG ret_ebx, OUT PULONG ret_ecx, OUT PULONG ret_edx )
{
	__asm
	{
		mov	eax, fn
		cpuid
		mov	esi, ret_eax
		mov	dword ptr [esi], eax
		mov	esi, ret_ebx
		mov	dword ptr [esi], ebx
		mov	esi, ret_ecx
		mov	dword ptr [esi], ecx
		mov	esi, ret_edx
		mov	dword ptr [esi], edx
	}
}

__declspec(naked) VOID ExecuteInvd()
{
	__asm
	{
		invd
		retn
	}
}

__declspec(naked) VOID ExitEventHandler()
{
	__asm
	{
		cli

		push 	edi
		push 	esi
		push 	ebp
		push 	esp
		push 	ebx
		push 	edx
		push 	ecx
		push 	eax

		mov	ebx, esp	// Guest Reg
		lea 	eax, [esp+20h]	// Cpu

		push	ebx		// Guest Reg
		push 	eax		// Cpu
		call	HandleVmExit

		pop 	eax
		pop 	ecx
		pop 	edx
		pop 	ebx
		pop 	ebp // pop esp
		pop 	ebp
		pop 	esi
		pop 	edi

		_emit 0x0F // 0F 01 C3 vmresume
		_emit 0x01
		_emit 0xC3

		retn
	}
}

VOID NewKiFastCallEntry(VOID)
{

}