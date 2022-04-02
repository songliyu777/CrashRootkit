#ifdef __cplusplus
extern "C" {
#endif
#include <ntddk.h>
#include <string.h>
#ifdef __cplusplus
}; // extern "C"
#endif

#include "KeSysFun.h"
#include "PETools_R0.h"
#include "MyWinNT.h"
#include "Log.h"

void CopySections(const unsigned char *data, PIMAGE_NT_HEADERS old_headers, PMEMORYMODULE module,ULONG_PTR address)
{
	KdPrint(("拷贝内存节点 \n"));
	int i, size;
	unsigned char *codeBase = module->codeBase;
	unsigned char *dest;
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(module->headers);
	for (i=0; i<module->headers->FileHeader.NumberOfSections; i++, section++) {
		if (section->SizeOfRawData == 0) {
			// section doesn't contain data in the dll itself, but may define
			// uninitialized data
			size = old_headers->OptionalHeader.SectionAlignment;
			if (size > 0) {
				dest = codeBase + section->VirtualAddress;
				section->Misc.PhysicalAddress = (POINTER_TYPE)dest;
				RtlZeroMemory(dest,size);
			}

			// section is empty
			continue;
		}

		// commit memory block and copy data from dll
		dest = codeBase + section->VirtualAddress;
		RtlCopyMemory(dest,data + section->PointerToRawData,section->SizeOfRawData);
		if(!address)
			section->Misc.PhysicalAddress = (POINTER_TYPE)dest;
	}
}

void PerformBaseRelocation(PMEMORYMODULE module, SIZE_T delta)
{
	KdPrint(("处理重定位表 \n"));
	DWORD i;
	unsigned char *codeBase = module->codeBase;

	PIMAGE_DATA_DIRECTORY directory = GET_HEADER_DICTIONARY(module, IMAGE_DIRECTORY_ENTRY_BASERELOC);
	if (directory->Size > 0) {
		PIMAGE_BASE_RELOCATION relocation = (PIMAGE_BASE_RELOCATION) (codeBase + directory->VirtualAddress);
		for (DWORD size = 0; size<directory->Size; ) {
			size += relocation->SizeOfBlock;
			unsigned char *dest = codeBase + relocation->VirtualAddress;
			unsigned short *relInfo = (unsigned short *)((unsigned char *)relocation + IMAGE_SIZEOF_BASE_RELOCATION);
			for (i=0; i<((relocation->SizeOfBlock-IMAGE_SIZEOF_BASE_RELOCATION) / 2); i++, relInfo++) {
				DWORD *patchAddrHL;
#ifdef _WIN64
				ULONGLONG *patchAddr64;
#endif
				int type, offset;

				// the upper 4 bits define the type of relocation
				type = *relInfo >> 12;
				// the lower 12 bits define the offset
				offset = *relInfo & 0xfff;

				switch (type)
				{
				case IMAGE_REL_BASED_ABSOLUTE:
					// skip relocation
					break;

				case IMAGE_REL_BASED_HIGHLOW:
					// change complete 32 bit address
					patchAddrHL = (DWORD *) (dest + offset);
					*patchAddrHL += delta;
					break;

#ifdef _WIN64
				case IMAGE_REL_BASED_DIR64:
					patchAddr64 = (ULONGLONG *) (dest + offset);
					*patchAddr64 += delta;
					break;
#endif

				default:
					//printf("Unknown relocation: %d\n", type);
					break;
				}
			}

			// advance to next relocation block
			relocation = (PIMAGE_BASE_RELOCATION) (((char *) relocation) + relocation->SizeOfBlock);
		}
	}
}

BOOL FindModuleInformationByName(PSYSTEM_MODULE_INFORMATION pSmi,PCHAR pModuleName)
{
	KdPrint(("查找模块信息 模块名: %s \n",pModuleName));
	BOOL	isFind = FALSE;
	PCHAR   pKernelName;
	PIMAGE_FILE_HEADER    pfh;
	PIMAGE_OPTIONAL_HEADER    poh;
	PIMAGE_SECTION_HEADER    psh;
	HANDLE  hMod;
	POINTER_TYPE p;

	p = GetInfoTable(SystemModuleInformation);

	if(p==NULL)
		return isFind;

	POINTER_TYPE count = *(POINTER_TYPE*)p;

	PSYSTEM_MODULE_INFORMATION pModule = PSYSTEM_MODULE_INFORMATION(p+sizeof(POINTER_TYPE));

	for(ULONG i = 0;i < count;i++)
	{
		pKernelName = (PCHAR)((POINTER_TYPE)pModule->ImageName + pModule->PathLength);//获取模块名称
		if(pKernelName!=NULL && strlen(pKernelName)>0)
		{
			STRING kn,nl,na,n64g,n128g,mn,hal,halacpi,halmacpi,hal64g;
			//nt内核
			RtlInitAnsiString(&kn,pKernelName);
			RtlInitAnsiString(&nl,"ntoskrnl.exe");
			RtlInitAnsiString(&na,"ntkrnlpa.exe");
			RtlInitAnsiString(&n64g,"ntkl64g.exe");
			RtlInitAnsiString(&n128g,"ntkr128g.exe");

			//hal内核
			RtlInitAnsiString(&hal,"hal.dll");
			RtlInitAnsiString(&halacpi,"halacpi.dll");
			RtlInitAnsiString(&halmacpi,"halmacpi.dll");
			RtlInitAnsiString(&hal64g,"hal64g.dll"); //玩家内核XP64G

			if(pModuleName != NULL)
			{
				RtlInitAnsiString(&mn,pModuleName);
				if(RtlCompareString(&mn,&nl,TRUE)==0 || RtlCompareString(&mn,&na,TRUE)==0 || RtlCompareString(&mn,&n64g,TRUE)==0 || RtlCompareString(&mn,&n128g,TRUE)==0)
				{
					//nt内核都是第一个加载
					RtlCopyMemory(pSmi,pModule,sizeof(SYSTEM_MODULE_INFORMATION));
					isFind = TRUE;
					break;
				}
				//内核nt.exe模块有2种，一种是支持多CPU的，一种是支持单CPU的，在这里其实都是一样的
				if(RtlCompareString(&kn,&nl,TRUE)==0 || RtlCompareString(&kn,&na,TRUE)==0 || RtlCompareString(&kn,&n64g,TRUE)==0 || RtlCompareString(&kn,&n128g,TRUE)==0)
				{
					if(RtlCompareString(&mn,&nl,TRUE)==0 || RtlCompareString(&mn,&na,TRUE)==0 || RtlCompareString(&mn,&n64g,TRUE)==0 || RtlCompareString(&mn,&n128g,TRUE)==0)
					{
						RtlCopyMemory(pSmi,pModule,sizeof(SYSTEM_MODULE_INFORMATION));
						isFind = TRUE;
						break;				
					}
				}
				//内核hal模块有3种，一种是支持多CPU的，一种是支持单CPU的，在这里其实都是一样的
				if(RtlCompareString(&kn,&hal,TRUE)==0 || RtlCompareString(&kn,&halacpi,TRUE)==0 || RtlCompareString(&kn,&halmacpi,TRUE)==0 || RtlCompareString(&kn,&hal64g,TRUE)==0)
				{
					if(RtlCompareString(&mn,&hal,TRUE)==0 || RtlCompareString(&mn,&halacpi,TRUE)==0 || RtlCompareString(&mn,&halmacpi,TRUE)==0 || RtlCompareString(&mn,&hal64g,TRUE)==0)
					{
						RtlCopyMemory(pSmi,pModule,sizeof(SYSTEM_MODULE_INFORMATION));
						isFind = TRUE;
						break;				
					}
				}
				if(RtlCompareString(&kn,&mn,TRUE)==0)
				{
					RtlCopyMemory(pSmi,pModule,sizeof(SYSTEM_MODULE_INFORMATION));
					isFind = TRUE;
					break;
				}
			}
		}
		pModule++;
	}
	ExFreePool((PVOID)p);
	return isFind;
}

FARPROC MemoryGetProcAddress(PMEMORYMODULE module, const char *name)
{
	unsigned char *codeBase = module->codeBase;
	int idx=-1;
	DWORD i, *nameRef;
	WORD *ordinal;
	PIMAGE_EXPORT_DIRECTORY exports;
	PIMAGE_DATA_DIRECTORY directory = GET_HEADER_DICTIONARY((PMEMORYMODULE)module, IMAGE_DIRECTORY_ENTRY_EXPORT);
	if (directory->Size == 0) {
		// no export table found
		return NULL;
	}

	exports = (PIMAGE_EXPORT_DIRECTORY) (codeBase + directory->VirtualAddress);
	if (exports->NumberOfNames == 0 || exports->NumberOfFunctions == 0) {
		// DLL doesn't export anything
		return NULL;
	}

	// search function name in list of exported names
	nameRef = (DWORD *) (codeBase + exports->AddressOfNames);
	ordinal = (WORD *) (codeBase + exports->AddressOfNameOrdinals);
	for (i=0; i<exports->NumberOfNames; i++, nameRef++, ordinal++) {
		if (strcmp(name, (const char *) (codeBase + (*nameRef))) == 0) {
			idx = *ordinal;
			break;
		}
	}

	if (idx == -1) {
		// exported symbol not found
		return NULL;
	}

	if ((DWORD)idx > exports->NumberOfFunctions) {
		// name <-> ordinal number don't match
		return NULL;
	}

	// AddressOfFunctions contains the RVAs to the "real" functions
	return (FARPROC) (codeBase + (*(DWORD *) (codeBase + exports->AddressOfFunctions + (idx*4))));
}

FARPROC GetProcAddress(HMODULE hMod,LPCSTR lpFunctionName)
{
	PIMAGE_DOS_HEADER dos_header;
	PIMAGE_NT_HEADERS old_header;
	unsigned char *code, *headers;
	dos_header = (PIMAGE_DOS_HEADER)hMod;
	if (dos_header->e_magic != IMAGE_DOS_SIGNATURE) {
		KdPrint(("Not a valid executable file.\n"));
		return NULL;
	}

	old_header = (PIMAGE_NT_HEADERS)&((const unsigned char *)(hMod))[dos_header->e_lfanew];
	if (old_header->Signature != IMAGE_NT_SIGNATURE) {
		KdPrint(("No PE header found.\n"));
		return NULL;
	}

	if((ULONG_PTR)lpFunctionName<(ULONG_PTR)0x80000000)
	{
		DWORD idx = -1;
		DWORD i, *nameRef;
		WORD *ordinal;
		PIMAGE_EXPORT_DIRECTORY exports;
		PIMAGE_DATA_DIRECTORY directory = &old_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
		if (directory->Size == 0) {
			// no export table found
			return NULL;
		}

		exports = (PIMAGE_EXPORT_DIRECTORY) ((const unsigned char *)hMod + directory->VirtualAddress);
		if (exports->NumberOfNames == 0 || exports->NumberOfFunctions == 0) {
			// DLL doesn't export anything
			return NULL;
		}
		idx = (DWORD)lpFunctionName;
		idx = idx - exports->Base;
		nameRef = (DWORD *) ((const unsigned char *)hMod + exports->AddressOfNames);
		ordinal = (WORD *) ((const unsigned char *)hMod + exports->AddressOfNameOrdinals);
		return (FARPROC) ((const unsigned char *)hMod + (*(DWORD *) ((const unsigned char *)hMod + exports->AddressOfFunctions + (idx*4))));
	}
	//构造一个模块模版
	code = (unsigned char *)hMod;
	MEMORYMODULE result;
	RtlZeroMemory(&result,sizeof(MEMORYMODULE));
	result.codeBase = code;
	result.numModules = 0;
	result.modules = NULL;
	result.initialized = 0;
	headers = code;
	// copy PE header to code
	RtlCopyMemory(headers,dos_header,dos_header->e_lfanew + old_header->OptionalHeader.SizeOfHeaders);
	result.headers = (PIMAGE_NT_HEADERS)&((const unsigned char *)(headers))[dos_header->e_lfanew];
	// update position
	result.headers->OptionalHeader.ImageBase = (POINTER_TYPE)code;
	return MemoryGetProcAddress(&result,lpFunctionName);
}

int BuildImportTable(PMEMORYMODULE module,SIZE_T address)
{
	KdPrint(("构建输入表 \n"));
	int result=1;
	unsigned char *codeBase = module->codeBase;
	PIMAGE_DATA_DIRECTORY directory = GET_HEADER_DICTIONARY(module, IMAGE_DIRECTORY_ENTRY_IMPORT);
	if (directory->Size > 0) {
		PIMAGE_IMPORT_DESCRIPTOR importDesc = (PIMAGE_IMPORT_DESCRIPTOR) (codeBase + directory->VirtualAddress);
		for (; MmIsAddressValid(importDesc) && importDesc->Name; importDesc++) {
			POINTER_TYPE *thunkRef;
			FARPROC *funcRef;
			HMODULE handle,*modules;
			LPCSTR moduleName = (LPCSTR) (codeBase + importDesc->Name);
			SYSTEM_MODULE_INFORMATION smi;
			if(FindModuleInformationByName(&smi,(PCHAR)moduleName))
			{
				handle = (HMODULE)smi.Base;
			}
			else
			{
				if(!address)
				{
					KdPrint(("Can't find library . It must have loaded the library before this! \n"));
					PrintLog(L"Can't find library [%S] \r\n",moduleName);
					result = 0;
					break;
				}
				
				else
				{
					continue;
				}
			}

			modules = (HMODULE *)ExAllocatePool(NonPagedPool,(module->numModules+1)*(sizeof(HMODULE)));
			if (modules == NULL) {
				if(module->modules!=NULL)
					ExFreePool(module->modules);
				result = 0;
				break;
			}
			if(module->modules!=NULL)
			{
				RtlCopyMemory(modules,module->modules,(module->numModules)*(sizeof(HMODULE)));
				ExFreePool(module->modules);
			}
			module->modules = modules;
			module->modules[module->numModules++] = handle;
			if (importDesc->OriginalFirstThunk) {
				thunkRef = (POINTER_TYPE *) (codeBase + importDesc->OriginalFirstThunk);
				funcRef = (FARPROC *) (codeBase + importDesc->FirstThunk);
			} else {
				// no hint table
				thunkRef = (POINTER_TYPE *) (codeBase + importDesc->FirstThunk);
				funcRef = (FARPROC *) (codeBase + importDesc->FirstThunk);
			}
			for (; *thunkRef; thunkRef++, funcRef++) {
				if (IMAGE_SNAP_BY_ORDINAL(*thunkRef)) {
					*funcRef = (FARPROC)GetProcAddress(handle, (LPCSTR)IMAGE_ORDINAL(*thunkRef));
				} else {
					PIMAGE_IMPORT_BY_NAME thunkData = (PIMAGE_IMPORT_BY_NAME) (codeBase + (*thunkRef));
					*funcRef = (FARPROC)GetProcAddress(handle, (LPCSTR)&thunkData->Name);
				}
				if (*funcRef == 0) {
					result = 0;
					break;
				} else {
					KdPrint(("获的函数地址 %p \n",*funcRef));
				}
			}

			if (!result) {
				break;
			}
		}
	}

	return result;
}

void FinalizeSections(PMEMORYMODULE module)
{
	KdPrint(("处理模块内存节点权限 \n"));
	int i;
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(module->headers);
#ifdef _WIN64
	POINTER_TYPE imageOffset = (module->headers->OptionalHeader.ImageBase & 0xffffffff00000000);
#else
#define imageOffset 0
#endif

	// loop through all sections and change access flags
	for (i=0; i<module->headers->FileHeader.NumberOfSections; i++, section++) {
		DWORD protect, oldProtect, size;
		int executable = (section->Characteristics & IMAGE_SCN_MEM_EXECUTE) != 0;
		int readable =   (section->Characteristics & IMAGE_SCN_MEM_READ) != 0;
		int writeable =  (section->Characteristics & IMAGE_SCN_MEM_WRITE) != 0;

		if (section->Characteristics & IMAGE_SCN_MEM_DISCARDABLE) {
			// section is not needed any more and can safely be freed
			//VirtualFree((LPVOID)((POINTER_TYPE)section->Misc.PhysicalAddress | imageOffset), section->SizeOfRawData, MEM_DECOMMIT);
			continue;
		}

		// determine protection flags based on characteristics
		protect = ProtectionFlags[executable][readable][writeable];
		if (section->Characteristics & IMAGE_SCN_MEM_NOT_CACHED) {
			protect |= PAGE_NOCACHE;
		}

		// determine size of region
		size = section->SizeOfRawData;
		if (size == 0) {
			if (section->Characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA) {
				size = module->headers->OptionalHeader.SizeOfInitializedData;
			} else if (section->Characteristics & IMAGE_SCN_CNT_UNINITIALIZED_DATA) {
				size = module->headers->OptionalHeader.SizeOfUninitializedData;
			}
		}

		if (size > 0) {
			// change memory access flags
			//if (VirtualProtect((LPVOID)((POINTER_TYPE)section->Misc.PhysicalAddress | imageOffset), size, protect, &oldProtect) == 0)
			//	KdPrint(("Error protecting memory page \n"));
		}
	}
#ifndef _WIN64
#undef imageOffset
#endif
}

void MemoryFreeLibrary(PMEMORYMODULE mod)
{
	KdPrint(("释放内存模块 \n"));
	int i;
	PMEMORYMODULE module = mod;

	if (module != NULL) {
		if (module->initialized != 0) {
			// notify library about detaching from process
			module->initialized = 0;
		}

		if (module->modules != NULL) {
			// free previously opened libraries
			for (i=0; i<module->numModules; i++) {
				if (module->modules[i] != NULL) {
					module->modules[i] = NULL;
				}
			}
			ExFreePool(module->modules);
			module->modules=NULL;
		}

		if (module->codeBase != NULL) {
			// release memory of library
			ExFreePool(module->codeBase);
		}

		ExFreePool(mod);
	}
}

PMEMORYMODULE MemoryLoadLibrary(const void *data,ULONG_PTR address)
{
	KdPrint(("加载内存模块 \n"));
	PMEMORYMODULE result;
	PIMAGE_DOS_HEADER dos_header;
	PIMAGE_NT_HEADERS old_header;
	unsigned char *code, *headers;
	SIZE_T locationDelta;
	BOOL successfull;

	dos_header = (PIMAGE_DOS_HEADER)data;
	if (dos_header->e_magic != IMAGE_DOS_SIGNATURE) {
		KdPrint(("Not a valid executable file.\r\n"));
		PrintLog(L"Not a valid executable file. \r\n");
		return NULL;
	}

	old_header = (PIMAGE_NT_HEADERS)&((const unsigned char *)(data))[dos_header->e_lfanew];
	if (old_header->Signature != IMAGE_NT_SIGNATURE) {
		KdPrint(("No PE header found.\n"));
		PrintLog(L"No PE header found.\r\n");
		return NULL;
	}

	// reserve memory for image of library
	code = (unsigned char *)ExAllocatePool(NonPagedPool,old_header->OptionalHeader.SizeOfImage);
	if (code == NULL) 
	{
		KdPrint(("Can't reserve memory \n"));
		PrintLog(L"Can't reserve memory.\r\n");
		return NULL;
	}

	result = (PMEMORYMODULE)ExAllocatePool(NonPagedPool,sizeof(MEMORYMODULE));
	RtlZeroMemory(result,sizeof(MEMORYMODULE));
	result->codeBase = code;
	result->SizeOfImage = old_header->OptionalHeader.SizeOfImage;
	result->numModules = 0;
	result->modules = NULL;
	result->initialized = 0;
	headers = code;
	// copy PE header to code
	RtlCopyMemory(headers,dos_header,dos_header->e_lfanew + old_header->OptionalHeader.SizeOfHeaders);
	//memcpy(headers, dos_header, dos_header->e_lfanew + old_header->OptionalHeader.SizeOfHeaders);
	result->headers = (PIMAGE_NT_HEADERS)&((const unsigned char *)(headers))[dos_header->e_lfanew];
	// update position
	result->headers->OptionalHeader.ImageBase = (POINTER_TYPE)code;
	// copy sections from DLL file block to new memory location
	CopySections((const unsigned char *)data, old_header, result,address);
	// adjust base address of imported data
	if(address)
	{
		locationDelta = (SIZE_T)(address - old_header->OptionalHeader.ImageBase);
	}
	else
	{
		locationDelta = (SIZE_T)(code - old_header->OptionalHeader.ImageBase);
	}
	if (locationDelta != 0) 
	{
		PerformBaseRelocation(result, locationDelta);
	}

	// load required dlls and adjust function table of imports
	if (!BuildImportTable(result,address)) {
		PrintLog(L"Can't build import table.\r\n");
		goto error;
	}

	// mark memory pages depending on section headers and release
	// sections that are marked as "discardable"
	FinalizeSections(result);

	// get entry point of loaded library
	if (result->headers->OptionalHeader.AddressOfEntryPoint != 0) {
		result->initialized = 1;
	}
	KdPrint(("Module Base [%p] \r\n",result->codeBase));
	PrintLog(L"Module Base [%p] \r\n",result->codeBase);
	return (PMEMORYMODULE)result;

error:
	// cleanup
	MemoryFreeLibrary(result);
	return NULL;
}

PBYTE GetSectionByName(const unsigned char *data, OUT SIZE_T* size,const char * name)
{
	PIMAGE_DOS_HEADER dos_header;
	PIMAGE_NT_HEADERS nt_header;
	STRING SectionName,TextName;

	__try
	{	
		dos_header = (PIMAGE_DOS_HEADER)data;
		if (dos_header->e_magic != IMAGE_DOS_SIGNATURE) {
			KdPrint(("Not a valid executable file.\r\n"));
			PrintLog(L"Not a valid executable file. \r\n");
			return NULL;
		}

		nt_header = (PIMAGE_NT_HEADERS)&((const unsigned char *)(data))[dos_header->e_lfanew];
		if (nt_header->Signature != IMAGE_NT_SIGNATURE) {
			KdPrint(("No PE header found.\n"));
			PrintLog(L"No PE header found.\r\n");
			return NULL;
		}

		RtlInitAnsiString(&TextName,name);

		PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(nt_header);
		WORD i;
		for (i=0; i<nt_header->FileHeader.NumberOfSections; i++, section++) 
		{
			if (section->SizeOfRawData == 0) {
				// section doesn't contain data in the dll itself, but may define
				// uninitialized data
				// section is empty
				continue;
			}
			RtlInitAnsiString(&SectionName,(PCSZ)section->Name);
			if(RtlCompareString(&SectionName,&TextName,TRUE)==0)
			{
				*size = section->Misc.VirtualSize;
				return PBYTE(data + section->VirtualAddress);
			}
		}

	} 
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		PrintLog(L"GetSectionByName exception.\r\n");
	}

	return NULL;
}

PBYTE GetSectionByCharacteristics(const unsigned char *data, OUT SIZE_T* size,DWORD Characteristics,PBYTE prevsection)
{
	PIMAGE_DOS_HEADER dos_header;
	PIMAGE_NT_HEADERS nt_header;

	__try
	{	
		dos_header = (PIMAGE_DOS_HEADER)data;
		if (dos_header->e_magic != IMAGE_DOS_SIGNATURE) {
			KdPrint(("Not a valid executable file.\r\n"));
			PrintLog(L"Not a valid executable file. \r\n");
			return NULL;
		}

		nt_header = (PIMAGE_NT_HEADERS)&((const unsigned char *)(data))[dos_header->e_lfanew];
		if (nt_header->Signature != IMAGE_NT_SIGNATURE) {
			KdPrint(("No PE header found.\n"));
			PrintLog(L"No PE header found.\r\n");
			return NULL;
		}

		PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(nt_header);
		WORD i;
		for (i=0; i<nt_header->FileHeader.NumberOfSections; i++, section++) 
		{
			if (section->SizeOfRawData == 0) {
				// section doesn't contain data in the dll itself, but may define
				// uninitialized data
				// section is empty
				continue;
			}
			if(section->Characteristics & Characteristics)
			{

				if(ULONG_PTR(data + section->VirtualAddress) > (ULONG_PTR)prevsection)
				{
					*size = section->Misc.VirtualSize;
					return PBYTE(data + section->VirtualAddress);
				}
				continue;
			}
		}
	} 
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		PrintLog(L"GetSectionByCharacteristics exception.\r\n");
	}

	return NULL;
}

PIMAGE_DATA_DIRECTORY GetDataDirectory(HMODULE hMod,DWORD index)
{
	PIMAGE_DOS_HEADER dos_header;
	PIMAGE_NT_HEADERS old_header;
	unsigned char *code, *headers;
	dos_header = (PIMAGE_DOS_HEADER)hMod;
	if (dos_header->e_magic != IMAGE_DOS_SIGNATURE) {
		KdPrint(("Not a valid executable file.\n"));
		return NULL;
	}

	old_header = (PIMAGE_NT_HEADERS)&((const unsigned char *)(hMod))[dos_header->e_lfanew];
	if (old_header->Signature != IMAGE_NT_SIGNATURE) {
		KdPrint(("No PE header found.\n"));
		return NULL;
	}

	PIMAGE_DATA_DIRECTORY directory = &old_header->OptionalHeader.DataDirectory[index];
	if (directory->Size == 0) {
		// no export table found
		return NULL;
	}
	
	return directory;
}