#pragma once

#include <vector>

using namespace std;

typedef struct {
	PIMAGE_NT_HEADERS headers;
	unsigned char *codeBase;
	HMODULE *modules;
	int numModules;
	int initialized;
} MEMORYMODULE, *PMEMORYMODULE;

typedef struct _UNICODE_STRING {
	USHORT Length;
	USHORT MaximumLength;
#ifdef MIDL_PASS
	[size_is(MaximumLength / 2), length_is((Length) / 2) ] USHORT * Buffer;
#else // MIDL_PASS
	__field_bcount_part(MaximumLength, Length) PWCH   Buffer;
#endif // MIDL_PASS
} UNICODE_STRING;
typedef UNICODE_STRING *PUNICODE_STRING;

typedef struct _IO_STATUS_BLOCK {
	union {
		LONG Status;
		PVOID Pointer;
	} DUMMYUNIONNAME;

	ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

#define OBJ_INHERIT             0x00000002L
#define OBJ_PERMANENT           0x00000010L
#define OBJ_EXCLUSIVE           0x00000020L
#define OBJ_CASE_INSENSITIVE    0x00000040L
#define OBJ_OPENIF              0x00000080L
#define OBJ_OPENLINK            0x00000100L
#define OBJ_KERNEL_HANDLE       0x00000200L
#define OBJ_FORCE_ACCESS_CHECK  0x00000400L
#define OBJ_VALID_ATTRIBUTES    0x000007F2L

typedef struct _OBJECT_ATTRIBUTES {
	ULONG Length;
	HANDLE RootDirectory;
	PUNICODE_STRING ObjectName;
	ULONG Attributes;
	PVOID SecurityDescriptor;        // Points to type SECURITY_DESCRIPTOR
	PVOID SecurityQualityOfService;  // Points to type SECURITY_QUALITY_OF_SERVICE
} OBJECT_ATTRIBUTES;
typedef OBJECT_ATTRIBUTES *POBJECT_ATTRIBUTES;

#define InitializeObjectAttributes( p, n, a, r, s ) { \
	(p)->Length = sizeof( OBJECT_ATTRIBUTES );          \
	(p)->RootDirectory = r;                             \
	(p)->Attributes = a;                                \
	(p)->ObjectName = n;                                \
	(p)->SecurityDescriptor = s;                        \
	(p)->SecurityQualityOfService = NULL;               \
}

typedef enum _SECTION_INHERIT {
	ViewShare = 1,
	ViewUnmap = 2
} SECTION_INHERIT;

typedef VOID (WINAPI * RTLINITUNICODESTRING)( __out PUNICODE_STRING DestinationString,
											 __in_z_opt PCWSTR SourceString);

typedef LONG (WINAPI * ZWCREATESECTION)(__out PHANDLE SectionHandle,
										 __in ACCESS_MASK DesiredAccess,
										 __in_opt POBJECT_ATTRIBUTES ObjectAttributes,
										 __in_opt PLARGE_INTEGER MaximumSize,
										 __in ULONG SectionPageProtection,
										 __in ULONG AllocationAttributes,
										 __in_opt HANDLE FileHandle);

typedef LONG (WINAPI * ZWOPENFILE)(
									__out PHANDLE FileHandle,
									__in ACCESS_MASK DesiredAccess,
									__in POBJECT_ATTRIBUTES ObjectAttributes,
									__out PIO_STATUS_BLOCK IoStatusBlock,
									__in ULONG ShareAccess,
									__in ULONG OpenOptions
									);

typedef LONG (WINAPI * ZWMAPVIEWOFSECTION)(
				   __in HANDLE SectionHandle,
				   __in HANDLE ProcessHandle,
				   __inout PVOID *BaseAddress,
				   __in ULONG_PTR ZeroBits,
				   __in SIZE_T CommitSize,
				   __inout_opt PLARGE_INTEGER SectionOffset,
				   __inout PSIZE_T ViewSize,
				   __in SECTION_INHERIT InheritDisposition,
				   __in ULONG AllocationType,
				   __in ULONG Win32Protect
				   );

CONSOLEDLL_API PMEMORYMODULE MemoryLoadLibrary(const void * data,ULONG_PTR address,ModuleInfoVector * pmiv,BOOL bBuildImportTableByNTDLL,ModuleInfoVector * prenamedllv);

CONSOLEDLL_API FARPROC MemoryGetProcAddress(PMEMORYMODULE module, const char * name);

CONSOLEDLL_API VOID MemoryFreeLibrary(PMEMORYMODULE mod);

CONSOLEDLL_API PBYTE GetSectionByName(const unsigned char * data, OUT SIZE_T * size, const char * name);

CONSOLEDLL_API ULONG_PTR GetPeImageBase(const unsigned char *data);

CONSOLEDLL_API PVOID GetLocalFileBuff(PWCHAR szPath,OUT DWORD & size);

CONSOLEDLL_API VOID FreeLoacalFileBuff(PVOID localfilebuff);

CONSOLEDLL_API BOOL WriteLoacalFile(PWCHAR szPath,PVOID buff,IN OUT DWORD & size);

CONSOLEDLL_API HANDLE CRMapViewOfFile(PWCHAR szPath,PVOID & StartAddress);

CONSOLEDLL_API VOID CRUnmapViewOfFile(PVOID pvFile);

CONSOLEDLL_API VOID GetCurrentPath(OUT PWCHAR wCurrentPath);

CONSOLEDLL_API BOOL IsSystem32Dir(PWCHAR FullPathName);

CONSOLEDLL_API BOOL IsWindowsDir(PWCHAR FullPathName);

CONSOLEDLL_API BOOL HasDllInImortApiTable(const unsigned char * data,LPCSTR DllName,ULONG_PTR & firstfunadr);

CONSOLEDLL_API FARPROC WINAPI GetRemoteProcAddressA(HMODULE hModuleBuff,unsigned char * realcodebase ,LPCSTR lpProcName,char * outputname = NULL);

#ifdef _WIN64
#define POINTER_TYPE ULONGLONG
#else
#define POINTER_TYPE DWORD
#endif


//
// Define the access check value for any access
//
//
// The FILE_READ_ACCESS and FILE_WRITE_ACCESS constants are also defined in
// ntioapi.h as FILE_READ_DATA and FILE_WRITE_DATA. The values for these
// constants *MUST* always be in sync.
//
//
// FILE_SPECIAL_ACCESS is checked by the NT I/O system the same as FILE_ANY_ACCESS.
// The file systems, however, may add additional access checks for I/O and FS controls
// that use this value.
//


#define FILE_ANY_ACCESS                 0
#define FILE_SPECIAL_ACCESS    (FILE_ANY_ACCESS)
#define FILE_READ_ACCESS          ( 0x0001 )    // file & pipe
#define FILE_WRITE_ACCESS         ( 0x0002 )    // file & pipe



//
// Define access rights to files and directories
//

//
// The FILE_READ_DATA and FILE_WRITE_DATA constants are also defined in
// devioctl.h as FILE_READ_ACCESS and FILE_WRITE_ACCESS. The values for these
// constants *MUST* always be in sync.
// The values are redefined in devioctl.h because they must be available to
// both DOS and NT.
//

#define FILE_READ_DATA            ( 0x0001 )    // file & pipe
#define FILE_LIST_DIRECTORY       ( 0x0001 )    // directory

#define FILE_WRITE_DATA           ( 0x0002 )    // file & pipe
#define FILE_ADD_FILE             ( 0x0002 )    // directory

#define FILE_APPEND_DATA          ( 0x0004 )    // file
#define FILE_ADD_SUBDIRECTORY     ( 0x0004 )    // directory
#define FILE_CREATE_PIPE_INSTANCE ( 0x0004 )    // named pipe


#define FILE_READ_EA              ( 0x0008 )    // file & directory

#define FILE_WRITE_EA             ( 0x0010 )    // file & directory

#define FILE_EXECUTE              ( 0x0020 )    // file
#define FILE_TRAVERSE             ( 0x0020 )    // directory

#define FILE_DELETE_CHILD         ( 0x0040 )    // directory

#define FILE_READ_ATTRIBUTES      ( 0x0080 )    // all

#define FILE_WRITE_ATTRIBUTES     ( 0x0100 )    // all

#define FILE_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0x1FF)

#define FILE_GENERIC_READ         (STANDARD_RIGHTS_READ     |\
	FILE_READ_DATA           |\
	FILE_READ_ATTRIBUTES     |\
	FILE_READ_EA             |\
	SYNCHRONIZE)


#define FILE_GENERIC_WRITE        (STANDARD_RIGHTS_WRITE    |\
	FILE_WRITE_DATA          |\
	FILE_WRITE_ATTRIBUTES    |\
	FILE_WRITE_EA            |\
	FILE_APPEND_DATA         |\
	SYNCHRONIZE)


#define FILE_GENERIC_EXECUTE      (STANDARD_RIGHTS_EXECUTE  |\
	FILE_READ_ATTRIBUTES     |\
	FILE_EXECUTE             |\
	SYNCHRONIZE)




//
// Define share access rights to files and directories
//

#define FILE_SHARE_READ                 0x00000001  
#define FILE_SHARE_WRITE                0x00000002  
#define FILE_SHARE_DELETE               0x00000004  
#define FILE_SHARE_VALID_FLAGS          0x00000007

//
// Define the file attributes values
//
// Note:  0x00000008 is reserved for use for the old DOS VOLID (volume ID)
//        and is therefore not considered valid in NT.
//
// Note:  Note also that the order of these flags is set to allow both the
//        FAT and the Pinball File Systems to directly set the attributes
//        flags in attributes words without having to pick each flag out
//        individually.  The order of these flags should not be changed!
//

#define FILE_ATTRIBUTE_READONLY             0x00000001  
#define FILE_ATTRIBUTE_HIDDEN               0x00000002  
#define FILE_ATTRIBUTE_SYSTEM               0x00000004  
//OLD DOS VOLID                             0x00000008

#define FILE_ATTRIBUTE_DIRECTORY            0x00000010  
#define FILE_ATTRIBUTE_ARCHIVE              0x00000020  
#define FILE_ATTRIBUTE_DEVICE               0x00000040  
#define FILE_ATTRIBUTE_NORMAL               0x00000080  

#define FILE_ATTRIBUTE_TEMPORARY            0x00000100  
#define FILE_ATTRIBUTE_SPARSE_FILE          0x00000200  
#define FILE_ATTRIBUTE_REPARSE_POINT        0x00000400  
#define FILE_ATTRIBUTE_COMPRESSED           0x00000800  

#define FILE_ATTRIBUTE_OFFLINE              0x00001000  
#define FILE_ATTRIBUTE_NOT_CONTENT_INDEXED  0x00002000  
#define FILE_ATTRIBUTE_ENCRYPTED            0x00004000  

#define FILE_ATTRIBUTE_VIRTUAL              0x00010000  

#define FILE_ATTRIBUTE_VALID_FLAGS          0x00007fb7
#define FILE_ATTRIBUTE_VALID_SET_FLAGS      0x000031a7

//
// Define the create disposition values
//

#define FILE_SUPERSEDE                  0x00000000
#define FILE_OPEN                       0x00000001
#define FILE_CREATE                     0x00000002
#define FILE_OPEN_IF                    0x00000003
#define FILE_OVERWRITE                  0x00000004
#define FILE_OVERWRITE_IF               0x00000005
#define FILE_MAXIMUM_DISPOSITION        0x00000005

//
// Define the create/open option flags
//

#define FILE_DIRECTORY_FILE                     0x00000001
#define FILE_WRITE_THROUGH                      0x00000002
#define FILE_SEQUENTIAL_ONLY                    0x00000004
#define FILE_NO_INTERMEDIATE_BUFFERING          0x00000008

#define FILE_SYNCHRONOUS_IO_ALERT               0x00000010
#define FILE_SYNCHRONOUS_IO_NONALERT            0x00000020
#define FILE_NON_DIRECTORY_FILE                 0x00000040
#define FILE_CREATE_TREE_CONNECTION             0x00000080

#define FILE_COMPLETE_IF_OPLOCKED               0x00000100
#define FILE_NO_EA_KNOWLEDGE                    0x00000200
#define FILE_OPEN_REMOTE_INSTANCE               0x00000400
#define FILE_RANDOM_ACCESS                      0x00000800

#define FILE_DELETE_ON_CLOSE                    0x00001000
#define FILE_OPEN_BY_FILE_ID                    0x00002000
#define FILE_OPEN_FOR_BACKUP_INTENT             0x00004000
#define FILE_NO_COMPRESSION                     0x00008000

#if (NTDDI_VERSION >= NTDDI_WIN7)
#define FILE_OPEN_REQUIRING_OPLOCK              0x00010000
#define FILE_DISALLOW_EXCLUSIVE                 0x00020000
#endif /* NTDDI_VERSION >= NTDDI_WIN7 */

#define FILE_RESERVE_OPFILTER                   0x00100000
#define FILE_OPEN_REPARSE_POINT                 0x00200000
#define FILE_OPEN_NO_RECALL                     0x00400000
#define FILE_OPEN_FOR_FREE_SPACE_QUERY          0x00800000


#define FILE_VALID_OPTION_FLAGS                 0x00ffffff
#define FILE_VALID_PIPE_OPTION_FLAGS            0x00000032
#define FILE_VALID_MAILSLOT_OPTION_FLAGS        0x00000032
#define FILE_VALID_SET_FLAGS                    0x00000036

//
// Define the I/O status information return values for NtCreateFile/NtOpenFile
//

#define FILE_SUPERSEDED                 0x00000000
#define FILE_OPENED                     0x00000001
#define FILE_CREATED                    0x00000002
#define FILE_OVERWRITTEN                0x00000003
#define FILE_EXISTS                     0x00000004
#define FILE_DOES_NOT_EXIST             0x00000005

//
// Define special ByteOffset parameters for read and write operations
//

#define FILE_WRITE_TO_END_OF_FILE       0xffffffff
#define FILE_USE_FILE_POINTER_POSITION  0xfffffffe

//
// Define alignment requirement values
//

#define FILE_BYTE_ALIGNMENT             0x00000000
#define FILE_WORD_ALIGNMENT             0x00000001
#define FILE_LONG_ALIGNMENT             0x00000003
#define FILE_QUAD_ALIGNMENT             0x00000007
#define FILE_OCTA_ALIGNMENT             0x0000000f
#define FILE_32_BYTE_ALIGNMENT          0x0000001f
#define FILE_64_BYTE_ALIGNMENT          0x0000003f
#define FILE_128_BYTE_ALIGNMENT         0x0000007f
#define FILE_256_BYTE_ALIGNMENT         0x000000ff
#define FILE_512_BYTE_ALIGNMENT         0x000001ff

//
// Define the maximum length of a filename string
//

#define MAXIMUM_FILENAME_LENGTH         256

//
// Define the various device characteristics flags
//

#define FILE_REMOVABLE_MEDIA                    0x00000001
#define FILE_READ_ONLY_DEVICE                   0x00000002
#define FILE_FLOPPY_DISKETTE                    0x00000004
#define FILE_WRITE_ONCE_MEDIA                   0x00000008
#define FILE_REMOTE_DEVICE                      0x00000010
#define FILE_DEVICE_IS_MOUNTED                  0x00000020
#define FILE_VIRTUAL_VOLUME                     0x00000040
#define FILE_AUTOGENERATED_DEVICE_NAME          0x00000080
#define FILE_DEVICE_SECURE_OPEN                 0x00000100
#define FILE_CHARACTERISTIC_PNP_DEVICE          0x00000800
#define FILE_CHARACTERISTIC_TS_DEVICE           0x00001000
#define FILE_CHARACTERISTIC_WEBDAV_DEVICE       0x00002000