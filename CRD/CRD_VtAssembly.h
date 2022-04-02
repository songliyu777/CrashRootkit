#pragma once

ULONG GetCr0();
ULONG GetCr3();
ULONG_PTR GetCr4();
VOID SetCr4(ULONG_PTR ulValue);
ULONG GetEflags();
ULONG64 ReadMsr(ULONG Index);
ULONG64 WriteMsr(ULONG MSRIndex,ULONG LowPart,ULONG HighPart);

ULONG GetGdtBase();
USHORT GetGdtLimit();
ULONG GetIdtBase();
USHORT GetIdtLimit();
VOID SetIdtr(ULONG Base, ULONG Limi);

USHORT GetCsSelector();
USHORT GetDsSelector();
USHORT GetEsSelector();
USHORT GetFsSelector();
USHORT GetGsSelector();
USHORT GetSsSelector();
USHORT GetLdtrSelector();
USHORT GetTrSelector();

VOID ExecuteVmxOn(ULONG PtrLowPart, ULONG PtrHighPart);
VOID ExecuteVmxOff( ULONG_PTR RegEsp, ULONG_PTR RegEip );
VOID ExecuteVmPtrLd( ULONG PtrLowPart, ULONG PtrHighPart );
VOID ExecuteVmClear( ULONG PtrLowPart, ULONG PtrHighPart );
VOID GuestEntryPoint( PVOID _GuestEsp );
NTSTATUS NTAPI GuestExitPoint();

VOID WriteVMCS( ULONG Field, ULONG Value );
ULONG ReadVMCS( ULONG Field );
ULONG VmFailInvalid();

ULONG ExecuteVmLaunch(VOID);
ULONG VmLaunchFailValid(VOID);

NTSTATUS NTAPI CmSubvert( PVOID );
NTSTATUS MakeHyperExitCall();

VOID ExecuteCpuId( ULONG fn, OUT PULONG ret_eax, OUT PULONG ret_ebx, OUT PULONG ret_ecx, OUT PULONG ret_edx );
VOID ExecuteInvd();

VOID NewKiFastCallEntry(VOID);