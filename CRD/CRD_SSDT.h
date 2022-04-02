#pragma once

#include "MyWinNT.h"
#define _CRD_
#include "Transfer.h"
//SSDT

typedef struct _tagSSDT {
	PVOID pvSSDTBase;
	PVOID pvServiceCounterTable;
	ULONG_PTR ulNumberOfServices;
	PVOID pvParamTableBase;
} SSDT, *PSSDT;

typedef struct _tagSSDTShadow {
	PVOID pvSSDTBase;
	PVOID pvServiceCounterTable;
	ULONG_PTR ulNumberOfServices;
	PVOID pvParamTableBase;
	PVOID pvSSDTShadowBase;
	PVOID pvServiceCounterTableShadow;
	ULONG_PTR ulNumberOfServicesShadow;
	PVOID pvParamTableBaseShadow;
} SSDTShadow, *PSSDTShadow;

VOID InitKeServiceDescriptorTableShadow();

VOID FreeSSDT();

VOID InitOriginalSSDT(ULONG_PTR pPe_NtKr,ULONG_PTR NtKrBase);

VOID InitOriginalSSDTShadow(ULONG_PTR pPe_Win32kBase,ULONG_PTR Win32kBase);

PSSDT GetCurrentSSDT();

PSSDT GetOriginalSSDT();

PSSDTShadow GetCurrentSSDTShadow();

PSSDTShadow GetOriginalSSDTShadow();

BOOL CRD_SSDTGet(PVOID buff,SIZE_T size);

BOOL CRD_SSDTShadowGet(PVOID buff,SIZE_T size);

NTSTATUS RecoverSSDT(PTransferMsg msg);

NTSTATUS RecoverSSDTShadow(PTransferMsg msg);

ULONG_PTR GetSSDTFunction(ULONG FuncIndex);

ULONG_PTR GetSSDTShadowFunction(ULONG FuncIndex);