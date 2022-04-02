#ifdef __cplusplus
extern "C" {
#endif
#include <ntddk.h>
#include <string.h>
#ifdef __cplusplus
}; // extern "C"
#endif

#include "CRD_ProcessModule.h"
#include "Relocate.h"
#include "Platform.h"
#include "KeSysFun.h"
#include "MyListEntry.h"
#include "MyWinNT.h"
#include "Log.h"
#include "PETools_R0.h"

SLISTENTRY g_sleModuleInfoHead = {0};

SIZE_T ListModuleOfProcess(IN PEPROCESS peprocess)
{
	FreeModuleInfo();
	if(peprocess ==NULL)return 0;

	ULONG_PTR pid = *PULONG_PTR((ULONG_PTR)peprocess + GetUniqueProcessId_Offset());

	HANDLE hModule = NULL;
	ULONG_PTR uPebPtr;
	ULONG_PTR uLdr=NULL ;
	ULONG uPebOffset=0;
	ULONG uLdrOffset=0xc;

	uPebOffset = GetPeb_Offset();
	if(uPebOffset==0)
	{
		PrintLog(L"this function do not supported current os.\r\n");
		return 0;
	}
	if(KeGetCurrentIrql() != PASSIVE_LEVEL)
		return 0;
	if(pid==4 || pid==0)
		return 0;
	KEATTACHPROCESS(GetRelocateFunction(KEATTACHPROCESS_INDEX))(peprocess);
	__try
	{    
		uPebPtr =(ULONG_PTR)((UCHAR *)peprocess +uPebOffset);
		if((uLdr=*(ULONG_PTR*)uPebPtr)==NULL )
		{      
			return 0;
		}
		uLdr += uLdrOffset;
		PEB_LDR_DATA *pld=(PEB_LDR_DATA*)(*(ULONG_PTR*)uLdr);
		LIST_ENTRY *pList=pld->InLoadOrderModuleList.Flink;  
		LIST_ENTRY *p=pList;    
		do{
			PLDR_MODULE pModule=(PLDR_MODULE)p;
			ProbeForRead((PVOID)pModule,sizeof(LDR_MODULE),1);
			if(pModule->BaseAddress)
			{
				if(AddModuleInfo(pModule))
				{
					KdPrint(("添加ModuleInfo实例成功\r\n"));
				}
				else
				{
					KdPrint(("添加ModuleInfo实例失败\r\n"));
				}
			}
			p=p->Flink;
		}while(p!=pList);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		PrintLog(L"This is an exception on CheckModuleOfProcess()...\r\n");
		KdPrint(("This is an exception on CheckModuleOfProcess()...\r\n"));
	}
	KEDETACHPROCESS(GetRelocateFunction(KEDETACHPROCESS_INDEX))();
	return SizeOfSListEntry(&g_sleModuleInfoHead);
}

BOOL AddModuleInfo(PLDR_MODULE pModule)
{
	PMODULEINFO pmi = (PMODULEINFO)ExAllocatePool(NonPagedPool,sizeof(MODULEINFO));
	__try
	{  
		if(pmi)
		{
			RtlZeroMemory(pmi,sizeof(MODULEINFO));
			pmi->BaseAddress = (ULONG_PTR)pModule->BaseAddress;
			pmi->SizeOfImage = pModule->SizeOfImage;
			pmi->EntryPoint = (ULONG_PTR)pModule->EntryPoint;
			if(pModule->FullDllName.Length && pModule->FullDllName.Length<=MAX_PATH*sizeof(WCHAR))
			{
				ProbeForRead((PVOID)pModule->FullDllName.Buffer,pModule->FullDllName.Length,1);
				RtlCopyMemory(pmi->FullDllName,pModule->FullDllName.Buffer,pModule->FullDllName.Length);
			}
			PushSLISTENTRY(&g_sleModuleInfoHead,&pmi->next);
			return TRUE;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		if(pmi)
		{
			ExFreePool(pmi);
		}
		return FALSE;
	}
	return FALSE;
}

VOID FreeModuleInfo()
{
	ReleaseSListEntry(&g_sleModuleInfoHead);
}

VOID GetModules(PVOID buff,SIZE_T size)
{
	PSLISTENTRY pSListEntry = &g_sleModuleInfoHead;
	PModuleInfo pmi_tr = (PModuleInfo)buff;
	while(!IsEmptySListEntry(pSListEntry))
	{
		PMODULEINFO pmi = (PMODULEINFO)NextSListEntry(pSListEntry);
		if(pmi)
		{
			pmi_tr->BaseAddress = pmi->BaseAddress;
			pmi_tr->EntryPoint = pmi->EntryPoint;
			pmi_tr->SizeOfImage = pmi->SizeOfImage;
			wcscpy(pmi_tr->FullDllName,pmi->FullDllName);
		}
		pmi_tr++;
	}
}

SIZE_T GetModulePeInfo(PVOID buff,SIZE_T size)
{
	PModuleInfo pmi_tr = (PModuleInfo)buff;
	ULONG_PTR pid = pmi_tr->ParentProcessId;
	ULONG_PTR SizeOfImage = pmi_tr->SizeOfImage;
	ULONG_PTR BaseAddress = pmi_tr->BaseAddress; 

	PEPROCESS peprocess;

	NTSTATUS status = PsLookupProcessByProcessId((HANDLE)pid,&peprocess);
	if(!NT_SUCCESS(status))
		return 0;

	if(KeGetCurrentIrql() != PASSIVE_LEVEL)
		return 0;

	KEATTACHPROCESS(GetRelocateFunction(KEATTACHPROCESS_INDEX))(peprocess);
	__try
	{
		for(ULONG_PTR i=0;i<SizeOfImage;i++)
		{
			BYTE b = *PBYTE(BaseAddress+i);
			*((PBYTE)buff + i) = b;
		}
		size = SizeOfImage;
	} 
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		PrintLog(L"This is an exception on GetModulePeInfo()...\r\n");
		KdPrint(("This is an exception on GetModulePeInfo()...\r\n"));
		size = 0;
	}
	KEDETACHPROCESS(GetRelocateFunction(KEDETACHPROCESS_INDEX))();

	return size;
}

BOOL HideProcessModuleByFullPath( IN PEPROCESS pEprocess, PUNICODE_STRING DllFullPath )
{
	PLIST_ENTRY pTemp;
	ULONG_PTR uPebPtr;
	ULONG_PTR uLdr=NULL ;
	ULONG uPebOffset=0;
	ULONG uLdrOffset=0xc;
	PPEB_LDR_DATA LdrData;
	PLDR_DATA_TABLE_ENTRY pDataEntry;
	PLIST_ENTRY InLoadOrder;
	PLIST_ENTRY InMemoryOrder;
	PLIST_ENTRY InInitializationOrder;
	BOOL success = TRUE;

	KEATTACHPROCESS(GetRelocateFunction(KEATTACHPROCESS_INDEX))(pEprocess);

	uPebPtr =(ULONG_PTR)pEprocess + GetPeb_Offset();

	//
	// _PEB_LDR_DATA 地址
	//
	
	if((uLdr=*(ULONG_PTR*)uPebPtr)==NULL )
	{      
		return FALSE;
	}
	uLdr += uLdrOffset;
	PEB_LDR_DATA *pld=(PEB_LDR_DATA*)(*(ULONG_PTR*)uLdr);

	if( !MmIsAddressValid((PVOID)pld))
	{
		return FALSE;
	}

	LdrData = ( PPEB_LDR_DATA )pld;

	InLoadOrder = &LdrData->InLoadOrderModuleList;

	InMemoryOrder = &LdrData->InMemoryOrderModuleList;

	InInitializationOrder = &LdrData->InInitializationOrderModuleList;

	KdPrint( ( "Ddvp-> InLoadOrder:%p, InMemoryOrder:%p, InInitializationOrder:%p !\n",
		InLoadOrder, InMemoryOrder, InInitializationOrder ) );

	//---------------------------------------------------------------------------
	// 遍历第一个链表
	//---------------------------------------------------------------------------
	pTemp = InLoadOrder->Flink;
	while( InLoadOrder != pTemp )
	{
		pDataEntry = ( PLDR_DATA_TABLE_ENTRY )pTemp;

		if( 0 == RtlCompareUnicodeString( &pDataEntry->FullDllName, DllFullPath, TRUE ) )
		{
			KdPrint( ( "InLoadOrder 模块基地址:%p, 模块路径:%wZ, 模块名称:%wZ 隐藏模块名称:%wZ !\n",
				pDataEntry->DllBase, &pDataEntry->FullDllName, &pDataEntry->BaseDllName, DllFullPath ) );

			//
			// 将链表中的模块剔除
			//
			RemoveEntryList( ( PLIST_ENTRY )pDataEntry );

			__try
			{
				//RtlZeroMemory( pDataEntry->FullDllName.Buffer, pDataEntry->FullDllName.MaximumLength );
				//RtlZeroMemory( &pDataEntry->FullDllName, sizeof( UNICODE_STRING ) );

				//RtlZeroMemory( pDataEntry->BaseDllName.Buffer, pDataEntry->BaseDllName.MaximumLength );
				//RtlZeroMemory( &pDataEntry->BaseDllName, sizeof( UNICODE_STRING ) );

				// RtlZeroMemory( pDataEntry->DllBase, sizeof( IMAGE_NT_HEADERS ) );
			}
			__except( EXCEPTION_EXECUTE_HANDLER )
			{
				success = FALSE;
				KdPrint( ( "Ddvp-> HideDebuggerModule ListEntryInLoadOrder Error:%p!\n",
					GetExceptionCode() ) );
			}

			pDataEntry->SizeOfImage = 0;
			pDataEntry->Flags       = 1;
			pDataEntry->DllBase     = 0;
			pDataEntry->EntryPoint  = 0;

			RemoveEntryList( &pDataEntry->HashLinks );
			break;
		}

		pTemp = pTemp->Flink;
	}

	//---------------------------------------------------------------------------
	// 遍历第二个链表
	//---------------------------------------------------------------------------
	pTemp = InMemoryOrder->Flink;

	pTemp = pTemp->Flink;
	while( InMemoryOrder != pTemp )
	{
		pDataEntry = ( PLDR_DATA_TABLE_ENTRY )pTemp;

		//
		// 第二个链表偏移要-8
		//
		pDataEntry = PLDR_DATA_TABLE_ENTRY((ULONG_PTR)pDataEntry - 0x08);

		if( 0 == RtlCompareUnicodeString( &pDataEntry->FullDllName, DllFullPath, TRUE ) )
		{
			KdPrint( ( "InMemoryOrder 模块基地址:%p, 模块路径:%wZ, 模块名称:%wZ 隐藏模块名称:%wZ !\n",
				pDataEntry->DllBase, &pDataEntry->FullDllName, &pDataEntry->BaseDllName, DllFullPath ) );

			//
			// 将链表中的模块剔除
			//
			RemoveEntryList( ( PLIST_ENTRY )pDataEntry );

			__try
			{
				//RtlZeroMemory( pDataEntry->FullDllName.Buffer, pDataEntry->FullDllName.MaximumLength );
				//RtlZeroMemory( &pDataEntry->FullDllName, sizeof( UNICODE_STRING ) );

				//RtlZeroMemory( pDataEntry->BaseDllName.Buffer, pDataEntry->BaseDllName.MaximumLength );
				//RtlZeroMemory( &pDataEntry->BaseDllName, sizeof( UNICODE_STRING ) );

				// RtlZeroMemory( pDataEntry->DllBase, sizeof( IMAGE_NT_HEADERS ) );
			}
			__except( EXCEPTION_EXECUTE_HANDLER )
			{
				success = FALSE;
				KdPrint( ( "Ddvp-> HideDebuggerModule ListEntryInMemoryOrder Error:%p!\n",
					GetExceptionCode() ) );
			}

			pDataEntry->SizeOfImage = 0;
			pDataEntry->Flags       = 1;
			pDataEntry->DllBase     = 0;
			pDataEntry->EntryPoint  = 0;

			RemoveEntryList( &pDataEntry->HashLinks );
			break;
		}

		pTemp = pTemp->Flink;
	}
	////---------------------------------------------------------------------------
	//// 遍历第三个链表
	////---------------------------------------------------------------------------
	pTemp = InInitializationOrder->Flink;

	pTemp = pTemp->Flink;

	while( InInitializationOrder != pTemp )
	{
		pDataEntry = (PLDR_DATA_TABLE_ENTRY)pTemp;

		// 第3个链表的偏移要-10
		pDataEntry = PLDR_DATA_TABLE_ENTRY((ULONG) pDataEntry - 0x10);

		if( 0 == RtlCompareUnicodeString( &pDataEntry->FullDllName, DllFullPath, TRUE ) )
		{
			KdPrint( ( "InInitializationOrder 模块基地址:%p, 模块路径:%wZ, 模块名称:%wZ 隐藏模块名称:%wZ !\n",
				pDataEntry->DllBase, &pDataEntry->FullDllName, &pDataEntry->BaseDllName, DllFullPath ) );

			//
			// 将链表中的模块剔除
			//
			RemoveEntryList( ( PLIST_ENTRY )pDataEntry );

			__try
			{
				RtlZeroMemory( pDataEntry->FullDllName.Buffer, pDataEntry->FullDllName.MaximumLength );
				RtlZeroMemory( &pDataEntry->FullDllName, sizeof( UNICODE_STRING ) );

				RtlZeroMemory( pDataEntry->BaseDllName.Buffer, pDataEntry->BaseDllName.MaximumLength );
				RtlZeroMemory( &pDataEntry->BaseDllName, sizeof( UNICODE_STRING ) );

				// RtlZeroMemory( pDataEntry->DllBase, sizeof( IMAGE_NT_HEADERS ) );
			}
			__except( EXCEPTION_EXECUTE_HANDLER )
			{
				KdPrint( ( "Ddvp-> HideDebuggerModule ListEntryInInitializationOrder Error:%p!\n",
					GetExceptionCode() ) );
				success = FALSE;
			}

			pDataEntry->SizeOfImage = 0;
			pDataEntry->Flags       = 1;
			pDataEntry->DllBase     = 0;
			pDataEntry->EntryPoint  = 0;

			RemoveEntryList( &pDataEntry->HashLinks );
			break;
		}

		pTemp = pTemp->Flink;
	}
	KEDETACHPROCESS(GetRelocateFunction(KEDETACHPROCESS_INDEX))();
	return success;
}

NTSTATUS HideProcessModule(PTransferMsg msg)
{
	PModuleInfo pmi = PModuleInfo(msg->buff);
	PEPROCESS eproc;
	NTSTATUS status = PsLookupProcessByProcessId((HANDLE)pmi->ParentProcessId,&eproc);
	if(NT_SUCCESS(status))
	{
		UNICODE_STRING DllFullPath;
		RtlInitUnicodeString(&DllFullPath,pmi->FullDllName);
		if(HideProcessModuleByFullPath(eproc,&DllFullPath))
		{
			return STATUS_SUCCESS;	
		}
	}
	return STATUS_UNSUCCESSFUL;
}