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
	driverHandle = CreateFile(lpszDevicePath,     // �豸·��
		GENERIC_READ | GENERIC_WRITE,        // ��д��ʽ
		FILE_SHARE_READ | FILE_SHARE_WRITE,  // ����ʽ
		NULL,                                // Ĭ�ϵİ�ȫ������
		OPEN_EXISTING,                       // ������ʽ
		0,                                   // ���������ļ�����
		NULL);                               // ����ͬ��

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
	GetModuleFileName(NULL,szDriverImagePath,MAX_PATH); //�õ���ǰģ��·��
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
//��������
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

	SC_HANDLE hServiceMgr = NULL;//SCM�������ľ��
	SC_HANDLE hServiceDDK = NULL;//NT��������ķ�����

	//�򿪷�����ƹ�����
	hServiceMgr = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );

	if( hServiceMgr == NULL )  
	{
		bRet = FALSE;
		goto BeforeLeave;
	}

	//������������Ӧ�ķ���
	hServiceDDK = CreateServiceW( hServiceMgr,
		lpszDriverFileName, //�����������ע����е�����  
		lpszDriverFileName, // ע������������ DisplayName ֵ  
		SERVICE_ALL_ACCESS, // ������������ķ���Ȩ��  
		SERVICE_KERNEL_DRIVER,// ��ʾ���صķ�������������  
		SERVICE_DEMAND_START, // ע������������ Start ֵ //SERVICE_AUTO_START,ϵͳ����ʱ���������� release �汾�������
		SERVICE_ERROR_IGNORE, // ע������������ ErrorControl ֵ  
		szDriverImagePath, // ע������������ ImagePath ֵ  
		NULL,  
		NULL,  
		NULL,  
		NULL,  
		NULL);  

	DWORD dwRtn;
	//�жϷ����Ƿ�ʧ��
	if( hServiceDDK == NULL )  
	{  
		dwRtn = GetLastError();
		if(dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_EXISTS)  
		{  
			//��������ԭ�򴴽�����ʧ��
			bRet = FALSE;
		}  

		// ���������Ѿ����أ�ֻ��Ҫ��  
		hServiceDDK = OpenServiceW(hServiceMgr, lpszDriverFileName, SERVICE_ALL_ACCESS);  
		if(hServiceDDK == NULL)  
		{
			//����򿪷���Ҳʧ�ܣ�����ζ����
			dwRtn = GetLastError();  
			bRet = FALSE;
			goto BeforeLeave;
		}  
	}  

	//�����������
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
				//�豸����ס
				//printf( "StartService() Faild ERROR_IO_PENDING ! \r\n");
				bRet = FALSE;
				goto BeforeLeave;
			}  
			else  
			{  
				//�����Ѿ�����
				//printf( "StartService() Faild ERROR_SERVICE_ALREADY_RUNNING ! \r\n");
				bRet = TRUE;
				goto BeforeLeave;
			}  
		}  
	}
	bRet = TRUE;
	//�뿪ǰ�رվ��
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

//ж������

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

	//ֹͣ�����������ֹͣʧ�ܣ�ֻ�������������ܣ��ٶ�̬���ء� 
	if(ControlService(hServiceTwdm, SERVICE_CONTROL_STOP , &SvrSta))
	{
		//��̬ж���������� 
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