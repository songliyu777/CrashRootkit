#ifndef __RING3__
#define __RING3__
#endif
#include "WinSvc.h"
#pragma warning( disable: 4996 )

//ERROR CODES
#define DRV_SUCCESS						 (DWORD)0		// Todo bien

#define DRV_ERROR_SCM					 (DWORD)-1		// ERROR al abrir el service manager
#define DRV_ERROR_SERVICE				 (DWORD)-2		// ERROR al crear el servicio
#define DRV_ERROR_MEMORY				 (DWORD)-3		// ERROR al reservar memoria
#define DRV_ERROR_INVALID_PATH_OR_FILE	 (DWORD)-4		// ERROR, Path no valido
#define DRV_ERROR_INVALID_HANDLE		 (DWORD)-5		// ERROR, driver handle no valido
#define DRV_ERROR_STARTING				 (DWORD)-6		// ERROR al poner en Start el driver
#define DRV_ERROR_STOPPING				 (DWORD)-7		// ERROR al parar el driver
#define DRV_ERROR_REMOVING				 (DWORD)-8		// ERROR eliminando el "servicio"
#define DRV_ERROR_IO					 (DWORD)-9		// ERROR en operacion de E/S
#define DRV_ERROR_NO_INITIALIZED		 (DWORD)-10		// ERROR, clase no inicializada
#define DRV_ERROR_ALREADY_INITIALIZED	 (DWORD)-11		// ERROR, clase ya inicializada
#define DRV_ERROR_NULL_POINTER			 (DWORD)-12		// ERROR, puntero a null como parametro
#define DRV_ERROR_UNKNOWN				 (DWORD)-13		// ERROR desconocido


class DRVControl
{
public:
	DRVControl(void);
	~DRVControl(void);

	WCHAR driverName[MAX_PATH];

	BOOL LoadDriver(
		IN LPCWSTR lpszDriverFileName,       //lpszDriverName �����ļ���
		IN LPCWSTR lpszDriverPathName = NULL //lpszDriverPath ����·��
		);									 //��������

	BOOL UnloadDriver(LPCWSTR lpszDriverFileName); // ж������

	BOOL UnloadDriver();						   // ж�ص�ǰ����

	LPCWSTR GetCurrentPath(OUT PWCHAR szDriverImagePath);

	HANDLE driverHandle;
	BOOL OpenDevice(IN LPCWSTR lpszDevicePath); 	// ���豸
	VOID CloseDevice();								// �ر��豸
	DWORD DoDeviceIoControl(DWORD code, PVOID inBuffer, DWORD inCount, PVOID outBuffer, DWORD & outCount);
};