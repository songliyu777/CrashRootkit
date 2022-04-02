#include "StdAfx.h"
#include "CRConsole.h"
#include "Task.h"
#include "ConsoleDll.h"
#include "KernelHookDoc.h"
#include "ProcessDoc.h"
#include "KernelModuleDoc.h"
#include "SSDTDoc.h"
#include "SSDTShadowResource.h"
#include "DpcTimerDoc.h"
#include "IoTimerDoc.h"
#include "Platform.h"
#include "PETools_R3.h"
#include "TaskManager.h"

CRConsole::CRConsole(void)
{
}

CRConsole::~CRConsole(void)
{
}

DWORD WINAPI CRConsole::DoKernelHookCheck(LPVOID pTask)
{
	CTask * pkhctask = (CTask *)pTask;
	CKernelHookDoc * pkhd = (CKernelHookDoc *)pkhctask->GetParameter();
	DWORD size = 0;
	PBYTE buff = NULL;
	ModuleInfoVector::iterator pos;
	CKernelModuleDoc * pkmd = (CKernelModuleDoc *)CTaskManager::GetInstance()->GetDocument(RUNTIME_CLASS(CKernelModuleDoc));
	pkhd->FreeMemoryHookVector();
	if(KernelHookCheck(size))
	{
		if(size)
		{
			buff = (PBYTE)malloc(size);
			if(buff)
			{
				if(KernelHookGet(buff,size))
				{
					DWORD offset = 0;
					while(offset<size)
					{
						MemoryHook mh;
						DWORD mhsize = *PDWORD(buff + offset);
						offset += sizeof(DWORD);
						wcscpy_s(mh.ModuleName,PWCHAR(buff + offset));
						offset += MAX_PATH * sizeof(WCHAR);
						ULONG_PTR Address = *PULONG_PTR(buff + offset);
						offset += sizeof(ULONG_PTR);
						ULONG_PTR JmpAddress = *PULONG_PTR(buff + offset);
						offset += sizeof(ULONG_PTR);
						HookType type = *(HookType*)(buff + offset);
						offset += sizeof(HookType);
						DWORD length = *PDWORD(buff + offset);
						offset += sizeof(DWORD);
						PBYTE Origin = (PBYTE)malloc(length);
						memcpy(Origin,buff + offset,length);
						offset += length;
						PBYTE Current = (PBYTE)malloc(length);
						memcpy(Current,buff + offset,length);
						offset += length;
						mh.Address = Address;
						mh.JmpAddress = JmpAddress;
						mh.Type = type;
						mh.Length = length;
						mh.Origin = Origin;
						mh.Current = Current;
						if(pkmd)
						{
							ZeroMemory(mh.JmpModuleName,MAX_PATH*sizeof(WCHAR));
							if(JmpAddress)
							{
								for(pos = pkmd->m_miv.begin();pos != pkmd->m_miv.end();pos++)
								{
									ModuleInfo mi = *pos;
									if(JmpAddress > mi.BaseAddress && JmpAddress < mi.BaseAddress + mi.SizeOfImage)
									{
										wcscpy_s(mh.JmpModuleName,mi.FullDllName);
										break;
									}
								}
							}
						}
						pkhd->m_mhv.push_back(mh);
					}
				}
				free(buff);
			}
		}
	}
	pkhctask->SetValue(100);
	return 0;
}

DWORD WINAPI CRConsole::DoProcessesList(LPVOID pTask)
{
	CTask * ppltask = (CTask *)pTask;
	CProcessDoc * ppd = (CProcessDoc *)ppltask->GetParameter();
	DWORD size;
	ppd->m_piv.clear();
	if(ProcessesList(size) && size>0)
	{
		PVOID buff = malloc(size*sizeof(ProcessInfo));
		if(buff)
		{
			if(ProcessesGet(buff,size*sizeof(ProcessInfo)))
			{
				for(DWORD i=0;i<size;i++)
				{
					PProcessInfo ppi = PProcessInfo(buff) + i;
					ppd->m_piv.push_back(*ppi);
				}
			}
			free(buff);
		}
	}
	PVOID p = GetInfoTable(SystemProcessesAndThreadsInformation);
	if(p)
	{
		PSYSTEM_PROCESSES pSystemProc = PSYSTEM_PROCESSES(p);
		do
		{
			if(pSystemProc->ProcessName.Buffer && pSystemProc->ProcessName.Length > 0)
			{
				ProcessInfoVector* piv = &ppd->m_piv;
				SIZE_T size = piv->size();
				bool hasfind = false;
				for(SIZE_T i=0;i<size;i++)
				{
					PProcessInfo ppi = &(*piv)[i];
					if(ppi->ProcessId == pSystemProc->ProcessId)
					{
						hasfind = true;
						break;
					}
				}
				if(!hasfind)
				{
					ProcessInfo pi;
					pi.ParentProcessId = pSystemProc->InheritedFromProcessId;
					pi.ProcessId = pSystemProc->ProcessId;
					wcscpy_s(pi.ImagePath,pSystemProc->ProcessName.Buffer);
					piv->push_back(pi);
				}
			}
	
			pSystemProc = (PSYSTEM_PROCESSES)( (unsigned long)pSystemProc + pSystemProc->NextEntryDelta ); 
		}while( pSystemProc->NextEntryDelta != 0);
		FreeInfoTable(p);
	}
	ppltask->SetValue(100);
	return 0;
}

typedef LONG (WINAPI * ZWOPENPROCESS)(
									  __out PHANDLE  ProcessHandle,
									  __in ACCESS_MASK  DesiredAccess,
									  __in POBJECT_ATTRIBUTES  ObjectAttributes,
									  __in_opt PCLIENT_ID  ClientId
								   );

DWORD WINAPI CRConsole::DoModulesList(LPVOID pTask)
{
	CTask * pmltask = (CTask *)pTask;
	CProcessDoc * ppd = (CProcessDoc *)pmltask->GetParameter();
	if(ppd->m_pCurrentProcessInfo)
	{
		ULONG_PTR pid = ppd->m_pCurrentProcessInfo->ProcessId;
		DWORD size;
		ppd->ClearModuleInfoVector();
		if(pid!=4)
		{
			if(ModulesList(pid,size) && size>0)
			{
				PVOID buff = malloc(size*sizeof(ModuleInfo));
				if(buff && ModulesGet(buff,size*sizeof(ModuleInfo)))
				{
					PModuleInfo pmi = (PModuleInfo)buff;
					for(DWORD i=0;i<size;i++)
					{
						pmi->ParentProcessId = pid;
						pmi->PEImage = NULL;
						pmi->SameNameNext = NULL;
						ppd->m_miv.push_back(*pmi);
						pmi++;
					}
				}
				if(buff)
					free(buff);
			}
		}
		else
		{
			DWORD size = 0;
			if(KernelModulesList(size) && size)
			{
				PVOID buff = malloc(size*sizeof(ModuleInfo));
				if(buff && KernelModulesGet(buff,size*sizeof(ModuleInfo)))
				{
					PModuleInfo pmi = (PModuleInfo)buff;
					for(DWORD i=0;i<size;i++)
					{
						pmi->ParentProcessId = pid;
						pmi->PEImage = NULL;
						pmi->SameNameNext = NULL;
						ppd->m_miv.push_back(*pmi);
						pmi++;
					}
				}
				if(buff)
					free(buff);
			}
		}
	}
	pmltask->SetValue(100);
	return 0;
}

DWORD WINAPI CRConsole::DoThreadsList(LPVOID pTask)
{
	CTask * ptltask = (CTask *)pTask;
	CProcessDoc * ppd = (CProcessDoc *)ptltask->GetParameter();
	if(ppd->m_pCurrentProcessInfo)
	{
		ULONG_PTR pid = ppd->m_pCurrentProcessInfo->ProcessId;
		DWORD size;
		ModuleInfoVector::iterator pos;
		ppd->m_tiv.clear();
		if(ThreadsList(pid,size) && size>0)
		{
			PVOID buff = malloc(size*sizeof(ThreadInfo));
			if(buff && ThreadsGet(buff,size*sizeof(ThreadInfo)))
			{
				PThreadInfo pti = (PThreadInfo)buff;
				for(DWORD i=0;i<size;i++)
				{
					ZeroMemory(pti->FullDllName,sizeof(WCHAR)*MAX_PATH);
					if(pti->Win32StartAddress)
					{
						for(pos=ppd->m_miv.begin();pos!=ppd->m_miv.end();pos++)
						{
							ModuleInfo mi = *pos;
							if(pti->Win32StartAddress > mi.BaseAddress && pti->Win32StartAddress < mi.BaseAddress + mi.SizeOfImage)
							{
								wcscpy_s(pti->FullDllName,mi.FullDllName);
								break;
							}
						}
					}
					else if(pti->StartAddress)
					{
						for(pos=ppd->m_miv.begin();pos!=ppd->m_miv.end();pos++)
						{
							ModuleInfo mi = *pos;
							if(pti->StartAddress > mi.BaseAddress && pti->StartAddress < mi.BaseAddress + mi.SizeOfImage)
							{
								wcscpy_s(pti->FullDllName,mi.FullDllName);
								break;
							}
						}
					}
					ppd->m_tiv.push_back(*pti);
					pti++;
				}
			}
			if(buff)
				free(buff);
		}
	}
	ptltask->SetValue(100);
	return 0;
}

DWORD WINAPI CRConsole::DoHookCheckList(LPVOID pTask)
{
	CTask * phcltask = (CTask *)pTask;
	CProcessDoc * ppd = (CProcessDoc *)phcltask->GetParameter();
	if(ppd->m_pCurrentProcessInfo)
	{
		ULONG_PTR pid = ppd->m_pCurrentProcessInfo->ProcessId;
		if(pid==4)
		{
			phcltask->SetValue(100);
			return 0;
		}
		DWORD size,count = ppd->m_miv.size();
		ppd->ClearMemoryHookVector();
		ModuleInfoVector::iterator pos;
		ModuleInfoVector rdv;//重名模块临时存储
		float taskvalue = 0;
		float perval = (float)20/(float)count;
		for(DWORD i=0;i<count;i++)
		{
			WCHAR wiImageName[MAX_PATH];
			WCHAR wjImageName[MAX_PATH];
			PModuleInfo pmi = &(ppd->m_miv[i]);
			pmi->ParentProcessId = pid;
			pmi->SameNameNext = NULL;
			GetImageNameByPath(wiImageName,pmi->FullDllName);
			PVOID outputbuff = malloc(pmi->SizeOfImage);
			if(outputbuff)
			{
				size = sizeof(ModuleInfo);
				if(pmi->PEImage)
					free(pmi->PEImage);
				if(ModulePeInfoGet(*pmi,outputbuff,size))
				{
					pmi->PEImage = outputbuff;
				}
				//else if(ReadProcessMemoryEx(pmi->ParentProcessId,pmi->BaseAddress,outputbuff,pmi->SizeOfImage,&size))
				//{
				//	pmi->PEImage = outputbuff;
				//}
				else
				{
					free(outputbuff);
					pmi->PEImage = NULL;
				}
			}
			//查找重名模块，然后做成一个链表以备以后使用
			for(DWORD j=i+1;j<count;j++)
			{
				PModuleInfo pmj =  &(ppd->m_miv[j]);
				GetImageNameByPath(wjImageName,pmj->FullDllName);
				if(!_wcsicmp(wiImageName,wjImageName))
				{
					pmi->SameNameNext = pmj;
					bool bHas = false;
					for(pos=rdv.begin();pos!=rdv.end();pos++)
					{
						ModuleInfo rdmi = (*pos);
						GetImageNameByPath(wjImageName,rdmi.FullDllName);
						if(!_wcsicmp(wiImageName,wjImageName))
						{
							bHas = true;
							break;
						}
					}
					if(!bHas)
					{
						rdv.push_back(*pmi);
					}
					break;
				}
			}
			//********************************************
			taskvalue += perval;
			phcltask->SetValue((DWORD)taskvalue);
		}
		PBYTE pTextSection1,pTextSection2,pTextSection3;
		DWORD TextSectionSize1,TextSectionSize2,TextSectionSize3;
		PMEMORYMODULE pmm2,pmm3;
		ULONG_PTR offset;
		perval = (float)80/(float)count;
		for(DWORD i=0;i<count;i++)
		{
			ModuleInfo mi = ppd->m_miv[i];
			if(mi.PEImage)
			{
				pTextSection1 = GetSectionByName((const unsigned char *)mi.PEImage,&TextSectionSize1,".text");
				if(pTextSection1)
				{
					offset = (ULONG_PTR)pTextSection1 - (ULONG_PTR)mi.PEImage;
				}
				else
				{
					continue;
				}
				//确认重名模块在当前模块中使用的是那一个
				WCHAR wImageName[MAX_PATH];
				CHAR ImageName[MAX_PATH];
				ModuleInfoVector confirmrenamedllvector;
				for(pos=rdv.begin();pos!=rdv.end();pos++)
				{
					ModuleInfo rdmi = (*pos);
					GetImageNameByPath(wImageName,rdmi.FullDllName);
					WideCharToMultiByte(CP_ACP, 0, wImageName, -1, ImageName, MAX_PATH, NULL, NULL);
					ULONG_PTR firstfunadr;//返回导入表第一个函数地址
					if(HasDllInImortApiTable((const unsigned char *)mi.PEImage,ImageName,firstfunadr))
					{
						do
						{
							if(firstfunadr)
							{
								if(rdmi.BaseAddress < firstfunadr && firstfunadr < rdmi.BaseAddress + rdmi.SizeOfImage)
								{
									confirmrenamedllvector.push_back(rdmi);
									break;
								}
							}
							else
							{
								break;
							}
							if(rdmi.SameNameNext)
							{
								rdmi = *rdmi.SameNameNext;
							}
							else
							{
								break;
							}
						}while(true);
					}
				}
				//***********************************************
				PVOID localfilebuff = GetLocalFileBuff(mi.FullDllName,TextSectionSize2);
				if(localfilebuff)
				{
					pmm2 = MemoryLoadLibrary(localfilebuff,mi.BaseAddress,&ppd->m_miv,FALSE,&confirmrenamedllvector);//内存加载PE镜像不用其他DLL修正
					if(pmm2)
					{
						ULONG_PTR ImageBase = GetPeImageBase((const unsigned char *)localfilebuff);//获取原来文件的加载ImageBase
						pmm3 = MemoryLoadLibrary(localfilebuff,mi.BaseAddress,&ppd->m_miv,TRUE,&confirmrenamedllvector);//内存加载PE镜像用其他DLL修正
						assert(pmm3!=NULL);
						pTextSection3 = GetSectionByName((const unsigned char *)pmm3->codeBase,&TextSectionSize3,".text"); 
						pTextSection2 = GetSectionByName((const unsigned char *)pmm2->codeBase,&TextSectionSize2,".text");
						if(TextSectionSize2==TextSectionSize1)
						{
							ULONG_PTR beginadr = -1,endadr = -1;
							for(DWORD j=0;j<TextSectionSize1;j++)
							{
								if(pTextSection1[j]!=pTextSection2[j] && pTextSection1[j]!=pTextSection3[j])
								{
									if(beginadr==-1)
										beginadr = j;
								}
								else
								{
									if(beginadr!=-1)
									{
										endadr = j;
										ppd->AddMemoryHook(mi.FullDllName,mi.BaseAddress + offset + beginadr,pTextSection3 + beginadr,pTextSection1 + beginadr,endadr - beginadr);
										beginadr=-1;
									}
								}
							}
						}
						MemoryFreeLibrary(pmm2);
						MemoryFreeLibrary(pmm3);
					}
					FreeLoacalFileBuff(localfilebuff);
				}
			}
			taskvalue += perval;
			phcltask->SetValue((DWORD)taskvalue);
		}
	}
	phcltask->SetValue(100);
	return 0;
}

DWORD WINAPI CRConsole::DoKernelModulesList(LPVOID pTask)
{
	CTask * pkmltask = (CTask *)pTask;
	CKernelModuleDoc * pkmd = (CKernelModuleDoc *)pkmltask->GetParameter();
	pkmd->ClearModuleInfoVector();
	DWORD size = 0;
	if(KernelModulesList(size) && size)
	{
		PVOID buff = malloc(size*sizeof(ModuleInfo));
		if(buff && KernelModulesGet(buff,size*sizeof(ModuleInfo)))
		{
			PModuleInfo pmi = (PModuleInfo)buff;
			for(DWORD i=0;i<size;i++)
			{
				pmi->ParentProcessId = 0;
				pmi->PEImage = NULL;
				pmi->SameNameNext = NULL;
				pkmd->m_miv.push_back(*pmi);
				pmi++;
			}
		}
		if(buff)
			free(buff);
	}
	pkmltask->SetValue(100);
	return 0;
}

#pragma pack(push, 1)
typedef struct _SSDTIndex
{
	BYTE byMov;		//B8 => MOV EAX, XXXX
	ULONG ulIndex;	//Service Number
} SSDTIndex, *PSSDTIndex;
#pragma pack(pop)

DWORD WINAPI CRConsole::DoSSDTList(LPVOID pTask)
{
	CTask * psltask = (CTask *)pTask;
	CSSDTDoc * psd = (CSSDTDoc *)psltask->GetParameter();
	psd->m_siv.clear();
	DWORD size = 0;
	ModuleInfoVector::iterator pos;
	CKernelModuleDoc * pkmd = (CKernelModuleDoc *)CTaskManager::GetInstance()->GetDocument(RUNTIME_CLASS(CKernelModuleDoc));
	if(SSDTList(size) && size)
	{
		PVOID buff = malloc(size*sizeof(SSDTInfo));
		if(buff && SSDTGet(buff,size*sizeof(SSDTInfo)))
		{
			PSSDTInfo psi = (PSSDTInfo)buff;
			PVOID BaseAddress;
			HANDLE hSection = CRMapViewOfFile(L"\\SystemRoot\\system32\\ntdll.dll",BaseAddress);
			if(hSection)//获取SSDT函数名
			{
				unsigned char *codeBase = (unsigned char *)BaseAddress;
				PIMAGE_DOS_HEADER dos_header = (PIMAGE_DOS_HEADER)codeBase;
				PIMAGE_NT_HEADERS nt_header = (PIMAGE_NT_HEADERS)&((const unsigned char *)(codeBase))[dos_header->e_lfanew];
				int idx=-1;
				DWORD i, *nameRef;
				WORD *ordinal;
				PIMAGE_EXPORT_DIRECTORY exports;
				PIMAGE_DATA_DIRECTORY directory = &nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
				exports = (PIMAGE_EXPORT_DIRECTORY) (codeBase + directory->VirtualAddress);
				nameRef = (DWORD *) (codeBase + exports->AddressOfNames);
				ordinal = (WORD *) (codeBase + exports->AddressOfNameOrdinals);
				for (i=0; i<exports->NumberOfNames; i++, nameRef++, ordinal++) {
					LPCSTR lpName = LPCSTR(codeBase + (*nameRef));
					if(strlen(lpName)>2 && lpName[0]=='N' && lpName[1]=='t')
					{
						idx = *ordinal;
						FARPROC address =  FARPROC(codeBase + (*(DWORD *) (codeBase + exports->AddressOfFunctions + (idx*4))));
						SSDTIndex sindex;
						memcpy(&sindex,(PVOID)address,sizeof(SSDTIndex));
						if(sindex.byMov==0xb8)
						{
							PSSDTInfo psi2 = psi + sindex.ulIndex;
							strcpy_s(psi2->FunctionName,lpName);
						}
					}
				}
				CRUnmapViewOfFile(hSection);
			}
			for(DWORD i=0;i<size;i++)
			{
				if(psi)
				{
					ZeroMemory(psi->FullDllName,MAX_PATH*sizeof(WCHAR));
					for(pos=pkmd->m_miv.begin();pos!=pkmd->m_miv.end();pos++)
					{
						ModuleInfo mi = *pos;
						if(psi->CurrentAddress > mi.BaseAddress && psi->CurrentAddress < mi.BaseAddress + mi.SizeOfImage)
						{
							wcscpy_s(psi->FullDllName,mi.FullDllName);
							break;
						}
					}
				}
				psd->m_siv.push_back(*psi);
				psi++;
			}
		}
		if(buff)
			free(buff);
	}
	psltask->SetValue(100);
	return 0;
}

DWORD WINAPI CRConsole::DoSSDTShadowList(LPVOID pTask)
{
	CTask * psltask = (CTask *)pTask;
	CSSDTDoc * psd = (CSSDTDoc *)psltask->GetParameter();
	psd->m_siv.clear();
	DWORD size = 0;
	ModuleInfoVector::iterator pos;
	CKernelModuleDoc * pkmd = (CKernelModuleDoc *)CTaskManager::GetInstance()->GetDocument(RUNTIME_CLASS(CKernelModuleDoc));
	if(SSDTShadowList(size) && size)
	{
		PVOID buff = malloc(size*sizeof(SSDTInfo));
		if(buff && SSDTShadowGet(buff,size*sizeof(SSDTInfo)))
		{
			PSSDTInfo psi = (PSSDTInfo)buff;
			for(DWORD i=0;i<size;i++)
			{
				SYSTEM_VERSION version  = GetSystemVersion();
				switch(version)
				{
				case WINDOWSXP:
					strcpy_s(psi->FunctionName,XPProcName[i]);
					break;
				case WINDOWS7:
					strcpy_s(psi->FunctionName,Win7ProcName[i]);
					break;
				}
				if(psi)
				{
					ZeroMemory(psi->FullDllName,MAX_PATH*sizeof(WCHAR));
					for(pos=pkmd->m_miv.begin();pos!=pkmd->m_miv.end();pos++)
					{
						ModuleInfo mi = *pos;
						if(psi->CurrentAddress > mi.BaseAddress && psi->CurrentAddress < mi.BaseAddress + mi.SizeOfImage)
						{
							wcscpy_s(psi->FullDllName,mi.FullDllName);
							break;
						}
					}
				}
				psd->m_siv.push_back(*psi);
				psi++;
			}
		}
		if(buff)
			free(buff);
	}
	psltask->SetValue(100);
	return 0;
}

DWORD WINAPI CRConsole::DoDpcTimerList(LPVOID pTask)
{
	CTask * pdtltask = (CTask *)pTask;
	CDpcTimerDoc * pdtd = (CDpcTimerDoc *)pdtltask->GetParameter();
	pdtd->m_dtiv.clear();
	DWORD size = 0;
	ModuleInfoVector::iterator pos;
	CKernelModuleDoc * pkmd = (CKernelModuleDoc *)CTaskManager::GetInstance()->GetDocument(RUNTIME_CLASS(CKernelModuleDoc));
	if(DpcTimerList(size) && size)
	{
		PVOID buff = malloc(sizeof(DpcTimerInfo)*size);
		if(buff)
		{
			if(DpcTimerGet(buff,sizeof(DpcTimerInfo)*size))
			{
				PDpcTimerInfo pdti = (PDpcTimerInfo)buff;
				for(DWORD i = 0;i < size;i++)
				{
					if(pkmd)
					{
						ZeroMemory(pdti->FullDllName,MAX_PATH*sizeof(WCHAR));
						for(pos=pkmd->m_miv.begin();pos!=pkmd->m_miv.end();pos++)
						{
							ModuleInfo mi = *pos;
							if(pdti->DpcRoutineAddress > mi.BaseAddress && pdti->DpcRoutineAddress < mi.BaseAddress + mi.SizeOfImage)
							{
								wcscpy_s(pdti->FullDllName,mi.FullDllName);
								break;
							}
						}
					}
					pdtd->m_dtiv.push_back(*pdti);
					pdti++;
				}
			}
			free(buff);
		}
	}
	pdtltask->SetValue(100);
	return 0;
}

DWORD WINAPI CRConsole::DoIoTimerList(LPVOID pTask)
{
	CTask * pitltask = (CTask *)pTask;
	CIoTimerDoc * pitd = (CIoTimerDoc *)pitltask->GetParameter();
	pitd->m_itiv.clear();
	DWORD size = 0;
	ModuleInfoVector::iterator pos;
	CKernelModuleDoc * pkmd = (CKernelModuleDoc *)CTaskManager::GetInstance()->GetDocument(RUNTIME_CLASS(CKernelModuleDoc));
	if(IoTimerList(size) && size)
	{
		PVOID buff = malloc(sizeof(IoTimerInfo)*size);
		if(buff)
		{
			if(IoTimerGet(buff,sizeof(IoTimerInfo)*size))
			{
				PIoTimerInfo piti = (PIoTimerInfo)buff;
				for(DWORD i = 0;i < size;i++)
				{
					if(pkmd)
					{
						ZeroMemory(piti->FullDllName,MAX_PATH*sizeof(WCHAR));
						for(pos=pkmd->m_miv.begin();pos!=pkmd->m_miv.end();pos++)
						{
							ModuleInfo mi = *pos;
							if(piti->IoTimerRoutineAddress > mi.BaseAddress && piti->IoTimerRoutineAddress < mi.BaseAddress + mi.SizeOfImage)
							{
								wcscpy_s(piti->FullDllName,mi.FullDllName);
								break;
							}
						}
					}
					pitd->m_itiv.push_back(*piti);
					piti++;
				}
			}
			free(buff);
		}
	}
	pitltask->SetValue(100);
	return 0;
}

DWORD WINAPI CRConsole::DoTerminateProcess(LPVOID pTask)
{
	CTask * ptptask = (CTask *)pTask;
	TransferMsg msg;
	msg.dwMsgId = TERMINATE_PROCESS;
	msg.size = 1;
	*PULONG_PTR(msg.buff) = (ULONG_PTR)ptptask->GetParameter();
	TransferMessage(msg);
	ptptask->SetValue(100);
	return 0;
}

DWORD WINAPI CRConsole::DoKillDpcTimer(LPVOID pTask)
{
	CTask * pkttask = (CTask *)pTask;
	TransferMsg msg;
	msg.dwMsgId = KILL_DPCTIMER;
	msg.size = 1;
	*PULONG_PTR(msg.buff) = (ULONG_PTR)pkttask->GetParameter();
	TransferMessage(msg);
	pkttask->SetValue(100);
	return 0;
}

DWORD WINAPI CRConsole::DoStopIoTimer(LPVOID pTask)
{
	CTask * psittask = (CTask *)pTask;
	TransferMsg msg;
	msg.dwMsgId = STOP_IOTIMER;
	msg.size = 1;
	*PULONG_PTR(msg.buff) = (ULONG_PTR)psittask->GetParameter();
	TransferMessage(msg);
	psittask->SetValue(100);
	return 0;
}

DWORD WINAPI CRConsole::DoStartIoTimer(LPVOID pTask)
{
	CTask * psittask = (CTask *)pTask;
	TransferMsg msg;
	msg.dwMsgId = START_IOTIMER;
	msg.size = 1;
	*PULONG_PTR(msg.buff) = (ULONG_PTR)psittask->GetParameter();
	TransferMessage(msg);
	psittask->SetValue(100);
	return 0;
}

DWORD WINAPI CRConsole::DoSSDTRecover(LPVOID pTask)
{
	CTask * psrtask = (CTask *)pTask;
	TransferMsg msg;
	msg.dwMsgId = RECOVER_SSDT;
	msg.size = 1;
	memcpy(msg.buff,psrtask->GetParameter(),sizeof(SSDTInfo));
	TransferMessage(msg);
	psrtask->SetValue(100);
	return 0;
}

DWORD WINAPI CRConsole::DoSSDTShadowRecover(LPVOID pTask)
{
	CTask * psrtask = (CTask *)pTask;
	TransferMsg msg;
	msg.dwMsgId = RECOVER_SSDTSHADOW;
	msg.size = 1;
	memcpy(msg.buff,psrtask->GetParameter(),sizeof(SSDTInfo));
	TransferMessage(msg);
	psrtask->SetValue(100);
	return 0;
}

DWORD WINAPI CRConsole::DoKernelHookRecover(LPVOID pTask)
{
	CTask * pkhrtask = (CTask *)pTask;
	TransferMsg msg;
	msg.dwMsgId = RECOVER_KERNELHOOK;
	msg.size = 1;
	memcpy(msg.buff,pkhrtask->GetParameter(),sizeof(MemoryHook));
	PMemoryHook pmh = (PMemoryHook)msg.buff;
	DWORD lenght = pmh->Length;
	PVOID origin = PVOID(ULONG_PTR(&pmh->Length) + sizeof(DWORD));
	PVOID current = PVOID(ULONG_PTR(origin) + lenght);
	pmh = (PMemoryHook)pkhrtask->GetParameter();
	memcpy(origin,pmh->Origin,lenght);
	memcpy(current,pmh->Current,lenght);
	TransferMessage(msg);
	pkhrtask->SetValue(100);
	return 0;
}

DWORD WINAPI CRConsole::DoTerminateThread(LPVOID pTask)
{
	CTask * ptttask = (CTask *)pTask;
	TransferMsg msg;
	msg.dwMsgId = KILL_THREAD;
	msg.size = 1;
	memcpy(msg.buff,ptttask->GetParameter(),sizeof(ThreadInfo));
	TransferMessage(msg);
	ptttask->SetValue(100);
	return 0;
}

DWORD WINAPI CRConsole::DoHookCheckRecover(LPVOID pTask)
{
	CTask * phcr = (CTask *)pTask;
	CProcessDoc * ppd = (CProcessDoc *)CTaskManager::GetInstance()->GetDocument(RUNTIME_CLASS(CProcessDoc));
	PMemoryHook pmh = (PMemoryHook)phcr->GetParameter();
	if(ppd->m_pCurrentProcessInfo)
	{
		ULONG_PTR ProcessId = ppd->m_pCurrentProcessInfo->ProcessId;
		HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS,TRUE,ProcessId);
		if(pmh && handle)
		{
			SIZE_T NumberOfBytesWritten;
			WriteProcessMemoryEx(handle,(LPVOID)pmh->Address,pmh->Origin,pmh->Length,&NumberOfBytesWritten);
		}
	}
	phcr->SetValue(100);
	return 0;
}

DWORD WINAPI CRConsole::GetProcAddressByName(LPVOID pTask)
{
	DWORD address = 0;
	CTask * ppabn = (CTask *)pTask;
	CProcessDoc * ppd = (CProcessDoc *)CTaskManager::GetInstance()->GetDocument(RUNTIME_CLASS(CProcessDoc));
	PProcAddressInfo ppai = (PProcAddressInfo)ppabn->GetParameter();
	WCHAR ImageName[MAX_PATH];
	ModuleInfoVector::iterator pos;
	if(ppd && ppai)
	{
		for(pos=ppd->m_miv.begin();pos!=ppd->m_miv.end();pos++)
		{
			ModuleInfo mi = *pos;
			GetImageNameByPath(ImageName,mi.FullDllName);
			if(!_wcsicmp(ImageName,ppai->dllname))
			{
				address = (DWORD)GetRemoteProcAddressA((HMODULE)mi.PEImage,(unsigned char *)mi.BaseAddress,ppai->functionname);
				break;
			}
		}
	}
	ppabn->SetValue(100);
	return address;
}

HANDLE g_hMDLEvent = NULL;
bool g_bMDLStart = false;

DWORD WINAPI CRConsole::MonitorDriverLoad(LPVOID pTask)
{
	WCHAR szInfo[4096] = {0};
	CTask * pmdl = (CTask *)pTask;
	BOOL flag = (BOOL)pmdl->GetParameter();
	if(flag)
	{
		g_hMDLEvent = CreateEvent(NULL, false, false, NULL); 
		if(SetSynchroniza(&g_hMDLEvent,sizeof(HANDLE)))
		{
			TransferMsg msg;
			msg.dwMsgId = MONITORDRIVERLOAD;
			*PBOOL(msg.buff) = flag;
			msg.size = 1;
			TransferMessage(msg);
			g_bMDLStart = true;
			while(g_bMDLStart) 
			{ 
				DWORD status = WaitForSingleObject(g_hMDLEvent, INFINITE);
				if(g_bMDLStart)
				{
					if(GetSynchronizaInfo(szInfo,4096))
					{
						MessageBox(AfxGetApp()->GetMainWnd()->m_hWnd,szInfo,L"Monitor Driver Load",MB_OK);
					}
					SetEvent(g_hMDLEvent);
				}
			}
		}
	}
	else
	{
		if(g_bMDLStart)
		{
			TransferMsg msg;
			msg.dwMsgId = MONITORDRIVERLOAD;
			*PBOOL(msg.buff) = flag;
			msg.size = 1;
			TransferMessage(msg);
			g_bMDLStart = false;
			SetEvent(g_hMDLEvent);
			SetSynchroniza(&flag,sizeof(HANDLE));
			CloseHandle(g_hMDLEvent);
			g_hMDLEvent = NULL;
		}
	}
	pmdl->End();
	return 0;
}