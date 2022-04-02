#pragma once
#include "MyWinNT.h"
#include "MyListEntry.h"
#define _CRD_
#include "Transfer.h"

typedef struct _MODULEINFO
{
	SLISTENTRY next;
	ULONG_PTR BaseAddress;
	ULONG_PTR EntryPoint;
	ULONG_PTR SizeOfImage;
	WCHAR FullDllName[MAX_PATH];
}MODULEINFO,*PMODULEINFO;

typedef struct _MODULEHOOKCHECKINFO
{
	PBYTE TextSection;
	SIZE_T TextSectionSize;
}MODULEHOOKCHECKINFO,*PMODULEHOOKCHECKINFO;

SIZE_T ListModuleOfProcess(IN PEPROCESS peprocess);
BOOL AddModuleInfo(PLDR_MODULE pModule);
VOID FreeModuleInfo();
VOID GetModules(PVOID buff,SIZE_T size);
SIZE_T GetModulePeInfo(PVOID buff,SIZE_T size);
BOOL HideProcessModuleByFullPath(IN PEPROCESS pEprocess, PUNICODE_STRING DllFullPath);
NTSTATUS HideProcessModule(PTransferMsg msg);