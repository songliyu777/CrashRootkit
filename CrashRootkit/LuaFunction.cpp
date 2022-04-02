#include "stdafx.h"
#include "Util.h"
#include "LuaFunction.h"
#include "LuaEngine.h"
#include "ConsoleDll.h"
#include "TaskManager.h"
#include "CRConsole.h"
#include "KernelHookDoc.h"
#include "ProcessDoc.h"
#include "KernelModuleDoc.h"
#include "SSDTDoc.h"
#include "DpcTimerDoc.h"
#include "IoTimerDoc.h"
#include "Login.h"
#include "Simulate.h"

#include "WebServiceClient.h"
#include <string>
using namespace std;

#define FUNCTION_SIZE 47

ULONG_PTR g_functionmap[FUNCTION_SIZE] = {	
								(ULONG_PTR)lua_print,								//0
								(ULONG_PTR)lua_print_on,							//1
								(ULONG_PTR)lua_print_off,							//2
								(ULONG_PTR)lua_DoProcessesList,						//3
								(ULONG_PTR)lua_DoKernelHookCheck,					//4
								(ULONG_PTR)lua_DoModulesList,						//5
								(ULONG_PTR)lua_DoThreadsList,						//6
								(ULONG_PTR)lua_DoHookCheckList,						//7
								(ULONG_PTR)lua_DoKernelModulesList,					//8
								(ULONG_PTR)lua_DoSSDTList,							//9
								(ULONG_PTR)lua_DoSSDTShadowList,					//10
								(ULONG_PTR)lua_DoDpcTimerList,						//11
								(ULONG_PTR)lua_DoIoTimerList,						//12
								(ULONG_PTR)lua_DoAntiProtectHook_1,					//13
								(ULONG_PTR)lua_DoAntiProtectHook_2,					//14
								(ULONG_PTR)lua_GetProcAddressByName,				//15
								(ULONG_PTR)lua_DoStopIoTimer,						//16
								(ULONG_PTR)lua_DoKillDpcTimer,						//17
								(ULONG_PTR)lua_DoKernelHookRecover,					//18
								(ULONG_PTR)lua_DoTerminateThread,					//19
								(ULONG_PTR)lua_WriteProcessMemory,					//20
								(ULONG_PTR)lua_KdDisableDebugger,					//21
								(ULONG_PTR)lua_KdEnableDebugger,					//22
								(ULONG_PTR)lua_HideKernelDebugger,  				//23
								(ULONG_PTR)lua_MonitorDriverLoad,					//24
								(ULONG_PTR)lua_AddTargetProcess,					//25
								(ULONG_PTR)lua_RecoverObject,						//26
								(ULONG_PTR)lua_AddProtectProcess,					//27
								(ULONG_PTR)lua_AddTargetKernelModule,				//28
								(ULONG_PTR)lua_AddFilterDrvIo,						//29
								(ULONG_PTR)lua_DoTerminateProcess,					//30
								(ULONG_PTR)lua_HideProcessModule,					//31
								(ULONG_PTR)lua_HideProcessMemory,					//32
								(ULONG_PTR)lua_SetKeSpeed,							//33
								(ULONG_PTR)lua_SetKeException,						//34
								(ULONG_PTR)lua_Exit,                                //35
								(ULONG_PTR)lua_CreateProcess,						//36
								(ULONG_PTR)lua_Statistics,							//37
								(ULONG_PTR)lua_AddSeparateProcess,					//38
								(ULONG_PTR)lua_Simulate,							//39
								(ULONG_PTR)lua_DoSuspendProcess,					//40
								(ULONG_PTR)lua_DoResumeProcess,						//41
								(ULONG_PTR)lua_Shutdown,							//42
								(ULONG_PTR)lua_HLogin,								//43
								(ULONG_PTR)lua_Binding,								//44
								(ULONG_PTR)lua_HideKernelModule,					//45
								(ULONG_PTR)lua_ModifyKernelMemory					//46
							};

static int lua_print(lua_State *L) {
	wchar_t wtmp[512];
	CString printstr;
	int n = lua_gettop(L);  /* number of arguments */
	int i;
	lua_getglobal(L, "tostring");
	for (i=1; i<=n; i++) {
		const char *s;
		size_t l;
		lua_pushvalue(L, -1);  /* function to be called */
		lua_pushvalue(L, i);   /* value to print */
		lua_call(L, 1, 1);
		s = lua_tolstring(L, -1, &l);  /* get result */
		if (s == NULL)
		{
			return luaL_error(L, LUA_QL("tostring") " must return a string to " LUA_QL("print"));
		}
		CharToWideChar(s,wtmp,512);
		printstr += wtmp;
		if (i>1)
		{
			printstr += L"\t";
		}
		CLuaEngine::GetInstance()->m_outputview->AddOutputString(printstr);
		lua_pop(L, 1);  /* pop result */
	}
	CLuaEngine::GetInstance()->m_outputview->AddOutputString(L"\r\n");
	return 0;
}

bool g_bprint = true;

static int lua_print_on(lua_State *L)
{
	g_bprint = true;
	return 0;
}

static int lua_print_off(lua_State *L)
{
	g_bprint = false;
	return 0;
}

void lua_printex(const char *fmt,...)
{
	if(!g_bprint)
		return;
	va_list argp;
	va_start(argp, fmt);
	strcpy_s(CLuaEngine::GetInstance()->GetScriptBuff(),MAXSCRIPTBUFF,"print(\"");
	vsprintf_s(CLuaEngine::GetInstance()->GetScriptBuff() + 7,MAXSCRIPTBUFF - 7, fmt, argp);
	strcat_s(CLuaEngine::GetInstance()->GetScriptBuff(),MAXSCRIPTBUFF,"\");");
	va_end(argp);
	CLuaEngine::GetInstance()->RunScript();
}

static int lua_DoProcessesList(lua_State *L)
{
	CProcessDoc * ppd = (CProcessDoc *)CTaskManager::GetInstance()->GetDocument(RUNTIME_CLASS(CProcessDoc));
	if(!ppd)
		return luaL_error(L,"无法获取 CProcessDoc 对象指针");
	CTaskManager::GetInstance()->CreateTaskUI(L"DoProcessesList",CRConsole::DoProcessesList,ppd);
	ProcessInfoVector* piv = &ppd->GetProcessInfoVector();
	SIZE_T size = piv->size();
	for(SIZE_T i=0;i<size;i++)
	{
		PProcessInfo ppi = &(*piv)[i];
		lua_printex("ProcessId[%d] ParentProcessId[%d] ImageFileName[%s]",ppi->ProcessId,ppi->ParentProcessId,ppi->ImageFileName);
	}
	return 0;
}

static int lua_DoKernelHookCheck(lua_State *L)
{
	CKernelHookDoc * pkhd = (CKernelHookDoc *)CTaskManager::GetInstance()->GetDocument(RUNTIME_CLASS(CKernelHookDoc));
	if(!pkhd)
		return luaL_error(L,"无法获取 CKernelHookDoc 对象指针");
	CTaskManager::GetInstance()->CreateTaskUI(L"DoKernelHookCheck",CRConsole::DoKernelHookCheck,pkhd);
	MemoryHookVector* pmiv = &pkhd->GetMemoryHookVector();
	SIZE_T size = pmiv->size();
	WCHAR szImageName[MAX_PATH];
	for(SIZE_T i=0;i<size;i++)
	{
		PMemoryHook pmh = &(*pmiv)[i];
		GetImageNameByPath(szImageName,pmh->ModuleName);
		lua_printex("Address[%p] ModuleName[%S] Length[%d]",pmh->Address,szImageName,pmh->Length);
	}
	return 0;
}

static int lua_DoModulesList(lua_State *L)
{
	wchar_t tmp[512];
	WCHAR szImageName[MAX_PATH];
	int n = lua_gettop(L);
	if(n!=1){
		return luaL_error(L,"只接受一个参数 ProcessId 或者 进程名");
	}
	CProcessDoc * ppd = (CProcessDoc *)CTaskManager::GetInstance()->GetDocument(RUNTIME_CLASS(CProcessDoc));
	if(!ppd)
		return luaL_error(L,"无法获取 CProcessDoc 对象指针");
	ULONG_PTR ProcessId = 0;
	if(lua_isstring(L,1))
	{
		const char * processname = lua_tostring(L,1);
		CharToWideChar(processname,tmp,512);
		ProcessInfoVector* piv = &ppd->GetProcessInfoVector();
		SIZE_T size = piv->size();
		for(SIZE_T i=0;i<size;i++)
		{
			PProcessInfo ppi = &(*piv)[i];
			GetImageNameByPath(szImageName,ppi->ImagePath);
			if(!_wcsicmp(szImageName,tmp))
			{
				ProcessId = ppi->ProcessId;
				break;
			}
		}
	}
	if(!ProcessId && !lua_isnumber(L,1))
	{
		return luaL_error(L,"参数出错");
	}
	if(!ProcessId)
		ProcessId = (ULONG_PTR)lua_tonumber(L,1);
	ProcessInfo pi;
	pi.ProcessId = ProcessId;
	ppd->m_pCurrentProcessInfo = &pi;
	CTaskManager::GetInstance()->CreateTaskUI(L"DoModulesList",CRConsole::DoModulesList,ppd);
	ModuleInfoVector * pmiv = &ppd->GetModuleInfoVector();
	SIZE_T size = pmiv->size();
	for(SIZE_T i=0;i<size;i++)
	{
		PModuleInfo pmi = &(*pmiv)[i];
		GetImageNameByPath(szImageName,pmi->FullDllName);
		lua_printex("BaseAddress[%p] EntryPoint[%p] SizeOfImage[%X] ModuleName[%S]",pmi->BaseAddress,pmi->EntryPoint,pmi->SizeOfImage,szImageName);
	}
	return 0;
}

static int lua_DoThreadsList(lua_State *L)
{
	wchar_t tmp[512];
	WCHAR szImageName[MAX_PATH];
	int n = lua_gettop(L);
	if(n!=1){
		return luaL_error(L,"只接受一个参数 ProcessId 或者 进程名");
	}
	CProcessDoc * ppd = (CProcessDoc *)CTaskManager::GetInstance()->GetDocument(RUNTIME_CLASS(CProcessDoc));
	if(!ppd)
		return luaL_error(L,"无法获取 CProcessDoc 对象指针");
	ULONG_PTR ProcessId = 0;
	if(lua_isstring(L,1))
	{
		const char * processname = lua_tostring(L,1);
		CharToWideChar(processname,tmp,512);
		ProcessInfoVector* piv = &ppd->GetProcessInfoVector();
		SIZE_T size = piv->size();
		for(SIZE_T i=0;i<size;i++)
		{
			PProcessInfo ppi = &(*piv)[i];
			GetImageNameByPath(szImageName,ppi->ImagePath);
			if(!_wcsicmp(szImageName,tmp))
			{
				ProcessId = ppi->ProcessId;
				break;
			}
		}
	}
	if(!ProcessId && !lua_isnumber(L,1))
	{
		return luaL_error(L,"参数出错\r\n");
	}
	if(!ProcessId)
		ProcessId = (ULONG_PTR)lua_tonumber(L,1);
	ProcessInfo pi;
	pi.ProcessId = ProcessId;
	ppd->m_pCurrentProcessInfo = &pi;
	CTaskManager::GetInstance()->CreateTaskUI(L"DoThreadsList",CRConsole::DoThreadsList,ppd);
	ThreadInfoVector * ptiv = &ppd->GetThreadInfoVector();
	SIZE_T size = ptiv->size();
	for(SIZE_T i=0;i<size;i++)
	{
		PThreadInfo pti = &(*ptiv)[i];
		GetImageNameByPath(szImageName,pti->FullDllName);
		lua_printex("Teb[%p] State[%d] UniqueThread[%d] StartAddress[%p] Win32StartAddress[%p] ModuleName[%S]",pti->Teb,pti->State,pti->UniqueThread,pti->StartAddress,pti->Win32StartAddress,szImageName);
	}
	return 0;
}

static int lua_DoHookCheckList(lua_State *L)
{
	wchar_t tmp[512];
	WCHAR szImageName[MAX_PATH];
	int n = lua_gettop(L);
	if(n!=1){
		return luaL_error(L,"只接受一个参数 ProcessId 或者 进程名");
	}
	CProcessDoc * ppd = (CProcessDoc *)CTaskManager::GetInstance()->GetDocument(RUNTIME_CLASS(CProcessDoc));
	if(!ppd)
		return luaL_error(L,"无法获取 CProcessDoc 对象指针");
	ULONG_PTR ProcessId = 0;
	if(lua_isstring(L,1))
	{
		const char * processname = lua_tostring(L,1);
		CharToWideChar(processname,tmp,512);
		ProcessInfoVector* piv = &ppd->GetProcessInfoVector();
		SIZE_T size = piv->size();
		for(SIZE_T i=0;i<size;i++)
		{
			PProcessInfo ppi = &(*piv)[i];
			GetImageNameByPath(szImageName,ppi->ImagePath);
			if(!_wcsicmp(szImageName,tmp))
			{
				ProcessId = ppi->ProcessId;
				break;
			}
		}
	}
	if(!ProcessId && !lua_isnumber(L,1))
	{
		return luaL_error(L,"参数出错");
	}
	if(!ProcessId)
		ProcessId = (ULONG_PTR)lua_tonumber(L,1);
	ProcessInfo pi;
	pi.ProcessId = ProcessId;
	ppd->m_pCurrentProcessInfo = &pi;
	CTaskManager::GetInstance()->CreateTaskUI(L"DoHookCheckList",CRConsole::DoHookCheckList,ppd,FALSE);;
	MemoryHookVector * pmhv = &ppd->GetMemoryHookVector();
	SIZE_T size = pmhv->size();
	for(SIZE_T i=0;i<size;i++)
	{
		PMemoryHook pmh = &(*pmhv)[i];
		GetImageNameByPath(szImageName,pmh->ModuleName);
		lua_printex("Address[%p] ModuleName[%S] Length[%d]",pmh->Address,szImageName,pmh->Length);
	}
	return 0;
}

static int lua_DoKernelModulesList(lua_State *L)
{
	WCHAR szImageName[MAX_PATH];
	CKernelModuleDoc * pkmd = (CKernelModuleDoc *)CTaskManager::GetInstance()->GetDocument(RUNTIME_CLASS(CKernelModuleDoc));
	if(!pkmd)
		return luaL_error(L,"无法获取 CKernelModuleDoc 对象指针");
	CTaskManager::GetInstance()->CreateTaskUI(L"DoKernelModulesList",CRConsole::DoKernelModulesList,pkmd);
	ModuleInfoVector* pmiv = &pkmd->GetModuleInfoVector();
	SIZE_T size = pmiv->size();
	for(SIZE_T i=0;i<size;i++)
	{
		PModuleInfo pmi = &(*pmiv)[i];
		GetImageNameByPath(szImageName,pmi->FullDllName);
		lua_printex("BaseAddress[%p] EntryPoint[%p] SizeOfImage[%X] ModuleName[%S]",pmi->BaseAddress,pmi->EntryPoint,pmi->SizeOfImage,szImageName);
	}
	return 0;
}

static int lua_DoSSDTList(lua_State *L)
{
	int n = lua_gettop(L);
	int all = 0;
	if(n!=0 && n!=1 && !lua_isboolean(L,1))
	{
		return luaL_error(L,"参数错误");
	}
	if(n==1)
		all = lua_toboolean(L,1);
	POSITION pos = NULL;
	CSSDTDoc * psd = (CSSDTDoc *)CTaskManager::GetInstance()->GetDocument(RUNTIME_CLASS(CSSDTDoc),&pos);
	if(!psd)
		return luaL_error(L,"无法获取 CSSDTDoc 对象指针");
	if(psd->IsSSDTShadow())
	{
		psd = (CSSDTDoc *)CTaskManager::GetInstance()->GetDocument(RUNTIME_CLASS(CSSDTDoc),&pos);
	}
	if(!psd)
		return luaL_error(L,"无法获取 CSSDTDoc 对象指针");
	CTaskManager::GetInstance()->CreateTaskUI(L"DoSSDTList",CRConsole::DoSSDTList,psd);
	SSDTInfoVector* siv = &psd->GetSSDTInfoVector();
	SIZE_T size = siv->size();
	for(SIZE_T i=0;i<size;i++)
	{
		PSSDTInfo psi = &(*siv)[i];
		if(all)
		{
			lua_printex("Index[%d] FunctionName[%s] CurrentAddress[%p] OriginalAddress[%p]",psi->Index,psi->FunctionName,psi->CurrentAddress,psi->OriginalAddress);
		}
		else
		{
			if(psi->OriginalAddress!=psi->CurrentAddress)
			{
				lua_printex("Index[%d] FunctionName[%s] CurrentAddress[%p] OriginalAddress[%p]",psi->Index,psi->FunctionName,psi->CurrentAddress,psi->OriginalAddress);
			}
		}

	}
	return 0;
}

static int lua_DoSSDTShadowList(lua_State *L)
{
	int n = lua_gettop(L);
	int all = 0;
	if(n!=0 && n!=1 && !lua_isboolean(L,1))
	{
		return luaL_error(L,"参数错误");
	}
	if(n==1)
		all = lua_toboolean(L,1);
	POSITION pos = NULL;
	CSSDTDoc * psd = (CSSDTDoc *)CTaskManager::GetInstance()->GetDocument(RUNTIME_CLASS(CSSDTDoc),&pos);
	if(!psd)
		return luaL_error(L,"无法获取 CSSDTDoc 对象指针");
	if(!psd->IsSSDTShadow())
	{
		psd = (CSSDTDoc *)CTaskManager::GetInstance()->GetDocument(RUNTIME_CLASS(CSSDTDoc),&pos);
	}
	if(!psd)
		return luaL_error(L,"无法获取 CSSDTDoc 对象指针");
	CTaskManager::GetInstance()->CreateTaskUI(L"DoSSDTShadowList",CRConsole::DoSSDTShadowList,psd);
	SSDTInfoVector* siv = &psd->GetSSDTInfoVector();
	SIZE_T size = siv->size();
	for(SIZE_T i=0;i<size;i++)
	{
		PSSDTInfo psi = &(*siv)[i];
		if(all)
		{
			lua_printex("Index[%d] FunctionName[%s] CurrentAddress[%p] OriginalAddress[%p]",psi->Index,psi->FunctionName,psi->CurrentAddress,psi->OriginalAddress);
		}
		else
		{
			if(psi->OriginalAddress!=psi->CurrentAddress)
			{
				lua_printex("Index[%d] FunctionName[%s] CurrentAddress[%p] OriginalAddress[%p]",psi->Index,psi->FunctionName,psi->CurrentAddress,psi->OriginalAddress);
			}
		}
	}
	return 0;
}

static int lua_DoDpcTimerList(lua_State *L)
{
	WCHAR szImageName[MAX_PATH];
	CDpcTimerDoc * pdtd = (CDpcTimerDoc *)CTaskManager::GetInstance()->GetDocument(RUNTIME_CLASS(CDpcTimerDoc));
	if(!pdtd)
		return luaL_error(L,"无法获取 CDpcTimerDoc 对象指针");
	CTaskManager::GetInstance()->CreateTaskUI(L"DoDpcTimerList",CRConsole::DoDpcTimerList,pdtd);
	DpcTimerInfoVector * pdtiv = &pdtd->GetDpcTimerInfoVector();
	DWORD size = pdtiv->size();
	for(DWORD i = 0;i < size;i++)
	{
		PDpcTimerInfo pdti = &(*pdtiv)[i];
		GetImageNameByPath(szImageName,pdti->FullDllName);
		lua_printex("TimerAddress[%p] DpcAddress[%p] DpcRoutineAddress[%p] Period[%d] ModuleName[%S]",pdti->TimerAddress,pdti->DpcAddress,pdti->DpcRoutineAddress,pdti->Period,szImageName);
	}
	return 0;
}

static int lua_DoIoTimerList(lua_State *L)
{
	WCHAR szImageName[MAX_PATH];
	CIoTimerDoc * pitd = (CIoTimerDoc *)CTaskManager::GetInstance()->GetDocument(RUNTIME_CLASS(CIoTimerDoc));
	if(!pitd)
		return luaL_error(L,"无法获取 CIoTimerDoc 对象指针");
	CTaskManager::GetInstance()->CreateTaskUI(L"DoIoTimerList",CRConsole::DoIoTimerList,pitd);
	IoTimerInfoVector * pitiv = &pitd->GetIoTimerInfoVector();
	DWORD size = pitiv->size();
	for(DWORD i = 0;i < size;i++)
	{
		PIoTimerInfo pdti = &(*pitiv)[i];
		GetImageNameByPath(szImageName,pdti->FullDllName);
		lua_printex("DeviceObject[%p] IoTimerRoutineAddress[%p] Status[%d] ModuleName[%S]",pdti->DeviceObject,pdti->IoTimerRoutineAddress,pdti->ulStatus,szImageName);
	}
	return 0;
}

static int lua_DoAntiProtectHook_1(lua_State *L)
{
	TransferMsg msg;
	msg.dwMsgId = ANTI_PROTECT_1;
	msg.size = 1;
	if(TransferMessage(msg))
	{
		lua_printex("DoAntiProtectHook_1 success");
	}
	else
	{
		lua_printex("DoAntiProtectHook_1 failed");
	}
	return 0;
}

static int lua_DoAntiProtectHook_2(lua_State *L)
{
	TransferMsg msg;
	msg.dwMsgId = ANTI_PROTECT_2;
	msg.size = 1;
	if(TransferMessage(msg))
	{
		lua_printex("DoAntiProtectHook_2 success");
	}
	else
	{
		lua_printex("DoAntiProtectHook_2 failed");
	}
	return 0;
}

static int lua_GetProcAddressByName(lua_State *L)
{
	DWORD result;
	int n = lua_gettop(L);
	if(n!=2 || !lua_isstring(L,1) || !lua_isstring(L,2))
		return luaL_error(L,"参数错误");
	wchar_t tmp[64];
	const char * dllname = lua_tostring(L,1);
	const char * functionname = lua_tostring(L,2);
	CharToWideChar(dllname,tmp,64);
	ProcAddressInfo pai;
	strcpy_s(pai.functionname,functionname);
	wcscpy_s(pai.dllname,tmp);
	result = CTaskManager::GetInstance()->CreateTaskUI(L"GetProcAddressByName",CRConsole::GetProcAddressByName,&pai);
	lua_pushinteger(L,result);
	return 1;
}

static int lua_DoStopIoTimer(lua_State *L)
{
	WCHAR szImageName[MAX_PATH];
	int n = lua_gettop(L);
	if(n!=1 && !lua_isstring(L,1))
	{
		return luaL_error(L,"参数错误");
	}
	wchar_t tmp[MAX_PATH];
	const char * kernelmodulename = lua_tostring(L,1);
	CharToWideChar(kernelmodulename,tmp,MAX_PATH);
	bool passtp = false;
	if(!strcmp("TesSafe.sys",kernelmodulename))
	{
		passtp = true;
	}
	CIoTimerDoc * pitd = (CIoTimerDoc *)CTaskManager::GetInstance()->GetDocument(RUNTIME_CLASS(CIoTimerDoc));
	if(pitd)
	{
		IoTimerInfoVector * pitiv = &pitd->GetIoTimerInfoVector();
		DWORD size = pitiv->size();
		for(DWORD i = 0;i < size;i++)
		{
			PIoTimerInfo piti = &(*pitiv)[i];
			if(passtp)
			{
				if(piti)
				{
					if(wcslen(piti->FullDllName)==0)
					{
						CTaskManager::GetInstance()->CreateTaskUI(L"DoStopIoTimer",CRConsole::DoStopIoTimer,(LPVOID)piti->DeviceObject);
					}
					GetImageNameByPath(szImageName,piti->FullDllName);
					if(!wcscmp(szImageName,tmp))
					{
						CTaskManager::GetInstance()->CreateTaskUI(L"DoStopIoTimer",CRConsole::DoStopIoTimer,(LPVOID)piti->DeviceObject);
					}
				}
			}
			else
			{
				if(piti)
				{
					GetImageNameByPath(szImageName,piti->FullDllName);
					if(!wcscmp(szImageName,tmp))
					{
						CTaskManager::GetInstance()->CreateTaskUI(L"DoStopIoTimer",CRConsole::DoStopIoTimer,(LPVOID)piti->DeviceObject);
					}
				}
			}
		}
	}
	return 0; 
}

static int lua_DoKillDpcTimer(lua_State *L)
{
	WCHAR szImageName[MAX_PATH];
	int n = lua_gettop(L);
	if(n!=1 && !lua_isstring(L,1))
	{
		return luaL_error(L,"参数错误");
	}
	wchar_t tmp[MAX_PATH];
	const char * kernelmodulename = lua_tostring(L,1);
	CharToWideChar(kernelmodulename,tmp,MAX_PATH);
	CDpcTimerDoc * pdtd = (CDpcTimerDoc *)CTaskManager::GetInstance()->GetDocument(RUNTIME_CLASS(CDpcTimerDoc));
	if(pdtd)
	{
		DpcTimerInfoVector * pdtiv = &pdtd->GetDpcTimerInfoVector();
		DWORD size = pdtiv->size();
		for(DWORD i = 0;i < size;i++)
		{
			PDpcTimerInfo pdti = &(*pdtiv)[i];
			if(pdti)
			{
				GetImageNameByPath(szImageName,pdti->FullDllName);
				if(!wcscmp(szImageName,tmp))
				{
					CTaskManager::GetInstance()->CreateTaskUI(L"DoKillDpcTimer",CRConsole::DoKillDpcTimer,(LPVOID)pdti->TimerAddress);
				}
			}
		}
	}
	return 0; 
}

static int lua_DoKernelHookRecover(lua_State *L)
{
	WCHAR szImageName[MAX_PATH];
	int n = lua_gettop(L);
	if(n!=1 && !lua_isstring(L,1))
	{
		return luaL_error(L,"参数错误");
	}
	wchar_t tmp[MAX_PATH];
	const char * kernelmodulename = lua_tostring(L,1);
	CharToWideChar(kernelmodulename,tmp,MAX_PATH);
	bool passtp = false;
	if(!strcmp("TesSafe.sys",kernelmodulename))
	{
		passtp = true;
	}
	CKernelHookDoc * pkhd = (CKernelHookDoc *)CTaskManager::GetInstance()->GetDocument(RUNTIME_CLASS(CKernelHookDoc));
	if(pkhd)
	{
		MemoryHookVector * pmhv = &pkhd->GetMemoryHookVector();
		DWORD size = pmhv->size();
		MemoryHookVector::iterator pos;
		for(DWORD i = 0;i < size;i++)
		{
			PMemoryHook pmh = &(*pmhv)[i];
			if(pmh)
			{
				if(passtp)
				{
					if(wcslen(pmh->JmpModuleName)==0 && (pmh->Length==4 || pmh->Length==6 ||pmh->Length==7 || pmh->Length==8))
					{
						CTaskManager::GetInstance()->CreateTaskUI(L"DoKernelHookRecover",CRConsole::DoKernelHookRecover,pmh);
					}
					GetImageNameByPath(szImageName,pmh->JmpModuleName);
					if(!wcscmp(szImageName,tmp))
					{
						CTaskManager::GetInstance()->CreateTaskUI(L"DoKernelHookRecover",CRConsole::DoKernelHookRecover,pmh);
					}
				}
				else
				{
					GetImageNameByPath(szImageName,pmh->JmpModuleName);
					if(!wcscmp(szImageName,tmp))
					{
						CTaskManager::GetInstance()->CreateTaskUI(L"DoKernelHookRecover",CRConsole::DoKernelHookRecover,pmh);
					}
				}
			}
		}
	}
	return 0; 
}

static int lua_DoTerminateThread(lua_State *L)
{
	WCHAR szImageName[MAX_PATH];
	int n = lua_gettop(L);
	if((n<1))
	{
		return luaL_error(L,"接受一个参数模块名或者入口地址和一个模块名");
	}
	if(!lua_isstring(L,1) && !lua_isnumber(L,1))
	{
		return luaL_error(L,"第一参数错误");
	}
	bool bIsStr = true;
	if(lua_isnumber(L,1))
	{
		bIsStr = false;
	}
	wchar_t modulename[512]={0};
	bool bIsModule = false;
	if(n>1)
	{
		const char * tmp = lua_tostring(L,2);
		CharToWideChar(tmp,modulename,512);
		bIsModule = true;
	}
	wchar_t tmp[MAX_PATH];
	const char * kernelmodulename = NULL;
	ULONG_PTR entry = 0;
	bool passtp = false;
	if(bIsStr)
	{
		kernelmodulename = lua_tostring(L,1);
		CharToWideChar(kernelmodulename,tmp,MAX_PATH);
		if(!strcmp("TesSafe.sys",kernelmodulename))
		{
			passtp = true;
		}
	}
	else
	{
		entry = (ULONG_PTR)lua_tonumber(L,1);
	}
	CProcessDoc * ppd = (CProcessDoc *)CTaskManager::GetInstance()->GetDocument(RUNTIME_CLASS(CProcessDoc));
	if(ppd)
	{
		ThreadInfoVector *ptiv = &ppd->GetThreadInfoVector();
		SIZE_T size = ptiv->size();
		for(SIZE_T i=0;i<size;i++)
		{
			PThreadInfo pti = &(*ptiv)[i];
			if(pti)
			{
				if(bIsStr)
				{
					if(passtp)
					{
						if(wcslen(pti->FullDllName)==0)
						{
							CTaskManager::GetInstance()->CreateTaskUI(L"DoTerminateThread",CRConsole::DoTerminateThread,pti);
						}
					}
					GetImageNameByPath(szImageName,pti->FullDllName);
					if(!wcscmp(szImageName,tmp))
					{
						CTaskManager::GetInstance()->CreateTaskUI(L"DoTerminateThread",CRConsole::DoTerminateThread,pti);
					}
				}
				else
				{
					if(bIsModule)
					{
						if(((entry & pti->Win32StartAddress) == entry || (entry & pti->StartAddress)  == entry) )
						{
							GetImageNameByPath(szImageName,pti->FullDllName);
							if(!wcscmp(szImageName,modulename))
							{
								CTaskManager::GetInstance()->CreateTaskUI(L"DoTerminateThread",CRConsole::DoTerminateThread,pti);
							}
						}
					}
					else
					{
						if(entry==pti->Win32StartAddress||entry==pti->StartAddress)
						{
							CTaskManager::GetInstance()->CreateTaskUI(L"DoTerminateThread",CRConsole::DoTerminateThread,pti);
						}
					}
				}

			}
		}
	}
	return 0;
}

static int lua_WriteProcessMemory(lua_State *L)
{
	wchar_t tmp[512];
	WCHAR szImageName[MAX_PATH];
	int n = lua_gettop(L);
	if(n!=4)
	{
		return luaL_error(L,"需要4个参数");
	}
	if(!lua_isnumber(L,1) && !lua_isstring(L,1))
	{
		return luaL_error(L,"参数1错误");
	}
	CProcessDoc * ppd = (CProcessDoc *)CTaskManager::GetInstance()->GetDocument(RUNTIME_CLASS(CProcessDoc));
	if(!ppd)
		return luaL_error(L,"无法获取 CProcessDoc 对象指针");
	ULONG_PTR ProcessId = 0;
	if(lua_isstring(L,1))
	{
		const char * processname = lua_tostring(L,1);
		CharToWideChar(processname,tmp,512);
		ProcessInfoVector* piv = &ppd->GetProcessInfoVector();
		SIZE_T size = piv->size();
		for(SIZE_T i=0;i<size;i++)
		{
			PProcessInfo ppi = &(*piv)[i];
			GetImageNameByPath(szImageName,ppi->ImagePath);
			if(!_wcsicmp(szImageName,tmp))
			{
				ProcessId = ppi->ProcessId;
				break;
			}
		}
	}
	if(!ProcessId && !lua_isnumber(L,1))
	{
		return luaL_error(L,"参数1出错");
	}
	if(!lua_isnumber(L,2))
	{
		return luaL_error(L,"参数2出错");
	}
	if(!lua_istable(L,3))
	{
		return luaL_error(L,"参数3出错");
	}
	if(!lua_isnumber(L,4))
	{
		return luaL_error(L,"参数4出错");
	}
	ULONG_PTR lpBaseAddress = (ULONG_PTR)lua_tonumber(L,2);
	SIZE_T size = (SIZE_T)lua_tonumber(L,4);
	PBYTE buff = (PBYTE)malloc(size);
	if(!buff)
		return luaL_error(L,"函数空间分配出错");
	int tabidx = 3;  // 取 table 索引值 
	int index = 0;
	lua_pushnil(L);  // nil 入栈作为初始 key 
	while( 0 != lua_next(L, tabidx)) 
	{ 
		if(lua_isnumber(L,-1))
		{
			int val = (int)lua_tonumber(L,-1);
			buff[index] = val & 0xFF;
			index++;
		}
		lua_pop(L, 1);
	}
	if(index!=size)
	{
		free(buff);
		return luaL_error(L,"传入参数不符合");
	}
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS,TRUE,ProcessId);
	if(!hProcess)
		return luaL_error(L,"进程打开失败");
	DWORD NumberOfBytesWritten;
	if(!WriteProcessMemoryEx(hProcess,(LPVOID)lpBaseAddress,buff,size,&NumberOfBytesWritten))
	{
		free(buff);
		return luaL_error(L,"函数执行失败");
	}
	free(buff);
	lua_pushnumber(L,NumberOfBytesWritten);
	return 1;
}

static int lua_KdDisableDebugger(lua_State *L)
{
	TransferMsg msg;
	msg.dwMsgId = KDDISABLEDEBUGGER;
	msg.size = 1;
	TransferMessage(msg);
	lua_printex("KdDisableDebugger OK");
	return 0;
}

static int lua_KdEnableDebugger(lua_State *L)
{
	TransferMsg msg;
	msg.dwMsgId = KDENABLEDEBUGGER;
	msg.size = 1;
	TransferMessage(msg);
	lua_printex("KdEnableDebugger OK");
	return 0;
}

static int lua_HideKernelDebugger(lua_State *L)
{
	int n = lua_gettop(L);
	if(n!=1 && !lua_isboolean(L,1))
	{
		return luaL_error(L,"参数错误");
	}
	BOOL flag = lua_toboolean(L,1);
	TransferMsg msg;
	msg.dwMsgId = HIDEKERNELDEBUGGER;
	msg.size = 1;
	*PBOOL(msg.buff) = flag;
	if(flag)
	{
		if(TransferMessage(msg))
		{
			lua_printex("HideKernelDebugger ON");
		}
	}
	else
	{
		if(TransferMessage(msg))
		{
			lua_printex("HideKernelDebugger OFF");
		}
	}
	return 0;
}

static int lua_MonitorDriverLoad(lua_State *L)
{
	int n = lua_gettop(L);
	if(n!=1 && !lua_isboolean(L,1))
	{
		return luaL_error(L,"参数错误");
	}
	BOOL flag = lua_toboolean(L,1);
	if(flag)
	{
		CTaskManager::GetInstance()->CreateTask(L"MonitorDriverLoad",CRConsole::MonitorDriverLoad,(LPVOID)flag);
		lua_printex("MonitorDriverLoad ON");
	}
	else
	{
		CTaskManager::GetInstance()->CreateTask(L"MonitorDriverLoad",CRConsole::MonitorDriverLoad,(LPVOID)flag);
		lua_printex("MonitorDriverLoad OFF");
	}
	return 0;
}

static int lua_AddTargetProcess(lua_State *L)
{
	wchar_t tmp[512];
	WCHAR szImageName[MAX_PATH];
	int n = lua_gettop(L);
	if(n!=1){
		return luaL_error(L,"只接受一个参数 ProcessId 或者 进程名");
	}
	CProcessDoc * ppd = (CProcessDoc *)CTaskManager::GetInstance()->GetDocument(RUNTIME_CLASS(CProcessDoc));
	if(!ppd)
		return luaL_error(L,"无法获取 CProcessDoc 对象指针");
	ULONG_PTR ProcessId = 0;
	if(lua_isstring(L,1))
	{
		const char * processname = lua_tostring(L,1);
		if(strcmp(processname,""))
		{
			CharToWideChar(processname,tmp,512);
			ProcessInfoVector* piv = &ppd->GetProcessInfoVector();
			SIZE_T size = piv->size();
			for(SIZE_T i=0;i<size;i++)
			{
				PProcessInfo ppi = &(*piv)[i];
				GetImageNameByPath(szImageName,ppi->ImagePath);
				if(!_wcsicmp(szImageName,tmp))
				{
					ProcessId = ppi->ProcessId;
					break;
				}
			}
		}
	}
	if(lua_isnumber(L,1))
		ProcessId = (ULONG_PTR)lua_tonumber(L,1);
	TransferMsg msg;
	msg.dwMsgId = ADDTARGETPROCESSID;
	msg.size = 1;
	*PHANDLE(msg.buff) = (HANDLE)ProcessId;
	if(TransferMessage(msg))
	{
		lua_printex("AddTargetProcess success");
	}
	else
	{
		lua_printex("AddTargetProcess failed");
	}
	return 0;
}

static int lua_RecoverObject(lua_State *L)
{
	TransferMsg msg;
	msg.dwMsgId = RECOVEROBJECT;
	msg.size = 1;
	if(TransferMessage(msg))
	{
		lua_printex("RecoverObject success");
	}
	else
	{
		lua_printex("RecoverObject failed");
	}
	return 0;
}

static int lua_AddProtectProcess(lua_State *L)
{
	wchar_t tmp[512];
	WCHAR szImageName[MAX_PATH];
	int n = lua_gettop(L);
	ULONG_PTR ProcessId = 0;
	if(n!=1){
		return luaL_error(L,"只接受一个参数 ProcessId 或者 进程名");
	}
	if(lua_isnumber(L,1))
	{
		ProcessId = (ULONG_PTR)lua_tonumber(L,1);
	}
	else
	{
		CProcessDoc * ppd = (CProcessDoc *)CTaskManager::GetInstance()->GetDocument(RUNTIME_CLASS(CProcessDoc));
		if(!ppd)
			return luaL_error(L,"无法获取 CProcessDoc 对象指针");
		if(lua_isstring(L,1))
		{
			const char * processname = lua_tostring(L,1);
			if(strcmp(processname,""))
			{
				CharToWideChar(processname,tmp,512);
				ProcessInfoVector* piv = &ppd->GetProcessInfoVector();
				SIZE_T size = piv->size();
				for(SIZE_T i=0;i<size;i++)
				{
					PProcessInfo ppi = &(*piv)[i];
					GetImageNameByPath(szImageName,ppi->ImagePath);
					if(!_wcsicmp(szImageName,tmp))
					{
						ProcessId = ppi->ProcessId;
						break;
					}
				}
			}
		}
	}
		
	TransferMsg msg;
	msg.dwMsgId = ADDPROTECTPROCESS;
	msg.size = 1;
	*PHANDLE(msg.buff) = (HANDLE)ProcessId;
	if(TransferMessage(msg))
	{
		lua_printex("AddProtectProcess success");
	}
	else
	{
		lua_printex("AddProtectProcess failed");
	}
	return 0;
}

static int lua_AddTargetKernelModule(lua_State *L)
{
	WCHAR szImageName[MAX_PATH];
	CKernelModuleDoc * pkmd = (CKernelModuleDoc *)CTaskManager::GetInstance()->GetDocument(RUNTIME_CLASS(CKernelModuleDoc));
	if(!pkmd)
		return luaL_error(L,"无法获取 CKernelModuleDoc 对象指针");
	int n = lua_gettop(L);
	if(n!=1 && !lua_isstring(L,1))
	{
		return luaL_error(L,"参数错误");
	}
	wchar_t tmp[MAX_PATH];
	const char * kernelmodulename = lua_tostring(L,1);
	CharToWideChar(kernelmodulename,tmp,MAX_PATH);
	ModuleInfoVector* pmiv = &pkmd->GetModuleInfoVector();
	SIZE_T size = pmiv->size();
	for(SIZE_T i=0;i<size;i++)
	{
		PModuleInfo pmi = &(*pmiv)[i];
		GetImageNameByPath(szImageName,pmi->FullDllName);
		if(!wcscmp(tmp,szImageName))
		{
			TransferMsg msg;
			msg.dwMsgId = ADDTARGETKERNELMODULE;
			msg.size = 1;
			memcpy(msg.buff,pmi,sizeof(ModuleInfo));
			if(TransferMessage(msg))
			{
				lua_printex("AddTargetKernelModule success");
			}
			else
			{
				lua_printex("AddTargetKernelModule failed");
			}
			break;
		}
	}
	return 0;
}

static int lua_AddFilterDrvIo(lua_State *L)
{
	wchar_t tmp[512];
	WCHAR szImageName[MAX_PATH];
	int n = lua_gettop(L);
	if(n!=2){
		return luaL_error(L,"只接受二个参数 ProcessId 或者 进程名 和 驱动名称");
	}
	CProcessDoc * ppd = (CProcessDoc *)CTaskManager::GetInstance()->GetDocument(RUNTIME_CLASS(CProcessDoc));
	if(!ppd)
		return luaL_error(L,"无法获取 CProcessDoc 对象指针");
	ULONG_PTR ProcessId = 0;
	if(lua_isstring(L,1))
	{
		const char * processname = lua_tostring(L,1);
		if(strcmp(processname,""))
		{
			CharToWideChar(processname,tmp,512);
			ProcessInfoVector* piv = &ppd->GetProcessInfoVector();
			SIZE_T size = piv->size();
			for(SIZE_T i=0;i<size;i++)
			{
				PProcessInfo ppi = &(*piv)[i];
				GetImageNameByPath(szImageName,ppi->ImagePath);
				if(!_wcsicmp(szImageName,tmp))
				{
					ProcessId = ppi->ProcessId;
					break;
				}
			}
		}
	}
	if(lua_isnumber(L,1))
		ProcessId = (ULONG_PTR)lua_tonumber(L,1);
	if(!lua_isstring(L,2))
	{
		return luaL_error(L,"第二参数错误");
	}
	wchar_t wtmp[32];
	const char * drvname = lua_tostring(L,2);
	CharToWideChar(drvname,wtmp,32);
	TransferMsg msg;
	msg.dwMsgId = ADDFILTERDRVIOINFO;
	msg.size = sizeof(FilterDrvInfo);
	PFilterDrvInfo(msg.buff)->ProcessId = ProcessId;
	wcscpy_s(PFilterDrvInfo(msg.buff)->ImageFileName,wtmp);
	if(TransferMessage(msg))
	{
		lua_printex("AddFilterDrvIo success");
	}
	else
	{
		lua_printex("AddFilterDrvIo failed");
	}
	return 0;
}

static int lua_DoTerminateProcess(lua_State *L)
{
	wchar_t tmp[512];
	WCHAR szImageName[MAX_PATH];
	int n = lua_gettop(L);
	if(n!=1){
		return luaL_error(L,"只接受一个参数 ProcessId 或者 进程名");
	}
	CProcessDoc * ppd = (CProcessDoc *)CTaskManager::GetInstance()->GetDocument(RUNTIME_CLASS(CProcessDoc));
	if(!ppd)
		return luaL_error(L,"无法获取 CProcessDoc 对象指针");
	ULONG_PTR ProcessId = 0;
	if(lua_isstring(L,1))
	{
		const char * processname = lua_tostring(L,1);
		if(strcmp(processname,""))
		{
			CharToWideChar(processname,tmp,512);
			ProcessInfoVector* piv = &ppd->GetProcessInfoVector();
			SIZE_T size = piv->size();
			for(SIZE_T i=0;i<size;i++)
			{
				PProcessInfo ppi = &(*piv)[i];
				GetImageNameByPath(szImageName,ppi->ImagePath);
				if(!_wcsicmp(szImageName,tmp))
				{
					ProcessId = ppi->ProcessId;
					break;
				}
			}
		}
	}
	if(lua_isnumber(L,1))
		ProcessId = (ULONG_PTR)lua_tonumber(L,1);
	TransferMsg msg;
	msg.dwMsgId = TERMINATE_PROCESS;
	msg.size = 1;
	*PHANDLE(msg.buff) = (HANDLE)ProcessId;
	if(TransferMessage(msg))
	{
		lua_printex("TerminateProcess success");
	}
	else
	{
		lua_printex("TerminateProcess failed");
	}
	return 0;
}

static int lua_DoSuspendProcess(lua_State *L)
{
	wchar_t tmp[512];
	WCHAR szImageName[MAX_PATH];
	int n = lua_gettop(L);
	if(n!=1){
		return luaL_error(L,"只接受一个参数 ProcessId 或者 进程名");
	}
	CProcessDoc * ppd = (CProcessDoc *)CTaskManager::GetInstance()->GetDocument(RUNTIME_CLASS(CProcessDoc));
	if(!ppd)
		return luaL_error(L,"无法获取 CProcessDoc 对象指针");
	ULONG_PTR ProcessId = 0;
	if(lua_isstring(L,1))
	{
		const char * processname = lua_tostring(L,1);
		CharToWideChar(processname,tmp,512);
		ProcessInfoVector* piv = &ppd->GetProcessInfoVector();
		SIZE_T size = piv->size();
		for(SIZE_T i=0;i<size;i++)
		{
			PProcessInfo ppi = &(*piv)[i];
			GetImageNameByPath(szImageName,ppi->ImagePath);
			if(!_wcsicmp(szImageName,tmp))
			{
				ProcessId = ppi->ProcessId;
				break;
			}
		}
	}
	if(!ProcessId && !lua_isnumber(L,1))
	{
		return luaL_error(L,"参数出错\r\n");
	}
	if(!ProcessId)
		ProcessId = (ULONG_PTR)lua_tonumber(L,1);
	ProcessInfo pi;
	pi.ProcessId = ProcessId;
	ppd->m_pCurrentProcessInfo = &pi;
	CTaskManager::GetInstance()->CreateTaskUI(L"DoThreadsList",CRConsole::DoThreadsList,ppd);
	ThreadInfoVector * ptiv = &ppd->GetThreadInfoVector();
	SIZE_T size = ptiv->size();
	for(SIZE_T i=0;i<size;i++)
	{
		PThreadInfo pti = &(*ptiv)[i];
		if(pti->State==5)
		{
			TransferMsg msg;
			msg.dwMsgId = SUSPENDTHREAD;
			msg.size = 1;
			*PHANDLE(msg.buff) = (HANDLE)pti->UniqueThread;
			if(TransferMessage(msg))
			{
				lua_printex("SuspendThread success");
			}
			else
			{
				lua_printex("SuspendThread failed");
			}		
		}
	}
	return 0;
}

static int lua_DoResumeProcess(lua_State *L)
{
	wchar_t tmp[512];
	WCHAR szImageName[MAX_PATH];
	int n = lua_gettop(L);
	if(n!=1){
		return luaL_error(L,"只接受一个参数 ProcessId 或者 进程名");
	}
	CProcessDoc * ppd = (CProcessDoc *)CTaskManager::GetInstance()->GetDocument(RUNTIME_CLASS(CProcessDoc));
	if(!ppd)
		return luaL_error(L,"无法获取 CProcessDoc 对象指针");
	ULONG_PTR ProcessId = 0;
	if(lua_isstring(L,1))
	{
		const char * processname = lua_tostring(L,1);
		CharToWideChar(processname,tmp,512);
		ProcessInfoVector* piv = &ppd->GetProcessInfoVector();
		SIZE_T size = piv->size();
		for(SIZE_T i=0;i<size;i++)
		{
			PProcessInfo ppi = &(*piv)[i];
			GetImageNameByPath(szImageName,ppi->ImagePath);
			if(!_wcsicmp(szImageName,tmp))
			{
				ProcessId = ppi->ProcessId;
				break;
			}
		}
	}
	if(!ProcessId && !lua_isnumber(L,1))
	{
		return luaL_error(L,"参数出错\r\n");
	}
	if(!ProcessId)
		ProcessId = (ULONG_PTR)lua_tonumber(L,1);
	ProcessInfo pi;
	pi.ProcessId = ProcessId;
	ppd->m_pCurrentProcessInfo = &pi;
	CTaskManager::GetInstance()->CreateTaskUI(L"DoThreadsList",CRConsole::DoThreadsList,ppd);
	ThreadInfoVector * ptiv = &ppd->GetThreadInfoVector();
	SIZE_T size = ptiv->size();
	for(SIZE_T i=0;i<size;i++)
	{
		PThreadInfo pti = &(*ptiv)[i];
		if(pti->State==5)
		{
			TransferMsg msg;
			msg.dwMsgId = RESUMETHREAD;
			msg.size = 1;
			*PHANDLE(msg.buff) = (HANDLE)pti->UniqueThread;
			if(TransferMessage(msg))
			{
				lua_printex("ResumeThread success");
			}
			else
			{
				lua_printex("ResumeThread failed");
			}		
		}
	}
	return 0;
}

static int lua_HideProcessModule(lua_State *L)
{
	wchar_t tmp[512];
	WCHAR szImageName[MAX_PATH];
	int n = lua_gettop(L);
	if(n!=2){
		return luaL_error(L,"只接受二个参数 ProcessId 或者 进程名 和 模块名称");
	}
	CProcessDoc * ppd = (CProcessDoc *)CTaskManager::GetInstance()->GetDocument(RUNTIME_CLASS(CProcessDoc));
	if(!ppd)
		return luaL_error(L,"无法获取 CProcessDoc 对象指针");
	ULONG_PTR ProcessId = 0;
	if(lua_isstring(L,1))
	{
		const char * processname = lua_tostring(L,1);
		if(strcmp(processname,""))
		{
			CharToWideChar(processname,tmp,512);
			ProcessInfoVector* piv = &ppd->GetProcessInfoVector();
			SIZE_T size = piv->size();
			for(SIZE_T i=0;i<size;i++)
			{
				PProcessInfo ppi = &(*piv)[i];
				GetImageNameByPath(szImageName,ppi->ImagePath);
				if(!_wcsicmp(szImageName,tmp))
				{
					ProcessId = ppi->ProcessId;
					break;
				}
			}
		}
	}
	if(lua_isnumber(L,1))
		ProcessId = (ULONG_PTR)lua_tonumber(L,1);
	if(!lua_isstring(L,2))
	{
		return luaL_error(L,"第二参数错误");
	}
	wchar_t wtmp[MAX_PATH];
	const char * modulename = lua_tostring(L,2);
	CharToWideChar(modulename,wtmp,MAX_PATH);
	PModuleInfo ptmi = NULL;
	ModuleInfoVector * pmiv = &ppd->GetModuleInfoVector();
	SIZE_T size = pmiv->size();
	for(SIZE_T i=0;i<size;i++)
	{
		PModuleInfo pmi = &(*pmiv)[i];
		if(!wcscmp(pmi->FullDllName,wtmp))
		{
			ptmi = pmi;
			break;
		}
	}
	if(!ptmi)
	{
		return luaL_error(L,"未找到模块[%s]",modulename);
	}
	TransferMsg msg;
	msg.dwMsgId = HIDEPROCESSMODULE;
	msg.size = sizeof(ModuleInfo);
	memcpy(msg.buff,ptmi,sizeof(ModuleInfo));
	if(TransferMessage(msg))
	{
		lua_printex("HideProcessModule success");
	}
	else
	{
		lua_printex("HideProcessModule failed");
	}
	return 0;
}

static int lua_HideProcessMemory(lua_State *L)
{
	int n = lua_gettop(L);
	if(n!=2)
	{
		return luaL_error(L,"需要2个参数");
	}
	if(!lua_isnumber(L,1) && !lua_isnumber(L,2))
	{
		return luaL_error(L,"第一参数为地址，第二个参数为模块大小");
	}
	ULONG_PTR lpBaseAddress = (ULONG_PTR)lua_tonumber(L,1);
	SIZE_T size = (SIZE_T)lua_tonumber(L,2);
	TransferMsg msg;
	msg.dwMsgId = HIDEPROCESSMEMORY;
	msg.size = sizeof(ModuleInfo);
	ModuleInfo mi;
	mi.BaseAddress = lpBaseAddress;
	mi.SizeOfImage = size;
	memcpy(msg.buff,&mi,sizeof(ModuleInfo));
	if(TransferMessage(msg))
	{
		lua_printex("HideProcessMemory [%X][%X] success",mi.BaseAddress,mi.SizeOfImage);
	}
	else
	{
		lua_printex("HideProcessMemory [%X][%X] failed",mi.BaseAddress,mi.SizeOfImage);
	}
	return 0;
}

static int lua_SetKeSpeed(lua_State *L)
{
	int n = lua_gettop(L);
	if(n!=1)
	{
		return luaL_error(L,"需要1个参数");
	}
	if(!lua_isnumber(L,1))
	{
		return luaL_error(L,"第一参数为加速倍数");
	}
	float speed = (float)lua_tonumber(L,1);
	TransferMsg msg;
	msg.dwMsgId = SETKESPEED;
	msg.size = sizeof(ULONG);
	*PULONG(msg.buff) = (ULONG)(speed * 100);
	if(TransferMessage(msg))
	{
		lua_printex("KeSpeed [%fx]",speed);
	}
	return 0;
}

static int lua_SetKeException(lua_State *L)
{
	int n = lua_gettop(L);
	if(n!=1)
	{
		return luaL_error(L,"需要1个参数");
	}
	if(!lua_isnumber(L,1))
	{
		return luaL_error(L,"第一参数为异常基地址");
	}
	ULONG ulExceptionBaseAddr = (ULONG)lua_tonumber(L,1);
	TransferMsg msg;
	msg.dwMsgId = SETKEEXCEPTION;
	msg.size = sizeof(ULONG);
	*PULONG(msg.buff) = ulExceptionBaseAddr;
	if(TransferMessage(msg))
	{
		lua_printex("ExceptionBaseAddr [%X]",ulExceptionBaseAddr);
	}
	return 0;
}

static int lua_Exit(lua_State *L)
{
	lua_printex("Exit");
	PostMessage(AfxGetApp()->GetMainWnd()->m_hWnd,WM_CLOSE,0,0);
	return 0;
}

static int lua_CreateProcess(lua_State *L)
{
	int n = lua_gettop(L);
	if(n!=2)
	{
		return luaL_error(L,"需要2个参数");
	}
	if(!lua_isstring(L,1) || !lua_isstring(L,2))
	{
		return luaL_error(L,"第一参数为全路径执行文件，第二个参数为传入参数");
	}
	const char * FilePath = lua_tostring(L,1);
	const char * Param =  lua_tostring(L,2);
	string cmdline = string(FilePath) + string(" ") + string(Param);
	STARTUPINFOA si = { 0 };
	si.cb = sizeof(si);

	PROCESS_INFORMATION pi = { 0 };

	if (CreateProcessA(
		NULL,
		(LPSTR)cmdline.c_str(),
		NULL,
		NULL,
		FALSE,
		0,
		NULL,
		NULL,
		&si,
		&pi) == FALSE) {
		lua_printex("CreateProcess [%s]",cmdline.c_str());
	}
	return 0;
}

static int lua_Statistics(lua_State *L)
{
	int n = lua_gettop(L);
	if(n!=3)
	{
		return luaL_error(L,"需要3个参数");
	}
	if(!lua_isstring(L,1) || !lua_isstring(L,2) || !lua_isnumber(L,3))
	{
		return luaL_error(L,"参数传入不正确");
	}
	const char * GameAccount = lua_tostring(L,1);
	const char * RoleName =  lua_tostring(L,2);
	double money = (double)lua_tonumber(L,3);
	if(CLogin::GetInstance()->Statistics(GameAccount,RoleName,money))
	{
		return 0;
	}
	return luaL_error(L,"网络环境异常");
}

static int lua_AddSeparateProcess(lua_State *L)
{
	wchar_t tmp[512];
	WCHAR szImageName[MAX_PATH];
	int n = lua_gettop(L);
	if(n!=1){
		return luaL_error(L,"只接受一个参数 ProcessId 或者 进程名");
	}
	CProcessDoc * ppd = (CProcessDoc *)CTaskManager::GetInstance()->GetDocument(RUNTIME_CLASS(CProcessDoc));
	if(!ppd)
		return luaL_error(L,"无法获取 CProcessDoc 对象指针");
	ULONG_PTR ProcessId = 0;
	if(lua_isstring(L,1))
	{
		const char * processname = lua_tostring(L,1);
		if(strcmp(processname,""))
		{
			CharToWideChar(processname,tmp,512);
			ProcessInfoVector* piv = &ppd->GetProcessInfoVector();
			SIZE_T size = piv->size();
			for(SIZE_T i=0;i<size;i++)
			{
				PProcessInfo ppi = &(*piv)[i];
				GetImageNameByPath(szImageName,ppi->ImagePath);
				if(!_wcsicmp(szImageName,tmp))
				{
					ProcessId = ppi->ProcessId;
					break;
				}
			}
		}
	}
	if(lua_isnumber(L,1))
		ProcessId = (ULONG_PTR)lua_tonumber(L,1);
	TransferMsg msg;
	msg.dwMsgId = ADDSEPARATEPROCESS;
	msg.size = 1;
	*PHANDLE(msg.buff) = (HANDLE)ProcessId;
	if(TransferMessage(msg))
	{
		lua_printex("AddSeparateProcess success");
	}
	else
	{
		lua_printex("AddSeparateProcess failed");
	}
	return 0;
}

static int lua_Simulate(lua_State *L)
{
	int nError = 0;
	int n = lua_gettop(L);
	if (n==0)
	{
		return luaL_error(L,"至少需要一个模拟类型参数");
	}
	if(!lua_isnumber(L,1))
		return luaL_error(L,"模拟类型参数必须为数字");

	CSimulate* m_simulate = new CSimulate();

	DWORD dwSimulateEvent = (DWORD)lua_tonumber(L,1);
	switch (dwSimulateEvent)
	{
	case MOUSEMOVE:
		{
			if (n!=3)
			{
				nError = luaL_error(L, "必须传入鼠标移动的x和y值");
				goto SIMULATEEND;
			}
			if (!lua_isnumber(L,2) || !lua_isnumber(L,3))
			{
				nError = luaL_error(L, "鼠标移动的x和y值必须是数字");
				goto SIMULATEEND;
			}
			int x = (int)lua_tonumber(L,2);
			int y = (int)lua_tonumber(L,3);
			m_simulate->MouseMove(x, y);
		}
		break;
	case MOUSELEFTCLICK:
		{
			m_simulate->MouseLeftClick();
		}
		break;
	case MOUSERIGHTCLICK:
		{
			m_simulate->MouseRightClick();
		}
		break;
	case KEYPRESS:
		{
			if (n!=2)
			{
				nError = luaL_error(L, "必须传入模拟按键的值");
				goto SIMULATEEND;
			}
			WORD key = (WORD)lua_tonumber(L,2);
			m_simulate->KeyPress(key);
		}
		break;
	case KEYRELEASE:
		{
			if (n!=2)
			{
				nError = luaL_error(L, "必须传入模拟按键的值");
				goto SIMULATEEND;
			}
			WORD key = (WORD)lua_tonumber(L,2);
			m_simulate->KeyRelease(key);
		}
		break;
	case KEYSMESSAGE:
		{
			if (n!=2)
			{
				nError = luaL_error(L, "必须传入打印信息的字符串");
				goto SIMULATEEND;
			}
			wchar_t wcstrTemp[1024];
			const char * keymsg = lua_tostring(L,2);
			CharToWideChar(keymsg,wcstrTemp,1024);
			m_simulate->KeysMessage(wcstrTemp);
		}
		break;
	}


SIMULATEEND:
	delete m_simulate;

	return nError;
}

static int lua_Shutdown(lua_State *L)
{
	TransferMsg msg;
	msg.dwMsgId = SHUTDOWN;
	msg.size = 1;
	if(TransferMessage(msg))
	{
		lua_printex("shutdown success");
	}
	else
	{
		lua_printex("shutdown failed");
	}
	return 0;
}

static int lua_HLogin(lua_State *L)
{
	CLogin::GetInstance()->Hack();
	return 0;
}

static int lua_Binding(lua_State *L)
{
	int n = lua_gettop(L);
	if (n==0)
	{
		return luaL_error(L,"绑定参数出错");
	}
	if(!lua_isboolean(L,1))
		return luaL_error(L,"绑定参数出错");
	BOOL bIsBind = (BOOL)lua_toboolean(L,1);
	if(bIsBind)
	{
		CLogin::GetInstance()->Binding(true);
	}
	else
	{
		CLogin::GetInstance()->Binding(false);
	}
	return 0;
}

static int lua_HideKernelModule(lua_State *L)
{
	WCHAR tmp[MAX_PATH]={0};
	int n = lua_gettop(L);
	if(n!=1){
		return luaL_error(L,"只接受一个参数 ProcessId 或者 进程名");
	}
	if(lua_isstring(L,1))
	{
		const char * modulename = lua_tostring(L,1);
		CharToWideChar(modulename,tmp,MAX_PATH*sizeof(WCHAR));
	}
	TransferMsg msg;
	msg.dwMsgId = HIDEKERNELMODULE;
	msg.size = 1;
	wcscpy_s((PWCHAR)msg.buff,MAX_PATH,tmp);
	if(TransferMessage(msg))
	{
		lua_printex("HideKernelModule success");
	}
	else
	{
		lua_printex("HideKernelModule failed");
	}
	return 0;
}

static int lua_CheckThreadState(lua_State *L)
{
	wchar_t tmp[512];
	WCHAR szImageName[MAX_PATH];
	int n = lua_gettop(L);
	if(n!=3){
		return luaL_error(L,"参数出错");
	}
	CProcessDoc * ppd = (CProcessDoc *)CTaskManager::GetInstance()->GetDocument(RUNTIME_CLASS(CProcessDoc));
	if(!ppd)
		return luaL_error(L,"无法获取 CProcessDoc 对象指针");
	ULONG_PTR ProcessId = 0;
	if(lua_isstring(L,1))
	{
		const char * processname = lua_tostring(L,1);
		CharToWideChar(processname,tmp,512);
		ProcessInfoVector* piv = &ppd->GetProcessInfoVector();
		SIZE_T size = piv->size();
		for(SIZE_T i=0;i<size;i++)
		{
			PProcessInfo ppi = &(*piv)[i];
			GetImageNameByPath(szImageName,ppi->ImagePath);
			if(!_wcsicmp(szImageName,tmp))
			{
				ProcessId = ppi->ProcessId;
				break;
			}
		}
	}
	if(!ProcessId && !lua_isnumber(L,1))
	{
		return luaL_error(L,"参数出错\r\n");
	}
	if(!ProcessId)
		ProcessId = (ULONG_PTR)lua_tonumber(L,1);

	ULONG_PTR entry = 0;
	entry = (ULONG_PTR)lua_tonumber(L,2);
	UCHAR State = (UCHAR)lua_tonumber(L,3);
	ProcessInfo pi;
	pi.ProcessId = ProcessId;
	ppd->m_pCurrentProcessInfo = &pi;
	CTaskManager::GetInstance()->CreateTaskUI(L"DoThreadsList",CRConsole::DoThreadsList,ppd);
	ThreadInfoVector * ptiv = &ppd->GetThreadInfoVector();
	SIZE_T size = ptiv->size();
	for(SIZE_T i=0;i<size;i++)
	{
		PThreadInfo pti = &(*ptiv)[i];
		if(pti->Win32StartAddress==entry)
		{
			if(pti->State == State)
			{
				lua_pushinteger(L,TRUE);
				return 1;
			}
			break;
		}
	}
	lua_pushboolean(L,FALSE);
	return 1;
}

static int lua_ModifyKernelMemory(lua_State *L)
{
	int n = lua_gettop(L);
	if(n!=3)
	{
		return luaL_error(L,"需要3个参数");
	}
	if(!lua_isnumber(L,1))
	{
		return luaL_error(L,"参数1出错");
	}
	if(!lua_istable(L,2))
	{
		return luaL_error(L,"参数2出错");
	}
	if(!lua_isnumber(L,3))
	{
		return luaL_error(L,"参数3出错");
	}
	ULONG_PTR lpBaseAddress = (ULONG_PTR)lua_tonumber(L,1);
	SIZE_T size = (SIZE_T)lua_tonumber(L,3);
	PBYTE buff = (PBYTE)malloc(size);
	if(!buff)
		return luaL_error(L,"函数空间分配出错");
	int tabidx = 2;  // 取 table 索引值 
	int index = 0;
	lua_pushnil(L);  // nil 入栈作为初始 key 
	while( 0 != lua_next(L, tabidx)) 
	{ 
		if(lua_isnumber(L,-1))
		{
			int val = (int)lua_tonumber(L,-1);
			buff[index] = val & 0xFF;
			index++;
		}
		lua_pop(L, 1);
	}
	if(index!=size)
	{
		free(buff);
		return luaL_error(L,"传入参数不符合");
	}
	TransferMsg msg;
	msg.dwMsgId = MODIFYKERNELMEMORY;
	msg.size = size;
	WriteMemoryInfo wmi;
	wmi.ProcessHandle = (HANDLE)1;
	wmi.nSize = size;
	wmi.StartAddress = lpBaseAddress;
	PVOID valbuff = PVOID(msg.buff + sizeof(WriteMemoryInfo));
	memcpy(msg.buff,&wmi,sizeof(WriteMemoryInfo));
	memcpy(valbuff,buff,sizeof(WriteMemoryInfo));
	free(buff);
	if(TransferMessage(msg))
	{
		lua_printex("ModifyKernelMemory success");
	}
	else
	{
		lua_printex("ModifyKernelMemory failed");
	}
	return 0;
}

static int lua_registerex(lua_State *L)
{
	int n = lua_gettop(L);
	if(n!=2 && !lua_isnumber(L,1) && !lua_isstring(L,2))
	{
		return luaL_error(L,"参数错误");
	}
	int index = (int)lua_tonumber(L,1);
	const char * functionname = lua_tostring(L,2);
	if(index<FUNCTION_SIZE)
	{
		lua_pushcfunction(L, (lua_CFunction)g_functionmap[index]);
		lua_setglobal(L, functionname);
	}
	return 0;
}
/*
RegisterEx(0,"print");
RegisterEx(1,"PRINT_ON");
RegisterEx(2,"PRINT_OFF");
RegisterEx(3,"ProcessesList");
RegisterEx(4,"KernelHookCheck");
RegisterEx(5,"ModulesList");
RegisterEx(6,"ThreadsList");
RegisterEx(7,"HookCheckList");
RegisterEx(8,"KernelModulesList");
RegisterEx(9,"SSDTList");
RegisterEx(10,"SSDTShadowList");
RegisterEx(11,"DpcTimerList");
RegisterEx(12,"IoTimerList");
RegisterEx(13,"AntiProtectHook1");
RegisterEx(14,"AntiProtectHook2");
RegisterEx(15,"GetProcAddressByName");
RegisterEx(16,"StopIoTimer");
RegisterEx(17,"KillDpcTimer");
RegisterEx(18,"KernelHookRecover");
RegisterEx(19,"TerminateThread");
RegisterEx(20,"WriteProcessMemory");
RegisterEx(21,"KdDisableDebugger");
RegisterEx(22,"KdEnableDebugger");
RegisterEx(23,"HideKernelDebugger");
RegisterEx(24,"MonitorDriverLoad");
RegisterEx(25,"AddTargetProcess");
RegisterEx(26,"RecoverObject");
RegisterEx(27,"AddProtectProcess");
RegisterEx(28,"AddTargetKernelModule");
RegisterEx(29,"AddFilterDrvIo");
RegisterEx(30,"TerminateProcess");
RegisterEx(31,"HideProcessModule");
RegisterEx(32,"HideProcessMemory");
RegisterEx(33,"SetKeSpeed");
RegisterEx(34,"SetKeException");
RegisterEx(35,"Exit");
RegisterEx(36,"CreateProcess");
RegisterEx(37,"Statistics");
RegisterEx(38,"AddSeparateProcess");
RegisterEx(39,"Simulate");
RegisterEx(40,"SuspendProcess");
RegisterEx(41,"ResumeProcess");
RegisterEx(42,"Shutdown");
RegisterEx(43,"HLogin");
RegisterEx(44,"Binding");
RegisterEx(45,"HideKernelModule");
RegisterEx(46,"ModifyKernelMemory");
*/

char * g_registerex_str = "x37x2xex1ax7x11x17x37x3dx78";

void RegisterFunction(lua_State *L)
{
	string registerex_str = g_registerex_str;
	DecryptString(registerex_str);
	lua_pushcfunction(L, lua_registerex);
	lua_setglobal(L, registerex_str.c_str());
}

bool IsFindRegisterFunction(char * buf)
{
	string registerex_str = g_registerex_str;
	DecryptString(registerex_str);
	string bufstr = buf;
	if(bufstr.find(registerex_str)==-1)
	{
		return false;
	}
	return true;
}