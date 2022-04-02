#pragma once

#include "MyWinNT.h"

typedef struct _SLISTENTRY
{
	struct _SLISTENTRY *Next;
}SLISTENTRY,*PSLISTENTRY;

VOID InitSLISTENTRY(PSLISTENTRY pPSListEntry);

VOID PushSLISTENTRY(PSLISTENTRY pPSListEntry,PSLISTENTRY pCSListEntry);

SIZE_T SizeOfSListEntry(PSLISTENTRY pPSListEntry);

BOOL IsEmptySListEntry(PSLISTENTRY pPSListEntry);

PSLISTENTRY NextSListEntry(PSLISTENTRY & pPSListEntry);

VOID ReleaseSListEntry(PSLISTENTRY pPSListEntry);

PSLISTENTRY RemoveSListEntry(PSLISTENTRY pPreSLE,PSLISTENTRY pRmvSLE);

typedef DWORD (*FILLMEMORYFUNCTION)(PBYTE dist,IN PVOID current,OUT PVOID * next);

SIZE_T CaculateFillMemoryBlockSize(FILLMEMORYFUNCTION fmfun, PVOID src);

PVOID AllocateAndFillMemoryBlock(FILLMEMORYFUNCTION fmfun, PVOID src, IN SIZE_T size);

