#ifdef __cplusplus
extern "C" {
#endif
#include <ntddk.h>
#include <string.h>
#ifdef __cplusplus
}; // extern "C"
#endif

#include "CRD_Process.h"
#include "CRD_HookFunction.h"
#include "Relocate.h"
#include "Platform.h"
#include "KeSysFun.h"
#include "MyListEntry.h"
#include "MyWinNT.h"
#include "Log.h"
#define _CRD_
#include "Transfer.h"

PEPROCESS g_CsrcssEprocess = NULL;

SLISTENTRY g_sleProcessInfoHead = {0};

ULONG_PTR GetPspCidTable_XP()
{
	PUCHAR addr;
	PUCHAR p;
	ULONG cid;
	addr = (PUCHAR)PsLookupProcessByProcessId;
	for(p=addr;p<addr+PAGE_SIZE;p++)
	{
		if((*(PUSHORT)p==0x35ff)&&(*(p+6)==0xe8))
		{
			cid=*(PULONG)(p+2);
			return cid;
			break;
		}
	}
	return 0;
}

//57              push    edi
//840af590 ff7508          push    dword ptr [ebp+8]
//840af593 8b3d345ff983    mov     edi,dword ptr [nt!PspCidTable (83f95f34)]

ULONG_PTR GetPspCidTable_WIN7()
{
	PUCHAR addr;
	PUCHAR p;
	ULONG cid;
	addr = (PUCHAR)PsLookupProcessByProcessId;
	for(p=addr;p<addr+PAGE_SIZE;p++)
	{
		if((*(PULONG)p==0x0875ff57)&&(*(p+4)==0x8b))
		{
			cid=*(PULONG)(p+6);
			return cid;
			break;
		}
	}
	return 0;
}

ULONG_PTR GetPspCidTable()
{
	SYSTEM_VERSION version = GetSystemVersion();
	switch(version)
	{
	case WINDOWSXP:
		return GetPspCidTable_XP();
	case WINDOWS7:
		return GetPspCidTable_WIN7();
	}
	return NULL;
}

ULONG GetGetProcessType_XP()
{	
	PVOID eproc = (PVOID)PsGetCurrentProcess();
	return GetObjectType_XP(eproc);
}

ULONG GetProcessType_WIN7()
{	
	ULONG eproc;
	ULONG type;
	ULONG total;
	eproc=(ULONG)PsGetCurrentProcess();
	type=(ULONG)GetObjectType_WIN7((PVOID)eproc);
	return type;
}

ULONG GetProcessType()
{
	SYSTEM_VERSION version = GetSystemVersion();
	switch(version)
	{
	case WINDOWSXP:
		return GetGetProcessType_XP();
	case WINDOWS7:
		return GetProcessType_WIN7();
	}
	return 0;
}

ULONG GetObjectType_XP(PVOID object)
{
	POBJECT_HEADER pObjHeader = (POBJECT_HEADER)OBJECT_TO_OBJECT_HEADER(object);
	if(MmIsAddressValid(pObjHeader) && MmIsAddressValid(&pObjHeader->Type))
		return *(PULONG)(&pObjHeader->Type);
	return 0;
}

ULONG GetObjectType_WIN7(PVOID object)
{
	if(MmIsAddressValid(object) && MmIsAddressValid(PVOID((ULONG)object-0xc)))
		return (ULONG)CRD_ObGetObjectType(object);
	return 0;
}

ULONG GetObjectType(PVOID object)
{
	SYSTEM_VERSION version = GetSystemVersion();
	switch(version)
	{
	case WINDOWSXP:
		return GetObjectType_XP(object);
	case WINDOWS7:
		return GetObjectType_WIN7(object);
	}
	return NULL;
}

//dt _HANDLE_TABLE_ENTRY 
//nt!_HANDLE_TABLE_ENTRY
//+0x000 Object           : Ptr32 Void
//+0x000 ObAttributes     : Uint4B
//+0x000 InfoTable        : Ptr32 _HANDLE_TABLE_ENTRY_INFO
//+0x000 Value            : Uint4B
//+0x004 GrantedAccess    : Uint4B
//+0x004 GrantedAccessIndex : Uint2B
//+0x006 CreatorBackTraceIndex : Uint2B
//+0x004 NextFreeTableEntry : Uint4B
//kd> dt _HANDLE_TABLE
//nt!_HANDLE_TABLE
//+0x000 TableCode        : Uint4B
//+0x004 QuotaProcess     : Ptr32 _EPROCESS
//+0x008 UniqueProcessId  : Ptr32 Void
//+0x00c HandleLock       : _EX_PUSH_LOCK
//+0x010 HandleTableList  : _LIST_ENTRY
//+0x018 HandleContentionEvent : _EX_PUSH_LOCK
//+0x01c DebugInfo        : Ptr32 _HANDLE_TRACE_DEBUG_INFO
//+0x020 ExtraInfoPages   : Int4B
//+0x024 Flags            : Uint4B
//+0x024 StrictFIFO       : Pos 0, 1 Bit
//+0x028 FirstFreeHandle  : Uint4B
//+0x02c LastFreeHandleEntry : Ptr32 _HANDLE_TABLE_ENTRY
//+0x030 HandleCount      : Uint4B
//+0x034 NextHandleNeedingPool : Uint4B
//+0x038 HandleCountHighWatermark : Uint4B

VOID GetPointerToObject(ULONG table,ULONG pid)//函数返回的object即指向EPROCESS的指针
{   
	PEPROCESS peprocess;
	//POBJECT_HEADER objectheader;
	ULONG NextFreeTableEntry;
	ULONG processtype,type;
	ULONG flags;
	processtype=GetProcessType();//调用函数获取进程类型
	if(MmIsAddressValid((PULONG)(table + pid*2)))
	{  
		if(MmIsAddressValid((PULONG)(table+ pid*2 + GetNextFreeTableEntry_Offset())))
		{

			NextFreeTableEntry = *(PULONG)(table + pid*2 + GetNextFreeTableEntry_Offset());
			if(NextFreeTableEntry == 0)//正常的handle_table_entry中NextFreeTableEntry为0
			{
				peprocess = PEPROCESS(*(PULONG)(table + pid*2));
				peprocess = PEPROCESS(((ULONG)peprocess | 0x80000000) & 0xfffffff8);//转换为对象指针
				//KdPrint(("[GetPointerToObject] object:0x%x\n",object));    //函数要记录的进程ERROCESS地址
				//objectheader = (POBJECT_HEADER)OBJECT_TO_OBJECT_HEADER(peprocess);//获取对象头指针
				//KdPrint(("[GetPointerToObject] objectheader:0x%x\n",objectheader));
				type = GetObjectType(peprocess);
				if(type == processtype)//表明是进程对象
				{  
					flags = *(PULONG)((ULONG)peprocess + GetFlags_Offset());//EPROCESS中Flags偏移量，指明了进程的死活
					//KdPrint(("[GetPointerToObject]) flags:0x%x\n",flags)
					if((flags & 0xc) != 0xc)//死进程的flags最后一位为C
					{
						if(AddProcessInfo(peprocess))
						{
							KdPrint(("添加ProcessInfo实例成功\r\n"));
						}
						else
						{
							KdPrint(("添加ProcessInfo实例失败\r\n"));
						}
					}      
				}
			}
		}
	}
}

//判断是否是进程对象，是则记录，不是则放弃
SIZE_T ListProcess()
{
	FreeProcessInfo();
	ULONG PspCidTable;
	ULONG TableCode = 0;
	ULONG table1,table2,table3,table4,table5;
	ULONG object,objectheader;
	ULONG NextFreeTableEntry;
	ULONG NextHandleNeedingPool;
	ULONG Processtype,type;
	ULONG flags;
	ULONG i,cid;
	PspCidTable = GetPspCidTable();
	Processtype = GetProcessType();
	if(PspCidTable==0)
	{
		return 0;
	}
	else
	{
		//TableCode的最后两位在XP中决定了句柄表的层数
		//TableCode=*(PULONG)(*(PULONG)PspCidTable);
		PULONG _HANDLE_TABLE = PULONG(*(PULONG)PspCidTable);
		//PHANDLE_TABLE pht = PHANDLE_TABLE(*(PULONG)PspCidTable);
		NextHandleNeedingPool = *PULONG((ULONG)_HANDLE_TABLE + GetNextHandleNeedingPool_Offset());
		//NextHandleNeedingPool = pht->NextHandleNeedingPool;
		TableCode = *_HANDLE_TABLE;
		if((TableCode & 0x3)==0x0)
		{
			table1 = TableCode;
			table2 = 0x0;
		}
		if((TableCode & 0x3)==0x1)
		{
			TableCode = TableCode & 0xfffffffc;
			table1 =*(PULONG)TableCode;
			table2 =*(PULONG)(TableCode+4);
			table3 =*(PULONG)(TableCode+8);
			table4 =*(PULONG)(TableCode+12);
			table5 =*(PULONG)(TableCode+16);
		}
		for(cid=0;cid<NextHandleNeedingPool;cid=cid+4)//要加4
		{
			if((table1!=0)&&(cid<=0x800))//在第一个表中
			{
				GetPointerToObject(table1,cid);
			}
			if((table2!=0)&&(0x800<cid && cid<=0x1000))//在第二个表中
			{
				cid=(ULONG)(cid-0x800);
				GetPointerToObject(table2,cid);
				cid=(ULONG)(cid+0x800);
			}
			if((table3!=0)&&(0x1000<cid && cid<=0x1800))//在第三个表中
			{
				cid=(ULONG)(cid-0x1000);
				GetPointerToObject(table3,cid);
				cid=(ULONG)(cid+0x1000);

			}
			if((table4!=0)&&(0x1800<cid && cid<=0x2000))//在第四个表中
			{
				cid=(ULONG)(cid-0x1800);
				GetPointerToObject(table4,cid);
				cid=(ULONG)(cid+0x1800);
			}
			if((table5!=0)&&(0x2000<cid && cid<=0x2800))//在第五个表中
			{
				cid=(ULONG)(cid-0x2000);
				GetPointerToObject(table5,cid);
				cid=(ULONG)(cid+0x2000);
			}
		}
	}
	PEPROCESS SystemEProcess;
	NTSTATUS status = PsLookupProcessByProcessId((HANDLE)4,&SystemEProcess);
	if(NT_SUCCESS(status))
	{
		PEPROCESS pCurrentEprocess = SystemEProcess;
		while(pCurrentEprocess != 0)
		{

			LIST_ENTRY * pListActiveProcess = (LIST_ENTRY *)((ULONG)pCurrentEprocess + GetActiveProcessLinks_Offset());
			pCurrentEprocess = (PEPROCESS)((ULONG)pListActiveProcess->Flink - GetActiveProcessLinks_Offset());
			if (pCurrentEprocess == SystemEProcess)
			{
				break;
			}
			else
			{
				ULONG FlagAddress = (ULONG)pCurrentEprocess + GetFlags_Offset();
				if(MmIsAddressValid(pCurrentEprocess) && MmIsAddressValid((PVOID)FlagAddress))
				{
					ULONG flags = *(PULONG)(FlagAddress);//EPROCESS中Flags偏移量，指明了进程的死活
					if((flags & 0xc) != 0xc)//死进程的flags最后一位为C
					{
						AddProcessInfo(pCurrentEprocess);
					}
				}
			}
		}
	}
	return SizeOfSListEntry(&g_sleProcessInfoHead);
}

BOOL AddProcessInfo(PEPROCESS peprocess)
{
	PSLISTENTRY pSListEntry = &g_sleProcessInfoHead;
	while(!IsEmptySListEntry(pSListEntry))
	{
		PPROCESSINFO ppi = (PPROCESSINFO)NextSListEntry(pSListEntry);
		if(ppi)
		{
			if(ppi->perocess == peprocess)
			{
				return FALSE;
			}
		}
	}
	PPROCESSINFO pi = (PPROCESSINFO)ExAllocatePool(NonPagedPool,sizeof(PROCESSINFO));
	if(pi)
	{
		RtlZeroMemory(pi,sizeof(PROCESSINFO));
		pi->perocess = peprocess;
		if(MmIsAddressValid(PVOID((ULONG_PTR)peprocess + GetUniqueProcessId_Offset())))
		{
			pi->UniqueProcessId = *PULONG_PTR((ULONG_PTR)peprocess + GetUniqueProcessId_Offset());
		}
		if(MmIsAddressValid(PVOID((ULONG_PTR)peprocess + GetInheritedFromUniqueProcessId_Offset())))
		{
			pi->InheritedFromUniqueProcessId = *PULONG_PTR((ULONG_PTR)peprocess + GetInheritedFromUniqueProcessId_Offset());
		}
		if(MmIsAddressValid(PVOID((ULONG_PTR)peprocess + GetImageFileName_Offset())))
		{
			RtlCopyMemory(pi->ImageFileName,PVOID((ULONG_PTR)peprocess + GetImageFileName_Offset()),16);
			if(!strncmp((const char *)pi->ImageFileName,"csrss.exe",16))
			{
				g_CsrcssEprocess = peprocess;
			}
		}
		GetProcessPath((ULONG)peprocess,pi->ImagePath);
		PushSLISTENTRY(&g_sleProcessInfoHead,&pi->next);
		return TRUE;
	}
	return FALSE;
}

VOID FreeProcessInfo()
{
	ReleaseSListEntry(&g_sleProcessInfoHead);
}

ULONG GetSectionObject_Offset()
{
	SYSTEM_VERSION version = GetSystemVersion();
	switch(version)
	{
	case WINDOWSXP:
		return 0x138;
	case WINDOWS7:
		return 0x128;
	}
	return 0;
}

//
//取进程全路径====================================================================
//原理Eprocess->sectionobject(0x138)->Segment(0x014)->ControlAera(0x000)->FilePointer(0x024)->(FileObject->FileName,FileObject->DeviceObject)
VOID GetProcessPath(ULONG eprocess,OUT PWCHAR ProcessPath)
{
	PFILE_OBJECT FilePointer;
	UNICODE_STRING path;  //路径
	UNICODE_STRING name;  //盘符
	RtlZeroMemory(ProcessPath,MAX_PATH * sizeof(WCHAR));
	if(MmIsAddressValid((PULONG)(eprocess + GetSectionObject_Offset())))//Eprocess->sectionobject(0x138)
	{
		PSECTION_OBJECT pSectionObject = PSECTION_OBJECT(*(PULONG_PTR)(eprocess + GetSectionObject_Offset()));
		if(MmIsAddressValid(&pSectionObject->Segment))
		{
			PSEGMENT_OBJECT pSegment = pSectionObject->Segment;
			if(MmIsAddressValid(&pSegment->ControlArea))
			{
				PCONTROL_AREA pControlArea = (PCONTROL_AREA)pSegment->BaseAddress;//BaseAddress才是ControlArea地址
				if(MmIsAddressValid(&pControlArea->FilePointer))
				{
					SYSTEM_VERSION version = GetSystemVersion();
					switch(version)
					{
					case WINDOWSXP:
						FilePointer = pControlArea->FilePointer;
						break;
					case WINDOWS7:
						PEX_FAST_REF pEx_fast_ref = (PEX_FAST_REF)pControlArea->FilePointer;
						FilePointer = PFILE_OBJECT(((ULONG)pEx_fast_ref | 0x80000000) & 0xfffffff8);//对象转换为对象指针
						break;
					}
					
				}
				else
					return;
			}
			else
				return;
		}
		else
			return;
	}
	else
		return;
	path.Length = 0;
	path.MaximumLength = 1024;
	path.Buffer = (PWCH)ExAllocatePool(NonPagedPool,1024);
	if(!path.Buffer)
	{
		return;
	}
	if(MmIsAddressValid((PVOID)FilePointer))
	{
		ObReferenceObjectByPointer((PVOID)FilePointer,0,NULL,KernelMode);//引用计数+1，操作对象
		if(MmIsAddressValid((PVOID)FilePointer->DeviceObject))
		{
			RtlVolumeDeviceToDosName(FilePointer->DeviceObject,&name); //获取盘符名
			RtlCopyUnicodeString(&path,&name);//盘符连接
			RtlAppendUnicodeStringToString(&path,&FilePointer->FileName);//路径连接
			ObDereferenceObject(FilePointer);         //关闭对象引用
			if(path.Length >= MAX_PATH*sizeof(WCHAR)) //保证以\0结尾
			{ 
				RtlCopyMemory(ProcessPath, path.Buffer, MAX_PATH*sizeof(WCHAR)); 
				*(ProcessPath + MAX_PATH - 1) = 0; 
			} 
			else 
			{ 
				RtlCopyMemory(ProcessPath, path.Buffer, path.Length); 
			}
		}
	}
	ExFreePool(path.Buffer);
}

VOID GetProcesses(PVOID buff,SIZE_T size)
{
	PSLISTENTRY pSListEntry = &g_sleProcessInfoHead;
	PProcessInfo ppi_tr = (PProcessInfo)buff;
	while(!IsEmptySListEntry(pSListEntry))
	{
		PPROCESSINFO ppi = (PPROCESSINFO)NextSListEntry(pSListEntry);
		if(ppi)
		{
			ppi_tr->ProcessId = ppi->UniqueProcessId;
			ppi_tr->ParentProcessId = ppi->InheritedFromUniqueProcessId;
			ppi_tr->PEProcess = (ULONG_PTR)ppi->perocess;
			wcscpy(ppi_tr->ImagePath,ppi->ImagePath);
			RtlCopyMemory(ppi_tr->ImageFileName,ppi->ImageFileName,16);
		}
		ppi_tr++;
	}
}

BOOL IsExitProcess(IN PEPROCESS peprocess)
{
	ULONG_PTR objtabel = GetObjectTable_Offset();
	if(MmIsAddressValid(peprocess) && *PULONG_PTR((ULONG_PTR)peprocess + objtabel)!=NULL)
	{
		return FALSE;
	}
	return TRUE;
}

PEPROCESS GetCsrssEprocess()
{
	return g_CsrcssEprocess;
}

NTSTATUS ZwKillProcess(PEPROCESS Process)
{
	HANDLE hprocess;
	NTSTATUS Status;
	NTSTATUS st;
	Status = STATUS_SUCCESS;
	__try
	{
		st = ObOpenObjectByPointer(Process, 0, 0, 0, 0, 0,&hprocess) ; //打开一个进程获得其句柄 OpenProcess 内核实现
		if NT_SUCCESS(st)
		{
			ZwTerminateProcess (hprocess,0);
			ZwClose(hprocess);
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		Status = STATUS_UNSUCCESSFUL;
	}
	return Status;
}

NTSTATUS CRD_TerminateProcess(PTransferMsg msg)
{
	PEPROCESS pEProcess;
	ULONG_PTR ProcessId = *((PULONG_PTR)msg->buff);
	NTSTATUS status = PsLookupProcessByProcessId((HANDLE)ProcessId,&pEProcess);
	if(NT_SUCCESS(status) && !IsExitProcess(pEProcess))
	{
		return ZwKillProcess(pEProcess);
	}
	return STATUS_UNSUCCESSFUL;
}

NTSTATUS CRD_ResumeThread(PTransferMsg msg)
{
	PETHREAD pEThread;
	ULONG_PTR ThreadId = *((PULONG_PTR)msg->buff);
	NTSTATUS status = PsLookupThreadByThreadId((HANDLE)ThreadId,&pEThread);
	if(NT_SUCCESS(status))
	{
		SYSTEM_VERSION version = GetSystemVersion();
		switch(version)
		{
		case WINDOWSXP:
			{
				KPROCESSOR_MODE PreviousMode = ExGetPreviousMode();
				status = PSRESUMETHREAD(GetRelocateFunction(PSRESUMETHREAD_INDEX))(pEThread,(PULONG)&PreviousMode);
			}
			break;
		case WINDOWS7:
			{
				KERESUMETHREAD KeResumeThread = (KERESUMETHREAD)GetRelocateFunction(PSRESUMETHREAD_INDEX);
				_asm
				{
					mov eax,pEThread
					call KeResumeThread
					mov status,eax
				}
			}
			break;
		}
	}
	return status;
}

NTSTATUS CRD_SuspendThread(PTransferMsg msg)
{
	PETHREAD pEThread;
	ULONG_PTR ThreadId = *((PULONG_PTR)msg->buff);
	NTSTATUS status = PsLookupThreadByThreadId((HANDLE)ThreadId,&pEThread);
	if(NT_SUCCESS(status))
	{
		KPROCESSOR_MODE PreviousMode = ExGetPreviousMode();
		status = PSSUSPENDTHREAD(GetRelocateFunction(PSSUSPENDTHREAD_INDEX))(pEThread,(PULONG)&PreviousMode);
	}
	return status;
}

NTSTATUS CRD_ShutDown(PTransferMsg msg)
{
	NTSTATUS status = STATUS_SUCCESS;

	RestartSystem();
	
	return status;
}

