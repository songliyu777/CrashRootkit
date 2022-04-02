#ifdef __cplusplus
extern "C" {
#endif
#include <ntddk.h>
#include <string.h>
#ifdef __cplusplus
}; // extern "C"
#endif

#include "CRD_VT.h"
#include "CRD_VtAssembly.h"


extern KMUTEX g_GlobleMutex;
//---------------------------------------------------------------------------
static LONG g_uSubvertedCPUs;

/* ��д VT Vmx�ṹ */
NTSTATUS SetupVMX( PCPU_VM_CONTEXT pCpu )
{
    NTSTATUS Status;
    ULONG ulSize;
    ULONG64	 Msr;
    PVMX_BASIC_MSR pvmx;

    Status = STATUS_SUCCESS;
//---------------------------------------------------------------------------
    // RtlZeroMemory( pCpu, sizeof( CPU_VM_CONTEXT ) );

    //
    // ���������.
    //
    pCpu->CpuIndex = KeGetCurrentProcessorNumber();

    //
    // ��ģ�ͼĴ���(������VMX�İ汾, ���ڴ�����)
    //
    Msr = ReadMsr( MSR_IA32_VMX_BASIC );
    pvmx = ( PVMX_BASIC_MSR )&Msr;

    //
    // Vmx�ṹ���С.
    //
    ulSize = pvmx->szVmxOnRegion;

    KdPrint( ( "Ddvp-> VMXON region Size: 0x%p, VMX revision ID: 0x%p !\n",
               ulSize, pvmx->RevId ) );

//---------------------------------------------------------------------------

    //
    // Allocate memory for VMON, Ȼ��ִ��vmxonָ��
    //
    pCpu->pVMONVirtualAddress = MmAllocateNonCachedMemory( 0x1000 );

    if( pCpu->pVMONVirtualAddress == NULL )
    {
        KdPrint( ( "Ddvp-> VMXON �ڴ����ʧ�� !\n" ) );
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto __Exit;
    }

    RtlZeroMemory( pCpu->pVMONVirtualAddress, 0x1000 );

    //
    // vmxon�汾��.
    //
    *( ULONG * )pCpu->pVMONVirtualAddress = pvmx->RevId;

    //
    // vmxon�ṹ�ĵ������ַ.
    //
    pCpu->pVMONPyhicalAddress  = MmGetPhysicalAddress( pCpu->pVMONVirtualAddress );

    KdPrint( ( "Ddvp-> VMXON �ڴ������ַ��λ:%p, ��λ:%p !\n",
               pCpu->pVMONPyhicalAddress.HighPart, pCpu->pVMONPyhicalAddress.LowPart ) );

    //
    // ִ��vmxonָ��.
    //

    ExecuteVmxOn( pCpu->pVMONPyhicalAddress.LowPart,
                  pCpu->pVMONPyhicalAddress.HighPart );

    // ���CF���λ
    if( VmFailInvalid() )
    {
        KdPrint( ( "Ddvp-> vmxon ָ��ִ��ʧ��!\n" ) );
        Status = STATUS_UNSUCCESSFUL;
        goto __Exit;
    }

//---------------------------------------------------------------------------
    //
    // ������VMCS�ṹ
    //
    pCpu->pVMCSVirtualAddress = MmAllocateNonCachedMemory( 0x1000 );

    if( pCpu->pVMCSVirtualAddress == NULL )
    {
        KdPrint( ( "Ddvp-> ���� VMCS�ṹ�ڴ�ʧ��!\n" ) );
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto __Exit;
    }

    RtlZeroMemory( pCpu->pVMCSVirtualAddress, 0x1000 );

    // ��д�汾��
    *( ULONG * )pCpu->pVMCSVirtualAddress = pvmx->RevId;

    pCpu->pVMCSPyhicalAddress = MmGetPhysicalAddress( pCpu->pVMCSVirtualAddress );

    KdPrint( ( "Ddvp-> VMCS �ڴ������ַ��λ:%p, ��λ:%p !\n",
               pCpu->pVMCSPyhicalAddress.HighPart, pCpu->pVMCSPyhicalAddress.LowPart ) );
//---------------------------------------------------------------------------
    //
    // �����MSRλͼ.
    //
    pCpu->pMSRBitmapVirtualAddress = MmAllocateNonCachedMemory( 0x4000 );

    if( pCpu->pMSRBitmapVirtualAddress == NULL )
    {
        KdPrint( ( "Ddvp-> ���� MSR Bitmap �ṹ�ڴ�ʧ��!\n" ) );
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto __Exit;
    }

    RtlZeroMemory( pCpu->pMSRBitmapVirtualAddress, 0x4000 );

    pCpu->pMSRBitmapPyhicalAddress = MmGetPhysicalAddress( pCpu->pMSRBitmapVirtualAddress );

    KdPrint( ( "Ddvp-> MSR Bitmap �ڴ������ַ��λ:%p, ��λ:%p !\n",
               pCpu->pMSRBitmapPyhicalAddress.HighPart, pCpu->pMSRBitmapPyhicalAddress.LowPart ) );

//---------------------------------------------------------------------------
    //
    // ����� IOBitmapA
    //
    pCpu->pIOBitmapVirtualAddressA = MmAllocateNonCachedMemory( 0x1000 );

    if( pCpu->pIOBitmapVirtualAddressA == NULL )
    {
        KdPrint( ( "Ddvp-> ���� IOBitmapA �ṹ�ڴ�ʧ��!\n" ) );
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto __Exit;
    }

    RtlZeroMemory( pCpu->pIOBitmapVirtualAddressA, 0x1000 );
    pCpu->pIOBitmapPyhicalAddressA = MmGetPhysicalAddress( pCpu->pIOBitmapVirtualAddressA );

    KdPrint( ( "Ddvp-> IOBitmapA �ڴ������ַ��λ:%p, ��λ:%p !\n",
               pCpu->pIOBitmapPyhicalAddressA.HighPart, pCpu->pIOBitmapPyhicalAddressA.LowPart ) );

//---------------------------------------------------------------------------
    //
    // ����� IOBitmapB
    //
    pCpu->pIOBitmapVirtualAddressB = MmAllocateNonCachedMemory( 0x1000 );

    if( pCpu->pIOBitmapVirtualAddressB == NULL )
    {
        KdPrint( ( "Ddvp-> ���� IOBitmapB �ṹ�ڴ�ʧ��!\n" ) );
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto __Exit;
    }

    RtlZeroMemory( pCpu->pIOBitmapVirtualAddressB, 0x1000 );

    pCpu->pIOBitmapPyhicalAddressB = MmGetPhysicalAddress( pCpu->pIOBitmapVirtualAddressB );

    KdPrint( ( "Ddvp-> IOBitmapB �ڴ������ַ��λ:%p, ��λ:%p !\n",
               pCpu->pIOBitmapPyhicalAddressB.HighPart, pCpu->pIOBitmapPyhicalAddressB.LowPart ) );
//---------------------------------------------------------------------------
    //
    // �������д Host Stack �ṹ, 8K�ڴ�, ע�������ﲻҪ��MmAllocateNonCachedMemory, ������.
    //
    pCpu->pHostEsp = ExAllocatePool( NonPagedPool, 0x8000 );

    if( pCpu->pHostEsp == NULL )
    {
        KdPrint( ( "Ddvp-> Host Stack �ڴ����ʧ��!\n" ) );
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto __Exit;
    }

    RtlZeroMemory( pCpu->pHostEsp, 0x8000 );

    KdPrint( ( "Ddvp-> Host Stack ��ַ :%p !\n", pCpu->pHostEsp ) );
//---------------------------------------------------------------------------
    //
    // �����ΪHost Idt �����ڴ�
    //
//     VirtualAddress = MmAllocateNonCachedMemory( sizeof( IDT_ENTRY ) * 256 );
//     if ( VirtualAddress == NULL )
//     {
//         KdPrint( ( "Ddvp-> Host Idt Table �ڴ����ʧ��!\n" ) );
//         Status = STATUS_INSUFFICIENT_RESOURCES;
//         goto __Exit;
//     }
//
//     RtlZeroMemory( VirtualAddress, sizeof( IDT_ENTRY ) * 256 );
//
//     pCpu->HostIdtArea = VirtualAddress;
//
//     KdPrint( ( "Ddvp-> Host Idt ��ַ :%p !\n", pCpu->HostIdtArea ) );
//---------------------------------------------------------------------------
__Exit:

    if( !NT_SUCCESS( Status ) )
    {
        if( pCpu->pVMONVirtualAddress )
        {
            MmFreeNonCachedMemory( pCpu->pVMONVirtualAddress, 0x1000 );
            pCpu->pVMONVirtualAddress = NULL;

        }

        if( pCpu->pVMCSVirtualAddress )
        {
            MmFreeNonCachedMemory( pCpu->pVMCSVirtualAddress, 0x1000 );
            pCpu->pVMCSVirtualAddress = NULL;
        }

        if( pCpu->pIOBitmapVirtualAddressA )
        {
            MmFreeNonCachedMemory( pCpu->pIOBitmapVirtualAddressA, 0x1000 );
            pCpu->pIOBitmapVirtualAddressA = NULL;
        }

        if( pCpu->pIOBitmapVirtualAddressB )
        {
            MmFreeNonCachedMemory( pCpu->pIOBitmapVirtualAddressB, 0x1000 );
            pCpu->pIOBitmapVirtualAddressB = NULL;
        }

        if( pCpu->pMSRBitmapVirtualAddress )
        {
            MmFreeNonCachedMemory( pCpu->pMSRBitmapVirtualAddress, 0x1000 );
            pCpu->pMSRBitmapVirtualAddress = NULL;
        }

        if( pCpu->pHostEsp )
        {
            ExFreePool( pCpu->pHostEsp );
            pCpu->pHostEsp = NULL;
        }

//         if ( pCpu->HostIdtArea )
//         {
//             MmFreeNonCachedMemory( pCpu->HostIdtArea, sizeof( IDT_ENTRY ) * 256 );
//             pCpu->HostIdtArea = NULL;
//         }
    }

    return Status;
}

ULONG VmxAdjustControls( ULONG Ctl, ULONG Msr )
{
    LARGE_INTEGER MsrValue;

    MsrValue.QuadPart = ReadMsr( Msr );
    Ctl &= MsrValue.HighPart;     /* bit == 0 in high word ==> must be zero */
    Ctl |= MsrValue.LowPart;      /* bit == 1 in low word  ==> must be one  */
    return Ctl;
}

// ��GDTȡ����Ӧ�Ķ�������, ���������ǱȽ�ϰ�ߵĸ�ʽ��ź�
NTSTATUS InitializeSegmentSelector( PSEGMENT_SELECTOR SegmentSelector, USHORT Selector, ULONG GdtBase )
{
    PSEGMENT_DESCRIPTOR2 SegDesc;

    if( !SegmentSelector )
    {
        return STATUS_INVALID_PARAMETER;
    }

    //
    // �����ѡ���ӵ�T1 = 1��ʾ����LDT�е���, ����û��ʵ���������
    //
    if( Selector & 0x4 )
    {
        KdPrint( ( "Ddvp-> Ҫ�����Ķ���������LDT��, ʧ��!\n" ) );
        return STATUS_INVALID_PARAMETER;
    }

    //
    // ��GDT��ȡ��ԭʼ�Ķ�������
    //
    SegDesc = ( PSEGMENT_DESCRIPTOR2 )( ( PUCHAR ) GdtBase + ( Selector & ~0x7 ) );

    //
    // ��ѡ����
    //
    SegmentSelector->Selector = Selector;

    //
    // �λ�ַ15-39λ 55-63λ
    //
    SegmentSelector->Base = SegDesc->base0 | SegDesc->base1 << 16 | SegDesc->base2 << 24;

    //
    // ���޳�0-15λ  47-51λ, ������ȡ��
    //
    SegmentSelector->Limit = SegDesc->limit0 | ( SegDesc->limit1attr1 & 0xf ) << 16;

    //
    // ������39-47 51-55 ע��۲�ȡ��
    //
    SegmentSelector->Attributes.UCHARs = SegDesc->attr0 | ( SegDesc->limit1attr1 & 0xf0 ) << 4;

    //
    // �����ж����Ե�DTλ, �ж��Ƿ���ϵͳ�����������Ǵ������ݶ�������
    //
    if( !( SegDesc->attr0 & LA_STANDARD ) )
    {
        ULONG64 tmp;

        //
        // �����ʾ��ϵͳ��������������������, �о�����Ϊ64λ׼���İ�,
        // 32λ����λ�ַֻ��32λ��. �ѵ�64λ������ʲô������?
        //
        tmp = ( *( PULONG64 )( ( PUCHAR ) SegDesc + 8 ) );

        SegmentSelector->Base = ( SegmentSelector->Base & 0xffffffff ) | ( tmp << 32 );
    }

    //
    // ���Ƕν��޵�����λ, 1Ϊ4K. 0Ϊ1BYTE
    //
    if( SegmentSelector->Attributes.fields.g )
    {
        //
        // �������λΪ1, ��ô�ͳ���4K. ���ƶ�12λ
        //
        SegmentSelector->Limit = ( SegmentSelector->Limit << 12 ) + 0xfff;
    }

    return STATUS_SUCCESS;
}

// ��ȡ���������޳�
ULONG GetSegmentDescriptorLimit( ULONG gdt_base, USHORT selector )
{
    SEGMENT_SELECTOR SegmentSelector = { 0 };

    //
    // ��GDT���������������޳�
    //
    InitializeSegmentSelector( &SegmentSelector, selector, gdt_base );

    return SegmentSelector.Limit;
}

ULONG
GetSegmentDescriptorAR(
    ULONG gdt_base,
    USHORT selector
)
{
    SEGMENT_SELECTOR SegmentSelector = { 0 };
    ULONG uAccessRights;

    InitializeSegmentSelector( &SegmentSelector, selector, gdt_base );

    uAccessRights = ( ( PUCHAR ) & SegmentSelector.Attributes )[0] + \
                    ( ( ( PUCHAR ) & SegmentSelector.Attributes )[1] << 12 );

    if( !selector )
        uAccessRights |= 0x10000;

    return uAccessRights;
}


// ��ȡ������������
ULONG GetSegmentDescriptorAttributes( ULONG gdt_base, USHORT Selector )
{
    SEGMENT_SELECTOR SegmentSelector = { 0 };
    ULONG uAccessRights;

    //
    // ��GDT��������������Ҫ�Ķ�������
    //
    InitializeSegmentSelector( &SegmentSelector, Selector, gdt_base );

    //
    // �����������������ΪʲôҪ����, �ο�Intel �ֲ�24��Table 24-2
    //
    uAccessRights = ( ( PUCHAR ) & SegmentSelector.Attributes )[0] +
                    ( ( ( PUCHAR ) & SegmentSelector.Attributes )[1] << 12 );

    if( !Selector )
    {
        uAccessRights |= 0x10000;
    }

    return uAccessRights;
}

// ��ȡ����������ַ
ULONG GetSegmentDescriptorBase( ULONG gdt_base, USHORT seg_selector )
{
    ULONG Base = 0;
    SEGMENT_DESCRIPTOR	SegDescriptor = {0};

    //
    // ��GDT����������������
    //
    RtlCopyMemory( &SegDescriptor, ( ULONG * )( gdt_base + ( seg_selector >> 3 ) * 8 ), 8 );

    //
    // ���������ĸ�8λ
    //
    Base = SegDescriptor.BaseHi;
    Base <<= 8;

    //
    // ����������31-39λ
    //
    Base |= SegDescriptor.BaseMid;
    Base <<= 16;

    //
    // ���������� 15-31λ
    //
    Base |= SegDescriptor.BaseLo;

    return Base;
}

//
// ���ǲ���ӭ��#VMExit����. ����ָ��Ϳ���
//
VOID HandleUnimplemented( PCPU_VM_CONTEXT pCpu, ULONG ExitCode )
{
    ULONG InstructionLength;
    ULONG IInfo, AState;

    IInfo = ReadVMCS( GUEST_INTERRUPTIBILITY_INFO );
    AState = ReadVMCS( GUEST_ACTIVITY_STATE );

    InstructionLength = ReadVMCS( VM_EXIT_INSTRUCTION_LEN );

    WriteVMCS( GUEST_RIP, ReadVMCS( GUEST_RIP ) + InstructionLength );
}

//
// ִ��cpuidָ��,
//
VOID HandleCpuid( PGUEST_REGS GuestReg )
{
    ULONG Function, eax, ebx, ecx, edx;
    ULONG InstructionLength;

    Function = GuestReg->RegEax;

    ExecuteCpuId( Function, &eax, &ebx, &ecx, &edx );

    GuestReg->RegEax = eax;
    GuestReg->RegEcx = ecx;
    GuestReg->RegEdx = edx;
    GuestReg->RegEbx = ebx;

    if( Function == 'Joen' )
    {
        GuestReg->RegEax = 0x31313131;
        GuestReg->RegEcx = 0x32323232;
        GuestReg->RegEdx = 0x33333333;
        GuestReg->RegEbx = 0x34343434;
    }

    InstructionLength = ReadVMCS( VM_EXIT_INSTRUCTION_LEN );

    WriteVMCS( GUEST_RIP, ReadVMCS( GUEST_RIP ) + InstructionLength );
}

VOID HandleInvd( PCPU_VM_CONTEXT pCpu )
{
    ULONG InstructionLength;

    ExecuteInvd();

    InstructionLength = ReadVMCS( VM_EXIT_INSTRUCTION_LEN );
    WriteVMCS( GUEST_RIP, ReadVMCS( GUEST_RIP ) + InstructionLength );
}


//
// ʶ��vmcallָ���Ƿ������������ر��������
// �������� eax == 'Joen' ecx == 'Ddvp' edx == 'Exit' ebx = 'Quit'
// ���ұ���������vmcall ָ�������. vmcall ��������0xc1010f
//
VOID HandleVmCall( PCPU_VM_CONTEXT Cpu, PGUEST_REGS GuestReg )
{
    PIDT_ENTRY IdtBase;
    ULONG InstructionLength, Eip, Esp;

    Eip = ReadVMCS( GUEST_RIP );

    InstructionLength = ReadVMCS( VM_EXIT_INSTRUCTION_LEN );

    if( ( GuestReg->RegEax == 'Joen' ) &&
            ( GuestReg->RegEcx == 'Ddvp' ) &&
            ( GuestReg->RegEdx == 'Exit' ) &&
            ( GuestReg->RegEbx == 'Quit' ) &&
            *( ULONG* )Eip ==  0x90c1010f )
    {

        __asm
        {
            xchg	sp, sp
            mov		eax, GUEST_IDTR_BASE
        }

        IdtBase = ( PIDT_ENTRY )ReadVMCS( GUEST_IDTR_BASE );

        IdtBase[Cpu->Interrupt1Index].Type = 0;
        IdtBase[Cpu->Interrupt3Index].Type = 0;

        //
        //  Set the EIP and call VMXOFF
        //
        Eip = ( ULONG )&GuestExitPoint;
        Esp = GuestReg->RegEsp;

        ExecuteVmxOff( Esp, Eip );
    }
    else
    {
        WriteVMCS( GUEST_RIP, Eip + InstructionLength );
    }
}

//
// �����ִ��vmxָ��, ��ô����ʧ��!
//
VOID HandleVmInstruction( PCPU_VM_CONTEXT pCpu )
{
    ULONG InstructionLength;

    WriteVMCS( GUEST_RFLAGS, ReadVMCS( GUEST_RFLAGS ) | 0x1 );

    InstructionLength = ReadVMCS( VM_EXIT_INSTRUCTION_LEN );
    WriteVMCS( GUEST_RIP, ReadVMCS( GUEST_RIP ) + InstructionLength );
}

// Msr �Ĵ�������.
VOID HandleMsrRead( PCPU_VM_CONTEXT Cpu, PGUEST_REGS GuestReg )
{
    ULONG RegEcx;
    LARGE_INTEGER Msr;
    ULONG InstructionLength;
    ULONG_PTR GuestCr3 = ReadVMCS( GUEST_CR3 );

    RegEcx = GuestReg->RegEcx;

    if( RegEcx == MSR_IA32_SYSENTER_EIP )
    {
        Msr.QuadPart = ( LONGLONG )Cpu->OldKiFastCallEntry;

        KdPrint( ( "Ddvp-> Cr3 %p Read MSR_IA32_SYSENTER_EIP Register %p !\n",
                   GuestCr3, Cpu->OldKiFastCallEntry ) );
    }
    else
    {
        Msr.QuadPart = ReadMsr( RegEcx );
    }

    GuestReg->RegEax = Msr.LowPart;
    GuestReg->RegEdx = Msr.HighPart;

    InstructionLength = ReadVMCS( VM_EXIT_INSTRUCTION_LEN );
    WriteVMCS( GUEST_RIP, ReadVMCS( GUEST_RIP ) + InstructionLength );
}

// MSR �Ĵ���д��.
VOID HandleMsrWrite( PCPU_VM_CONTEXT Cpu, PGUEST_REGS GuestReg )
{
    LARGE_INTEGER Msr;
    ULONG InstructionLength;
    ULONG_PTR GuestCr3 = ReadVMCS( GUEST_CR3 );

    if( GuestReg->RegEcx == MSR_IA32_SYSENTER_EIP )
    {
        Msr.LowPart  = GuestReg->RegEax;
        Msr.HighPart = GuestReg->RegEdx;

        Cpu->OldKiFastCallEntry = ( PVOID )Msr.QuadPart;

        KdPrint( ( "Ddvp-> Cr3 %p Write MSR_IA32_SYSENTER_EIP Register %p !\n",
                   GuestCr3, Cpu->OldKiFastCallEntry ) );
    }
    else
    {
        WriteMsr( GuestReg->RegEcx, GuestReg->RegEax, GuestReg->RegEdx );
    }

    InstructionLength = ReadVMCS( VM_EXIT_INSTRUCTION_LEN );
    WriteVMCS( GUEST_RIP, ReadVMCS( GUEST_RIP ) + InstructionLength );
}

ULONG EipArray[10];

// Cr�Ĵ�������
VOID HandleCrAccess( PGUEST_REGS GuestReg )
{
    ULONG Exit;
    ULONG Cr;
    ULONG Reg;
    // DBG_ITEM* DbgItem;
    ULONG InstructionLength;
    PMOV_CR_QUALIFICATION pExitQualification;
    ULONG GuestEip = ReadVMCS( GUEST_RIP );

    Exit = ReadVMCS( EXIT_QUALIFICATION );
    pExitQualification = ( PMOV_CR_QUALIFICATION )&Exit;

    if( 0 == pExitQualification->ControlRegister )
    {
        Cr = ReadVMCS( GUEST_CR0 );

    }
    else if( 3 == pExitQualification->ControlRegister )
    {
        Cr = ReadVMCS( GUEST_CR3 );
    }
    else if( 4 == pExitQualification->ControlRegister )
    {
        Cr = ReadVMCS( GUEST_CR4 );
    }

    if( 0 == pExitQualification->Register )
    {
        Reg = GuestReg->RegEax;
    }
    else if( 1 == pExitQualification->Register )
    {
        Reg = GuestReg->RegEcx;
    }
    else if( 2 == pExitQualification->Register )
    {
        Reg = GuestReg->RegEdx;
    }
    else if( 3 == pExitQualification->Register )
    {
        Reg = GuestReg->RegEbx;
    }
    else if( 4 == pExitQualification->Register )
    {
        Reg = GuestReg->RegEsp;
    }
    else if( 5 == pExitQualification->Register )
    {
        Reg = GuestReg->RegEbp;
    }
    else if( 6 == pExitQualification->Register )
    {
        Reg = GuestReg->RegEsi;
    }
    else if( 7 == pExitQualification->Register )
    {
        Reg = GuestReg->RegEdi;
    }

//---------------------------------------------------------------------------
    if( 0 == pExitQualification->AccessType )
    {
        // MOV_TO_CR
        if( 0 == pExitQualification->ControlRegister )
        {
            WriteVMCS( GUEST_CR0, Reg );
        }
        else if( 3 == pExitQualification->ControlRegister )
        {
//             DbgItem = DbgItemCr3FindItem( Reg, 0 );
//             if( DbgItem )
//             {
//                 DbgItemDeRefItem( DbgItem );
// 
// 				if (GuestEip != 0x80546b6c )
// 				{
// 					KdPrint( ( "Ddvp-> Write Of Debugger Cr3 %p Eip %p\n", Reg, GuestEip ) );
// 				}
//             }
			WriteVMCS( GUEST_CR3, Reg );
        }
        else if( 4 == pExitQualification->ControlRegister )
        {
            WriteVMCS( GUEST_CR4, Reg );
        }
    }
    else if( 1 == pExitQualification->AccessType )
    {
        if( 3 == pExitQualification->ControlRegister )
        {
//             DbgItem = DbgItemCr3FindItem( Cr, 0 );
//             if( DbgItem )
//             {
//                 DbgItemDeRefItem( DbgItem );
// 
//                 for( i = 0; i < sizeof( EipArray )/ sizeof( ULONG ); i++ )
//                 {
//                     if( EipArray[i] == GuestEip )
//                     {
//                         goto _End2;
//                     }
//                 }
// 
//                 KdPrint( ( "Ddvp-> Read Debugger Of Cr3 %p Eip %p\n", Cr, GuestEip ) );
// 
//                 for( i = 0; i < sizeof( EipArray )/ sizeof( ULONG ); i++ )
//                 {
//                     if( EipArray[i] == 0 )
//                     {
//                         EipArray[i] =GuestEip;
//                         break;
//                     }
//                 }
// 
//             }
//             _End2:
//             ;
        }

        // MOV_FROM_CR
        if( 0 == pExitQualification->Register )
        {
            GuestReg->RegEax = Cr;
        }
        else if( 1 == pExitQualification->Register )
        {
            GuestReg->RegEcx = Cr;
        }
        else if( 2 == pExitQualification->Register )
        {
            GuestReg->RegEdx = Cr;
        }
        else if( 3 == pExitQualification->Register )
        {
            GuestReg->RegEbx = Cr;
        }
        else if( 4 == pExitQualification->Register )
        {
            GuestReg->RegEsp = Cr;
        }
        else if( 5 == pExitQualification->Register )
        {
            GuestReg->RegEbp = Cr;
        }
        else if( 6 == pExitQualification->Register )
        {
            GuestReg->RegEsi = Cr;
        }
        else if( 7 == pExitQualification->Register )
        {
            GuestReg->RegEdi = Cr;
        }
    }

    InstructionLength = ReadVMCS( VM_EXIT_INSTRUCTION_LEN );
    WriteVMCS( GUEST_RIP, ReadVMCS( GUEST_RIP ) + InstructionLength );
}

VOID HandleDrAccess( PCPU_VM_CONTEXT Cpu, PGUEST_REGS GuestReg )
{
    ULONG Exit;
    ULONG Dr;
    ULONG Reg;
    ULONG Temp;
    ULONG InstructionLength;
    ULONG GuestCr3;
    PMOV_DR_QUALIFICATION pExitQualification;

    ULONG RegDr0, RegDr1, RegDr2, RegDr3, RegDr6, RegDr7;

    GuestCr3 = ReadVMCS( GUEST_CR3 );

    Exit = ReadVMCS( EXIT_QUALIFICATION );
    pExitQualification = ( PMOV_DR_QUALIFICATION )&Exit;

    // 0 = mov to Dr, 1= mov from Dr
    if( 1 == pExitQualification->DirectionAccess )
    {

        if( 0 == pExitQualification->DebugRegIndex )
        {
            __asm
            {
                mov		eax, dr0
                mov		Dr, eax
            }
        }
        else if( 1 == pExitQualification ->DebugRegIndex )
        {
            __asm
            {
                mov		eax, dr1
                mov		Dr, eax
            }
        }
        else if( 2 == pExitQualification ->DebugRegIndex )
        {
            __asm
            {
                mov		eax, dr2
                mov		Dr, eax
            }
        }
        else if( 3 == pExitQualification ->DebugRegIndex )
        {
            __asm
            {
                mov		eax, dr3
                mov		Dr, eax
            }
        }
        else if( 4 == pExitQualification ->DebugRegIndex )
        {
            __asm
            {
                xchg		sp, sp
            }
        }
        else if( 5 == pExitQualification ->DebugRegIndex )
        {
            __asm
            {
                xchg		sp, sp
            }
        }
        else if( 6 == pExitQualification ->DebugRegIndex )
        {
            __asm
            {
                mov		eax, dr6
                mov		Dr, eax
            }
        }
        else if( 7 == pExitQualification ->DebugRegIndex )
        {
            Dr = ReadVMCS( GUEST_DR7 );
        }

        if( 0 == pExitQualification->GeneralReg )
        {
            GuestReg->RegEax = Dr;
        }
        else if( 1 == pExitQualification->GeneralReg )
        {
            GuestReg->RegEcx = Dr;
        }
        else if( 2 == pExitQualification->GeneralReg )
        {
            GuestReg->RegEdx = Dr;
        }
        else if( 3 == pExitQualification->GeneralReg )
        {
            GuestReg->RegEbx = Dr;
        }
        else if( 4 == pExitQualification->GeneralReg )
        {
            GuestReg->RegEsp = Dr;
        }
        else if( 5 == pExitQualification->GeneralReg )
        {
            GuestReg->RegEbp = Dr;
        }
        else if( 6 == pExitQualification->GeneralReg )
        {
            GuestReg->RegEsi = Dr;
        }
        else if( 7 == pExitQualification->GeneralReg )
        {
            GuestReg->RegEdi = Dr;
        }
    }
    else
    {
        if( 0 == pExitQualification->GeneralReg )
        {
            Reg = GuestReg->RegEax;
        }
        else if( 1 == pExitQualification->GeneralReg )
        {
            Reg = GuestReg->RegEcx;
        }
        else if( 2 == pExitQualification->GeneralReg )
        {
            Reg = GuestReg->RegEdx;
        }
        else if( 3 == pExitQualification->GeneralReg )
        {
            Reg = GuestReg->RegEbx;
        }
        else if( 4 == pExitQualification->GeneralReg )
        {
            Reg = GuestReg->RegEsp;
        }
        else if( 5 == pExitQualification->GeneralReg )
        {
            Reg = GuestReg->RegEbp;
        }
        else if( 6 == pExitQualification->GeneralReg )
        {
            Reg = GuestReg->RegEsi;
        }
        else if( 7 == pExitQualification->GeneralReg )
        {
            Reg = GuestReg->RegEdi;
        }

        if( 0 == pExitQualification->DebugRegIndex )
        {
            __asm
            {
                mov		eax, Reg
                mov		dr0, eax
            }
        }
        else if( 1 == pExitQualification->DebugRegIndex )
        {
            __asm
            {
                mov		eax, Reg
                mov		dr1, eax
            }
        }
        else if( 2 == pExitQualification->DebugRegIndex )
        {
            __asm
            {
                mov		eax, Reg
                mov		dr2, eax
            }
        }
        else if( 3 == pExitQualification->DebugRegIndex )
        {
            __asm
            {
                mov		eax, Reg
                mov		dr3, eax
            }
        }
        else if( 4 == pExitQualification->DebugRegIndex )
        {
            __asm
            {
                xchg		sp, sp
            }
        }
        else if( 5 == pExitQualification->DebugRegIndex )
        {
            __asm
            {
                xchg		sp, sp
            }
        }
        else if( 6 == pExitQualification->DebugRegIndex )
        {
            __asm
            {
                mov		eax, Reg
                mov		dr6, eax
            }
        }
        else if( 7 == pExitQualification->DebugRegIndex )
        {
            WriteVMCS( GUEST_DR7, Reg );
        }
    }

    __asm
    {
        mov		eax, fs:[0x124]
        mov		Temp, eax

        mov		eax, dr0
        mov		RegDr0, eax
        mov		eax, dr1
        mov		RegDr1, eax
        mov		eax, dr2
        mov		RegDr1, eax
        mov		eax, dr3
        mov		RegDr3, eax
        mov		eax, dr6
        mov		RegDr6, eax
    }

    RegDr7 = ReadVMCS( GUEST_DR7 );

    // 0 = mov to Dr, 1= mov from Dr
    KdPrint( ( "Ddvp-> Dr :%5s Cr3:%p Thread:%p \n",
               pExitQualification->DirectionAccess ? "Read" : "Write",
               GuestCr3, Temp ) );

    KdPrint( ( "Ddvp-> Dr0:%p Dr1:%p Dr2:%p Dr3:%p Dr6:%p Dr7:%p \n",
               RegDr0, RegDr1, RegDr2, RegDr3, RegDr6, RegDr7 ) );

    InstructionLength = ReadVMCS( VM_EXIT_INSTRUCTION_LEN );
    WriteVMCS( GUEST_RIP, ReadVMCS( GUEST_RIP ) + InstructionLength );
}

static void VmxInternalHvmInjectException( ULONG type, ULONG trap, ULONG error_code )
{
    ULONG v;

    //
    // Write the VM-entry interruption-information field
    //
    v = ( INTR_INFO_VALID_MASK | trap | type );

    //
    // Check if bits 11 (deliver code) and 31 (valid) are set. In this
    // case, error code has to be delivered to guest OS
    //
    if( error_code != HVM_DELIVER_NO_ERROR_CODE )
    {
        WriteVMCS( VM_ENTRY_EXCEPTION_ERROR_CODE, error_code );
        v |= INTR_INFO_DELIVER_CODE_MASK;
    }

    WriteVMCS( VM_ENTRY_INTR_INFO_FIELD, v );
}

static void VmxHvmInjectHwException( ULONG trap, ULONG error_code )
{
    VmxInternalHvmInjectException( INTR_TYPE_HW_EXCEPTION, trap, error_code );
}

// ������쳣
VOID HandleException( PCPU_VM_CONTEXT pCpu )
{
	
}

//
// ��������о���Windows�Ĵ��ڹ��̺�����. ������Ϣ�����ﴦ��
// �����Ϣ������ʱʱ������.
//
VOID HandleVmExit( PCPU_VM_CONTEXT pCpu, PGUEST_REGS GuestReg )
{
    ULONG ExitCode;

    //
    // �޸�һ��HOST����IDT Limit
    //
    SetIdtr( ReadVMCS( HOST_IDTR_BASE ), 0x7FF );

    //
    // �л�������#VmExit�Ľ��̿ռ�
    //
    // AttachGuestProcess();

    //
    // �ѿͻ���esp�Ĵ��������
    //
    GuestReg->RegEsp = ReadVMCS( GUEST_RSP );

    //
    // �˳�VM��ԭ��
    //
    ExitCode = ReadVMCS( VM_EXIT_REASON );
//---------------------------------------------------------------------------

    if( ExitCode == EXIT_REASON_EXCEPTION_NMI )
    {

        //
        // �ж��쳣����, ���ﲻ��Ҫ����Guest.Eip
        //
        HandleException( pCpu );
    }
    else if( ExitCode == EXIT_REASON_EXTERNAL_INTERRUPT ||
             ExitCode == EXIT_REASON_TRIPLE_FAULT ||
             ExitCode == EXIT_REASON_INIT ||
             ExitCode == EXIT_REASON_SIPI ||
             ExitCode == EXIT_REASON_IO_SMI ||
             ExitCode == EXIT_REASON_OTHER_SMI ||
             ExitCode == EXIT_REASON_PENDING_INTERRUPT ||
             ExitCode == EXIT_REASON_TASK_SWITCH )
    {
        HandleUnimplemented( pCpu, ExitCode );
    }
    else if( ExitCode == EXIT_REASON_CPUID )
    {
        //
        // ִ��cpuidָ��
        //
        HandleCpuid( GuestReg );
    }
    else if( ExitCode == EXIT_REASON_HLT )
    {
        HandleUnimplemented( pCpu, ExitCode );
    }
    else if( ExitCode == EXIT_REASON_INVD )
    {
        HandleInvd( pCpu );
    }
    else if( ExitCode == EXIT_REASON_INVLPG ||
             ExitCode == EXIT_REASON_RDPMC ||
             ExitCode == EXIT_REASON_RDTSC ||
             ExitCode == EXIT_REASON_RSM )
    {
        HandleUnimplemented( pCpu, ExitCode );
    }
    else if( ExitCode == EXIT_REASON_VMCALL )
    {
        //
        // #VMExit Off Vmx
        //
        HandleVmCall( pCpu, GuestReg );
    }
    else if( ExitCode == EXIT_REASON_VMCLEAR ||
             ExitCode == EXIT_REASON_VMLAUNCH ||
             ExitCode == EXIT_REASON_VMPTRLD ||
             ExitCode == EXIT_REASON_VMPTRST ||
             ExitCode == EXIT_REASON_VMREAD ||
             ExitCode == EXIT_REASON_VMRESUME ||
             ExitCode == EXIT_REASON_VMWRITE ||
             ExitCode == EXIT_REASON_VMXOFF ||
             ExitCode == EXIT_REASON_VMXON )
    {
        //
        // Exe Vmx Instruction
        //
        HandleVmInstruction( pCpu );
    }
    else if( ExitCode == EXIT_REASON_CR_ACCESS )
    {
        //
        // Cr �Ĵ���������д��
        //
        HandleCrAccess( GuestReg );
    }
    else if( ExitCode == EXIT_REASON_DR_ACCESS )
    {
        //
        // Dr �Ĵ���������д��
        //
        HandleDrAccess( pCpu, GuestReg );
    }
    else if( ExitCode == EXIT_REASON_IO_INSTRUCTION )
    {
        __asm
        {
            xchg	sp, sp
            mov		eax, EXIT_REASON_IO_INSTRUCTION
        }
        // HandleIoAccess( pCpu );
    }
    else if( ExitCode == EXIT_REASON_MSR_READ )
    {
        //
        // MSR �Ĵ�������
        //
        HandleMsrRead( pCpu, GuestReg );
    }
    else if( ExitCode == EXIT_REASON_MSR_WRITE )
    {
        //
        // MSR�Ĵ���д��
        //
        HandleMsrWrite( pCpu, GuestReg );
    }
    else if( ExitCode == EXIT_REASON_INVALID_GUEST_STATE ||
             ExitCode == EXIT_REASON_MSR_LOADING ||
             ExitCode == EXIT_REASON_MWAIT_INSTRUCTION ||
             ExitCode == EXIT_REASON_MONITOR_INSTRUCTION ||
             ExitCode == EXIT_REASON_PAUSE_INSTRUCTION ||
             ExitCode == EXIT_REASON_MACHINE_CHECK ||
             ExitCode == EXIT_REASON_TPR_BELOW_THRESHOLD )
    {
        HandleUnimplemented( pCpu, ExitCode );
    }
    else
    {
        HandleUnimplemented( pCpu, ExitCode );
    }

    //
    // дesp���ͻ���,esp���ܻ����,��CrAccess�ǲ���
    //
    WriteVMCS( GUEST_RSP, GuestReg->RegEsp );

}

void CmSetBit32( ULONG* dword, ULONG bit )
{
    ULONG mask = ( 1 << bit );
    *dword = *dword | mask;
}

void CmClearBit32( ULONG* dword, ULONG bit )
{
    ULONG mask = 0xFFFFFFFF;
    ULONG sub = ( 1 << bit );
    mask = mask - sub;
    *dword = *dword & mask;
}

//
// ����VTʱ����дVMCS�ṹ
//
NTSTATUS SetupVMCS( PCPU_VM_CONTEXT pCpu, PVOID GuestEsp )
{
    ULONG64	Msr;
    ULONG_PTR Cr0;
    ULONG GdtBase;
    ULONG ExceptionBitmap;
    PHYSICAL_ADDRESS VMCSPhysicalAddress;

    if( !pCpu || !pCpu->pVMCSVirtualAddress )
    {
        return STATUS_INVALID_PARAMETER;
    }

    VMCSPhysicalAddress = pCpu->pVMCSPyhicalAddress;

    //
    // ִ�� Vmxclean �����, �� vmx_ptrld �󶨵���ǰProcessor
    //
    ExecuteVmClear( VMCSPhysicalAddress.LowPart, VMCSPhysicalAddress.HighPart );
    ExecuteVmPtrLd( VMCSPhysicalAddress.LowPart, VMCSPhysicalAddress.HighPart );

    //
    // ���VMCS��ƫ��4��VMX�˳�ԭ��ָʾ��(VMX-abort Indicator )
    //
    RtlZeroMemory( ( PULONG )pCpu->pVMCSVirtualAddress + 1, 4 );
//---------------------------------------------------------------------------
    /*
     * �����ǰ���Intel �ֲ�������VMCS����������ɲ���������д
     * ��ϸ��Ϣ�ο�[������־2].VMCS�������ܹ�6����ɲ���. �ֱ���
     *
     * 1.�ͻ���״̬��(Guest State Area)
     *
     * 2.������״̬��(Host State Area )
     *
     * 3.��������п�����( VM-Execuction Control Fields )
     *
     * 4.VMEntry��Ϊ������( VM-Entry Control Fields )
     *
     * 5.VMExit��Ϊ������( VM-Exit Control Fields )
     *
     * 6.VMExit�����Ϣ��( VM-Exit Information Fields )(ֻ��)
     */
//---------------------------------------------------------------------------
    //
    // ��д1.�ͻ���״̬��(Guest State Area)
    //

    // ��дGuest ���ƼĴ���״̬
    Cr0 = GetCr0();
    CmSetBit32( &Cr0, 0 );	// PE
    CmSetBit32( &Cr0, 5 );	// NE
    CmSetBit32( &Cr0, 31 );	// PG
    WriteVMCS( GUEST_CR0, Cr0 );
    WriteVMCS( GUEST_CR3, GetCr3() );
    WriteVMCS( GUEST_CR4, GetCr4() );

    // ��дDR7�Ĵ���
    WriteVMCS( GUEST_DR7, 0x400 );

    // ��дGuest Esp, Eip, Eflags
    WriteVMCS( GUEST_RSP, ( ULONG ) GuestEsp );

    //
    // GUEST_RIP ָ��ĵ�ַ, �ڿ���VT�Ժ�ʹ����￪ʼ���е�
    //
    WriteVMCS( GUEST_RIP, ( ULONG )&GuestEntryPoint );
    WriteVMCS( GUEST_RFLAGS, GetEflags() );

    // ��дGuest������ѡ���� CS, SS, DS, ES, FS, GS, LDTR, TR
    WriteVMCS( GUEST_CS_SELECTOR, GetCsSelector() & 0xfff8 );
    WriteVMCS( GUEST_SS_SELECTOR, GetSsSelector() & 0xfff8 );
    WriteVMCS( GUEST_DS_SELECTOR, GetDsSelector() & 0xfff8 );
    WriteVMCS( GUEST_ES_SELECTOR, GetEsSelector() & 0xfff8 );
    WriteVMCS( GUEST_FS_SELECTOR, GetFsSelector() & 0xfff8 );
    WriteVMCS( GUEST_GS_SELECTOR, GetGsSelector() & 0xfff8 );
    WriteVMCS( GUEST_LDTR_SELECTOR, GetLdtrSelector() & 0xfff8 );
    WriteVMCS( GUEST_TR_SELECTOR, GetTrSelector() & 0xfff8 );

    // ��дGuest����������ַ
    GdtBase = GetGdtBase();
    WriteVMCS( GUEST_CS_BASE, GetSegmentDescriptorBase( GdtBase, GetCsSelector() ) );
    WriteVMCS( GUEST_SS_BASE, GetSegmentDescriptorBase( GdtBase, GetSsSelector() ) );
    WriteVMCS( GUEST_DS_BASE, GetSegmentDescriptorBase( GdtBase, GetDsSelector() ) );
    WriteVMCS( GUEST_ES_BASE, GetSegmentDescriptorBase( GdtBase, GetEsSelector() ) );
    WriteVMCS( GUEST_FS_BASE, GetSegmentDescriptorBase( GdtBase, GetFsSelector() ) );
    WriteVMCS( GUEST_GS_BASE, GetSegmentDescriptorBase( GdtBase, GetGsSelector() ) );
    WriteVMCS( GUEST_LDTR_BASE, GetSegmentDescriptorBase( GdtBase, GetLdtrSelector() ) );
    WriteVMCS( GUEST_TR_BASE, GetSegmentDescriptorBase( GdtBase, GetTrSelector() ) );

    // ��дGuest���������޳�
    WriteVMCS( GUEST_CS_LIMIT, GetSegmentDescriptorLimit( GdtBase, GetCsSelector() ) );
    WriteVMCS( GUEST_SS_LIMIT, GetSegmentDescriptorLimit( GdtBase, GetSsSelector() ) );
    WriteVMCS( GUEST_DS_LIMIT, GetSegmentDescriptorLimit( GdtBase, GetDsSelector() ) );
    WriteVMCS( GUEST_ES_LIMIT, GetSegmentDescriptorLimit( GdtBase, GetEsSelector() ) );
    WriteVMCS( GUEST_FS_LIMIT, GetSegmentDescriptorLimit( GdtBase, GetFsSelector() ) );
    WriteVMCS( GUEST_GS_LIMIT, GetSegmentDescriptorLimit( GdtBase, GetGsSelector() ) );
    WriteVMCS( GUEST_LDTR_LIMIT, GetSegmentDescriptorLimit( GdtBase, GetLdtrSelector() ) );
    WriteVMCS( GUEST_TR_LIMIT, GetSegmentDescriptorLimit( GdtBase, GetTrSelector() ) );

    // ��дGuest������������
    WriteVMCS( GUEST_CS_AR_BYTES, GetSegmentDescriptorAttributes( GdtBase, GetCsSelector() ) );
    WriteVMCS( GUEST_SS_AR_BYTES, GetSegmentDescriptorAttributes( GdtBase, GetSsSelector() ) );
    WriteVMCS( GUEST_DS_AR_BYTES, GetSegmentDescriptorAttributes( GdtBase, GetDsSelector() ) );
    WriteVMCS( GUEST_ES_AR_BYTES, GetSegmentDescriptorAttributes( GdtBase, GetEsSelector() ) );
    WriteVMCS( GUEST_FS_AR_BYTES, GetSegmentDescriptorAttributes( GdtBase, GetFsSelector() ) );
    WriteVMCS( GUEST_GS_AR_BYTES, GetSegmentDescriptorAttributes( GdtBase, GetGsSelector() ) );
    WriteVMCS( GUEST_LDTR_AR_BYTES, GetSegmentDescriptorAttributes( GdtBase, GetLdtrSelector() ) );
    WriteVMCS( GUEST_TR_AR_BYTES, GetSegmentDescriptorAttributes( GdtBase, GetTrSelector() ) );

    // ��дGuest GDTR/IDTR��ַ
    WriteVMCS( GUEST_GDTR_BASE, GetGdtBase() );
    WriteVMCS( GUEST_IDTR_BASE, GetIdtBase() );

    // ��дGuest GDTR/IDTR �޳�
    WriteVMCS( GUEST_GDTR_LIMIT, GetGdtLimit() );
    WriteVMCS( GUEST_IDTR_LIMIT, GetIdtLimit() );

    // ��дGuest MSR IA32_DEBUGCTL �Ĵ���ֵ
    Msr = ReadMsr( MSR_IA32_DEBUGCTL );
    WriteVMCS( GUEST_IA32_DEBUGCTL, ( ULONG )( Msr & 0xFFFFFFFF ) );
    WriteVMCS( GUEST_IA32_DEBUGCTL_HIGH, ( ULONG )( Msr >> 32 ) );

    // ��дGuest MSR IA32_SYSENTER_CS
    Msr = ReadMsr( MSR_IA32_SYSENTER_CS );
    WriteVMCS( GUEST_SYSENTER_CS, ( ULONG )( Msr & 0xFFFFFFFF ) );

    // ��дGuest MSR IA32_SYSENTER_ESP
    Msr = ReadMsr( MSR_IA32_SYSENTER_ESP );
    WriteVMCS( GUEST_SYSENTER_ESP, ( ULONG )( Msr & 0xFFFFFFFF ) );

    // ��дGuest MSR IA32_SYSENTER_EIP
    pCpu->OldKiFastCallEntry = ( PVOID )ReadMsr( MSR_IA32_SYSENTER_EIP );

    WriteVMCS( GUEST_SYSENTER_EIP, ( ULONG )( &NewKiFastCallEntry ) );

    KdPrint( ( "Ddvp-> OldKiFastCallEntry %p NewKiFastCallEntry %p !\n",
               pCpu->OldKiFastCallEntry, NewKiFastCallEntry ) );

    //
    // ��Ȼ����������Է��� IA32_PERF_GLOBAL_CTRL(���ܼ���), IA32_PAT,
    // IA32_EFER(����64λҪ��) �⼸��MSR�Ĵ���û����д.
    //
//===========================================================================

    //
    // ��д2.������״̬��(Host State Area )
    //

    // Guest���״̬
    WriteVMCS( GUEST_ACTIVITY_STATE, 0 );

    // Guest�ж�����״̬
    WriteVMCS( GUEST_INTERRUPTIBILITY_INFO, 0 );

    // VMCS����ָ��(����)
    WriteVMCS( VMCS_LINK_POINTER, 0xFFFFFFFF );
    WriteVMCS( VMCS_LINK_POINTER_HIGH, 0xFFFFFFFF );

    //
    // �����Ƴٵ����쳣, VMX��ռ��ʱ��, PDPTE�û����д
    //
//===========================================================================

    //
    // ��д2.������״̬��(Host State Area )
    //

    // ���ƼĴ��� CR0, CR3, CR4
    WriteVMCS( HOST_CR0, GetCr0() );
    WriteVMCS( HOST_CR3, GetCr3() );
    WriteVMCS( HOST_CR4, GetCr4() );

    // ����Host ESP, EIP
    WriteVMCS( HOST_RSP, ( ULONG )( ( PUCHAR ) pCpu->pHostEsp + 0x7000 ) );

    KdPrint( ( "Ddvp-> HostEsp:%p HostEsp+0x7000:%p!\n",
               pCpu->pHostEsp, ( PUCHAR )pCpu->pHostEsp + 0x7000 ) );

    //�� VIRT_CPU �ṹ�帴�Ƶ�HOST��ջ��ȥ
    RtlMoveMemory( ( PUCHAR )pCpu->pHostEsp + 0x7000, pCpu, sizeof( CPU_VM_CONTEXT ) );

    pCpu = ( PCPU_VM_CONTEXT )( ( PUCHAR )pCpu->pHostEsp + 0x7000 );

    //
    // ���ֵ�ǳ���Ҫ��. ����#VMExit�¼���ʱ��, ��������
    //
    WriteVMCS( HOST_RIP, ( ULONG ) ExitEventHandler );


    // ��дHost CS, SS, DS, ES, FS, GS��TR�Ĵ���
    WriteVMCS( HOST_CS_SELECTOR, GetCsSelector() & 0xfff8 );
    WriteVMCS( HOST_SS_SELECTOR, GetSsSelector() & 0xfff8 );
    WriteVMCS( HOST_DS_SELECTOR, GetDsSelector() & 0xfff8 );
    WriteVMCS( HOST_ES_SELECTOR, GetEsSelector() & 0xfff8 );
    WriteVMCS( HOST_FS_SELECTOR, GetFsSelector() & 0xfff8 );
    WriteVMCS( HOST_GS_SELECTOR, GetGsSelector() & 0xfff8 );
    WriteVMCS( HOST_TR_SELECTOR, GetTrSelector() & 0xfff8 );

    // ��д Host FS, GS and TR ��ַ
    WriteVMCS( HOST_FS_BASE, GetSegmentDescriptorBase( GdtBase, GetFsSelector() ) );
    WriteVMCS( HOST_GS_BASE, GetSegmentDescriptorBase( GdtBase, GetGsSelector() ) );
    WriteVMCS( HOST_TR_BASE, GetSegmentDescriptorBase( GdtBase, GetTrSelector() ) );


    // ��дHost GDTR/IDTR base
    WriteVMCS( HOST_GDTR_BASE, GetGdtBase() );
    WriteVMCS( HOST_IDTR_BASE, GetIdtBase() );

    // ��ʼ�� Guest �� Idt
    // InitVMMIDT( pCpu, ( PIDT_ENTRY )pCpu->HostIdtArea );

    // ��д Host IA32_SYSENTER_ESP/EIP/CS
    Msr = ReadMsr( MSR_IA32_SYSENTER_ESP );
    WriteVMCS( HOST_IA32_SYSENTER_ESP, ( ULONG )( Msr & 0xFFFFFFFF ) );

    Msr = ReadMsr( MSR_IA32_SYSENTER_EIP );
    WriteVMCS( HOST_IA32_SYSENTER_EIP, ( ULONG )( Msr & 0xFFFFFFFF ) );

    Msr = ReadMsr( MSR_IA32_SYSENTER_CS );
    WriteVMCS( HOST_IA32_SYSENTER_CS, ( ULONG )( Msr & 0xFFFFFFFF ) );

    //
    // IA32_PERF_GLOBAL_CTRL, IA32_PAT, IA32_EFER û����д
    //
//===========================================================================
    //
    // ��д 3.��������п�����( VM-Execuction Control Fields )
    //

    // ��д 3.1 ������ŵ������ִ�п��� Pin-based VM-execution controls
    WriteVMCS( PIN_BASED_VM_EXEC_CONTROL,
               VmxAdjustControls( 0, MSR_IA32_VMX_PINBASED_CTLS ) );

    // ��д 3.2 ���ڴ������������ִ�п��� Primary processor-based VM-execution controls
    // ����������쳣λͼ, DR�Ĵ�������, ����MSR�����Ƿ����#VMExit. ����Ҫ.
    WriteVMCS( CPU_BASED_VM_EXEC_CONTROL,
               VmxAdjustControls(
                   CPU_BASED_ACTIVATE_MSR_BITMAP |
                   CPU_BASED_ACTIVATE_IO_BITMAP
                   /*CPU_BASED_MOV_DR_EXITING*/,
                   MSR_IA32_VMX_PROCBASED_CTLS ) );

    // ��д 3.3 Exception bitmap �����쳣λͼ, ������������˶ϵ��쳣�͵����쳣
    ExceptionBitmap = 0;
    ExceptionBitmap |= 1 << DEBUG_EXCEPTION;
    ExceptionBitmap |= 1 << BREAKPOINT_EXCEPTION;

    //ExceptionBitmap |= 1<< PAGE_FAULT_EXCEPTION ���Ǹ���Ȥ���쳣
    WriteVMCS( EXCEPTION_BITMAP, ExceptionBitmap );

    // ����ҳ����
    WriteVMCS( PAGE_FAULT_ERROR_CODE_MASK, 0 );
    WriteVMCS( PAGE_FAULT_ERROR_CODE_MATCH, 0 );

    //
    // ��д 3.4 I/O bitmap IOλͼ, �������������60�Ŷ˿�.. Ҳ���Ǽ�������
    //
    WriteVMCS( IO_BITMAP_A_HIGH, pCpu->pIOBitmapPyhicalAddressA.HighPart );
    WriteVMCS( IO_BITMAP_A,      pCpu->pIOBitmapPyhicalAddressA.LowPart );
    WriteVMCS( IO_BITMAP_B_HIGH, pCpu->pIOBitmapPyhicalAddressB.HighPart );
    WriteVMCS( IO_BITMAP_B,      pCpu->pIOBitmapPyhicalAddressB.LowPart );

    // ioPort = 0x60;

    // ( ( PUCHAR )( pCpu->pIOBitmapVirtualAddressA ) )[ioPort / 8] = 1 << ( ioPort % 8 );

    // ��д 3.5 ʱ���������ƫ��(Time-Stamp Counter Offset )
    WriteVMCS( TSC_OFFSET, 0 );
    WriteVMCS( TSC_OFFSET_HIGH, 0 );

    // ��д 3.6 �����/Hypervisor���κ� CR0/CR4������������, û����

    // ��д 3.7 CR3���ʿ���.
    WriteVMCS( CR3_TARGET_COUNT, 0 );
    WriteVMCS( CR3_TARGET_VALUE0, 0 );
    WriteVMCS( CR3_TARGET_VALUE1, 0 );
    WriteVMCS( CR3_TARGET_VALUE2, 0 );
    WriteVMCS( CR3_TARGET_VALUE3, 0 );

    // ��д 3.8 APIC���ʿ���,û����


    //
    // ��д3.9 MSR λͼ��ַ(MSR Bitmap Address )
    // ������ֻ��Ҫ�ػ� MSR_IA32_SYSENTER_EIP MSR�Ĵ����Ķ�д
    // ���������Ż����ڻ�û��.. �Ȳ���
    //
    RtlFillMemory( pCpu->pMSRBitmapVirtualAddress , 0x4000, 0xff );

    WriteVMCS( MSR_BITMAP, pCpu->pMSRBitmapPyhicalAddress.LowPart );
    WriteVMCS( MSR_BITMAP_HIGH, pCpu->pMSRBitmapPyhicalAddress.HighPart );

    // 3.10 ִ����VMCSָ��(Executive-VMCS Pointer ), û����
    // 3.11 EPTָ��( Extended Page Table Pointer ), û����
    // 3.12 �������ʶ��(Virtual Processor Identifier, VPID), û����
//===========================================================================
    //
    // 4.VMEntry��Ϊ������( VM-Entry Control Fields )
    //

    WriteVMCS( VM_ENTRY_CONTROLS, VmxAdjustControls( 0, MSR_IA32_VMX_ENTRY_CTLS ) );

    // ��д#VMEntryʱ�洢����MSR�Ĵ�������
    WriteVMCS( VM_ENTRY_MSR_LOAD_COUNT, 0 );
    WriteVMCS( VM_ENTRY_INTR_INFO_FIELD, 0 );

    //
    // ������Ȼ�кܶ���Ϣû���õ�
    //
//===========================================================================
    //
    // 5.VMExit��Ϊ������( VM-Exit Control Fields )
    //
    WriteVMCS( VM_EXIT_CONTROLS,
               VmxAdjustControls( VM_EXIT_ACK_INTR_ON_EXIT, MSR_IA32_VMX_EXIT_CTLS ) );

    // ��д#VmExitʱ�洢����MSR�Ĵ�������
    WriteVMCS( VM_EXIT_MSR_STORE_COUNT, 0 );
    WriteVMCS( VM_EXIT_MSR_LOAD_COUNT, 0 );

    //
    // ������Ȼ�кܶ���Ϣû���õ�
    //
//---------------------------------------------------------------------------

    return STATUS_SUCCESS;
}

// �������⻯
NTSTATUS LaunchVirtualize( PCPU_VM_CONTEXT pCpu )
{
    //
    // VMLaunch��Ӧ�÷���, �ɹ��Ļ�Ӧ�ý���VMCS�е�Guest Eip��
    //
    ExecuteVmLaunch();

    if( VmFailInvalid() )
    {
        KdPrint( ( "Ddvp-> VMLaunchʧ��:%d!\n" ) );
        return STATUS_UNSUCCESSFUL;
    }

    if( VmLaunchFailValid() )
    {
        KdPrint( ( "Ddvp-> VMLaunch��Ч ������:%p !\n", ReadVMCS( VM_INSTRUCTION_ERROR ) ) );
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_UNSUCCESSFUL;
}

//
//	��ĳ��Processor���濪��VT. ��������GuestEsp
//
NTSTATUS NTAPI SubvertCpu( PVOID GuestEsp )
{
    int i;
    NTSTATUS Status;
    PIDT_ENTRY IdtBase;
    ULONG_PTR lpKiTrap0F;
    PCPU_VM_CONTEXT pCpu;

    KdPrint( ( "Ddvp-> ��ǰ������:%d!\n", KeGetCurrentProcessorNumber() ) );

    //
    // �ٴμ��CPU�Ƿ�֧��VT, �����Ƿ���VT.
    //
    Status = CheckBiosIsEnabled();

    if( Status == STATUS_UNSUCCESSFUL )
    {
        KdPrint( ( "Ddvp-> ��ǰProcessor BIOS û�д�VT����!\n" ) );
        return STATUS_UNSUCCESSFUL;
    }

//---------------------------------------------------------------------------
    //
    // ΪProcessor����ṹ, ����ṹ�Ǻ���Ҫ��.
    //
    pCpu = ( PCPU_VM_CONTEXT )ExAllocatePoolWithTag( NonPagedPool,
            sizeof( CPU_VM_CONTEXT ), 'Joen' );

    if( !pCpu )
    {
        KdPrint( ( "Ddvp-> pCpu �ṹ�ڴ����ʧ�� !\n" ) );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory( pCpu, sizeof( CPU_VM_CONTEXT ) );

    KdPrint( ( "Ddvp-> Vmx Cpu Address :%p!\n", pCpu ) );
//---------------------------------------------------------------------------
    //IdtBase = ( PIDT_ENTRY )GetIdtBase();

    //lpKiTrap0F = ( ( ULONG_PTR )g_StReloadKernel.OriginalKernelBase +
    //               ( ULONG_PTR )g_StSymbolsInfo.lpKiTrap0F );

    //// Copy int 1 Interrupt;
    //for( i = 0x15; i <= 0x1f; i++ )
    //{
    //    //
    //    // �ҵ�һ����KiTrap0F��ͬ����. �滻. KiTrap0F ������. ��Ϊ
    //    // int 1 ����Ӳ���ж�, ���ܴ���0x20. û�취ֻ���滻KiTrap0F��
    //    //
    //    if( ( IdtBase[i].OffsetHigh << 16 | IdtBase[i].OffsetLow ) == lpKiTrap0F )
    //    {
    //        pCpu->Interrupt1Index = i;

    //        IdtBase[i] = IdtBase[1];

    //        IdtBase[i].OffsetLow = ( ( ULONG_PTR )g_StSymbolsInfo.lpKiTrap01 +
    //                                 ( ULONG_PTR )g_StReloadKernel.NewKernelBase ) & 0xffff;

    //        IdtBase[i].OffsetHigh = ( ( ULONG_PTR )g_StSymbolsInfo.lpKiTrap01 +
    //                                  ( ULONG_PTR )g_StReloadKernel.NewKernelBase ) >> 16 & 0xffff;

    //        KdPrint( ( "Ddvp-> Interrupt Replace New int 1 Index: %x Address: %p!\n",
    //                   pCpu->Interrupt1Index, ( IdtBase[i].OffsetHigh << 16 ) | IdtBase[i].OffsetLow ) );

    //        break;
    //    }
    //}

    //// Copy int 3 Interrupt;
    //for( i = 0; i <= 0xff; i++ )
    //{
    //    if( IdtBase[i].Type == 0 )
    //    {
    //        pCpu->Interrupt3Index = i;

    //        IdtBase[i] = IdtBase[3];

    //        IdtBase[i].OffsetLow = ( ( ULONG_PTR )g_StSymbolsInfo.lpKiTrap03 +
    //                                 ( ULONG_PTR )g_StReloadKernel.NewKernelBase ) & 0xffff;

    //        IdtBase[i].OffsetHigh = ( ( ULONG_PTR )g_StSymbolsInfo.lpKiTrap03 +
    //                                  ( ULONG_PTR )g_StReloadKernel.NewKernelBase ) >> 16 & 0xffff;

    //        KdPrint( ( "Ddvp-> Interrupt Replace New int 3 Index: %x Address: %p!\n",
    //                   pCpu->Interrupt3Index, ( IdtBase[i].OffsetHigh << 16 ) | IdtBase[i].OffsetLow ) );

    //        break;
    //    }
    //}

    //if( ( !pCpu->Interrupt1Index ) || ( !pCpu->Interrupt3Index ) )
    //{
    //    ExFreePool( pCpu );
    //    KdPrint( ( "Ddvp-> Interrupt Replace Error!\n" ) );
    //    return STATUS_UNSUCCESSFUL;
    //}

//---------------------------------------------------------------------------
    //
    // ��дProcessor ��Cpu�ṹ ��Ҫ�Ƿ����ڴ�Vmx�ṹ, ����VMCS. IOλͼ,
    // VMCS.MSRλͼ VMCS.Host Stack, VMCS.IDTλͼ.
    //
    Status = SetupVMX( pCpu );

    if( Status == STATUS_UNSUCCESSFUL )
    {
        ExFreePool( pCpu );
        return STATUS_UNSUCCESSFUL;
    }

//---------------------------------------------------------------------------
    //
    // ��дProcessor ��Cpu�ṹ��Ҫ����дVMCS�ṹ.
    //
    Status = SetupVMCS( pCpu, GuestEsp );

    if( Status == STATUS_UNSUCCESSFUL )
    {
        ExFreePool( pCpu );
        return STATUS_UNSUCCESSFUL;
    }

    InterlockedIncrement(&g_uSubvertedCPUs);

    //
    // �������⻯. ���������Ӧ�÷���
    //
    Status = LaunchVirtualize( pCpu );

    InterlockedDecrement(&g_uSubvertedCPUs);

    return Status;
}

//===========================================================================
//	����ǽ��CPU��.����ÿ��Processor���涼ִ��һ��
//===========================================================================
static NTSTATUS NTAPI LiberateCpu( PVOID Param )
{
    NTSTATUS Status;
    ULONG64 Efer;

    //
    // �жϼ��ж�, ���봦�� DPC ������
    //
    if( KeGetCurrentIrql() != DISPATCH_LEVEL )
    {
        return STATUS_UNSUCCESSFUL;
    }

    //
    // ����Ĵ�����ʾ���Ƿ�����HVM.
    //
    Efer = ReadMsr( MSR_EFER );

    KdPrint( ( "Ddvp-> Reading MSR_EFER on entry: 0x%X\n", Efer ) );

    //
    // ��������� Hypervisor ��������ж�ص�NBP_HYPERCALL_UNLOAD��Ϣ
    //
    if( !NT_SUCCESS( Status = MakeHyperExitCall( ) ) )
    {
        KdPrint( ( "Ddvp-> MakeHyperExitCall() failed on processor #%d, status 0x%08hX\n",
                   KeGetCurrentProcessorNumber(), Status ) );

        return Status;
    }

    Efer = ReadMsr( MSR_EFER );

    KdPrint( ( "Ddvp-> Reading MSR_EFER on exit: 0x%X\n", Efer ) );

    return STATUS_SUCCESS;
}

//===========================================================================
//	����ж����ȼ�(DISPATCH_LEVEL)����ָ���Ĵ�������ִ��һ���ص�,
//	(�˴�ΪCallbackProc)����, ͬʱ��ִ�����ָ�ԭ�ȸô��������׺���,
//===========================================================================
NTSTATUS NTAPI CmDeliverToProcessor( CCHAR cProcessorNumber, PCALLBACK_PROC CallbackProc,
                                     PVOID CallbackParam, PNTSTATUS pCallbackStatus )
{
    KIRQL OldIrql;
    NTSTATUS CallbackStatus;

    if( !CallbackProc )
    {
        return STATUS_INVALID_PARAMETER;
    }

    if( pCallbackStatus )
    {
        *pCallbackStatus = STATUS_UNSUCCESSFUL;
    }

    KeSetSystemAffinityThread( ( KAFFINITY )( 1 << cProcessorNumber ) );

    //
    // ����IRQL��DISPATCH_LEVEL
    //
    OldIrql = KeRaiseIrqlToDpcLevel();

    //
    // ���ûص�����
    //
    CallbackStatus = CallbackProc( CallbackParam );

    KeLowerIrql( OldIrql );

    KeRevertToUserAffinityThread();

    //
    // ����callback�ķ���ֵ
    //
    if( pCallbackStatus )
    {
        *pCallbackStatus = CallbackStatus;
    }

    return STATUS_SUCCESS;
}

/* �ر�VT�ں˵��� */
NTSTATUS NTAPI StopVirtualTechnology()
{
    CCHAR i;
    NTSTATUS Status, CallbackStatus;

    //
    // ����ͬ���ȴ�����¼�, �Է����ʵ����ͻ
    //
    KeWaitForSingleObject( &g_GlobleMutex, Executive, KernelMode, FALSE, NULL );

    for( i = 0; i < KeNumberProcessors; i++ )
    {

        KdPrint( ( "Ddvp-> ���������:%d!\n", i ) );

        //
        // ����жϼ�, Ȼ����ÿ��Processor����ִ��һ������. ������ LiberateCpu
        //
        Status = CmDeliverToProcessor( i, LiberateCpu, NULL, &CallbackStatus );

        if( !NT_SUCCESS( Status ) )
        {
            KdPrint( ( "Ddvp-> CmDeliverToProcessor() ����ʧ�� %p !\n", Status ) );

            KeReleaseMutex( &g_GlobleMutex, FALSE );

            return STATUS_UNSUCCESSFUL;
        }

        if( !NT_SUCCESS( CallbackStatus ) )
        {
            KdPrint( ( "Ddvp-> LiberateCpu() ����ʧ�� %p !\n", CallbackStatus ) );

            KeReleaseMutex( &g_GlobleMutex, FALSE );
            return STATUS_UNSUCCESSFUL;
        }
    }

    KeReleaseMutex( &g_GlobleMutex, FALSE );

    return STATUS_SUCCESS;

}

//
// ����Intel VT�ں˵���
//
NTSTATUS StartVirtualTechnology()
{
    CCHAR i;
    NTSTATUS Status, CallbackStatus;

    //
    // �ȴ�������, ��֤ÿ��ʱ��ֻ��һ������VT������
    //
    KeWaitForSingleObject( &g_GlobleMutex, Executive, KernelMode, FALSE, NULL );

    //
    // ��ÿһ��Process����ִ��һ������VT�ĺ���, ��˵Ҫ��KeQueryActiveProcessors()
    //
    for( i = 0; i < KeNumberProcessors; i++ )
    {
        KdPrint( ( "Ddvp-> ���������:%d!\n", i ) );

        //
        // ��ĳ��Processor��ִ��һ������. CmSubvert �ڻ���ļ���, ����(x86/x64)����ʵ��
        // �������ĺ�����, SubvertCpu, ����NewBluePill����Ĵ���, ţ�˾���ţ��, �Ժܶ�
        // ��������ǵ���. �����.
        //
        Status = CmDeliverToProcessor( i, CmSubvert, NULL, &CallbackStatus );

        if( !NT_SUCCESS( Status ) )
        {
            KdPrint( ( "Ddvp-> CmDeliverToProcessor() ����ʧ��: %p!\n", Status ) );

            KeReleaseMutex( &g_GlobleMutex, FALSE );

            StopVirtualTechnology();

            return Status;
        }

        //
        // ����������ֵ���ж�. ������
        //
        if( !NT_SUCCESS( CallbackStatus ) )
        {
            KdPrint( ( "Ddvp-> SubvertCpu() ����ʧ��: %p!\n", CallbackStatus ) );

            KeReleaseMutex( &g_GlobleMutex, FALSE );

            StopVirtualTechnology();

            return CallbackStatus;
        }
    }

    KeReleaseMutex( &g_GlobleMutex, FALSE );

    //
    // ��Ⱦ���ں���������, ��ôҲ�ǲ��е�
    //
    if( KeNumberProcessors != g_uSubvertedCPUs )
    {

        KdPrint( ( "Ddvp-> ��Ⱦ���ں�����:%d ����.!\n", g_uSubvertedCPUs ) );

        StopVirtualTechnology();
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;

}



//
// ���BIOS�Ƿ��Intel-VT
//
NTSTATUS CheckBiosIsEnabled()
{
    ULONG cr4;
    ULONG64 msr;

    //
    // ���Cpu�Ƿ�֧��vmxonָ��
    //
    SetCr4( GetCr4() | X86_CR4_VMXE );
    cr4 = GetCr4();

    if( !( cr4 & X86_CR4_VMXE ) )
    {
        KdPrint( ( "Ddvp-> vmoxnָ��֧��û�д�...\n" ) );
        return STATUS_UNSUCCESSFUL;
    }

    //
    // ���BIOS�Ƿ���VT
    //
    msr = ReadMsr( MSR_IA32_FEATURE_CONTROL );

    if( !( msr & 4 ) )
    {
        KdPrint( ( "Ddvp-> BIOS MSR_IA32_FEATURE_CONTROL�Ĵ���ֵ:%p !\n", msr ) );
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;
}