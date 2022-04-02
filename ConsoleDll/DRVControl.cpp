#include "stdafx.h"
#include "DRVControl.h"


DRVControl::DRVControl(void)
{

}

DRVControl::~DRVControl(void)
{

}

BOOL DRVControl::OpenDevice(IN LPCWSTR lpszDevicePath)
{
	driverHandle = CreateFile(lpszDevicePath,     // 设备路径
		GENERIC_READ | GENERIC_WRITE,        // 读写方式
		FILE_SHARE_READ | FILE_SHARE_WRITE,  // 共享方式
		NULL,                                // 默认的安全描述符
		OPEN_EXISTING,                       // 创建方式
		0,                                   // 不需设置文件属性
		NULL);                               // 不需同步

	if(driverHandle == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	return TRUE;
}

VOID DRVControl::CloseDevice()
{
	if(driverHandle != NULL)
	{
		CloseHandle(driverHandle); 
		driverHandle = NULL; 
	}
}

LPCWSTR DRVControl::GetCurrentPath(OUT PWCHAR szDriverImagePath)
{
	GetModuleFileName(NULL,szDriverImagePath,MAX_PATH); //得到当前模块路径
	for (int i=wcslen(szDriverImagePath);i>0;i--)
	{
		if (L'\\'==szDriverImagePath[i])
		{
			szDriverImagePath[i+1]=L'\0';
			break;
		}
	}
	return szDriverImagePath;
}
///////////////////////////////////////////////////
//加载驱动
BOOL DRVControl::LoadDriver(IN LPCWSTR lpszDriverFileName, IN LPCWSTR lpszDriverPathName)
{
	wcscpy_s(driverName,lpszDriverFileName);
	WCHAR szDriverImagePath[MAX_PATH] = {0};

	if(lpszDriverPathName == NULL)
	{
		GetCurrentPath(szDriverImagePath);
		wcscat(szDriverImagePath,lpszDriverFileName);
		wcscat(szDriverImagePath,L".sys");
	}
	else
	{
		wcscat(szDriverImagePath,lpszDriverPathName);
	}

	BOOL bRet = FALSE;

	SC_HANDLE hServiceMgr = NULL;//SCM管理器的句柄
	SC_HANDLE hServiceDDK = NULL;//NT驱动程序的服务句柄

	//打开服务控制管理器
	hServiceMgr = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );

	if( hServiceMgr == NULL )  
	{
		bRet = FALSE;
		goto BeforeLeave;
	}

	//创建驱动所对应的服务
	hServiceDDK = CreateServiceW( hServiceMgr,
		lpszDriverFileName, //驱动程序的在注册表中的名字  
		lpszDriverFileName, // 注册表驱动程序的 DisplayName 值  
		SERVICE_ALL_ACCESS, // 加载驱动程序的访问权限  
		SERVICE_KERNEL_DRIVER,// 表示加载的服务是驱动程序  
		SERVICE_DEMAND_START, // 注册表驱动程序的 Start 值 //SERVICE_AUTO_START,系统启动时启动该驱动 release 版本将会采用
		SERVICE_ERROR_IGNORE, // 注册表驱动程序的 ErrorControl 值  
		szDriverImagePath, // 注册表驱动程序的 ImagePath 值  
		NULL,  
		NULL,  
		NULL,  
		NULL,  
		NULL);  

	DWORD dwRtn;
	//判断服务是否失败
	if( hServiceDDK == NULL )  
	{  
		dwRtn = GetLastError();
		if(dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_EXISTS)  
		{  
			//由于其他原因创建服务失败
			bRet = FALSE;
		}  

		// 驱动程序已经加载，只需要打开  
		hServiceDDK = OpenServiceW(hServiceMgr, lpszDriverFileName, SERVICE_ALL_ACCESS);  
		if(hServiceDDK == NULL)  
		{
			//如果打开服务也失败，则意味错误
			dwRtn = GetLastError();  
			bRet = FALSE;
			goto BeforeLeave;
		}  
	}  

	//开启此项服务
	bRet= StartServiceW(hServiceDDK, NULL, NULL );  
	if( !bRet )  
	{  
		DWORD dwRtn = GetLastError();  
		if( dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_ALREADY_RUNNING )  
		{  
			//printf( "StartService() Faild %d ! \r\n", dwRtn );  
			bRet = FALSE;
			goto BeforeLeave;
		}  
		else  
		{  
			if( dwRtn == ERROR_IO_PENDING )  
			{  
				//设备被挂住
				//printf( "StartService() Faild ERROR_IO_PENDING ! \r\n");
				bRet = FALSE;
				goto BeforeLeave;
			}  
			else  
			{  
				//服务已经开启
				//printf( "StartService() Faild ERROR_SERVICE_ALREADY_RUNNING ! \r\n");
				bRet = TRUE;
				goto BeforeLeave;
			}  
		}  
	}
	bRet = TRUE;
	//离开前关闭句柄
BeforeLeave:
	if(hServiceDDK)
	{
		CloseServiceHandle(hServiceDDK);
	}
	if(hServiceMgr)
	{
		CloseServiceHandle(hServiceMgr);
	}
	return bRet;
}
///////////////////////////////////////////////////

//卸载驱动

BOOL DRVControl::UnloadDriver(LPCWSTR lpszDriverFileName) 
{
	SC_HANDLE hServiceMgr, hServiceTwdm; 
	SERVICE_STATUS SvrSta;
	BOOL bRet = FALSE;

	hServiceMgr = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS ); 
	if( hServiceMgr == NULL ) 
	{ 
		return bRet; 
	} 

	hServiceTwdm = OpenService( hServiceMgr, lpszDriverFileName, SERVICE_ALL_ACCESS ); 

	if( hServiceTwdm == NULL ) 
	{ 
		CloseServiceHandle( hServiceMgr ); 
		return bRet; 
	} 

	//停止驱动程序，如果停止失败，只有重新启动才能，再动态加载。 
	if(ControlService(hServiceTwdm, SERVICE_CONTROL_STOP , &SvrSta))
	{
		//动态卸载驱动程序。 
		if(DeleteService(hServiceTwdm)) 
		{ 
			bRet = TRUE;
		} 
	}


	CloseServiceHandle( hServiceTwdm ); 
	CloseServiceHandle( hServiceMgr );
	return bRet; 
}

BOOL DRVControl::UnloadDriver()
{
	CloseDevice();
	if(driverName != NULL)
	{
		return UnloadDriver(driverName);
	}
	return TRUE;
}

DWORD DRVControl::DoDeviceIoControl(DWORD code, PVOID inBuffer, DWORD inCount, PVOID outBuffer, DWORD & outCount)
{
	if(driverHandle == NULL)
	{
		//AfxMessageBox(L"RawIo Fail");
		return DRV_ERROR_INVALID_HANDLE;
	}
	DWORD bytesReturned;
	BOOL retCode = DeviceIoControl(driverHandle,
								   code,
								   inBuffer,
								   inCount,
								   outBuffer,
								   outCount,
								   &bytesReturned,
								   NULL);
	outCount = bytesReturned;
	if(!retCode)
		return DRV_ERROR_IO;

	return bytesReturned;
}