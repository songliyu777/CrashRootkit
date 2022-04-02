// ConsoleDll.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "ConsoleDll.h"
#include "DRVControl.h"

#include "CRD.h"
#include "Log.h"

DRVControl control;

// ���ǵ���������һ��ʾ����
CONSOLEDLL_API BOOL InitializeCRD()
{
	WCHAR drvname[MAX_PATH] = {0};
	InitDrvPath(drvname,DEVICE_FILE_NAME);
	if(control.LoadDriver(DEVICE_FILE_NAME,drvname))
	{
		PrintDbgString(L"����[%s]���سɹ� \r\n",drvname);
		PrintLog(L"����[%s]���سɹ� \r\n",drvname);
	}
	else
	{
		PrintDbgString(L"����[%s]����ʧ�� error[%d] \r\n",drvname, GetLastError());
		PrintLog(L"����[%s]����ʧ�� error[%d] \r\n",drvname, GetLastError());
	}
	if(control.OpenDevice(OPEN_SYMLINK_NAME))
	{
		PrintDbgString(L"����[%s]�򿪳ɹ� \r\n",OPEN_SYMLINK_NAME);
		PrintLog(L"����[%s]�򿪳ɹ�  \r\n",OPEN_SYMLINK_NAME);
	}
	else
	{
		PrintDbgString(L"����[%s]��ʧ�� error[%d] \r\n",OPEN_SYMLINK_NAME, GetLastError());
		PrintLog(L"����[%s]��ʧ�� error[%d] \r\n",OPEN_SYMLINK_NAME, GetLastError());
		ReleaseCRD();
		return FALSE;
	}	
	return TRUE;
}

CONSOLEDLL_API BOOL OpenCRD()
{
	if(control.OpenDevice(OPEN_SYMLINK_NAME))
	{
		PrintDbgString(L"����[%s]�򿪳ɹ� \r\n",OPEN_SYMLINK_NAME);
		PrintLog(L"����[%s]�򿪳ɹ�  \r\n",OPEN_SYMLINK_NAME);
	}
	else
	{
		PrintDbgString(L"����[%s]��ʧ�� error[%d] \r\n",OPEN_SYMLINK_NAME, GetLastError());
		PrintLog(L"����[%s]��ʧ�� error[%d] \r\n",OPEN_SYMLINK_NAME, GetLastError());
		return FALSE;
	}	
	return TRUE;
}

CONSOLEDLL_API BOOL ReleaseCRD()
{
	//���ͷ�һ��
	TransferMsg msg;
	msg.dwMsgId = FREEALL;
	TransferMessage(msg);
	//��ж��
	if(control.UnloadDriver())
	{
		PrintDbgString(L"����[%s]ж�سɹ� \r\n",DEVICE_FILE_NAME);
		PrintLog(L"����[%s]ж�سɹ� \r\n",DEVICE_FILE_NAME);
		return true;
	}
	else
	{
		PrintDbgString(L"����[%s]ж��ʧ�� \r\n",DEVICE_FILE_NAME);
		PrintLog(L"����[%s]ж��ʧ�� \r\n",DEVICE_FILE_NAME);
		return false;
	}
	return TRUE;
}

CONSOLEDLL_API VOID InitFilePath(OUT PWCHAR szFilePath,IN PWCHAR FileName)
{
	WCHAR filename[MAX_PATH] = {0};
	WCHAR szFileImagePath[MAX_PATH] = {0};
	wcscpy_s(filename,FileName);
	GetModuleFileName(NULL,szFileImagePath,MAX_PATH); //�õ���ǰģ��·��
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
		PrintDbgString(L"SetLogPath�ɹ� \r\n");
		PrintLog(L"SetLogPath�ɹ� \r\n");
		return TRUE;
	}
	PrintDbgString(L"SetLogPathʧ�� \r\n");
	PrintLog(L"SetLogPathʧ�� \r\n");
	return FALSE;		
}

CONSOLEDLL_API BOOL TransferMessage(TransferMsg & msg)
{
	DWORD dwRet,outCount = sizeof(TransferMsg);
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_TRANSFER,&msg,sizeof(TransferMsg),&msg,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"������Ϣ�������гɹ� \r\n");
		return TRUE;
	}
	PrintDbgString(L"������Ϣ��������ʧ�� \r\n");
	return FALSE;		
}

CONSOLEDLL_API BOOL KernelHookCheck(OUT DWORD & size)
{
	DWORD dwRet,outCount = 0;
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_KERNELHOOK_CHECK,NULL,0,NULL,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"KernelHookCheck�ɹ� \r\n");
		PrintLog(L"KernelHookCheck�ɹ� \r\n");
		size = outCount;
		return TRUE;
	}
	PrintDbgString(L"KernelHookCheckʧ�� \r\n");
	PrintLog(L"KernelHookCheckʧ�� \r\n");
	return FALSE;		
}

CONSOLEDLL_API BOOL KernelHookGet(IN OUT PVOID buff,IN DWORD size)
{
	DWORD dwRet,outCount = size;
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_KERNELHOOK_GET,buff,size,buff,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"KernelHookGet�ɹ� \r\n");
		PrintLog(L"KernelHookGet�ɹ� \r\n");
		return TRUE;
	}
	PrintDbgString(L"KernelHookGetʧ�� \r\n");
	PrintLog(L"KernelHookGetʧ�� \r\n");
	return FALSE;		
}

CONSOLEDLL_API BOOL ProcessesList(OUT DWORD & size)
{
	DWORD dwRet,outCount = 0;
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_PROCESSES_LIST,NULL,0,NULL,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"ProcessesList�ɹ� \r\n");
		PrintLog(L"ProcessesList�ɹ� \r\n");
		size = outCount;
		return TRUE;
	}
	PrintDbgString(L"ProcessesListʧ�� \r\n");
	PrintLog(L"ProcessesListʧ�� \r\n");
	return FALSE;	
}

CONSOLEDLL_API BOOL ProcessesGet(PVOID buff, DWORD size)
{
	DWORD dwRet,outCount = size;
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_PROCESSES_GET,buff,size,buff,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"ProcessesGet�ɹ� \r\n");
		PrintLog(L"ProcessesGet�ɹ� \r\n");
		size = outCount;
		return TRUE;
	}
	PrintDbgString(L"ProcessesGetʧ�� \r\n");
	PrintLog(L"ProcessesGetʧ�� \r\n");
	return FALSE;	
}

CONSOLEDLL_API BOOL ModulesList(IN ULONG_PTR pid, OUT DWORD & size)
{
	DWORD dwRet,outCount = 0;
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_MODULES_LIST,&pid,sizeof(ULONG_PTR),NULL,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"ModulesList�ɹ� \r\n");
		PrintLog(L"ModulesList�ɹ� \r\n");
		size = outCount;
		return TRUE;
	}
	PrintDbgString(L"ModulesListʧ�� \r\n");
	PrintLog(L"ModulesListʧ�� \r\n");
	return FALSE;	
}

CONSOLEDLL_API BOOL ModulesGet(PVOID buff, DWORD size)
{
	DWORD dwRet,outCount = size;
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_MODULES_GET,buff,size,buff,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"ModulesGet�ɹ� \r\n");
		PrintLog(L"ModulesGet�ɹ� \r\n");
		size = outCount;
		return TRUE;
	}
	PrintDbgString(L"ModulesGetʧ�� \r\n");
	PrintLog(L"ModulesGetʧ�� \r\n");
	return FALSE;	
}

CONSOLEDLL_API BOOL ThreadsList(IN ULONG_PTR pid, OUT DWORD & size)
{
	DWORD dwRet,outCount = 0;
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_THREADS_LIST,&pid,sizeof(ULONG_PTR),NULL,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"ThreadsList�ɹ� \r\n");
		PrintLog(L"ThreadsList�ɹ� \r\n");
		size = outCount;
		return TRUE;
	}
	PrintDbgString(L"ThreadsListʧ�� \r\n");
	PrintLog(L"ThreadsListʧ�� \r\n");
	return FALSE;	
}

CONSOLEDLL_API BOOL ThreadsGet(PVOID buff, DWORD size)
{
	DWORD dwRet,outCount = size;
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_THREADS_GET,buff,size,buff,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"ThreadsGet�ɹ� \r\n");
		PrintLog(L"ThreadsGet�ɹ� \r\n");
		size = outCount;
		return TRUE;
	}
	PrintDbgString(L"ThreadsGetʧ�� \r\n");
	PrintLog(L"ThreadsGetʧ�� \r\n");
	return FALSE;	
}

CONSOLEDLL_API BOOL ModulePeInfoGet(ModuleInfo Moduleinfo,OUT PVOID outputbuff,IN OUT DWORD & size)
{
	DWORD dwRet,outCount = Moduleinfo.SizeOfImage;
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_MODULE_PE_INFO_GET,&Moduleinfo,size,outputbuff,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"ModulePeInfoGet�ɹ� \r\n");
		PrintLog(L"ModulePeInfoGet�ɹ� \r\n");
		size = outCount;
		return TRUE;
	}
	PrintDbgString(L"ModulePeInfoGetʧ�� \r\n");
	PrintLog(L"ModulePeInfoGetʧ�� \r\n");
	return FALSE;	
}

CONSOLEDLL_API BOOL CR4Get(ULONG & cr4)
{
	DWORD dwRet,outCount = sizeof(ULONG);
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_CR4_GET,&cr4,sizeof(ULONG),&cr4,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"CR4Get�ɹ� \r\n");
		PrintLog(L"CR4Get�ɹ� \r\n");
		return TRUE;
	}
	PrintDbgString(L"CR4Getʧ�� \r\n");
	PrintLog(L"CR4Getʧ�� \r\n");
	return FALSE;	
}

CONSOLEDLL_API HANDLE OpenProcessEx(__in DWORD dwProcessId)
{
	DWORD dwRet,outCount = sizeof(HANDLE);
	HANDLE hProcess = NULL;
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_OPENPROCESS,&dwProcessId,sizeof(ULONG),&hProcess,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"OpenProcessEx�ɹ� \r\n");
		PrintLog(L"OpenProcessEx�ɹ� \r\n");
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
			PrintDbgString(L"ReadProcessMemoryEx�ɹ� \r\n");
			PrintLog(L"ReadProcessMemoryEx�ɹ� \r\n");
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
			PrintDbgString(L"WriteProcessMemoryEx�ɹ� \r\n");
			PrintLog(L"WriteProcessMemoryEx�ɹ� \r\n");
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
		PrintDbgString(L"KernelModulesList�ɹ� \r\n");
		PrintLog(L"KernelModulesList�ɹ� \r\n");
		size = outCount;
		return TRUE;
	}
	PrintDbgString(L"KernelModulesListʧ�� \r\n");
	PrintLog(L"KernelModulesListʧ�� \r\n");
	return FALSE;
}

CONSOLEDLL_API BOOL KernelModulesGet(PVOID buff, DWORD size)
{
	DWORD dwRet,outCount = size;
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_KERNEL_MODULES_GET,buff,size,buff,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"KernelModulesGet�ɹ� \r\n");
		PrintLog(L"KernelModulesGet�ɹ� \r\n");
		size = outCount;
		return TRUE;
	}
	PrintDbgString(L"KernelModulesGetʧ�� \r\n");
	PrintLog(L"KernelModulesGetʧ�� \r\n");
	return FALSE;	
}

CONSOLEDLL_API BOOL SSDTList(OUT DWORD & size)
{
	DWORD dwRet,outCount = 0;
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_KERNEL_SSDT_LIST,NULL,size,NULL,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"SSDTList�ɹ� \r\n");
		PrintLog(L"SSDTList�ɹ� \r\n");
		size = outCount;
		return TRUE;
	}
	PrintDbgString(L"SSDTListʧ�� \r\n");
	PrintLog(L"SSDTListʧ�� \r\n");
	return FALSE;	
}

CONSOLEDLL_API BOOL SSDTGet(PVOID buff, DWORD size)
{
	DWORD dwRet,outCount = size;
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_KERNEL_SSDT_GET,buff,size,buff,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"SSDTGet�ɹ� \r\n");
		PrintLog(L"SSDTGet�ɹ� \r\n");
		size = outCount;
		return TRUE;
	}
	PrintDbgString(L"SSDTGetʧ�� \r\n");
	PrintLog(L"SSDTGetʧ�� \r\n");
	return FALSE;	
}

CONSOLEDLL_API BOOL SSDTShadowList(OUT DWORD & size)
{
	DWORD dwRet,outCount = 0;
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_KERNEL_SSDTSHADOW_LIST,NULL,size,NULL,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"SSDTShadowList�ɹ� \r\n");
		PrintLog(L"SSDTShadowList�ɹ� \r\n");
		size = outCount;
		return TRUE;
	}
	PrintDbgString(L"SSDTShadowListʧ�� \r\n");
	PrintLog(L"SSDTShadowListʧ�� \r\n");
	return FALSE;	
}

CONSOLEDLL_API BOOL SSDTShadowGet(PVOID buff, DWORD size)
{
	DWORD dwRet,outCount = size;
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_KERNEL_SSDTSHADOW_GET,buff,size,buff,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"SSDTShadowGet�ɹ� \r\n");
		PrintLog(L"SSDTShadowGet�ɹ� \r\n");
		size = outCount;
		return TRUE;
	}
	PrintDbgString(L"SSDTShadowGetʧ�� \r\n");
	PrintLog(L"SSDTShadowGetʧ�� \r\n");
	return FALSE;	
}

CONSOLEDLL_API BOOL DpcTimerList(OUT DWORD & size)
{
	DWORD dwRet,outCount = size;
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_KERNEL_DPCTIMER_LIST,NULL,size,NULL,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"DpcTimerList�ɹ� \r\n");
		PrintLog(L"DpcTimerList�ɹ� \r\n");
		size = outCount;
		return TRUE;
	}
	PrintDbgString(L"DpcTimerListʧ�� \r\n");
	PrintLog(L"DpcTimerListʧ�� \r\n");
	return FALSE;	
}

CONSOLEDLL_API BOOL DpcTimerGet(PVOID buff, DWORD size)
{
	DWORD dwRet,outCount = size;
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_KERNEL_DPCTIMER_GET,buff,size,buff,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"DpcTimerGet�ɹ� \r\n");
		PrintLog(L"DpcTimerGet�ɹ� \r\n");
		size = outCount;
		return TRUE;
	}
	PrintDbgString(L"DpcTimerGetʧ�� \r\n");
	PrintLog(L"DpcTimerGetʧ�� \r\n");
	return FALSE;
}

CONSOLEDLL_API BOOL IoTimerList(OUT DWORD & size)
{
	DWORD dwRet,outCount = size;
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_KERNEL_IOTIMER_LIST,NULL,size,NULL,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"IoTimerList�ɹ� \r\n");
		PrintLog(L"IoTimerList�ɹ� \r\n");
		size = outCount;
		return TRUE;
	}
	PrintDbgString(L"IoTimerListʧ�� \r\n");
	PrintLog(L"IoTimerListʧ�� \r\n");
	return FALSE;	
}

CONSOLEDLL_API BOOL IoTimerGet(PVOID buff, DWORD size)
{
	DWORD dwRet,outCount = size;
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_KERNEL_IOTIMER_GET,buff,size,buff,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"IoTimerGet�ɹ� \r\n");
		PrintLog(L"IoTimerGet�ɹ� \r\n");
		size = outCount;
		return TRUE;
	}
	PrintDbgString(L"IoTimerGetʧ�� \r\n");
	PrintLog(L"IoTimerGetʧ�� \r\n");
	return FALSE;
}

CONSOLEDLL_API BOOL SetSynchroniza(PVOID buff, DWORD size)
{
	DWORD dwRet,outCount = size;
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_SYNCHRONIZA,buff,size,buff,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"SetSynchroniza�ɹ� \r\n");
		PrintLog(L"SetSynchroniza�ɹ� \r\n");
		return TRUE;
	}
	PrintDbgString(L"SetSynchronizaʧ�� \r\n");
	PrintLog(L"SetSynchronizaʧ�� \r\n");
	return FALSE;
}

CONSOLEDLL_API BOOL GetSynchronizaInfo(PVOID buff, DWORD size)
{
	DWORD dwRet,outCount = size;
	dwRet = control.DoDeviceIoControl(IOCTL_CRD_OPERATION_GETSYNCHRONIZEINFO,buff,size,buff,outCount);
	if(dwRet != DRV_ERROR_IO)
	{
		PrintDbgString(L"GetSynchronizaInfo�ɹ� \r\n");
		PrintLog(L"GetSynchronizaInfo�ɹ� \r\n");
		return TRUE;
	}
	PrintDbgString(L"GetSynchronizaInfoʧ�� \r\n");
	PrintLog(L"GetSynchronizaInfoʧ�� \r\n");
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

#define   KBC_KEY_CMD   0x64         //��������˿� 
#define   KBC_KEY_DATA   0x60       //�������ݶ˿� 

void KBCWait4IBE() 
{ 
	char   dwRegVal=0; 
	do 
	{   
		GetPortVal(0x64,&dwRegVal,1); 
	} 
	while(dwRegVal   &   0x00000001); 
} 


CONSOLEDLL_API void KeyDownEx(long vKeyCoad)       //ģ����չ�����£�����vKeyCoad����չ���������� 
{ 
	BYTE byScancode = (BYTE)(MapVirtualKey(vKeyCoad,0) & 0xFF);

	KBCWait4IBE();       //�ȴ����̻�����Ϊ�� 
	SetPortVal(KBC_KEY_CMD, (BYTE)(0xD2 & 0xFF), 1);       //���ͼ���д������ 
	KBCWait4IBE(); 
	SetPortVal(KBC_KEY_DATA,(BYTE)(0xE0 & 0xFF), 1);   //д����չ����־��Ϣ 


	KBCWait4IBE();       //�ȴ����̻�����Ϊ�� 
	SetPortVal(KBC_KEY_CMD,(BYTE)(0xD2 & 0xFF), 1);         //���ͼ���д������ 
	KBCWait4IBE(); 
	SetPortVal(KBC_KEY_DATA,byScancode, 1);   //д�밴����Ϣ,���¼� 
} 


CONSOLEDLL_API void KeyUpEx(long vKeyCoad)       //ģ����չ������ 
{ 
	BYTE byScancode = (BYTE)MapVirtualKey(vKeyCoad,0) & 0xFF;   

	KBCWait4IBE();       //�ȴ����̻�����Ϊ�� 
	SetPortVal(KBC_KEY_CMD,(BYTE)(0xD2 & 0xFF),1);         //���ͼ���д������ 
	KBCWait4IBE(); 
	SetPortVal   (KBC_KEY_DATA,(BYTE)(0xE0 & 0xFF),1);   //д����չ����־��Ϣ 

	KBCWait4IBE();     //�ȴ����̻�����Ϊ�� 
	SetPortVal(KBC_KEY_CMD,(BYTE)(0xD2 & 0xFF),   1   );         //���ͼ���д������ 
	KBCWait4IBE(); 
	SetPortVal(KBC_KEY_DATA,(byScancode|0x80),   1);     //д�밴����Ϣ���ͷż� 
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
