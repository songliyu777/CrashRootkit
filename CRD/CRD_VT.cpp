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

/* 填写 VT Vmx结构 */
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
    // 处理器编号.
    //
    pCpu->CpuIndex = KeGetCurrentProcessorNumber();

    //
    // 读模型寄存器(这里有VMX的版本, 和内存类型)
    //
    Msr = ReadMsr( MSR_IA32_VMX_BASIC );
    pvmx = ( PVMX_BASIC_MSR )&Msr;

    //
    // Vmx结构体大小.
    //
    ulSize = pvmx->szVmxOnRegion;

    KdPrint( ( "Ddvp-> VMXON region Size: 0x%p, VMX revision ID: 0x%p !\n",
               ulSize, pvmx->RevId ) );

//---------------------------------------------------------------------------

    //
    // Allocate memory for VMON, 然后执行vmxon指令
    //
    pCpu->pVMONVirtualAddress = MmAllocateNonCachedMemory( 0x1000 );

    if( pCpu->pVMONVirtualAddress == NULL )
    {
        KdPrint( ( "Ddvp-> VMXON 内存分配失败 !\n" ) );
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto __Exit;
    }

    RtlZeroMemory( pCpu->pVMONVirtualAddress, 0x1000 );

    //
    // vmxon版本号.
    //
    *( ULONG * )pCpu->pVMONVirtualAddress = pvmx->RevId;

    //
    // vmxon结构的的物理地址.
    //
    pCpu->pVMONPyhicalAddress  = MmGetPhysicalAddress( pCpu->pVMONVirtualAddress );

    KdPrint( ( "Ddvp-> VMXON 内存物理地址高位:%p, 低位:%p !\n",
               pCpu->pVMONPyhicalAddress.HighPart, pCpu->pVMONPyhicalAddress.LowPart ) );

    //
    // 执行vmxon指令.
    //

    ExecuteVmxOn( pCpu->pVMONPyhicalAddress.LowPart,
                  pCpu->pVMONPyhicalAddress.HighPart );

    // 检测CF标记位
    if( VmFailInvalid() )
    {
        KdPrint( ( "Ddvp-> vmxon 指令执行失败!\n" ) );
        Status = STATUS_UNSUCCESSFUL;
        goto __Exit;
    }

//---------------------------------------------------------------------------
    //
    // 这边填充VMCS结构
    //
    pCpu->pVMCSVirtualAddress = MmAllocateNonCachedMemory( 0x1000 );

    if( pCpu->pVMCSVirtualAddress == NULL )
    {
        KdPrint( ( "Ddvp-> 分配 VMCS结构内存失败!\n" ) );
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto __Exit;
    }

    RtlZeroMemory( pCpu->pVMCSVirtualAddress, 0x1000 );

    // 填写版本号
    *( ULONG * )pCpu->pVMCSVirtualAddress = pvmx->RevId;

    pCpu->pVMCSPyhicalAddress = MmGetPhysicalAddress( pCpu->pVMCSVirtualAddress );

    KdPrint( ( "Ddvp-> VMCS 内存物理地址高位:%p, 低位:%p !\n",
               pCpu->pVMCSPyhicalAddress.HighPart, pCpu->pVMCSPyhicalAddress.LowPart ) );
//---------------------------------------------------------------------------
    //
    // 这边是MSR位图.
    //
    pCpu->pMSRBitmapVirtualAddress = MmAllocateNonCachedMemory( 0x4000 );

    if( pCpu->pMSRBitmapVirtualAddress == NULL )
    {
        KdPrint( ( "Ddvp-> 分配 MSR Bitmap 结构内存失败!\n" ) );
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto __Exit;
    }

    RtlZeroMemory( pCpu->pMSRBitmapVirtualAddress, 0x4000 );

    pCpu->pMSRBitmapPyhicalAddress = MmGetPhysicalAddress( pCpu->pMSRBitmapVirtualAddress );

    KdPrint( ( "Ddvp-> MSR Bitmap 内存物理地址高位:%p, 低位:%p !\n",
               pCpu->pMSRBitmapPyhicalAddress.HighPart, pCpu->pMSRBitmapPyhicalAddress.LowPart ) );

//---------------------------------------------------------------------------
    //
    // 这边是 IOBitmapA
    //
    pCpu->pIOBitmapVirtualAddressA = MmAllocateNonCachedMemory( 0x1000 );

    if( pCpu->pIOBitmapVirtualAddressA == NULL )
    {
        KdPrint( ( "Ddvp-> 分配 IOBitmapA 结构内存失败!\n" ) );
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto __Exit;
    }

    RtlZeroMemory( pCpu->pIOBitmapVirtualAddressA, 0x1000 );
    pCpu->pIOBitmapPyhicalAddressA = MmGetPhysicalAddress( pCpu->pIOBitmapVirtualAddressA );

    KdPrint( ( "Ddvp-> IOBitmapA 内存物理地址高位:%p, 低位:%p !\n",
               pCpu->pIOBitmapPyhicalAddressA.HighPart, pCpu->pIOBitmapPyhicalAddressA.LowPart ) );

//---------------------------------------------------------------------------
    //
    // 这边是 IOBitmapB
    //
    pCpu->pIOBitmapVirtualAddressB = MmAllocateNonCachedMemory( 0x1000 );

    if( pCpu->pIOBitmapVirtualAddressB == NULL )
    {
        KdPrint( ( "Ddvp-> 分配 IOBitmapB 结构内存失败!\n" ) );
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto __Exit;
    }

    RtlZeroMemory( pCpu->pIOBitmapVirtualAddressB, 0x1000 );

    pCpu->pIOBitmapPyhicalAddressB = MmGetPhysicalAddress( pCpu->pIOBitmapVirtualAddressB );

    KdPrint( ( "Ddvp-> IOBitmapB 内存物理地址高位:%p, 低位:%p !\n",
               pCpu->pIOBitmapPyhicalAddressB.HighPart, pCpu->pIOBitmapPyhicalAddressB.LowPart ) );
//---------------------------------------------------------------------------
    //
    // 这边是填写 Host Stack 结构, 8K内存, 注意了这里不要用MmAllocateNonCachedMemory, 很慢的.
    //
    pCpu->pHostEsp = ExAllocatePool( NonPagedPool, 0x8000 );

    if( pCpu->pHostEsp == NULL )
    {
        KdPrint( ( "Ddvp-> Host Stack 内存分配失败!\n" ) );
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto __Exit;
    }

    RtlZeroMemory( pCpu->pHostEsp, 0x8000 );

    KdPrint( ( "Ddvp-> Host Stack 地址 :%p !\n", pCpu->pHostEsp ) );
//---------------------------------------------------------------------------
    //
    // 这边是为Host Idt 分配内存
    //
//     VirtualAddress = MmAllocateNonCachedMemory( sizeof( IDT_ENTRY ) * 256 );
//     if ( VirtualAddress == NULL )
//     {
//         KdPrint( ( "Ddvp-> Host Idt Table 内存分配失败!\n" ) );
//         Status = STATUS_INSUFFICIENT_RESOURCES;
//         goto __Exit;
//     }
//
//     RtlZeroMemory( VirtualAddress, sizeof( IDT_ENTRY ) * 256 );
//
//     pCpu->HostIdtArea = VirtualAddress;
//
//     KdPrint( ( "Ddvp-> Host Idt 地址 :%p !\n", pCpu->HostIdtArea ) );
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

// 在GDT取到相应的段描述符, 并按照我们比较习惯的格式存放好
NTSTATUS InitializeSegmentSelector( PSEGMENT_SELECTOR SegmentSelector, USHORT Selector, ULONG GdtBase )
{
    PSEGMENT_DESCRIPTOR2 SegDesc;

    if( !SegmentSelector )
    {
        return STATUS_INVALID_PARAMETER;
    }

    //
    // 如果段选择子的T1 = 1表示索引LDT中的项, 这里没有实现这个功能
    //
    if( Selector & 0x4 )
    {
        KdPrint( ( "Ddvp-> 要索引的段描述符在LDT中, 失败!\n" ) );
        return STATUS_INVALID_PARAMETER;
    }

    //
    // 在GDT中取出原始的段描述符
    //
    SegDesc = ( PSEGMENT_DESCRIPTOR2 )( ( PUCHAR ) GdtBase + ( Selector & ~0x7 ) );

    //
    // 段选择子
    //
    SegmentSelector->Selector = Selector;

    //
    // 段基址15-39位 55-63位
    //
    SegmentSelector->Base = SegDesc->base0 | SegDesc->base1 << 16 | SegDesc->base2 << 24;

    //
    // 段限长0-15位  47-51位, 看它的取法
    //
    SegmentSelector->Limit = SegDesc->limit0 | ( SegDesc->limit1attr1 & 0xf ) << 16;

    //
    // 段属性39-47 51-55 注意观察取法
    //
    SegmentSelector->Attributes.UCHARs = SegDesc->attr0 | ( SegDesc->limit1attr1 & 0xf0 ) << 4;

    //
    // 这里判断属性的DT位, 判断是否是系统段描述符还是代码数据段描述符
    //
    if( !( SegDesc->attr0 & LA_STANDARD ) )
    {
        ULONG64 tmp;

        //
        // 这里表示是系统段描述符或者门描述符, 感觉这是为64位准备的吧,
        // 32位下面段基址只有32位啊. 难道64位下面有什么区别了?
        //
        tmp = ( *( PULONG64 )( ( PUCHAR ) SegDesc + 8 ) );

        SegmentSelector->Base = ( SegmentSelector->Base & 0xffffffff ) | ( tmp << 32 );
    }

    //
    // 这是段界限的粒度位, 1为4K. 0为1BYTE
    //
    if( SegmentSelector->Attributes.fields.g )
    {
        //
        // 如果粒度位为1, 那么就乘以4K. 左移动12位
        //
        SegmentSelector->Limit = ( SegmentSelector->Limit << 12 ) + 0xfff;
    }

    return STATUS_SUCCESS;
}

// 获取段描述符限长
ULONG GetSegmentDescriptorLimit( ULONG gdt_base, USHORT selector )
{
    SEGMENT_SELECTOR SegmentSelector = { 0 };

    //
    // 在GDT中索引段描述符限长
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


// 获取段描述符属性
ULONG GetSegmentDescriptorAttributes( ULONG gdt_base, USHORT Selector )
{
    SEGMENT_SELECTOR SegmentSelector = { 0 };
    ULONG uAccessRights;

    //
    // 在GDT中索引到我们需要的段描述符
    //
    InitializeSegmentSelector( &SegmentSelector, Selector, gdt_base );

    //
    // 这个段描述符的属性为什么要这样, 参考Intel 手册24章Table 24-2
    //
    uAccessRights = ( ( PUCHAR ) & SegmentSelector.Attributes )[0] +
                    ( ( ( PUCHAR ) & SegmentSelector.Attributes )[1] << 12 );

    if( !Selector )
    {
        uAccessRights |= 0x10000;
    }

    return uAccessRights;
}

// 获取段描述符基址
ULONG GetSegmentDescriptorBase( ULONG gdt_base, USHORT seg_selector )
{
    ULONG Base = 0;
    SEGMENT_DESCRIPTOR	SegDescriptor = {0};

    //
    // 从GDT中索引到段描述符
    //
    RtlCopyMemory( &SegDescriptor, ( ULONG * )( gdt_base + ( seg_selector >> 3 ) * 8 ), 8 );

    //
    // 段描述符的高8位
    //
    Base = SegDescriptor.BaseHi;
    Base <<= 8;

    //
    // 段描述符的31-39位
    //
    Base |= SegDescriptor.BaseMid;
    Base <<= 16;

    //
    // 段描述符的 15-31位
    //
    Base |= SegDescriptor.BaseLo;

    return Base;
}

//
// 我们不欢迎的#VMExit处理. 跳过指令就可以
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
// 执行cpuid指令,
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
// 识别vmcall指令是否是我们用来关闭虚拟机的
// 特征码是 eax == 'Joen' ecx == 'Ddvp' edx == 'Exit' ebx = 'Quit'
// 并且必须是由于vmcall 指令引起的. vmcall 机器码是0xc1010f
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
// 如果再执行vmx指令, 那么返回失败!
//
VOID HandleVmInstruction( PCPU_VM_CONTEXT pCpu )
{
    ULONG InstructionLength;

    WriteVMCS( GUEST_RFLAGS, ReadVMCS( GUEST_RFLAGS ) | 0x1 );

    InstructionLength = ReadVMCS( VM_EXIT_INSTRUCTION_LEN );
    WriteVMCS( GUEST_RIP, ReadVMCS( GUEST_RIP ) + InstructionLength );
}

// Msr 寄存器访问.
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

// MSR 寄存器写入.
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

// Cr寄存器访问
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

// 虚拟机异常
VOID HandleException( PCPU_VM_CONTEXT pCpu )
{
	
}

//
// 这个函数感觉和Windows的窗口过程很像了. 所有消息在这里处理
// 这个消息几乎是时时产生的.
//
VOID HandleVmExit( PCPU_VM_CONTEXT pCpu, PGUEST_REGS GuestReg )
{
    ULONG ExitCode;

    //
    // 修复一下HOST机的IDT Limit
    //
    SetIdtr( ReadVMCS( HOST_IDTR_BASE ), 0x7FF );

    //
    // 切换到产生#VmExit的进程空间
    //
    // AttachGuestProcess();

    //
    // 把客户机esp寄存器保存好
    //
    GuestReg->RegEsp = ReadVMCS( GUEST_RSP );

    //
    // 退出VM的原因
    //
    ExitCode = ReadVMCS( VM_EXIT_REASON );
//---------------------------------------------------------------------------

    if( ExitCode == EXIT_REASON_EXCEPTION_NMI )
    {

        //
        // 中断异常处理, 这里不需要调整Guest.Eip
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
        // 执行cpuid指令
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
        // Cr 寄存器访问与写入
        //
        HandleCrAccess( GuestReg );
    }
    else if( ExitCode == EXIT_REASON_DR_ACCESS )
    {
        //
        // Dr 寄存器访问与写入
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
        // MSR 寄存器访问
        //
        HandleMsrRead( pCpu, GuestReg );
    }
    else if( ExitCode == EXIT_REASON_MSR_WRITE )
    {
        //
        // MSR寄存器写入
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
    // 写esp到客户机,esp可能会更改,看CrAccess那部分
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
// 开启VT时的填写VMCS结构
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
    // 执行 Vmxclean 解除绑定, 和 vmx_ptrld 绑定到当前Processor
    //
    ExecuteVmClear( VMCSPhysicalAddress.LowPart, VMCSPhysicalAddress.HighPart );
    ExecuteVmPtrLd( VMCSPhysicalAddress.LowPart, VMCSPhysicalAddress.HighPart );

    //
    // 清除VMCS中偏移4的VMX退出原因指示器(VMX-abort Indicator )
    //
    RtlZeroMemory( ( PULONG )pCpu->pVMCSVirtualAddress + 1, 4 );
//---------------------------------------------------------------------------
    /*
     * 这里是按照Intel 手册描述的VMCS的数据区组成部分依次填写
     * 详细信息参考[开发日志2].VMCS数据区总共6个组成部分. 分别是
     *
     * 1.客户区状态域(Guest State Area)
     *
     * 2.宿主机状态域(Host State Area )
     *
     * 3.虚拟机运行控制域( VM-Execuction Control Fields )
     *
     * 4.VMEntry行为控制域( VM-Entry Control Fields )
     *
     * 5.VMExit行为控制域( VM-Exit Control Fields )
     *
     * 6.VMExit相关信息域( VM-Exit Information Fields )(只读)
     */
//---------------------------------------------------------------------------
    //
    // 填写1.客户区状态域(Guest State Area)
    //

    // 填写Guest 控制寄存器状态
    Cr0 = GetCr0();
    CmSetBit32( &Cr0, 0 );	// PE
    CmSetBit32( &Cr0, 5 );	// NE
    CmSetBit32( &Cr0, 31 );	// PG
    WriteVMCS( GUEST_CR0, Cr0 );
    WriteVMCS( GUEST_CR3, GetCr3() );
    WriteVMCS( GUEST_CR4, GetCr4() );

    // 填写DR7寄存器
    WriteVMCS( GUEST_DR7, 0x400 );

    // 填写Guest Esp, Eip, Eflags
    WriteVMCS( GUEST_RSP, ( ULONG ) GuestEsp );

    //
    // GUEST_RIP 指向的地址, 在开启VT以后就从这里开始运行的
    //
    WriteVMCS( GUEST_RIP, ( ULONG )&GuestEntryPoint );
    WriteVMCS( GUEST_RFLAGS, GetEflags() );

    // 填写Guest各个段选择子 CS, SS, DS, ES, FS, GS, LDTR, TR
    WriteVMCS( GUEST_CS_SELECTOR, GetCsSelector() & 0xfff8 );
    WriteVMCS( GUEST_SS_SELECTOR, GetSsSelector() & 0xfff8 );
    WriteVMCS( GUEST_DS_SELECTOR, GetDsSelector() & 0xfff8 );
    WriteVMCS( GUEST_ES_SELECTOR, GetEsSelector() & 0xfff8 );
    WriteVMCS( GUEST_FS_SELECTOR, GetFsSelector() & 0xfff8 );
    WriteVMCS( GUEST_GS_SELECTOR, GetGsSelector() & 0xfff8 );
    WriteVMCS( GUEST_LDTR_SELECTOR, GetLdtrSelector() & 0xfff8 );
    WriteVMCS( GUEST_TR_SELECTOR, GetTrSelector() & 0xfff8 );

    // 填写Guest段描述符基址
    GdtBase = GetGdtBase();
    WriteVMCS( GUEST_CS_BASE, GetSegmentDescriptorBase( GdtBase, GetCsSelector() ) );
    WriteVMCS( GUEST_SS_BASE, GetSegmentDescriptorBase( GdtBase, GetSsSelector() ) );
    WriteVMCS( GUEST_DS_BASE, GetSegmentDescriptorBase( GdtBase, GetDsSelector() ) );
    WriteVMCS( GUEST_ES_BASE, GetSegmentDescriptorBase( GdtBase, GetEsSelector() ) );
    WriteVMCS( GUEST_FS_BASE, GetSegmentDescriptorBase( GdtBase, GetFsSelector() ) );
    WriteVMCS( GUEST_GS_BASE, GetSegmentDescriptorBase( GdtBase, GetGsSelector() ) );
    WriteVMCS( GUEST_LDTR_BASE, GetSegmentDescriptorBase( GdtBase, GetLdtrSelector() ) );
    WriteVMCS( GUEST_TR_BASE, GetSegmentDescriptorBase( GdtBase, GetTrSelector() ) );

    // 填写Guest段描述符限长
    WriteVMCS( GUEST_CS_LIMIT, GetSegmentDescriptorLimit( GdtBase, GetCsSelector() ) );
    WriteVMCS( GUEST_SS_LIMIT, GetSegmentDescriptorLimit( GdtBase, GetSsSelector() ) );
    WriteVMCS( GUEST_DS_LIMIT, GetSegmentDescriptorLimit( GdtBase, GetDsSelector() ) );
    WriteVMCS( GUEST_ES_LIMIT, GetSegmentDescriptorLimit( GdtBase, GetEsSelector() ) );
    WriteVMCS( GUEST_FS_LIMIT, GetSegmentDescriptorLimit( GdtBase, GetFsSelector() ) );
    WriteVMCS( GUEST_GS_LIMIT, GetSegmentDescriptorLimit( GdtBase, GetGsSelector() ) );
    WriteVMCS( GUEST_LDTR_LIMIT, GetSegmentDescriptorLimit( GdtBase, GetLdtrSelector() ) );
    WriteVMCS( GUEST_TR_LIMIT, GetSegmentDescriptorLimit( GdtBase, GetTrSelector() ) );

    // 填写Guest段描述符属性
    WriteVMCS( GUEST_CS_AR_BYTES, GetSegmentDescriptorAttributes( GdtBase, GetCsSelector() ) );
    WriteVMCS( GUEST_SS_AR_BYTES, GetSegmentDescriptorAttributes( GdtBase, GetSsSelector() ) );
    WriteVMCS( GUEST_DS_AR_BYTES, GetSegmentDescriptorAttributes( GdtBase, GetDsSelector() ) );
    WriteVMCS( GUEST_ES_AR_BYTES, GetSegmentDescriptorAttributes( GdtBase, GetEsSelector() ) );
    WriteVMCS( GUEST_FS_AR_BYTES, GetSegmentDescriptorAttributes( GdtBase, GetFsSelector() ) );
    WriteVMCS( GUEST_GS_AR_BYTES, GetSegmentDescriptorAttributes( GdtBase, GetGsSelector() ) );
    WriteVMCS( GUEST_LDTR_AR_BYTES, GetSegmentDescriptorAttributes( GdtBase, GetLdtrSelector() ) );
    WriteVMCS( GUEST_TR_AR_BYTES, GetSegmentDescriptorAttributes( GdtBase, GetTrSelector() ) );

    // 填写Guest GDTR/IDTR基址
    WriteVMCS( GUEST_GDTR_BASE, GetGdtBase() );
    WriteVMCS( GUEST_IDTR_BASE, GetIdtBase() );

    // 填写Guest GDTR/IDTR 限长
    WriteVMCS( GUEST_GDTR_LIMIT, GetGdtLimit() );
    WriteVMCS( GUEST_IDTR_LIMIT, GetIdtLimit() );

    // 填写Guest MSR IA32_DEBUGCTL 寄存器值
    Msr = ReadMsr( MSR_IA32_DEBUGCTL );
    WriteVMCS( GUEST_IA32_DEBUGCTL, ( ULONG )( Msr & 0xFFFFFFFF ) );
    WriteVMCS( GUEST_IA32_DEBUGCTL_HIGH, ( ULONG )( Msr >> 32 ) );

    // 填写Guest MSR IA32_SYSENTER_CS
    Msr = ReadMsr( MSR_IA32_SYSENTER_CS );
    WriteVMCS( GUEST_SYSENTER_CS, ( ULONG )( Msr & 0xFFFFFFFF ) );

    // 填写Guest MSR IA32_SYSENTER_ESP
    Msr = ReadMsr( MSR_IA32_SYSENTER_ESP );
    WriteVMCS( GUEST_SYSENTER_ESP, ( ULONG )( Msr & 0xFFFFFFFF ) );

    // 填写Guest MSR IA32_SYSENTER_EIP
    pCpu->OldKiFastCallEntry = ( PVOID )ReadMsr( MSR_IA32_SYSENTER_EIP );

    WriteVMCS( GUEST_SYSENTER_EIP, ( ULONG )( &NewKiFastCallEntry ) );

    KdPrint( ( "Ddvp-> OldKiFastCallEntry %p NewKiFastCallEntry %p !\n",
               pCpu->OldKiFastCallEntry, NewKiFastCallEntry ) );

    //
    // 显然到这里你可以发现 IA32_PERF_GLOBAL_CTRL(性能计数), IA32_PAT,
    // IA32_EFER(进入64位要用) 这几个MSR寄存器没有填写.
    //
//===========================================================================

    //
    // 填写2.宿主机状态域(Host State Area )
    //

    // Guest活动性状态
    WriteVMCS( GUEST_ACTIVITY_STATE, 0 );

    // Guest中断能力状态
    WriteVMCS( GUEST_INTERRUPTIBILITY_INFO, 0 );

    // VMCS连接指针(保留)
    WriteVMCS( VMCS_LINK_POINTER, 0xFFFFFFFF );
    WriteVMCS( VMCS_LINK_POINTER_HIGH, 0xFFFFFFFF );

    //
    // 这里推迟调试异常, VMX抢占计时器, PDPTE项都没有填写
    //
//===========================================================================

    //
    // 填写2.宿主机状态域(Host State Area )
    //

    // 控制寄存器 CR0, CR3, CR4
    WriteVMCS( HOST_CR0, GetCr0() );
    WriteVMCS( HOST_CR3, GetCr3() );
    WriteVMCS( HOST_CR4, GetCr4() );

    // 设置Host ESP, EIP
    WriteVMCS( HOST_RSP, ( ULONG )( ( PUCHAR ) pCpu->pHostEsp + 0x7000 ) );

    KdPrint( ( "Ddvp-> HostEsp:%p HostEsp+0x7000:%p!\n",
               pCpu->pHostEsp, ( PUCHAR )pCpu->pHostEsp + 0x7000 ) );

    //把 VIRT_CPU 结构体复制到HOST堆栈中去
    RtlMoveMemory( ( PUCHAR )pCpu->pHostEsp + 0x7000, pCpu, sizeof( CPU_VM_CONTEXT ) );

    pCpu = ( PCPU_VM_CONTEXT )( ( PUCHAR )pCpu->pHostEsp + 0x7000 );

    //
    // 这个值非常重要了. 产生#VMExit事件的时候, 进入这里
    //
    WriteVMCS( HOST_RIP, ( ULONG ) ExitEventHandler );


    // 填写Host CS, SS, DS, ES, FS, GS和TR寄存器
    WriteVMCS( HOST_CS_SELECTOR, GetCsSelector() & 0xfff8 );
    WriteVMCS( HOST_SS_SELECTOR, GetSsSelector() & 0xfff8 );
    WriteVMCS( HOST_DS_SELECTOR, GetDsSelector() & 0xfff8 );
    WriteVMCS( HOST_ES_SELECTOR, GetEsSelector() & 0xfff8 );
    WriteVMCS( HOST_FS_SELECTOR, GetFsSelector() & 0xfff8 );
    WriteVMCS( HOST_GS_SELECTOR, GetGsSelector() & 0xfff8 );
    WriteVMCS( HOST_TR_SELECTOR, GetTrSelector() & 0xfff8 );

    // 填写 Host FS, GS and TR 基址
    WriteVMCS( HOST_FS_BASE, GetSegmentDescriptorBase( GdtBase, GetFsSelector() ) );
    WriteVMCS( HOST_GS_BASE, GetSegmentDescriptorBase( GdtBase, GetGsSelector() ) );
    WriteVMCS( HOST_TR_BASE, GetSegmentDescriptorBase( GdtBase, GetTrSelector() ) );


    // 填写Host GDTR/IDTR base
    WriteVMCS( HOST_GDTR_BASE, GetGdtBase() );
    WriteVMCS( HOST_IDTR_BASE, GetIdtBase() );

    // 初始化 Guest 的 Idt
    // InitVMMIDT( pCpu, ( PIDT_ENTRY )pCpu->HostIdtArea );

    // 填写 Host IA32_SYSENTER_ESP/EIP/CS
    Msr = ReadMsr( MSR_IA32_SYSENTER_ESP );
    WriteVMCS( HOST_IA32_SYSENTER_ESP, ( ULONG )( Msr & 0xFFFFFFFF ) );

    Msr = ReadMsr( MSR_IA32_SYSENTER_EIP );
    WriteVMCS( HOST_IA32_SYSENTER_EIP, ( ULONG )( Msr & 0xFFFFFFFF ) );

    Msr = ReadMsr( MSR_IA32_SYSENTER_CS );
    WriteVMCS( HOST_IA32_SYSENTER_CS, ( ULONG )( Msr & 0xFFFFFFFF ) );

    //
    // IA32_PERF_GLOBAL_CTRL, IA32_PAT, IA32_EFER 没有填写
    //
//===========================================================================
    //
    // 填写 3.虚拟机运行控制域( VM-Execuction Control Fields )
    //

    // 填写 3.1 基于针脚的虚拟机执行控制 Pin-based VM-execution controls
    WriteVMCS( PIN_BASED_VM_EXEC_CONTROL,
               VmxAdjustControls( 0, MSR_IA32_VMX_PINBASED_CTLS ) );

    // 填写 3.2 基于处理器的虚拟机执行控制 Primary processor-based VM-execution controls
    // 控制下面的异常位图, DR寄存器访问, 或者MSR访问是否产生#VMExit. 很重要.
    WriteVMCS( CPU_BASED_VM_EXEC_CONTROL,
               VmxAdjustControls(
                   CPU_BASED_ACTIVATE_MSR_BITMAP |
                   CPU_BASED_ACTIVATE_IO_BITMAP
                   /*CPU_BASED_MOV_DR_EXITING*/,
                   MSR_IA32_VMX_PROCBASED_CTLS ) );

    // 填写 3.3 Exception bitmap 设置异常位图, 这边我们拦截了断点异常和调试异常
    ExceptionBitmap = 0;
    ExceptionBitmap |= 1 << DEBUG_EXCEPTION;
    ExceptionBitmap |= 1 << BREAKPOINT_EXCEPTION;

    //ExceptionBitmap |= 1<< PAGE_FAULT_EXCEPTION 我们感兴趣的异常
    WriteVMCS( EXCEPTION_BITMAP, ExceptionBitmap );

    // 配置页故障
    WriteVMCS( PAGE_FAULT_ERROR_CODE_MASK, 0 );
    WriteVMCS( PAGE_FAULT_ERROR_CODE_MATCH, 0 );

    //
    // 填写 3.4 I/O bitmap IO位图, 这里可以拦截了60号端口.. 也就是键盘输入
    //
    WriteVMCS( IO_BITMAP_A_HIGH, pCpu->pIOBitmapPyhicalAddressA.HighPart );
    WriteVMCS( IO_BITMAP_A,      pCpu->pIOBitmapPyhicalAddressA.LowPart );
    WriteVMCS( IO_BITMAP_B_HIGH, pCpu->pIOBitmapPyhicalAddressB.HighPart );
    WriteVMCS( IO_BITMAP_B,      pCpu->pIOBitmapPyhicalAddressB.LowPart );

    // ioPort = 0x60;

    // ( ( PUCHAR )( pCpu->pIOBitmapVirtualAddressA ) )[ioPort / 8] = 1 << ( ioPort % 8 );

    // 填写 3.5 时间戳计数器偏移(Time-Stamp Counter Offset )
    WriteVMCS( TSC_OFFSET, 0 );
    WriteVMCS( TSC_OFFSET_HIGH, 0 );

    // 填写 3.6 虚拟机/Hypervisor屏蔽和 CR0/CR4访问隐藏设置, 没有用

    // 填写 3.7 CR3访问控制.
    WriteVMCS( CR3_TARGET_COUNT, 0 );
    WriteVMCS( CR3_TARGET_VALUE0, 0 );
    WriteVMCS( CR3_TARGET_VALUE1, 0 );
    WriteVMCS( CR3_TARGET_VALUE2, 0 );
    WriteVMCS( CR3_TARGET_VALUE3, 0 );

    // 填写 3.8 APIC访问控制,没有用


    //
    // 填写3.9 MSR 位图地址(MSR Bitmap Address )
    // 这里我只需要截获 MSR_IA32_SYSENTER_EIP MSR寄存器的读写
    // 对于其他优化现在还没有.. 先不管
    //
    RtlFillMemory( pCpu->pMSRBitmapVirtualAddress , 0x4000, 0xff );

    WriteVMCS( MSR_BITMAP, pCpu->pMSRBitmapPyhicalAddress.LowPart );
    WriteVMCS( MSR_BITMAP_HIGH, pCpu->pMSRBitmapPyhicalAddress.HighPart );

    // 3.10 执行体VMCS指针(Executive-VMCS Pointer ), 没有用
    // 3.11 EPT指针( Extended Page Table Pointer ), 没有用
    // 3.12 虚拟机标识符(Virtual Processor Identifier, VPID), 没有用
//===========================================================================
    //
    // 4.VMEntry行为控制域( VM-Entry Control Fields )
    //

    WriteVMCS( VM_ENTRY_CONTROLS, VmxAdjustControls( 0, MSR_IA32_VMX_ENTRY_CTLS ) );

    // 填写#VMEntry时存储加载MSR寄存器数量
    WriteVMCS( VM_ENTRY_MSR_LOAD_COUNT, 0 );
    WriteVMCS( VM_ENTRY_INTR_INFO_FIELD, 0 );

    //
    // 这里依然有很多信息没有用到
    //
//===========================================================================
    //
    // 5.VMExit行为控制域( VM-Exit Control Fields )
    //
    WriteVMCS( VM_EXIT_CONTROLS,
               VmxAdjustControls( VM_EXIT_ACK_INTR_ON_EXIT, MSR_IA32_VMX_EXIT_CTLS ) );

    // 填写#VmExit时存储加载MSR寄存器数量
    WriteVMCS( VM_EXIT_MSR_STORE_COUNT, 0 );
    WriteVMCS( VM_EXIT_MSR_LOAD_COUNT, 0 );

    //
    // 这里依然有很多信息没有用到
    //
//---------------------------------------------------------------------------

    return STATUS_SUCCESS;
}

// 开启虚拟化
NTSTATUS LaunchVirtualize( PCPU_VM_CONTEXT pCpu )
{
    //
    // VMLaunch不应该返回, 成功的话应该进入VMCS中的Guest Eip中
    //
    ExecuteVmLaunch();

    if( VmFailInvalid() )
    {
        KdPrint( ( "Ddvp-> VMLaunch失败:%d!\n" ) );
        return STATUS_UNSUCCESSFUL;
    }

    if( VmLaunchFailValid() )
    {
        KdPrint( ( "Ddvp-> VMLaunch无效 错误码:%p !\n", ReadVMCS( VM_INSTRUCTION_ERROR ) ) );
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_UNSUCCESSFUL;
}

//
//	在某个Processor上面开启VT. 参数就是GuestEsp
//
NTSTATUS NTAPI SubvertCpu( PVOID GuestEsp )
{
    int i;
    NTSTATUS Status;
    PIDT_ENTRY IdtBase;
    ULONG_PTR lpKiTrap0F;
    PCPU_VM_CONTEXT pCpu;

    KdPrint( ( "Ddvp-> 当前处理器:%d!\n", KeGetCurrentProcessorNumber() ) );

    //
    // 再次检测CPU是否支持VT, 或者是否开启VT.
    //
    Status = CheckBiosIsEnabled();

    if( Status == STATUS_UNSUCCESSFUL )
    {
        KdPrint( ( "Ddvp-> 当前Processor BIOS 没有打开VT功能!\n" ) );
        return STATUS_UNSUCCESSFUL;
    }

//---------------------------------------------------------------------------
    //
    // 为Processor分配结构, 这个结构是很重要了.
    //
    pCpu = ( PCPU_VM_CONTEXT )ExAllocatePoolWithTag( NonPagedPool,
            sizeof( CPU_VM_CONTEXT ), 'Joen' );

    if( !pCpu )
    {
        KdPrint( ( "Ddvp-> pCpu 结构内存分配失败 !\n" ) );
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
    //    // 找到一个和KiTrap0F相同的项. 替换. KiTrap0F 可以用. 因为
    //    // int 1 属于硬件中断, 不能大于0x20. 没办法只能替换KiTrap0F了
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
    // 填写Processor 的Cpu结构 主要是分配内存Vmx结构, 还有VMCS. IO位图,
    // VMCS.MSR位图 VMCS.Host Stack, VMCS.IDT位图.
    //
    Status = SetupVMX( pCpu );

    if( Status == STATUS_UNSUCCESSFUL )
    {
        ExFreePool( pCpu );
        return STATUS_UNSUCCESSFUL;
    }

//---------------------------------------------------------------------------
    //
    // 填写Processor 的Cpu结构主要是填写VMCS结构.
    //
    Status = SetupVMCS( pCpu, GuestEsp );

    if( Status == STATUS_UNSUCCESSFUL )
    {
        ExFreePool( pCpu );
        return STATUS_UNSUCCESSFUL;
    }

    InterlockedIncrement(&g_uSubvertedCPUs);

    //
    // 开启虚拟化. 这个函数不应该返回
    //
    Status = LaunchVirtualize( pCpu );

    InterlockedDecrement(&g_uSubvertedCPUs);

    return Status;
}

//===========================================================================
//	这就是解放CPU了.会在每个Processor上面都执行一遍
//===========================================================================
static NTSTATUS NTAPI LiberateCpu( PVOID Param )
{
    NTSTATUS Status;
    ULONG64 Efer;

    //
    // 中断级判断, 必须处在 DPC 级别上
    //
    if( KeGetCurrentIrql() != DISPATCH_LEVEL )
    {
        return STATUS_UNSUCCESSFUL;
    }

    //
    // 这个寄存器标示了是否开启了HVM.
    //
    Efer = ReadMsr( MSR_EFER );

    KdPrint( ( "Ddvp-> Reading MSR_EFER on entry: 0x%X\n", Efer ) );

    //
    // 这个函数向 Hypervisor 发出申请卸载的NBP_HYPERCALL_UNLOAD消息
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
//	提高中断优先级(DISPATCH_LEVEL)并在指定的处理器上执行一个回调,
//	(此处为CallbackProc)函数, 同时在执行最后恢复原先该处理器的亲和性,
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
    // 提升IRQL至DISPATCH_LEVEL
    //
    OldIrql = KeRaiseIrqlToDpcLevel();

    //
    // 调用回调函数
    //
    CallbackStatus = CallbackProc( CallbackParam );

    KeLowerIrql( OldIrql );

    KeRevertToUserAffinityThread();

    //
    // 保存callback的返回值
    //
    if( pCallbackStatus )
    {
        *pCallbackStatus = CallbackStatus;
    }

    return STATUS_SUCCESS;
}

/* 关闭VT内核调试 */
NTSTATUS NTAPI StopVirtualTechnology()
{
    CCHAR i;
    NTSTATUS Status, CallbackStatus;

    //
    // 这里同样等待多个事件, 以防多个实例冲突
    //
    KeWaitForSingleObject( &g_GlobleMutex, Executive, KernelMode, FALSE, NULL );

    for( i = 0; i < KeNumberProcessors; i++ )
    {

        KdPrint( ( "Ddvp-> 处理器编号:%d!\n", i ) );

        //
        // 提高中断级, 然后在每个Processor上面执行一个函数. 这里是 LiberateCpu
        //
        Status = CmDeliverToProcessor( i, LiberateCpu, NULL, &CallbackStatus );

        if( !NT_SUCCESS( Status ) )
        {
            KdPrint( ( "Ddvp-> CmDeliverToProcessor() 调用失败 %p !\n", Status ) );

            KeReleaseMutex( &g_GlobleMutex, FALSE );

            return STATUS_UNSUCCESSFUL;
        }

        if( !NT_SUCCESS( CallbackStatus ) )
        {
            KdPrint( ( "Ddvp-> LiberateCpu() 调用失败 %p !\n", CallbackStatus ) );

            KeReleaseMutex( &g_GlobleMutex, FALSE );
            return STATUS_UNSUCCESSFUL;
        }
    }

    KeReleaseMutex( &g_GlobleMutex, FALSE );

    return STATUS_SUCCESS;

}

//
// 开启Intel VT内核调试
//
NTSTATUS StartVirtualTechnology()
{
    CCHAR i;
    NTSTATUS Status, CallbackStatus;

    //
    // 等待互斥量, 保证每个时刻只有一个开启VT的请求
    //
    KeWaitForSingleObject( &g_GlobleMutex, Executive, KernelMode, FALSE, NULL );

    //
    // 在每一个Process上面执行一个开启VT的函数, 传说要用KeQueryActiveProcessors()
    //
    for( i = 0; i < KeNumberProcessors; i++ )
    {
        KdPrint( ( "Ddvp-> 处理器编号:%d!\n", i ) );

        //
        // 在某个Processor上执行一个函数. CmSubvert 在汇编文件中, 并且(x86/x64)都有实现
        // 最后调到的函数是, SubvertCpu, 这是NewBluePill里面的代码, 牛人就是牛人, 对很多
        // 情况都考虑到了. 佩服下.
        //
        Status = CmDeliverToProcessor( i, CmSubvert, NULL, &CallbackStatus );

        if( !NT_SUCCESS( Status ) )
        {
            KdPrint( ( "Ddvp-> CmDeliverToProcessor() 调用失败: %p!\n", Status ) );

            KeReleaseMutex( &g_GlobleMutex, FALSE );

            StopVirtualTechnology();

            return Status;
        }

        //
        // 对两个返回值的判断. 看清了
        //
        if( !NT_SUCCESS( CallbackStatus ) )
        {
            KdPrint( ( "Ddvp-> SubvertCpu() 调用失败: %p!\n", CallbackStatus ) );

            KeReleaseMutex( &g_GlobleMutex, FALSE );

            StopVirtualTechnology();

            return CallbackStatus;
        }
    }

    KeReleaseMutex( &g_GlobleMutex, FALSE );

    //
    // 侵染的内核数量不对, 那么也是不行的
    //
    if( KeNumberProcessors != g_uSubvertedCPUs )
    {

        KdPrint( ( "Ddvp-> 侵染的内核数量:%d 不对.!\n", g_uSubvertedCPUs ) );

        StopVirtualTechnology();
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;

}



//
// 检查BIOS是否打开Intel-VT
//
NTSTATUS CheckBiosIsEnabled()
{
    ULONG cr4;
    ULONG64 msr;

    //
    // 检测Cpu是否支持vmxon指令
    //
    SetCr4( GetCr4() | X86_CR4_VMXE );
    cr4 = GetCr4();

    if( !( cr4 & X86_CR4_VMXE ) )
    {
        KdPrint( ( "Ddvp-> vmoxn指令支持没有打开...\n" ) );
        return STATUS_UNSUCCESSFUL;
    }

    //
    // 检测BIOS是否开启VT
    //
    msr = ReadMsr( MSR_IA32_FEATURE_CONTROL );

    if( !( msr & 4 ) )
    {
        KdPrint( ( "Ddvp-> BIOS MSR_IA32_FEATURE_CONTROL寄存器值:%p !\n", msr ) );
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;
}