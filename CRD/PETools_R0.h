#pragma once

typedef POINTER_TYPE *HMODULE;
typedef POINTER_TYPE (FAR __stdcall *FARPROC)();

typedef struct _MEMORYMODULE{
	PIMAGE_NT_HEADERS headers;
	unsigned char *codeBase;
	SIZE_T SizeOfImage;
	HMODULE *modules;
	int numModules;
	int initialized;
} MEMORYMODULE, *PMEMORYMODULE;

#define GET_HEADER_DICTIONARY(module, idx)	&(module)->headers->OptionalHeader.DataDirectory[idx]
#ifndef IMAGE_SIZEOF_BASE_RELOCATION
#define IMAGE_SIZEOF_BASE_RELOCATION (sizeof(IMAGE_BASE_RELOCATION))
#endif

// Protection flags for memory pages (Executable, Readable, Writeable) 执行权限
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

PBYTE GetSectionByName(const unsigned char *data, OUT SIZE_T* size,const char * name);
//内存模块加载，可传入一个地址，根据这个地址重定位代码
PMEMORYMODULE MemoryLoadLibrary(const void *data,ULONG_PTR address = NULL);
void MemoryFreeLibrary(PMEMORYMODULE mod);

FARPROC GetProcAddress(HMODULE hMod,LPCSTR lpFunctionName);

PBYTE GetSectionByCharacteristics(const unsigned char *data, OUT SIZE_T* size,DWORD Characteristics,PBYTE prevsection);

PIMAGE_DATA_DIRECTORY GetDataDirectory(HMODULE hMod,DWORD index);
