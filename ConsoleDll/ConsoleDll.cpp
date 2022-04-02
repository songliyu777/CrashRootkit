// ConsoleDll.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "ConsoleDll.h"
#include "DRVControl.h"

#include "CRD.h"
#include "Log.h"

DRVControl control;

// 这是导出函数的一个示例。
CONSOLEDLL_API BOOL InitializeCRD()
{
	WCHAR drvname[MAX_PATH] = {0};
	InitDrvPath(drvname,DEVICE_FILE_NAME);
	if(control.LoadDriver(DEVICE_FILE_NAME,drvname))
	{
		PrintDbgString(L"驱动[%s]加载成功 \r\n",drvname);
		PrintLog(L"驱动[%s]加载成功 \r\n",drvname);
	}
	else
	{
		PrintDbgString(L"驱动[%s]加载失败 error[%d] \r\n",drvname, GetLastError());
		PrintLog(L"驱动[%s]加载失败 error[%d] \r\n",drvname, GetLastError());
	}
	if(control.OpenDevice(OPEN_SYMLINK_NAME))
	{
		PrintDbgString(L"服务[%s]打开成功 \r\n",OPEN_SYMLINK_NAME);
		PrintLog(L"服务[%s]打开成功  \r\n",OPEN_SYMLINK_NAME);
	}
	else
	{
		PrintDbgString(L"服务[%s]打开失败 error[%d] \r\n",OPEN_SYMLINK_NAME, GetLastError());
		PrintLog(L"服务[%s]打开失败 error[%d] \r\n",OPEN_SYMLINK_NAME, GetLastError());
		ReleaseCRD();
		return FALSE;
	}	
	return TRUE;
}

CONSOLEDLL_API BOOL OpenCRD()
{
	if(control.OpenDevice(OPEN_SYMLINK_NAME))
	{
		PrintDbgString(L"服务[%s]打开成功 \r\n",OPEN_SYMLINK_NAME);
		PrintLog(L"服务[%s]打开成功  \r\n",OPEN_SYMLINK_NAME);
	}
	else
	{
		PrintDbgString(L"服务[%s]打开失败 error[%d] \r\n",OPEN_SYMLINK_NAME, GetLastError());
		PrintLog(L"服务[%s]打开失败 error[%d] \r\n",OPEN_SYMLINK_NAME, GetLastError());
		return FALSE;
	}	
	return TRUE;
}

CONSOLEDLL_API BOOL ReleaseCRD()
{
	//先释放一下
	TransferMsg msg;
	msg.dwMsgId = FREEALL;
	TransferMessage(msg);
	//再卸载
	if(control.UnloadDriver())
	{
		PrintDbgString(L"驱动[%s]卸载成功 \r\n",DEVICE_FILE_NAME);
		PrintLog(L"驱动[%s]卸载成功 \r\n",DEVICE_FILE_NAME);
		return true;
	}
	else
	{
		PrintDbgString(L"驱动[%s]卸载失败 \r\n",DEVICE_FILE_NAME);
		PrintLog(L"驱动[%s]卸载失败 \r\n",DEVICE_FILE_NAME);
		return false;
	}
	return TRUE;
}

CONSOLEDLL_API VOID InitFilePath(OUT PWCHAR szFilePath,IN PWCHAR FileName)
{
	WCHAR filename[MAX_PATH] = {0};
	WCHAR szFileImagePath[MAX_PATH] = {0};
	wcscpy_s(filename,FileName);
	GetModuleFileName(NULL,szFileImagePath,MAX_PATH); //得到当前模块路径
	for (int i=wcslen(szFileImagePath);i>0;i--)
	{
		if (L'\\'==szFileImagePath[i])
		{
			szFileImagePath[i+1]=L'\0';
			break;
		}
	}
	wcscat(szFileImagePath,filename);
	wcscpy(szFilePath,szFileImagePath);
}

VOID InitDrvPath(OUT PWCHAR szFilePath,IN PWCHAR FileName)
{
	InitFilePath(szFilePath,FileName);
	wcscat(szFilePath,L".sys");
}

CONSOLEDLL_API VOID GetImageNameByPath(OUT PWCHAR szImageName,IN PWCHAR szPath)
{
	for (int i = wcslen(szPath);i > 0;i--)
	{
		if (L'\\'== szPath[i])
		{
			wcscpy_s(szImageName,MAX_PATH,szPath + i + 1);
			return;
		}
	}
	wcscpy_s(szImageName,MAX_PATH,szPath);
}

CONSOLEDLL_API BOOL SetLogPath()
{
	WCHAR path[MAX_PATH] = {0};
	WCHAR ImagePath[MAX_PATH] = {0};
	InitFilePath(ImagePath,L"YYFDLog.txt");
	wcscpy(path,L"\\\?\?\\");
	wcscat(path,ImagePath);
	DWORD dwRet,outCount = (wcslen(path)+1)*sizeof(WCHAR);
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_SETLOGPATH,path,outCount,path,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"SetLogPath成功 \r\n");
		PrintLog(L"SetLogPath成功 \r\n");
		return TRUE;
	}
	PrintDbgString(L"SetLogPath失败 \r\n");
	PrintLog(L"SetLogPath失败 \r\n");
	return FALSE;		
}

CONSOLEDLL_API BOOL TransferMessage(TransferMsg & msg)
{
	DWORD dwRet,outCount = sizeof(TransferMsg);
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_TRANSFER,&msg,sizeof(TransferMsg),&msg,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"发送消息到驱动中成功 \r\n");
		return TRUE;
	}
	PrintDbgString(L"发送消息到驱动中失败 \r\n");
	return FALSE;		
}

CONSOLEDLL_API BOOL KernelHookCheck(OUT DWORD & size)
{
	DWORD dwRet,outCount = 0;
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_KERNELHOOK_CHECK,NULL,0,NULL,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"KernelHookCheck成功 \r\n");
		PrintLog(L"KernelHookCheck成功 \r\n");
		size = outCount;
		return TRUE;
	}
	PrintDbgString(L"KernelHookCheck失败 \r\n");
	PrintLog(L"KernelHookCheck失败 \r\n");
	return FALSE;		
}

CONSOLEDLL_API BOOL KernelHookGet(IN OUT PVOID buff,IN DWORD size)
{
	DWORD dwRet,outCount = size;
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_KERNELHOOK_GET,buff,size,buff,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"KernelHookGet成功 \r\n");
		PrintLog(L"KernelHookGet成功 \r\n");
		return TRUE;
	}
	PrintDbgString(L"KernelHookGet失败 \r\n");
	PrintLog(L"KernelHookGet失败 \r\n");
	return FALSE;		
}

CONSOLEDLL_API BOOL ProcessesList(OUT DWORD & size)
{
	DWORD dwRet,outCount = 0;
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_PROCESSES_LIST,NULL,0,NULL,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"ProcessesList成功 \r\n");
		PrintLog(L"ProcessesList成功 \r\n");
		size = outCount;
		return TRUE;
	}
	PrintDbgString(L"ProcessesList失败 \r\n");
	PrintLog(L"ProcessesList失败 \r\n");
	return FALSE;	
}

CONSOLEDLL_API BOOL ProcessesGet(PVOID buff, DWORD size)
{
	DWORD dwRet,outCount = size;
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_PROCESSES_GET,buff,size,buff,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"ProcessesGet成功 \r\n");
		PrintLog(L"ProcessesGet成功 \r\n");
		size = outCount;
		return TRUE;
	}
	PrintDbgString(L"ProcessesGet失败 \r\n");
	PrintLog(L"ProcessesGet失败 \r\n");
	return FALSE;	
}

CONSOLEDLL_API BOOL ModulesList(IN ULONG_PTR pid, OUT DWORD & size)
{
	DWORD dwRet,outCount = 0;
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_MODULES_LIST,&pid,sizeof(ULONG_PTR),NULL,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"ModulesList成功 \r\n");
		PrintLog(L"ModulesList成功 \r\n");
		size = outCount;
		return TRUE;
	}
	PrintDbgString(L"ModulesList失败 \r\n");
	PrintLog(L"ModulesList失败 \r\n");
	return FALSE;	
}

CONSOLEDLL_API BOOL ModulesGet(PVOID buff, DWORD size)
{
	DWORD dwRet,outCount = size;
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_MODULES_GET,buff,size,buff,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"ModulesGet成功 \r\n");
		PrintLog(L"ModulesGet成功 \r\n");
		size = outCount;
		return TRUE;
	}
	PrintDbgString(L"ModulesGet失败 \r\n");
	PrintLog(L"ModulesGet失败 \r\n");
	return FALSE;	
}

CONSOLEDLL_API BOOL ThreadsList(IN ULONG_PTR pid, OUT DWORD & size)
{
	DWORD dwRet,outCount = 0;
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_THREADS_LIST,&pid,sizeof(ULONG_PTR),NULL,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"ThreadsList成功 \r\n");
		PrintLog(L"ThreadsList成功 \r\n");
		size = outCount;
		return TRUE;
	}
	PrintDbgString(L"ThreadsList失败 \r\n");
	PrintLog(L"ThreadsList失败 \r\n");
	return FALSE;	
}

CONSOLEDLL_API BOOL ThreadsGet(PVOID buff, DWORD size)
{
	DWORD dwRet,outCount = size;
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_THREADS_GET,buff,size,buff,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"ThreadsGet成功 \r\n");
		PrintLog(L"ThreadsGet成功 \r\n");
		size = outCount;
		return TRUE;
	}
	PrintDbgString(L"ThreadsGet失败 \r\n");
	PrintLog(L"ThreadsGet失败 \r\n");
	return FALSE;	
}

CONSOLEDLL_API BOOL ModulePeInfoGet(ModuleInfo Moduleinfo,OUT PVOID outputbuff,IN OUT DWORD & size)
{
	DWORD dwRet,outCount = Moduleinfo.SizeOfImage;
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_MODULE_PE_INFO_GET,&Moduleinfo,size,outputbuff,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"ModulePeInfoGet成功 \r\n");
		PrintLog(L"ModulePeInfoGet成功 \r\n");
		size = outCount;
		return TRUE;
	}
	PrintDbgString(L"ModulePeInfoGet失败 \r\n");
	PrintLog(L"ModulePeInfoGet失败 \r\n");
	return FALSE;	
}

CONSOLEDLL_API BOOL CR4Get(ULONG & cr4)
{
	DWORD dwRet,outCount = sizeof(ULONG);
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_CR4_GET,&cr4,sizeof(ULONG),&cr4,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"CR4Get成功 \r\n");
		PrintLog(L"CR4Get成功 \r\n");
		return TRUE;
	}
	PrintDbgString(L"CR4Get失败 \r\n");
	PrintLog(L"CR4Get失败 \r\n");
	return FALSE;	
}

CONSOLEDLL_API HANDLE OpenProcessEx(__in DWORD dwProcessId)
{
	DWORD dwRet,outCount = sizeof(HANDLE);
	HANDLE hProcess = NULL;
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_OPENPROCESS,&dwProcessId,sizeof(ULONG),&hProcess,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"OpenProcessEx成功 \r\n");
		PrintLog(L"OpenProcessEx成功 \r\n");
		return hProcess;
	}
	return NULL;	
}

CONSOLEDLL_API BOOL ReadProcessMemoryEx(__in      HANDLE hProcess,
										__in      LPCVOID lpBaseAddress,
										__out_bcount_part(nSize, *lpNumberOfBytesRead) LPVOID lpBuffer,
										__in      SIZE_T nSize,
										__out_opt SIZE_T * lpNumberOfBytesRead)
{
	DWORD dwRet,outCount = nSize;
	ReadMemoryInfo rmi;
	rmi.ProcessHandle = hProcess;
	rmi.StartAddress = (ULONG_PTR)lpBaseAddress;
	rmi.nSize = nSize;
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_READPROCESSMEMORY,&rmi,sizeof(ReadMemoryInfo),lpBuffer,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		*lpNumberOfBytesRead = outCount;
		if(nSize==*lpNumberOfBytesRead)
		{
			PrintDbgString(L"ReadProcessMemoryEx成功 \r\n");
			PrintLog(L"ReadProcessMemoryEx成功 \r\n");
			return TRUE;
		}
	}
	return FALSE;	
}

CONSOLEDLL_API BOOL WriteProcessMemoryEx(__in      HANDLE hProcess,
										 __in      LPVOID lpBaseAddress,
										 __in_bcount(nSize) LPCVOID lpBuffer,
										 __in      SIZE_T nSize,
										 __out_opt SIZE_T * lpNumberOfBytesWritten)
{
	DWORD dwRet,outCount = nSize;
	WriteMemoryInfo wmi;
	wmi.ProcessHandle = hProcess;
	wmi.StartAddress = (ULONG_PTR)lpBaseAddress;
	wmi.nSize = nSize;
	PVOID buff = malloc(nSize + sizeof(WriteMemoryInfo));
	if(!buff)
		return FALSE;
	memcpy(buff,&wmi,sizeof(WriteMemoryInfo));
	memcpy((char*)buff + sizeof(WriteMemoryInfo),lpBuffer,nSize);
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_WRITEPROCESSMEMORY,buff,nSize + sizeof(WriteMemoryInfo),buff,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		*lpNumberOfBytesWritten = outCount;
		if(nSize==*lpNumberOfBytesWritten)
		{
			PrintDbgString(L"WriteProcessMemoryEx成功 \r\n");
			PrintLog(L"WriteProcessMemoryEx成功 \r\n");
			free(buff);
			return TRUE;
		}
	}
	free(buff);
	return FALSE;	
}

CONSOLEDLL_API UINT SendInputEx(UINT nInputs, LPINPUT pInputs, int cbSize)
{
	DWORD dwRet, outCount = 0;
	TransferSendInput tsi;
	tsi.nInputs = nInputs;
	tsi.pInputs = pInputs;
	tsi.cbSize = cbSize;
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_SENDINPUT, &tsi, sizeof(TransferSendInput), NULL, outCount);
	if (dwRet != DRV_ERROR_IO)
	{
		return outCount;
	}
	return outCount;
}

CONSOLEDLL_API HWND FindWindowExtend(PWCHAR ClassName,PWCHAR WindowName)
{
	DWORD dwRet, outCount = 4,size = sizeof(TransferFindWindowEx);
	HWND hwnd = NULL;
	RTLINITUNICODESTRING RtlInitUnicodeString = (RTLINITUNICODESTRING)GetProcAddress(GetModuleHandle(L"ntdll"),"RtlInitUnicodeString");
	UNICODE_STRING strClassName,strWindowName;
	RtlInitUnicodeString(&strClassName,ClassName);
	RtlInitUnicodeString(&strWindowName,WindowName);
	TransferFindWindowEx tfw;
	ZeroMemory(&tfw,size);
	tfw.strClassName = (ULONG_PTR)&strClassName;
	tfw.strWindowName = (ULONG_PTR)&strWindowName;
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_FINDWINDOWEX, &tfw, size, &hwnd, outCount);
	if (dwRet != DRV_ERROR_IO)
	{
		return hwnd;
	}
	return hwnd;
}

CONSOLEDLL_API BOOL KernelModulesList(OUT DWORD & size)
{
	DWORD dwRet,outCount = 0;
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_KERNEL_MODULES_LIST,NULL,0,NULL,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"KernelModulesList成功 \r\n");
		PrintLog(L"KernelModulesList成功 \r\n");
		size = outCount;
		return TRUE;
	}
	PrintDbgString(L"KernelModulesList失败 \r\n");
	PrintLog(L"KernelModulesList失败 \r\n");
	return FALSE;
}

CONSOLEDLL_API BOOL KernelModulesGet(PVOID buff, DWORD size)
{
	DWORD dwRet,outCount = size;
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_KERNEL_MODULES_GET,buff,size,buff,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"KernelModulesGet成功 \r\n");
		PrintLog(L"KernelModulesGet成功 \r\n");
		size = outCount;
		return TRUE;
	}
	PrintDbgString(L"KernelModulesGet失败 \r\n");
	PrintLog(L"KernelModulesGet失败 \r\n");
	return FALSE;	
}

CONSOLEDLL_API BOOL SSDTList(OUT DWORD & size)
{
	DWORD dwRet,outCount = 0;
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_KERNEL_SSDT_LIST,NULL,size,NULL,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"SSDTList成功 \r\n");
		PrintLog(L"SSDTList成功 \r\n");
		size = outCount;
		return TRUE;
	}
	PrintDbgString(L"SSDTList失败 \r\n");
	PrintLog(L"SSDTList失败 \r\n");
	return FALSE;	
}

CONSOLEDLL_API BOOL SSDTGet(PVOID buff, DWORD size)
{
	DWORD dwRet,outCount = size;
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_KERNEL_SSDT_GET,buff,size,buff,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"SSDTGet成功 \r\n");
		PrintLog(L"SSDTGet成功 \r\n");
		size = outCount;
		return TRUE;
	}
	PrintDbgString(L"SSDTGet失败 \r\n");
	PrintLog(L"SSDTGet失败 \r\n");
	return FALSE;	
}

CONSOLEDLL_API BOOL SSDTShadowList(OUT DWORD & size)
{
	DWORD dwRet,outCount = 0;
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_KERNEL_SSDTSHADOW_LIST,NULL,size,NULL,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"SSDTShadowList成功 \r\n");
		PrintLog(L"SSDTShadowList成功 \r\n");
		size = outCount;
		return TRUE;
	}
	PrintDbgString(L"SSDTShadowList失败 \r\n");
	PrintLog(L"SSDTShadowList失败 \r\n");
	return FALSE;	
}

CONSOLEDLL_API BOOL SSDTShadowGet(PVOID buff, DWORD size)
{
	DWORD dwRet,outCount = size;
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_KERNEL_SSDTSHADOW_GET,buff,size,buff,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"SSDTShadowGet成功 \r\n");
		PrintLog(L"SSDTShadowGet成功 \r\n");
		size = outCount;
		return TRUE;
	}
	PrintDbgString(L"SSDTShadowGet失败 \r\n");
	PrintLog(L"SSDTShadowGet失败 \r\n");
	return FALSE;	
}

CONSOLEDLL_API BOOL DpcTimerList(OUT DWORD & size)
{
	DWORD dwRet,outCount = size;
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_KERNEL_DPCTIMER_LIST,NULL,size,NULL,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"DpcTimerList成功 \r\n");
		PrintLog(L"DpcTimerList成功 \r\n");
		size = outCount;
		return TRUE;
	}
	PrintDbgString(L"DpcTimerList失败 \r\n");
	PrintLog(L"DpcTimerList失败 \r\n");
	return FALSE;	
}

CONSOLEDLL_API BOOL DpcTimerGet(PVOID buff, DWORD size)
{
	DWORD dwRet,outCount = size;
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_KERNEL_DPCTIMER_GET,buff,size,buff,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"DpcTimerGet成功 \r\n");
		PrintLog(L"DpcTimerGet成功 \r\n");
		size = outCount;
		return TRUE;
	}
	PrintDbgString(L"DpcTimerGet失败 \r\n");
	PrintLog(L"DpcTimerGet失败 \r\n");
	return FALSE;
}

CONSOLEDLL_API BOOL IoTimerList(OUT DWORD & size)
{
	DWORD dwRet,outCount = size;
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_KERNEL_IOTIMER_LIST,NULL,size,NULL,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"IoTimerList成功 \r\n");
		PrintLog(L"IoTimerList成功 \r\n");
		size = outCount;
		return TRUE;
	}
	PrintDbgString(L"IoTimerList失败 \r\n");
	PrintLog(L"IoTimerList失败 \r\n");
	return FALSE;	
}

CONSOLEDLL_API BOOL IoTimerGet(PVOID buff, DWORD size)
{
	DWORD dwRet,outCount = size;
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_KERNEL_IOTIMER_GET,buff,size,buff,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"IoTimerGet成功 \r\n");
		PrintLog(L"IoTimerGet成功 \r\n");
		size = outCount;
		return TRUE;
	}
	PrintDbgString(L"IoTimerGet失败 \r\n");
	PrintLog(L"IoTimerGet失败 \r\n");
	return FALSE;
}

CONSOLEDLL_API BOOL SetSynchroniza(PVOID buff, DWORD size)
{
	DWORD dwRet,outCount = size;
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_SYNCHRONIZA,buff,size,buff,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"SetSynchroniza成功 \r\n");
		PrintLog(L"SetSynchroniza成功 \r\n");
		return TRUE;
	}
	PrintDbgString(L"SetSynchroniza失败 \r\n");
	PrintLog(L"SetSynchroniza失败 \r\n");
	return FALSE;
}

CONSOLEDLL_API BOOL GetSynchronizaInfo(PVOID buff, DWORD size)
{
	DWORD dwRet,outCount = size;
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_GETSYNCHRONIZEINFO,buff,size,buff,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"GetSynchronizaInfo成功 \r\n");
		PrintLog(L"GetSynchronizaInfo成功 \r\n");
		return TRUE;
	}
	PrintDbgString(L"GetSynchronizaInfo失败 \r\n");
	PrintLog(L"GetSynchronizaInfo失败 \r\n");
	return FALSE;
}

CONSOLEDLL_API PVOID GetInfoTable(SYSTEM_INFORMATION_CLASS SystemInformationClass)
{
	ZWQUERYSYSTEMINFORMATION fnZwQuerySystemInformation = (ZWQUERYSYSTEMINFORMATION)GetProcAddress(GetModuleHandleA("ntdll.dll"),"ZwQuerySystemInformation");
	if(fnZwQuerySystemInformation)
	{
		ULONG mSize = 0x4000;
		PVOID mPtr = NULL;
		LONG St;
		do
		{
			mPtr = malloc(mSize);
			memset(mPtr, 0, mSize);
			if (mPtr)
			{
				St = fnZwQuerySystemInformation(SystemInformationClass, mPtr, mSize, NULL);
			} else return NULL;
			if (St == STATUS_INFO_LENGTH_MISMATCH)
			{
				free(mPtr);
				mSize = mSize * 2;
			}
		} while (St == STATUS_INFO_LENGTH_MISMATCH);
		if (St == STATUS_SUCCESS) return mPtr;
		free(mPtr);
	}
	return NULL;
}

CONSOLEDLL_API VOID FreeInfoTable(PVOID Ptr)
{
	free(Ptr);
}

CONSOLEDLL_API BOOL GetPortVal(WORD wPortAddr, char * pdwPortVal, BYTE bSize)
{
	BOOL success = FALSE;
	TransferMsg msg;
	msg.dwMsgId = GETPORT;
	tagPort32Struct ps,*pps;
	ps.wPortAddr = wPortAddr;
	ps.bSize = bSize;
	memcpy(msg.buff,&ps,sizeof(tagPort32Struct));
	if(TransferMessage(msg))
	{
		success = TRUE;
		pps = (tagPort32Struct*)msg.buff;
		*pdwPortVal = pps->dwPortVal;
	}
	return success;
}

CONSOLEDLL_API BOOL SetPortVal(WORD wPortAddr, char dwPortVal, BYTE bSize)
{
	TransferMsg msg;
	msg.dwMsgId = SETPORT;
	tagPort32Struct ps;
	ps.wPortAddr = wPortAddr;
	ps.dwPortVal = dwPortVal;
	ps.bSize = bSize;
	memcpy(msg.buff,&ps,sizeof(tagPort32Struct));
	return TransferMessage(msg);
}

#define   KBC_KEY_CMD   0x64         //键盘命令端口 
#define   KBC_KEY_DATA   0x60       //键盘数据端口 

void KBCWait4IBE() 
{ 
	char   dwRegVal=0; 
	do 
	{   
		GetPortVal(0x64,&dwRegVal,1); 
	} 
	while(dwRegVal   &   0x00000001); 
} 


CONSOLEDLL_API void KeyDownEx(long vKeyCoad)       //模拟扩展键按下，参数vKeyCoad是扩展键的虚拟码 
{ 
	BYTE byScancode = (BYTE)(MapVirtualKey(vKeyCoad,0) & 0xFF);

	KBCWait4IBE();       //等待键盘缓冲区为空 
	SetPortVal(KBC_KEY_CMD, (BYTE)(0xD2 & 0xFF), 1);       //发送键盘写入命令 
	KBCWait4IBE(); 
	SetPortVal(KBC_KEY_DATA,(BYTE)(0xE0 & 0xFF), 1);   //写入扩展键标志信息 


	KBCWait4IBE();       //等待键盘缓冲区为空 
	SetPortVal(KBC_KEY_CMD,(BYTE)(0xD2 & 0xFF), 1);         //发送键盘写入命令 
	KBCWait4IBE(); 
	SetPortVal(KBC_KEY_DATA,byScancode, 1);   //写入按键信息,按下键 
} 


CONSOLEDLL_API void KeyUpEx(long vKeyCoad)       //模拟扩展键弹起 
{ 
	BYTE byScancode = (BYTE)MapVirtualKey(vKeyCoad,0) & 0xFF;   

	KBCWait4IBE();       //等待键盘缓冲区为空 
	SetPortVal(KBC_KEY_CMD,(BYTE)(0xD2 & 0xFF),1);         //发送键盘写入命令 
	KBCWait4IBE(); 
	SetPortVal   (KBC_KEY_DATA,(BYTE)(0xE0 & 0xFF),1);   //写入扩展键标志信息 

	KBCWait4IBE();     //等待键盘缓冲区为空 
	SetPortVal(KBC_KEY_CMD,(BYTE)(0xD2 & 0xFF),   1   );         //发送键盘写入命令 
	KBCWait4IBE(); 
	SetPortVal(KBC_KEY_DATA,(byScancode|0x80),   1);     //写入按键信息，释放键 
} 

CONSOLEDLL_API void KeyDown(long vKeyCoad) 
{ 
	BYTE byScancode =(BYTE)MapVirtualKey(vKeyCoad,0) & 0xFF;    
	KBCWait4IBE();  
	SetPortVal(KBC_KEY_CMD,(BYTE)(0xD2 & 0xFF),1);
	KBCWait4IBE();
	SetPortVal(KBC_KEY_DATA,byScancode,1);
} 


CONSOLEDLL_API void KeyUp(long   vKeyCoad) 
{ 
	BYTE byScancode=(BYTE)MapVirtualKey(vKeyCoad,0) & 0xFF;   
	KBCWait4IBE(); 
	SetPortVal(KBC_KEY_CMD,(BYTE)(0xD2 & 0xFF),1);
	KBCWait4IBE();
	SetPortVal(KBC_KEY_DATA,byScancode | 0x80,1);
} 
