

#include "stdafx.h"
#include <Windows.h>
#include <winnt.h>
#include "Log.h"
#include "PETools_R3.h"
#include <string>
#include <algorithm>

using namespace std;

#ifndef IMAGE_SIZEOF_BASE_RELOCATION
// Vista SDKs no longer define IMAGE_SIZEOF_BASE_RELOCATION!?
#define IMAGE_SIZEOF_BASE_RELOCATION (sizeof(IMAGE_BASE_RELOCATION))
#endif

typedef BOOL (WINAPI *DllEntryProc)(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved);

#define GET_HEADER_DICTIONARY(module, idx)	&(module)->headers->OptionalHeader.DataDirectory[idx]

static void CopySections(const unsigned char *data, PIMAGE_NT_HEADERS old_headers, PMEMORYMODULE module,ULONG_PTR address)
{
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
				dest = (unsigned char *)VirtualAlloc(codeBase + section->VirtualAddress,
					size,
					MEM_COMMIT,
					PAGE_READWRITE);
				if(!address)
				{
					section->Misc.PhysicalAddress = (POINTER_TYPE)dest;
				}
				memset(dest, 0, size);
			}

			// section is empty
			continue;
		}

		// commit memory block and copy data from dll
		dest = (unsigned char *)VirtualAlloc(codeBase + section->VirtualAddress,
							section->SizeOfRawData,
							MEM_COMMIT,
							PAGE_READWRITE);
		memcpy(dest, data + section->PointerToRawData, section->SizeOfRawData);
		if(!address)
		{
			section->Misc.PhysicalAddress = (POINTER_TYPE)dest;
		}
	}
}

// Protection flags for memory pages (Executable, Readable, Writeable)
static int ProtectionFlags[2][2][2] = {
	{
		// not executable
		{PAGE_NOACCESS, PAGE_WRITECOPY},
		{PAGE_READONLY, PAGE_READWRITE},
	}, {
		// executable
		{PAGE_EXECUTE, PAGE_EXECUTE_WRITECOPY},
		{PAGE_EXECUTE_READ, PAGE_EXECUTE_READWRITE},
	},
};

static void FinalizeSections(PMEMORYMODULE module)
{
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
			VirtualFree((LPVOID)((POINTER_TYPE)section->Misc.PhysicalAddress | imageOffset), section->SizeOfRawData, MEM_DECOMMIT);
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
			if (VirtualProtect((LPVOID)((POINTER_TYPE)section->Misc.PhysicalAddress | imageOffset), size, protect, &oldProtect) == 0)
			{
				PrintLog(L"Error protecting memory page");
			}
		}
	}
#ifndef _WIN64
#undef imageOffset
#endif
}

static void PerformBaseRelocation(PMEMORYMODULE module, SIZE_T delta,SIZE_T ImageBase)
{
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
					*patchAddrHL += (delta - ImageBase);
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

FARPROC WINAPI GetRemoteProcAddressA(HMODULE hModuleBuff,unsigned char * realcodebase ,LPCSTR lpProcName,char * outputname)
{
	unsigned char *codeBase = (unsigned char *)hModuleBuff;
	PIMAGE_DOS_HEADER dos_header = (PIMAGE_DOS_HEADER)codeBase;
	PIMAGE_NT_HEADERS nt_header = (PIMAGE_NT_HEADERS)&((const unsigned char *)(codeBase))[dos_header->e_lfanew];
	int idx=-1;
	DWORD i, *nameRef;
	WORD *ordinal;
	PIMAGE_EXPORT_DIRECTORY exports;
	PIMAGE_DATA_DIRECTORY directory = &nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
	if (directory->Size == 0) {
		// no export table found
		return NULL;
	}

	exports = (PIMAGE_EXPORT_DIRECTORY) (codeBase + directory->VirtualAddress);
	if (exports->NumberOfNames == 0 || exports->NumberOfFunctions == 0) {
		// DLL doesn't export anything
		return NULL;
	}

	if(IsBadReadPtr(lpProcName,1))
	{
		idx = (int)lpProcName;
		idx = idx - exports->Base;
		if(outputname)
		{
			nameRef = (DWORD *) (codeBase + exports->AddressOfNames);
			ordinal = (WORD *) (codeBase + exports->AddressOfNameOrdinals);
			for (i=0; i<exports->NumberOfNames; i++, nameRef++, ordinal++) {
				LPCSTR lpName = LPCSTR(codeBase + (*nameRef));
				if(idx == *ordinal)
				{
					if(!IsBadReadPtr(lpName,1))
					{
						strcpy_s(outputname,MAX_PATH,lpName);
					}
					break;
				}
			}
		}
		return (FARPROC) (realcodebase + (*(DWORD *) (codeBase + exports->AddressOfFunctions + (idx*4))));
	}
	// search function name in list of exported names
	nameRef = (DWORD *) (codeBase + exports->AddressOfNames);
	ordinal = (WORD *) (codeBase + exports->AddressOfNameOrdinals);
	for (i=0; i<exports->NumberOfNames; i++, nameRef++, ordinal++) {
		LPCSTR lpName = LPCSTR(codeBase + (*nameRef));
		if (_stricmp(lpProcName, lpName) == 0) {
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
	return (FARPROC) (realcodebase + (*(DWORD *) (codeBase + exports->AddressOfFunctions + (idx*4))));
}

VOID GetKernel32ApiNameToNtdllApiName(IN LPCSTR kapiname,OUT LPCSTR napiname)
{
	char headRtl[4] = {0};
	memcpy(headRtl,kapiname,3);
	if(_stricmp(headRtl,"Rtl")==0)
	{
		strcpy_s((char *)napiname,MAX_PATH,kapiname);
		return;
	}
	if(_stricmp(kapiname,"VerSetConditionMask")==0)
	{
		strcpy_s((char *)napiname,MAX_PATH,kapiname);
		return;
	}
	if(_stricmp(kapiname,"GetLastError")==0)
	{
		strcpy_s((char *)napiname,MAX_PATH,"RtlGetLastWin32Error");
		return;
	}
	if(_stricmp(kapiname,"SetLastError")==0)
	{
		strcpy_s((char *)napiname,MAX_PATH,"RtlSetLastWin32Error");
		return;
	}
	if(_stricmp(kapiname,"HeapFree")==0)
	{
		strcpy_s((char *)napiname,MAX_PATH,"RtlFreeHeap");
		return;
	}
	if(_stricmp(kapiname,"HeapAlloc")==0)
	{
		strcpy_s((char *)napiname,MAX_PATH,"RtlAllocateHeap");
		return;
	}
	if(_stricmp(kapiname,"HeapReAlloc")==0)
	{
		strcpy_s((char *)napiname,MAX_PATH,"RtlReAllocateHeap");
		return;
	}
	if(_stricmp(kapiname,"HeapSize")==0)
	{
		strcpy_s((char *)napiname,MAX_PATH,"RtlSizeHeap");
		return;
	}
	strcpy_s((char *)napiname,MAX_PATH,"Rtl");
	strcat_s((char *)napiname,MAX_PATH,kapiname);
}

VOID GetSecurityNameToSecur32Name(IN LPCSTR kapiname,OUT LPCSTR _apiname)
{
	strcpy_s((char *)_apiname,MAX_PATH,kapiname);
}

VOID GetSCHANNELNameToSecur32Name(IN LPCSTR kapiname,OUT LPCSTR _apiname)
{
	strcpy_s((char *)_apiname,MAX_PATH,kapiname);
}

VOID GetWMINameToAdvapi32Name(IN LPCSTR kapiname,OUT LPCSTR _apiname)
{
	strcpy_s((char *)_apiname,MAX_PATH,kapiname);
}

VOID GetCfgmgr32NameToSetupapiName(IN LPCSTR kapiname,OUT LPCSTR _apiname)
{
	strcpy_s((char *)_apiname,MAX_PATH,kapiname);
}

BOOL IsWindowsDir(PWCHAR FullPathName)
{
	WCHAR wSysDir[MAX_PATH];
	GetWindowsDirectory(wSysDir, MAX_PATH);
	wstring fulldllname = FullPathName;
	wstring systemdir = wSysDir;
	transform(fulldllname.begin(),fulldllname.end(),fulldllname.begin(),toupper);
	transform(systemdir.begin(),systemdir.end(),systemdir.begin(),toupper);
	if(fulldllname.find(systemdir)!=wstring::npos)
		return TRUE;
	return FALSE;
}

BOOL IsSystem32Dir(PWCHAR FullPathName)
{
	WCHAR wSysDir[MAX_PATH];
	GetSystemDirectory(wSysDir, MAX_PATH);
	wstring fulldllname = FullPathName;
	wstring systemdir = wSysDir;
	transform(fulldllname.begin(),fulldllname.end(),fulldllname.begin(),toupper);
	transform(systemdir.begin(),systemdir.end(),systemdir.begin(),toupper);
	if(fulldllname.find(systemdir)!=wstring::npos)
		return TRUE;
	return FALSE;
}

static int BuildImportTable(PMEMORYMODULE module,ModuleInfoVector * pmiv,BOOL bBuildImportTableByOtherDll,ModuleInfoVector * prenamedllv)
{
	int result=1;
	unsigned char *codeBase = module->codeBase;
	WCHAR wImageName[MAX_PATH];
	CHAR ImageName[MAX_PATH];
	CHAR FunctionName[MAX_PATH];
	
	unsigned char * realcodebase;
	PModuleInfo p_ntdll_mi = NULL;
	PModuleInfo p_ws2_32_mi = NULL;
	PModuleInfo p_secur32_mi = NULL;
	PModuleInfo p_mswsock_mi = NULL;
	PModuleInfo p_advapi32_mi = NULL;
	PModuleInfo p_adsldpc_mi = NULL;
	PModuleInfo p_setupapi_mi = NULL;

	if(pmiv && bBuildImportTableByOtherDll)
	{
		ModuleInfoVector::iterator pos;
		for(pos=pmiv->begin();pos!=pmiv->end();pos++)
		{
			ModuleInfo mi = (*pos);
			GetImageNameByPath(wImageName,mi.FullDllName);
			WideCharToMultiByte(CP_ACP, 0, wImageName, -1, ImageName, MAX_PATH, NULL, NULL);
			if(!_stricmp(ImageName,"ntdll.dll"))
			{
				p_ntdll_mi = &(*pos);
			}
			if(!_stricmp(ImageName,"ws2_32.dll"))
			{
				p_ws2_32_mi = &(*pos);
			}
			if(!_stricmp(ImageName,"secur32.dll"))
			{
				p_secur32_mi = &(*pos);
			}
			if(!_stricmp(ImageName,"mswsock.dll"))
			{
				p_mswsock_mi =  &(*pos);
			}
			if(!_stricmp(ImageName,"advapi32.dll"))
			{
				p_advapi32_mi =  &(*pos);
			}
			if(!_stricmp(ImageName,"adsldpc.dll"))
			{
				p_adsldpc_mi =  &(*pos);
			}
			if(!_stricmp(ImageName,"setupapi.dll"))
			{
				p_setupapi_mi =  &(*pos);
			}
		}
	}
	PIMAGE_DATA_DIRECTORY directory = GET_HEADER_DICTIONARY(module, IMAGE_DIRECTORY_ENTRY_IMPORT);
	if (directory->Size > 0) {
		PIMAGE_IMPORT_DESCRIPTOR importDesc = (PIMAGE_IMPORT_DESCRIPTOR) (codeBase + directory->VirtualAddress);
		for (; !IsBadReadPtr(importDesc, sizeof(IMAGE_IMPORT_DESCRIPTOR)) && importDesc->Name; importDesc++) {
			POINTER_TYPE *thunkRef;
			FARPROC *funcRef;
			HMODULE handle = NULL;
			LPCSTR lpcstr = LPCSTR(codeBase + importDesc->Name);
			if(pmiv)
			{
				ModuleInfoVector::iterator pos;
				for(pos=pmiv->begin();pos!=pmiv->end();pos++)
				{
					ModuleInfo mi = (*pos);
					GetImageNameByPath(wImageName,mi.FullDllName);
					WideCharToMultiByte(CP_ACP, 0, wImageName, -1, ImageName, MAX_PATH, NULL, NULL);
					if(!_stricmp(ImageName,lpcstr))
					{
						if(mi.SameNameNext)
						{
							while(true)
							{
								for(size_t i=0;i<prenamedllv->size();i++)
								{
									ModuleInfo rdmi = (*prenamedllv)[i];
									if(mi.BaseAddress == rdmi.BaseAddress)
									{
										mi = rdmi;
										goto breakflag_1;
									}
								}
								if(mi.SameNameNext)
								{								
									mi = *mi.SameNameNext;
								}
								else
								{
breakflag_1:
									break;
								}
							}
						}
						handle = (HMODULE)mi.PEImage;
						realcodebase = (unsigned char * )mi.BaseAddress;
						break;
					}
				}
				if(handle==NULL)
				{
					PrintDbgString(L"Can't find library %S \r\n",lpcstr);
					PrintLog(L"Can't find library %S \r\n",lpcstr);
					result = 0;
					break;
				}
			}
			else
			{
				handle = LoadLibraryA((LPCSTR) (codeBase + importDesc->Name));
				realcodebase = (unsigned char * )handle;
			}
			if (handle == NULL) {
				PrintLog(L"Can't load library");
				result = 0;
				break;
			}
			if(!pmiv)
			{
				module->modules = (HMODULE *)realloc(module->modules, (module->numModules+1)*(sizeof(HMODULE)));
				if (module->modules == NULL) {
					result = 0;
					break;
				}

				module->modules[module->numModules++] = handle;
			}
			if (importDesc->OriginalFirstThunk) {
				thunkRef = (POINTER_TYPE *) (codeBase + importDesc->OriginalFirstThunk);
				funcRef = (FARPROC *) (codeBase + importDesc->FirstThunk);
			} else {
				// no hint table
				thunkRef = (POINTER_TYPE *) (codeBase + importDesc->FirstThunk);
				funcRef = (FARPROC *) (codeBase + importDesc->FirstThunk);
			}
			for (; *thunkRef; thunkRef++, funcRef++) {
				if (IMAGE_SNAP_BY_ORDINAL(*thunkRef)) 
				{
					if(_stricmp(lpcstr,"ACTIVEDS.dll")==0)
					{
						char exportapiname[MAX_PATH];
						FARPROC function1 = (FARPROC)GetRemoteProcAddressA(handle,realcodebase,(LPCSTR)IMAGE_ORDINAL(*thunkRef),exportapiname);
						FARPROC function2 = NULL;
						if(p_adsldpc_mi)
						{
							function2 = (FARPROC)GetRemoteProcAddressA((HMODULE)p_adsldpc_mi->PEImage,(unsigned char *)p_adsldpc_mi->BaseAddress,(LPCSTR)exportapiname);
						}
						*funcRef = function2 ? function2 : function1 ? function1 : *funcRef;
					}
					else if(_stricmp(lpcstr,"wsock32.dll")==0)
					{
						char exportapiname[MAX_PATH];
						FARPROC function1 = (FARPROC)GetRemoteProcAddressA(handle,realcodebase,(LPCSTR)IMAGE_ORDINAL(*thunkRef),exportapiname);
						FARPROC function2 = NULL;
						if(p_ws2_32_mi)
						{
							function2 = (FARPROC)GetRemoteProcAddressA((HMODULE)p_ws2_32_mi->PEImage,(unsigned char *)p_ws2_32_mi->BaseAddress,(LPCSTR)exportapiname);
						}
						if(!function2 && p_mswsock_mi)
						{
							function2 = (FARPROC)GetRemoteProcAddressA((HMODULE)p_mswsock_mi->PEImage,(unsigned char *)p_mswsock_mi->BaseAddress,(LPCSTR)exportapiname);
						}
						*funcRef = function2 ? function2 : function1 ? function1 : *funcRef;
					}
					else
					{
						FARPROC function = (FARPROC)GetRemoteProcAddressA(handle,realcodebase,(LPCSTR)IMAGE_ORDINAL(*thunkRef));
						*funcRef = function ? function : *funcRef;
					}
				} else {
					PIMAGE_IMPORT_BY_NAME thunkData = (PIMAGE_IMPORT_BY_NAME) (codeBase + (*thunkRef));
					if (pmiv)
					{
						if(p_ntdll_mi && _stricmp(lpcstr,"KERNEL32.dll")==0)
						{
							GetKernel32ApiNameToNtdllApiName((LPCSTR)&thunkData->Name,FunctionName);
							FARPROC function = (FARPROC)GetRemoteProcAddressA((HMODULE)p_ntdll_mi->PEImage,(unsigned char *)p_ntdll_mi->BaseAddress,(LPCSTR)FunctionName);
							*funcRef = function ? function : (FARPROC)GetRemoteProcAddressA(handle,realcodebase,(LPCSTR)&thunkData->Name);
						}
						else if(p_secur32_mi && _stricmp(lpcstr,"Security.dll")==0)
						{
							GetSecurityNameToSecur32Name((LPCSTR)&thunkData->Name,FunctionName);
							FARPROC function = (FARPROC)GetRemoteProcAddressA((HMODULE)p_secur32_mi->PEImage,(unsigned char *)p_secur32_mi->BaseAddress,(LPCSTR)FunctionName);
							*funcRef = function ? function : (FARPROC)GetRemoteProcAddressA(handle,realcodebase,(LPCSTR)&thunkData->Name);
						}
						else if(p_secur32_mi && _stricmp(lpcstr,"SCHANNEL.dll")==0)
						{
							GetWMINameToAdvapi32Name((LPCSTR)&thunkData->Name,FunctionName);
							FARPROC function = (FARPROC)GetRemoteProcAddressA((HMODULE)p_secur32_mi->PEImage,(unsigned char *)p_secur32_mi->BaseAddress,(LPCSTR)FunctionName);
							*funcRef = function ? function : (FARPROC)GetRemoteProcAddressA(handle,realcodebase,(LPCSTR)&thunkData->Name);
						}
						else if(p_advapi32_mi && _stricmp(lpcstr,"WMI.dll")==0)
						{
							GetWMINameToAdvapi32Name((LPCSTR)&thunkData->Name,FunctionName);
							FARPROC function = (FARPROC)GetRemoteProcAddressA((HMODULE)p_advapi32_mi->PEImage,(unsigned char *)p_advapi32_mi->BaseAddress,(LPCSTR)FunctionName);
							*funcRef = function ? function : (FARPROC)GetRemoteProcAddressA(handle,realcodebase,(LPCSTR)&thunkData->Name);
						}
						else if(p_setupapi_mi && _stricmp(lpcstr,"CFGMGR32.dll")==0)
						{
							GetCfgmgr32NameToSetupapiName((LPCSTR)&thunkData->Name,FunctionName);
							FARPROC function = (FARPROC)GetRemoteProcAddressA((HMODULE)p_setupapi_mi->PEImage,(unsigned char *)p_setupapi_mi->BaseAddress,(LPCSTR)FunctionName);
							*funcRef = function ? function : (FARPROC)GetRemoteProcAddressA(handle,realcodebase,(LPCSTR)&thunkData->Name);
						}
						else
						{
							*funcRef = (FARPROC)GetRemoteProcAddressA(handle,realcodebase,(LPCSTR)&thunkData->Name);
						}
					}
					else
					{
						*funcRef = (FARPROC)GetProcAddress(handle, (LPCSTR)&thunkData->Name);
					}
				}
				if (*funcRef == 0) {
					result = 0;
					break;
				}
			}

			if (!result) {
				break;
			}
		}
	}

	return result;
}

PMEMORYMODULE MemoryLoadLibrary(const void * data,ULONG_PTR address,ModuleInfoVector * pmiv,BOOL bBuildImportTableByNTDLL,ModuleInfoVector * prenamedllv)
{
	PMEMORYMODULE result;
	PIMAGE_DOS_HEADER dos_header;
	PIMAGE_NT_HEADERS old_header;
	unsigned char *code, *headers;
	SIZE_T locationDelta;
	DllEntryProc DllEntry;
	BOOL successfull;

	dos_header = (PIMAGE_DOS_HEADER)data;
	if (dos_header->e_magic != IMAGE_DOS_SIGNATURE) {
		PrintLog(L"Not a valid executable file.\n");
		return NULL;
	}

	old_header = (PIMAGE_NT_HEADERS)&((const unsigned char *)(data))[dos_header->e_lfanew];
	if (old_header->Signature != IMAGE_NT_SIGNATURE) {
		PrintLog(L"No PE header found.\n");
		return NULL;
	}

	// reserve memory for image of library
	code = (unsigned char *)VirtualAlloc((LPVOID)(old_header->OptionalHeader.ImageBase),
		old_header->OptionalHeader.SizeOfImage,
		MEM_RESERVE,
		PAGE_READWRITE);

    if (code == NULL) {
        // try to allocate memory at arbitrary position
        code = (unsigned char *)VirtualAlloc(NULL,
            old_header->OptionalHeader.SizeOfImage,
            MEM_RESERVE,
            PAGE_READWRITE);
		if (code == NULL) {
			PrintLog(L"Can't reserve memory");
			return NULL;
		}
	}
    
	result = (PMEMORYMODULE)HeapAlloc(GetProcessHeap(), 0, sizeof(MEMORYMODULE));
	result->codeBase = code;
	result->numModules = 0;
	result->modules = NULL;
	result->initialized = 0;

	// XXX: is it correct to commit the complete memory region at once?
    //      calling DllEntry raises an exception if we don't...
	VirtualAlloc(code,
		old_header->OptionalHeader.SizeOfImage,
		MEM_COMMIT,
		PAGE_READWRITE);

	// commit memory for headers
	headers = (unsigned char *)VirtualAlloc(code,
		old_header->OptionalHeader.SizeOfHeaders,
		MEM_COMMIT,
		PAGE_READWRITE);
	
	// copy PE header to code
	memcpy(headers, dos_header, dos_header->e_lfanew + old_header->OptionalHeader.SizeOfHeaders);
	result->headers = (PIMAGE_NT_HEADERS)&((const unsigned char *)(headers))[dos_header->e_lfanew];

	// update position
	result->headers->OptionalHeader.ImageBase = (POINTER_TYPE)code;

	// copy sections from DLL file block to new memory location
	CopySections((const unsigned char *)data, old_header, result, address);

	// adjust base address of imported data
	locationDelta = (SIZE_T)(code - old_header->OptionalHeader.ImageBase);
	if(address)
	{
		if(old_header->OptionalHeader.ImageBase == address)
		{
			locationDelta = 0;
		}
		else
		{
			locationDelta = address;
		}
	}
	if (locationDelta != 0) {
		PerformBaseRelocation(result, locationDelta,old_header->OptionalHeader.ImageBase);
	}

	// load required dlls and adjust function table of imports
	if (!BuildImportTable(result,pmiv,bBuildImportTableByNTDLL,prenamedllv)) 
	{
			goto error;
	}


	// mark memory pages depending on section headers and release
	// sections that are marked as "discardable"
	if(!address)
	{
		FinalizeSections(result);
	}

	// get entry point of loaded library
	if (!address && result->headers->OptionalHeader.AddressOfEntryPoint != 0) {
		DllEntry = (DllEntryProc) (code + result->headers->OptionalHeader.AddressOfEntryPoint);
		if (DllEntry == 0) {
			PrintLog(L"Library has no entry point.\n");
			goto error;
		}

		// notify library about attaching to process
		successfull = (*DllEntry)((HINSTANCE)code, DLL_PROCESS_ATTACH, 0);
		if (!successfull) {
			PrintLog(L"Can't attach library.\n");
			goto error;
		}
		result->initialized = 1;
	}

	return result;

error:
	// cleanup
	MemoryFreeLibrary(result);
	return NULL;
}

FARPROC MemoryGetProcAddress(PMEMORYMODULE module, const char *name)
{
	unsigned char *codeBase = ((PMEMORYMODULE)module)->codeBase;
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
		if (_stricmp(name, (const char *) (codeBase + (*nameRef))) == 0) {
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

VOID MemoryFreeLibrary(PMEMORYMODULE mod)
{
	int i;
	PMEMORYMODULE module = (PMEMORYMODULE)mod;

	if (module != NULL) {
		if (module->initialized != 0) {
			// notify library about detaching from process
			DllEntryProc DllEntry = (DllEntryProc) (module->codeBase + module->headers->OptionalHeader.AddressOfEntryPoint);
			(*DllEntry)((HINSTANCE)module->codeBase, DLL_PROCESS_DETACH, 0);
			module->initialized = 0;
		}

		if (module->modules != NULL) {
			// free previously opened libraries
			for (i=0; i<module->numModules; i++) {
				if (module->modules[i] != INVALID_HANDLE_VALUE) {
					FreeLibrary(module->modules[i]);
				}
			}

			free(module->modules);
		}

		if (module->codeBase != NULL) {
			// release memory of library
			VirtualFree(module->codeBase, 0, MEM_RELEASE);
		}

		HeapFree(GetProcessHeap(), 0, module);
	}
}

PIMAGE_IMPORT_DESCRIPTOR GetImortApiTable(const unsigned char * data)
{
	PIMAGE_DOS_HEADER dos_header;
	PIMAGE_NT_HEADERS nt_header;

	dos_header = (PIMAGE_DOS_HEADER)data;
	if (dos_header->e_magic != IMAGE_DOS_SIGNATURE) {
		PrintLog(L"Not a valid executable file. \r\n");
		return NULL;
	}

	nt_header = (PIMAGE_NT_HEADERS)&((const unsigned char *)(data))[dos_header->e_lfanew];
	if (nt_header->Signature != IMAGE_NT_SIGNATURE) {
		PrintLog(L"No PE header found.\r\n");
		return NULL;
	}

	PIMAGE_DATA_DIRECTORY directory = &(nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT]);
	PIMAGE_IMPORT_DESCRIPTOR importDesc = (PIMAGE_IMPORT_DESCRIPTOR) (data + directory->VirtualAddress);
	return importDesc;
}

BOOL HasDllInImortApiTable(const unsigned char * data,LPCSTR DllName,ULONG_PTR & firstfunadr)
{
	PIMAGE_IMPORT_DESCRIPTOR importDesc = GetImortApiTable(data);
	if(!importDesc)
		return FALSE;
	const unsigned char *codeBase = data;
	POINTER_TYPE *thunkRef;
	FARPROC *funcRef;
	for (; !IsBadReadPtr(importDesc, sizeof(IMAGE_IMPORT_DESCRIPTOR)) && importDesc->Name; importDesc++) {
		LPCSTR lpcstr = LPCSTR(codeBase + importDesc->Name);
		if(IsBadReadPtr(lpcstr,1))
		{
			return FALSE;
		}
		if(!_stricmp(lpcstr,DllName))
		{
			if (importDesc->OriginalFirstThunk) {
				thunkRef = (POINTER_TYPE *) (codeBase + importDesc->OriginalFirstThunk);
				funcRef = (FARPROC *) (codeBase + importDesc->FirstThunk);
			} else {
				// no hint table
				thunkRef = (POINTER_TYPE *) (codeBase + importDesc->FirstThunk);
				funcRef = (FARPROC *) (codeBase + importDesc->FirstThunk);
			}
			for (; *thunkRef; thunkRef++, funcRef++) {
				firstfunadr = (ULONG_PTR)*funcRef;
				break;
			}
			return TRUE;
		}
	}
	return FALSE;
}

PBYTE GetSectionByName(const unsigned char *data, OUT SIZE_T * size,const char * name)
{
	PIMAGE_DOS_HEADER dos_header;
	PIMAGE_NT_HEADERS nt_header;

	dos_header = (PIMAGE_DOS_HEADER)data;
	if (dos_header->e_magic != IMAGE_DOS_SIGNATURE) {
		PrintLog(L"Not a valid executable file. \r\n");
		return NULL;
	}

	nt_header = (PIMAGE_NT_HEADERS)&((const unsigned char *)(data))[dos_header->e_lfanew];
	if (nt_header->Signature != IMAGE_NT_SIGNATURE) {
		PrintLog(L"No PE header found.\r\n");
		return NULL;
	}

	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(nt_header);
	WORD i;
	for (i=0; i<nt_header->FileHeader.NumberOfSections; i++, section++) {
		if (section->SizeOfRawData == 0) {
			// section doesn't contain data in the dll itself, but may define
			// uninitialized data
			// section is empty
			continue;
		}
		if(_stricmp((const char *)section->Name,name)==0)
		{
			*size = section->Misc.VirtualSize;
			return PBYTE(data + section->VirtualAddress);
		}
	}
	return NULL;
}

ULONG_PTR GetPeImageBase(const unsigned char *data)
{
	PIMAGE_DOS_HEADER dos_header;
	PIMAGE_NT_HEADERS nt_header;

	dos_header = (PIMAGE_DOS_HEADER)data;
	if (dos_header->e_magic != IMAGE_DOS_SIGNATURE) {
		PrintLog(L"Not a valid executable file. \r\n");
		return NULL;
	}

	nt_header = (PIMAGE_NT_HEADERS)&((const unsigned char *)(data))[dos_header->e_lfanew];
	if (nt_header->Signature != IMAGE_NT_SIGNATURE) {
		PrintLog(L"No PE header found.\r\n");
		return NULL;
	}

	return nt_header->OptionalHeader.ImageBase;
}

PVOID GetLocalFileBuff(PWCHAR szPath,OUT DWORD & size)
{
	CHAR *pBuffer;
	DWORD RSize;
	int fileSize = 0;
	wstring filepath = szPath;
	int index = filepath.find(L"\\\?\?\\");
	if(index!=wstring::npos && index==0)
	{
		filepath = filepath.substr(2*sizeof(WCHAR));
	}
	HANDLE hOpenFile = (HANDLE)CreateFile(filepath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
	if (hOpenFile == INVALID_HANDLE_VALUE)
	{
		hOpenFile = NULL;
		PrintDbgString(L"无法打开文件[%s] \r\n",filepath.c_str());
		PrintLog(L"无法打开文件[%s] \r\n",filepath.c_str());
		return NULL;
	}
	fileSize = GetFileSize(hOpenFile, NULL);
	pBuffer = (CHAR *) malloc(fileSize);
	if(!pBuffer)
		return NULL;
	if(!ReadFile(hOpenFile, pBuffer, fileSize, &RSize, NULL))
	{
		free(pBuffer);
		PrintDbgString(L"无法读取文件[%s] \r\n",filepath.c_str());
		PrintLog(L"无法读取文件[%s] \r\n",filepath.c_str());
		CloseHandle(hOpenFile);
		return NULL;
	}
	CloseHandle(hOpenFile);
	size = fileSize;
	return pBuffer;
}

VOID FreeLoacalFileBuff(PVOID localfilebuff)
{
	free(localfilebuff);
}

BOOL WriteLoacalFile(PWCHAR szPath,PVOID buff,IN OUT DWORD & size)
{
	wstring filepath = szPath;
	HANDLE hOpenFile = (HANDLE)CreateFile(filepath.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, NULL, NULL);
	if (hOpenFile == INVALID_HANDLE_VALUE)
	{
		hOpenFile = NULL;
		PrintDbgString(L"无法创建文件[%s] \r\n",filepath.c_str());
		PrintLog(L"无法创建文件[%s] \r\n",filepath.c_str());
		return FALSE;
	}
	if(!WriteFile(hOpenFile, buff, size, &size, NULL))
	{
		PrintDbgString(L"无法写入文件[%s] \r\n",filepath.c_str());
		PrintLog(L"无法写入文件[%s] \r\n",filepath.c_str());
		CloseHandle(hOpenFile);
		return FALSE;
	}
	CloseHandle(hOpenFile);
	return TRUE;
}

HANDLE CRMapViewOfFile(PWCHAR szPath,PVOID & StartAddress)
{
	LONG status;
	HANDLE hFile ,hSection;
	SIZE_T size = 0;
	PVOID BaseAddress = NULL;
	IO_STATUS_BLOCK iostatus;
	OBJECT_ATTRIBUTES objattr;

	RTLINITUNICODESTRING RtlInitUnicodeString = (RTLINITUNICODESTRING)GetProcAddress(GetModuleHandle(L"ntdll"),"RtlInitUnicodeString");
	UNICODE_STRING wModulePath;
	RtlInitUnicodeString(&wModulePath,szPath);
	
	InitializeObjectAttributes(&objattr, &wModulePath, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

	ZWOPENFILE ZwOpenFile = (ZWOPENFILE)GetProcAddress(GetModuleHandle(L"ntdll"),"ZwOpenFile");
	status = ZwOpenFile(&hFile, FILE_EXECUTE | SYNCHRONIZE, &objattr, &iostatus, FILE_SHARE_READ, FILE_NON_DIRECTORY_FILE|FILE_RANDOM_ACCESS|FILE_SYNCHRONOUS_IO_NONALERT);

	if(status<0)
	{
		PrintDbgString(L"打开内核文件失败[%s] \r\n",szPath);
		PrintLog(L"Open Kernel File %S Failed \r\n",szPath);
		return NULL;
	}

	objattr.ObjectName = 0;

	ZWCREATESECTION ZwCreateSection = (ZWCREATESECTION)GetProcAddress(GetModuleHandle(L"ntdll"),"ZwCreateSection");

	status = ZwCreateSection(&hSection, SECTION_ALL_ACCESS, &objattr, 0,PAGE_EXECUTE, SEC_IMAGE, hFile);

	if(status<0)
	{
		PrintDbgString(L"ZwCreateSection[%s] 失败\r\n",szPath);
		PrintLog(L"ZwCreateSection %S Failed \r\n",szPath);
		CloseHandle(hFile);
		return NULL;
	}

	ZWMAPVIEWOFSECTION ZwMapViewOfSection = (ZWMAPVIEWOFSECTION)GetProcAddress(GetModuleHandle(L"ntdll"),"ZwMapViewOfSection");

	status = ZwMapViewOfSection(hSection, GetCurrentProcess(), &BaseAddress, 0, 1000, 0, &size, (SECTION_INHERIT)1, MEM_TOP_DOWN, PAGE_READWRITE);

	if(status<0)
	{
		PrintDbgString(L"ZwMapViewOfSection[%s] 失败\r\n",szPath);
		PrintLog(L"ZwMapViewOfSection %S Failed \r\n",szPath);
		CloseHandle(hFile);
		CloseHandle(hSection);
		return NULL;
	}

	StartAddress = BaseAddress;

	CloseHandle(hFile);

	return hSection;
}

VOID CRUnmapViewOfFile(HANDLE hSection)
{
	CloseHandle(hSection);
}

VOID GetCurrentPath(OUT PWCHAR wCurrentPath)
{
	GetModuleFileName(NULL,wCurrentPath,MAX_PATH); //得到当前模块路径
	for (int i=wcslen(wCurrentPath);i>0;i--)
	{
		if ('\\'==wCurrentPath[i])
		{
			wCurrentPath[i+1]='\0';
			break;
		}
	}
}

