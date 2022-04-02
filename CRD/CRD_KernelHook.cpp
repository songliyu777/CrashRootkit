#ifdef __cplusplus
extern "C" {
#endif
#include <ntddk.h>
#include <string.h>
#ifdef __cplusplus
}; // extern "C"
#endif

#include "CRD_KernelHook.h"
#include "CRD_SSDT.h"
#include "CRD_Process.h"
#include "KeSysFun.h"
#include "MyWinNT.h"
#include "MyListEntry.h"
#include "Log.h"
#include "Relocate.h"

SLISTENTRY g_sleMemeryHookHead = {0};

ULONG_PTR g_dwNtKrBase = 0;								//Nt核心基地址
ULONG_PTR g_dwWin32kBase = 0;							//Win32k核心基地址

SIZE_T NtKrCheck()
{
	FreeKernelHook();

	ULONG_PTR p = GetInfoTable(SystemModuleInformation);

	if(p==NULL)
		return 0;

	ULONG_PTR   start;
	ULONG_PTR	end;
	PVOID buff = NULL;
	SIZE_T outputlen = 0;
	SIZE_T size = 0,section_size1 = 0,section_size2 = 0;
	PBYTE section1,section2;
	NTSTATUS status;

	PSYSTEM_MODULE_INFORMATION pModule = PSYSTEM_MODULE_INFORMATION(p + sizeof(POINTER_TYPE));

	POINTER_TYPE count = *(POINTER_TYPE*)p;

	NtKrexeCheck(pModule);
	Win32ksysCheck(pModule,count);

	ExFreePool((PVOID)p); 

	PSLISTENTRY pSListEntry = &g_sleMemeryHookHead;
	while(!IsEmptySListEntry(pSListEntry))
	{
		PMEMORYHOOK pmh = (PMEMORYHOOK)NextSListEntry(pSListEntry);
		if(pmh)
		{
			KdPrint(("Address %X Length %d\r\n",pmh->Address,pmh->Length));
		}
	}
	outputlen = CaculateFillMemoryBlockSize(FillMemoryFunction,g_sleMemeryHookHead.Next);

	return outputlen;
}

//处理核心文件
VOID NtKrexeCheck(PSYSTEM_MODULE_INFORMATION pModule)
{
	PVOID buff = NULL;
	SIZE_T size = 0,section_size1 = 0,section_size2 = 0,hooklength = 0;
	ULONG_PTR hookaddress = 0;
	PBYTE section1,section2;
	NTSTATUS status;

	CHAR szModulePath[MAX_PATH] = {0};
	ANSI_STRING aModulePath;
	ANSI_STRING aImageName,aKernelImageName;
	UNICODE_STRING wModulePath;
	strcat(szModulePath,"\\SystemRoot");

	g_dwNtKrBase = pModule->Base;

	for(SIZE_T i = 1;i < pModule->PathLength;i++)
	{
		if(pModule->ImageName[i]=='\\')
		{
			strcat(szModulePath, pModule->ImageName + i);
			break;
		}
	}

	RtlInitAnsiString(&aModulePath,szModulePath);
	RtlAnsiStringToUnicodeString(&wModulePath,&aModulePath,TRUE);

	if(GetMemoryModuleNtKr())
	{
		DWORD count = 0;
		section1 = NULL;
		section2 = NULL;
		do
		{
			section1 = GetSectionByCharacteristics((const unsigned char *)pModule->Base,&section_size1,0x20,section1);
			section2 = GetSectionByCharacteristics((const unsigned char *)GetMemoryModuleNtKr()->codeBase,&section_size2,0x20,section2);
			if(section2 && section1 && section_size1 == section_size2)
			{
				KdPrint(("length equal \r\n"));
				count++;
				ULONG_PTR beginadr = -1,endadr = -1,codesame = 0;
				for(SIZE_T i=0;i<section_size1;i++)
				{
					if(section1[i]!=section2[i])
					{
						codesame = 0;
						if(beginadr==-1)
							beginadr = i;
					}
					else
					{
						if(beginadr!=-1)
						{	
							if(codesame>2 || section_size1 - i<3)
							{
								endadr = i - codesame;
								hookaddress = ULONG_PTR(section1 + beginadr);
								hooklength = endadr - beginadr;
								HookType hooktype = GetHookType((const unsigned char *)pModule->Base,(const unsigned char *)GetMemoryModuleNtKr()->codeBase,hookaddress,hooklength);
								ULONG_PTR jmpaddress = GetJmpAddress(hookaddress, hooklength,hooktype);
								if(AddMemoryHookEntry(wModulePath.Buffer,hookaddress,section2 + beginadr,section1 + beginadr,hooklength,hooktype,jmpaddress))
								{
									KdPrint(("添加内存Hook实例成功\r\n"));
								}
								else
								{
									KdPrint(("添加内存Hook实例失败\r\n"));
								}
								beginadr=-1;
							}
							codesame++;
						}
					}
				}
			}
		}while(section1!=NULL && section2!=NULL && count < 5);
	}
	RtlFreeUnicodeString(&wModulePath);
}

//处理Win32k.sys
VOID Win32ksysCheck(PSYSTEM_MODULE_INFORMATION pModule,POINTER_TYPE count)
{
	PVOID buff = NULL;
	SIZE_T size = 0,section_size1 = 0,section_size2 = 0;
	PBYTE section1,section2;
	NTSTATUS status;
	CHAR szModulePath[MAX_PATH] = {0};
	ANSI_STRING aModulePath;
	ANSI_STRING aImageName,aKernelImageName;
	UNICODE_STRING wModulePath;

	RtlInitAnsiString(&aImageName,"win32k.sys");

	for(ULONG i = 0;i < count;i++)
	{
		PCHAR pKernelName = (PCHAR)((POINTER_TYPE)pModule->ImageName + pModule->PathLength);//获取模块名称
		if(pKernelName!=NULL && strlen(pKernelName)>0)
		{
			RtlInitAnsiString(&aKernelImageName,pKernelName);
			if(!RtlCompareString(&aKernelImageName,&aImageName,TRUE))
			{
				g_dwWin32kBase = (DWORD)pModule->Base;
				break;
			}
		}
		pModule++;
	}

	if(!g_dwWin32kBase)
		return;

	RtlZeroMemory(szModulePath,MAX_PATH);
	strcat(szModulePath,"\\SystemRoot");
	for(SIZE_T i = 1;i < pModule->PathLength;i++)
	{
		if(pModule->ImageName[i]=='\\')
		{
			strcat(szModulePath, pModule->ImageName + i);
			break;
		}
	}

	RtlInitAnsiString(&aModulePath,szModulePath);
	RtlAnsiStringToUnicodeString(&wModulePath,&aModulePath,TRUE);

	RtlFreeUnicodeString(&wModulePath);
}

BOOL AddMemoryHookEntry(PWCHAR ModuleName,ULONG_PTR Address,PBYTE Origin,PBYTE Current,SIZE_T Length,HookType hooktype,ULONG_PTR jmpaddress)
{
	PMEMORYHOOK pmh = (PMEMORYHOOK)ExAllocatePool(NonPagedPool,sizeof(MEMORYHOOK));
	if(pmh)
	{
		wcscpy(pmh->ModuleName,ModuleName);
		pmh->Address = Address;
		pmh->Origin = Origin;
		pmh->Current = Current;
		pmh->Length = Length;
		pmh->Type = hooktype;
		pmh->JmpAddress = jmpaddress;
		PushSLISTENTRY(&g_sleMemeryHookHead,&pmh->next);
		return TRUE;
	}
	return FALSE;
}

DWORD FillMemoryFunction(PBYTE dist,IN PVOID current,OUT PVOID * next)//空为返回长度请求 src为下一个指针的
{
	if(dist==NULL)
	{
		if(current)
		{
			PMEMORYHOOK pmh = (PMEMORYHOOK)current;
			DWORD count = MAX_PATH *sizeof(WCHAR) + sizeof(ULONG_PTR) + sizeof(ULONG_PTR) + sizeof(HookType) + sizeof(DWORD) + pmh->Length + pmh->Length;
			*next = ((PSLISTENTRY)current)->Next;
			return count;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		if(current)
		{
			PMEMORYHOOK pmh = (PMEMORYHOOK)current;
			DWORD count = MAX_PATH *sizeof(WCHAR) + sizeof(ULONG_PTR) + sizeof(ULONG_PTR) + sizeof(HookType) + sizeof(DWORD) + pmh->Length + pmh->Length;
			RtlCopyMemory(dist,pmh->ModuleName,sizeof(pmh->ModuleName));
			dist += sizeof(pmh->ModuleName);
			RtlCopyMemory(dist,&pmh->Address,sizeof(ULONG_PTR));
			dist += sizeof(ULONG_PTR);
			RtlCopyMemory(dist,&pmh->JmpAddress,sizeof(ULONG_PTR));
			dist += sizeof(ULONG_PTR);
			RtlCopyMemory(dist,&pmh->Type,sizeof(HookType));
			dist += sizeof(HookType);
			RtlCopyMemory(dist,&pmh->Length,sizeof(DWORD));
			dist += sizeof(DWORD);
			RtlCopyMemory(dist,pmh->Origin,pmh->Length);
			dist += pmh->Length;
			RtlCopyMemory(dist,pmh->Current,pmh->Length);
			*next = ((PSLISTENTRY)current)->Next;
			return count;
		}
		else
		{
			return 0;
		}
		
	}
}

VOID NtKrGet(PVOID & buff,SIZE_T size)
{
	buff = AllocateAndFillMemoryBlock(FillMemoryFunction,g_sleMemeryHookHead.Next,size);
}

VOID FreeKernelHook()
{
	ReleaseSListEntry(&g_sleMemeryHookHead);
}

NTSTATUS RecoverKernelHook(PTransferMsg msg)
{
	PMemoryHook pmh = (PMemoryHook)msg->buff;
	if(!GetCsrssEprocess())
		return STATUS_UNSUCCESSFUL;
	PVOID dist = (PVOID)pmh->Address;
	DWORD lenght = pmh->Length;
	PVOID origin = PVOID(ULONG_PTR(&pmh->Length) + sizeof(DWORD));
	PVOID current = PVOID(ULONG_PTR(origin) + lenght);
	KeAttachProcess(GetCsrssEprocess());
	PAGEPROTECTOFF();
	KIRQL Irql;
	KeRaiseIrql(DISPATCH_LEVEL,&Irql);
	if(RtlCompareMemory(dist,current,lenght)==lenght)
	{
		RtlCopyMemory(dist,origin,lenght);
	}
	KeLowerIrql(Irql);
	PAGEPROTECTON();
	KeDetachProcess();
	return STATUS_SUCCESS;
}

HookType GetHookType(const unsigned char *currentdata, const unsigned char *originaldata, ULONG_PTR hookaddress, SIZE_T length)
{
	if(length==4)
	{
		PIMAGE_DATA_DIRECTORY directory;
		directory = GetDataDirectory((HMODULE)originaldata,IMAGE_DIRECTORY_ENTRY_IMPORT);
		if(directory && directory->Size)
		{
			DWORD result = 1;
			PIMAGE_IMPORT_DESCRIPTOR importDesc = (PIMAGE_IMPORT_DESCRIPTOR) (originaldata + directory->VirtualAddress);
			for (;(ULONG_PTR)importDesc < (ULONG_PTR)originaldata + directory->VirtualAddress + directory->Size && MmIsAddressValid(importDesc) && importDesc->Name; importDesc++) {
				POINTER_TYPE *thunkRef;
				FARPROC *funcRef;
				DWORD count = 0;
				if (importDesc->OriginalFirstThunk) {
					thunkRef = (POINTER_TYPE *) (originaldata + importDesc->OriginalFirstThunk);
					funcRef = (FARPROC *) (originaldata + importDesc->FirstThunk);
				} else {
					// no hint table
					thunkRef = (POINTER_TYPE *) (originaldata + importDesc->FirstThunk);
					funcRef = (FARPROC *) (originaldata + importDesc->FirstThunk);
				}
				for (; *thunkRef; thunkRef++, funcRef++) {
					if (*funcRef == 0) {
						result = 0;
						break;
					}
					count++; 
				}
				funcRef = (FARPROC *) (currentdata + importDesc->FirstThunk);
				if(hookaddress >= (ULONG_PTR)funcRef && hookaddress < (ULONG_PTR)funcRef + count * sizeof(FARPROC *))
				{
					return IAT_HOOK;
				}
				if (!result) {
					break;
				}
			}
		}
		directory = GetDataDirectory((HMODULE)originaldata,IMAGE_DIRECTORY_ENTRY_EXPORT);
		if(directory && directory->Size && hookaddress > (ULONG_PTR)currentdata + directory->VirtualAddress && hookaddress < (ULONG_PTR)currentdata + directory->VirtualAddress + directory->Size)
		{
			return EAT_HOOK;
		}
		if(GetCurrentSSDT())
		{
			if(hookaddress >= ULONG_PTR(GetCurrentSSDT()->pvSSDTBase) && hookaddress < ULONG_PTR(GetCurrentSSDT()->pvSSDTBase) + GetCurrentSSDT()->ulNumberOfServices * sizeof(ULONG))
			{
				return SSDT_HOOK;
			}
		}
		if(GetCurrentSSDTShadow())
		{
			if(hookaddress >= ULONG_PTR(GetCurrentSSDTShadow()->pvSSDTShadowBase) && hookaddress < ULONG_PTR(GetCurrentSSDTShadow()->pvSSDTShadowBase) + GetCurrentSSDTShadow()->ulNumberOfServicesShadow * sizeof(ULONG))
			{
				return SSDT_HOOK;
			}
		}
		if(MmIsAddressValid((PVOID)hookaddress))
		{
			return INLINE_HOOK;
		}
	}
	if(MmIsAddressValid((PVOID)hookaddress))
	{
		if(*(PBYTE)hookaddress == 0xE9 && length>4)
		{
			return INLINE_HOOK;
		}
		if(*(PBYTE)hookaddress == 0xB8 && length==7)
		{
			return INLINE_HOOK;
		}
	}
	return UNKNOW_HOOK;
}

ULONG_PTR GetJmpAddress_Head_E9(ULONG_PTR address)
{
	if(MmIsAddressValid((PVOID)address))
	{
		if(*(PBYTE)address == 0xE9 && MmIsAddressValid(PVOID(address + 1)))
		{
			address += *PULONG_PTR(address + 1) + 5;
			address = GetJmpAddress_Head_E9(address);
		}
	}
	return address;
}

ULONG_PTR GetJmpAddress(ULONG_PTR hookaddress, SIZE_T length,HookType type)
{
	ULONG_PTR address = hookaddress;
	switch(type)
	{
	case IAT_HOOK:
	case EAT_HOOK:
	case SSDT_HOOK:
		address = *PULONG_PTR(address);
		break;
	case INLINE_HOOK:
		{
			if(length==4)
			{
				address += *PULONG_PTR(address) + 4;
				address = GetJmpAddress_Head_E9(address);
			}
			if(length==5)
			{
				address = GetJmpAddress_Head_E9(address);
			}
			if(length==7 && *(PBYTE)address == (BYTE)0xB8)
			{
				address = *PULONG_PTR(address + 1);
			}
		}
		break;
	case UNKNOW_HOOK:
		address = 0;
		break;
	}
	return address;
}