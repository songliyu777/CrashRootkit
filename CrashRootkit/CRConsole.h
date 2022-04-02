#pragma once

typedef struct _ProcAddressInfo
{
	WCHAR dllname[64];		//模块名
	char functionname[64];	//函数名
}ProcAddressInfo,*PProcAddressInfo;

class CRConsole
{
public:
	CRConsole(void);
	~CRConsole(void);
public:
	static DWORD WINAPI DoKernelHookCheck(LPVOID pTask);
	static DWORD WINAPI DoProcessesList(LPVOID pTask);
	static DWORD WINAPI DoModulesList(LPVOID pTask);
	static DWORD WINAPI DoThreadsList(LPVOID pTask);
	static DWORD WINAPI DoHookCheckList(LPVOID pTask);
	static DWORD WINAPI DoKernelModulesList(LPVOID pTask);
	static DWORD WINAPI DoSSDTList(LPVOID pTask);
	static DWORD WINAPI DoSSDTShadowList(LPVOID pTask);
	static DWORD WINAPI DoDpcTimerList(LPVOID pTask);
	static DWORD WINAPI DoIoTimerList(LPVOID pTask);
	static DWORD WINAPI DoTerminateProcess(LPVOID pTask);
	static DWORD WINAPI DoKillDpcTimer(LPVOID pTask);
	static DWORD WINAPI DoStopIoTimer(LPVOID pTask);
	static DWORD WINAPI DoStartIoTimer(LPVOID pTask);
	static DWORD WINAPI DoSSDTRecover(LPVOID pTask);
	static DWORD WINAPI DoSSDTShadowRecover(LPVOID pTask);
	static DWORD WINAPI DoKernelHookRecover(LPVOID pTask);
	static DWORD WINAPI DoTerminateThread(LPVOID pTask);
	static DWORD WINAPI DoHookCheckRecover(LPVOID pTask);


	static DWORD WINAPI GetProcAddressByName(LPVOID pTask);
	static DWORD WINAPI MonitorDriverLoad(LPVOID pTask);
};
