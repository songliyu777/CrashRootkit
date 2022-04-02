#ifdef __cplusplus
extern "C" {
#endif
#include <ntddk.h>
#include <string.h>
#ifdef __cplusplus
}; // extern "C"
#endif

#include "MyListEntry.h"
#include "KeSysFun.h"
#include "Log.h"


VOID InitSLISTENTRY(PSLISTENTRY pPSListEntry)
{
	pPSListEntry->Next = NULL;
}

VOID PushSLISTENTRY(PSLISTENTRY pPSListEntry,PSLISTENTRY pCSListEntry)
{
	pCSListEntry->Next = NULL;
	PSLISTENTRY pSListEntry = pPSListEntry;
	while(pSListEntry->Next!=NULL && MmIsAddressValid(pSListEntry->Next))
	{
		pSListEntry = pSListEntry->Next;
	}
	pSListEntry->Next = pCSListEntry;
}

BOOL IsEmptySListEntry(PSLISTENTRY pPSListEntry)
{
	if(pPSListEntry && pPSListEntry->Next && MmIsAddressValid(pPSListEntry->Next))
		return FALSE;
	return TRUE;
}

PSLISTENTRY NextSListEntry(PSLISTENTRY & pPSListEntry)
{
	PSLISTENTRY pCurrentSLE = pPSListEntry->Next;
	pPSListEntry = pPSListEntry->Next;
	return pCurrentSLE;
}

VOID ReleaseSListEntry(PSLISTENTRY pPSListEntry)
{
	PSLISTENTRY pPreSLE = NULL,pSListEntry = pPSListEntry;
	while(!IsEmptySListEntry(pSListEntry))
	{
		PSLISTENTRY pCurSLE = NextSListEntry(pSListEntry);
		if(pPreSLE!=NULL)
		{
			ExFreePool(pPreSLE);
		}
		pPreSLE = pCurSLE;
	}
	if(pPreSLE!=NULL)
	{
		ExFreePool(pPreSLE);
	}
	pPSListEntry->Next = NULL;
}

PSLISTENTRY RemoveSListEntry(PSLISTENTRY pPreSLE,PSLISTENTRY pRmvSLE)
{
	if(pPreSLE && pRmvSLE)
	{
		pPreSLE->Next = pRmvSLE->Next;
		ExFreePool(pRmvSLE);
	}
	return pPreSLE;
}

SIZE_T SizeOfSListEntry(PSLISTENTRY pPSListEntry)
{
	PSLISTENTRY pSListEntry = pPSListEntry;
	SIZE_T count = 0;
	while(!IsEmptySListEntry(pSListEntry))
	{
		PSLISTENTRY pCurSLE = NextSListEntry(pSListEntry);
		count++; 
	}
	return count;
}

SIZE_T CaculateFillMemoryBlockSize(FILLMEMORYFUNCTION fmfun, PVOID src)
{
	if(!fmfun)
		return NULL;
	PBYTE curbuff = NULL,prebuff=NULL;
	PVOID next = NULL;
	DWORD all_size = 0;
	DWORD length = fmfun(NULL,src,&next);
	while(length)
	{
		all_size += (sizeof(DWORD) + length);
		src = next;
		length = fmfun(NULL,src,&next);
	}
	return all_size;
}

PVOID AllocateAndFillMemoryBlock(FILLMEMORYFUNCTION fmfun, PVOID src, IN SIZE_T size)
{
	if(!fmfun||!size)
		return NULL;
	PBYTE curbuff = NULL;
	curbuff = (PBYTE)ExAllocatePool(NonPagedPool,size);
	if(!curbuff)
	{
		goto AllocateAndFillMemoryBlock_ERROR;
	}
	PBYTE offset = curbuff;
	DWORD offset_size = 0;
	PVOID next = NULL;
	DWORD length = fmfun(NULL,src,&next);
	while(length)
	{
		*PDWORD(offset) = length;
		offset+=sizeof(DWORD);
		if(length != fmfun(offset,src,&next))
		{
			goto AllocateAndFillMemoryBlock_ERROR;
		}
		offset+=length;
		src = next;
		length = fmfun(NULL,src,&next);
	}
	return curbuff;
AllocateAndFillMemoryBlock_ERROR:
	if(curbuff)
		ExFreePool(curbuff);
	return NULL;
}