///////////////////////////////////////////////////////////////////////////////
///
/// Copyright (c) 2013 - <company name here>
///
/// Original filename: CRD.cpp
/// Project          : CRD
/// Date of creation : 2013-03-15
/// Author(s)        : <author name(s)>
///
/// Purpose          : <description>
///
/// Revisions:
///  0000 [2013-03-15] Initial revision.
///
///////////////////////////////////////////////////////////////////////////////

// $Id$

#ifdef __cplusplus
extern "C" {
#endif
#include <ntddk.h>
#include <string.h>
#ifdef __cplusplus
}; // extern "C"
#endif

#include "CRD.h"
#include "Dispatch.h"
#include "Relocate.h"
#include "MyWinNT.h"
#include "Log.h"
#include "CRD_HookFunction.h"
#include "CRD_ProtectTools.h"
#include "CRD_ReplaceFunction.h"
#include "CRD_IDT.h"

#ifdef __cplusplus
namespace { // anonymous namespace to limit the scope of this global variable!
#endif
	extern "C" extern PDRIVER_OBJECT pdoGlobalDrvObj = 0;
#ifdef __cplusplus
}; // anonymous namespace
#endif

KMUTEX g_GlobleMutex;

NTSTATUS CRD_DispatchCreate(
							IN PDEVICE_OBJECT		DeviceObject,
							IN PIRP					Irp
							)
{
	NTSTATUS status = STATUS_SUCCESS;
	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}

NTSTATUS CRD_DispatchCreateClose(
								 IN PDEVICE_OBJECT		DeviceObject,
								 IN PIRP					Irp
								 )
{
	NTSTATUS status = STATUS_SUCCESS;
	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}

NTSTATUS CRD_DispatchDeviceControl(
								   IN PDEVICE_OBJECT		DeviceObject,
								   IN PIRP					Irp
								   )
{
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);

	PVOID InputBuffer;
	PVOID OutputBuffer;
	ULONG InputBufferLength;
	ULONG OutputBufferLength;
	ULONG IoControlCode;

	InputBuffer = OutputBuffer = Irp->AssociatedIrp.SystemBuffer;
	InputBufferLength = irpStack->Parameters.DeviceIoControl.InputBufferLength;
	OutputBufferLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;
	IoControlCode = irpStack->Parameters.DeviceIoControl.IoControlCode;

	switch(IoControlCode)
	{
	case IOCTL_CRD_OPERATION_TRANSFER:
		{
			if(InputBufferLength==sizeof(TransferMsg))
			{
				PTransferMsg msg = (PTransferMsg)InputBuffer;
				status = DispatchMessage(msg);
				Irp->IoStatus.Information = InputBufferLength;
			}
			else
			{
				status = STATUS_UNSUCCESSFUL;
				Irp->IoStatus.Information = InputBufferLength;
			}
		}
		break;
	case IOCTL_CRD_OPERATION_KERNELHOOK_CHECK:
		{
			status = KernelHookCheck(OutputBufferLength);
			Irp->IoStatus.Information = OutputBufferLength;
		}
		break;
	case IOCTL_CRD_OPERATION_KERNELHOOK_GET:
		{
			PVOID buff = NULL;
			DWORD size = InputBufferLength;
			status = KernelHookGet(buff,size);
			if(NT_SUCCESS(status))
			{
				RtlCopyMemory(OutputBuffer,buff,size);
				ExFreePool(buff);
			}
			Irp->IoStatus.Information = InputBufferLength;
		}
		break;
	case IOCTL_CRD_OPERATION_PROCESSES_LIST:
		{
			ProcessList(OutputBufferLength);
			Irp->IoStatus.Information = OutputBufferLength;
		}
		break;
	case IOCTL_CRD_OPERATION_PROCESSES_GET:
		{
			PVOID buff = InputBuffer;
			DWORD size = InputBufferLength;
			status = ProcessListGet(buff,size);
			Irp->IoStatus.Information = InputBufferLength;
		}
		break;
	case IOCTL_CRD_OPERATION_MODULES_LIST:
		{
			ULONG_PTR pid = *PULONG_PTR(InputBuffer);
			status = ModulesList(pid,OutputBufferLength);
			Irp->IoStatus.Information = OutputBufferLength;
		}
		break;
	case IOCTL_CRD_OPERATION_MODULES_GET:
		{
			PVOID buff = InputBuffer;
			DWORD size = InputBufferLength;
			status = ModulesListGet(buff,size);
			Irp->IoStatus.Information = InputBufferLength;
		}
		break;
	case IOCTL_CRD_OPERATION_THREADS_LIST:
		{
			ULONG_PTR pid = *PULONG_PTR(InputBuffer);
			status = ThreadsList(pid,OutputBufferLength);
			Irp->IoStatus.Information = OutputBufferLength;
		}
		break;
	case IOCTL_CRD_OPERATION_THREADS_GET:
		{
			PVOID buff = InputBuffer;
			DWORD size = InputBufferLength;
			status = ThreadsListGet(buff,size);
			Irp->IoStatus.Information = InputBufferLength;
		}
		break;
	case IOCTL_CRD_OPERATION_MODULE_PE_INFO_GET:
		{
			PVOID buff = InputBuffer;
			DWORD size = InputBufferLength;
			status = ModulePeInfoGet(buff,size);
			OutputBufferLength = size;
			Irp->IoStatus.Information = OutputBufferLength;
		}
		break;
	case IOCTL_CRD_OPERATION_CR4_GET:
		{
			PVOID buff = InputBuffer;
			status = CR4Get(*(PULONG)InputBuffer);
			OutputBufferLength = InputBufferLength;
			Irp->IoStatus.Information = OutputBufferLength;
		}
		break;
	case IOCTL_CRD_OPERATION_READPROCESSMEMORY:
		{
			PVOID buff = InputBuffer;
			DWORD size = InputBufferLength;
			status = ReadProcessMemory(buff,InputBufferLength,OutputBufferLength);
			Irp->IoStatus.Information = OutputBufferLength;
		}
		break;
	case IOCTL_CRD_OPERATION_WRITEPROCESSMEMORY:
		{
			PVOID buff = InputBuffer;
			DWORD size = InputBufferLength;
			status = WriteProcessMemory(buff,InputBufferLength,OutputBufferLength);
			Irp->IoStatus.Information = OutputBufferLength;
		}
		break;
	case IOCTL_CRD_OPERATION_KERNEL_MODULES_LIST:
		{
			status = KernelModuleList(OutputBufferLength);
			Irp->IoStatus.Information = OutputBufferLength;
		}
		break;
	case IOCTL_CRD_OPERATION_KERNEL_MODULES_GET:
		{
			PVOID buff = InputBuffer;
			DWORD size = InputBufferLength;
			status = KernelModuleListGet(InputBuffer,size);
			OutputBufferLength = size;
			Irp->IoStatus.Information = OutputBufferLength;
		}
		break;
	case IOCTL_CRD_OPERATION_KERNEL_SSDT_LIST:
		{
			status = SSDTList(OutputBufferLength);
			Irp->IoStatus.Information = OutputBufferLength;
		}
		break;
	case IOCTL_CRD_OPERATION_KERNEL_SSDT_GET:
		{
			PVOID buff = InputBuffer;
			DWORD size = InputBufferLength;
			status = SSDTGet(InputBuffer,size);
			OutputBufferLength = size;
			Irp->IoStatus.Information = OutputBufferLength;
		}
		break;
	case IOCTL_CRD_OPERATION_KERNEL_SSDTSHADOW_LIST:
		{
			status = SSDTShadowList(OutputBufferLength);
			Irp->IoStatus.Information = OutputBufferLength;
		}
		break;
	case IOCTL_CRD_OPERATION_KERNEL_SSDTSHADOW_GET:
		{
			PVOID buff = InputBuffer;
			DWORD size = InputBufferLength;
			status = SSDTShadowGet(InputBuffer,size);
			OutputBufferLength = size;
			Irp->IoStatus.Information = OutputBufferLength;
		}
		break;
	case IOCTL_CRD_OPERATION_KERNEL_DPCTIMER_LIST:
		{
			status = DpcTimerList(OutputBufferLength);
			Irp->IoStatus.Information = OutputBufferLength;
		}
		break;
	case IOCTL_CRD_OPERATION_KERNEL_DPCTIMER_GET:
		{
			PVOID buff = InputBuffer;
			DWORD size = InputBufferLength;
			status = DpcTimerGet(InputBuffer,size);
			OutputBufferLength = size;
			Irp->IoStatus.Information = OutputBufferLength;
		}
		break;
	case IOCTL_CRD_OPERATION_KERNEL_IOTIMER_LIST:
		{
			status = IoTimerList(OutputBufferLength);
			Irp->IoStatus.Information = OutputBufferLength;
		}
		break;
	case IOCTL_CRD_OPERATION_KERNEL_IOTIMER_GET:
		{
			PVOID buff = InputBuffer;
			DWORD size = InputBufferLength;
			status = IoTimerGet(InputBuffer,size);
			OutputBufferLength = size;
			Irp->IoStatus.Information = OutputBufferLength;
		}
		break;
	case IOCTL_CRD_OPERATION_SETLOGPATH:
		{
			status = SetLogPath((PWCHAR)InputBuffer);
			Irp->IoStatus.Information = OutputBufferLength;
			if(!InitRelocate())
			{
				status = STATUS_UNSUCCESSFUL;
			}
			else
			{
				if(!CreateProtectThread())
				{
					status = STATUS_UNSUCCESSFUL;
				}
			}
		}
		break;
	case IOCTL_CRD_OPERATION_SYNCHRONIZA:
		{
			HANDLE hEvent = *((HANDLE *)InputBuffer);
			if(hEvent)
			{
				status = SetSynchronizaEvent(hEvent);
			}
			else
			{
				status = FreeSynchronizaEvent();
			}
		}
		break;
	case IOCTL_CRD_OPERATION_GETSYNCHRONIZEINFO:
		{
			status = GetSynchronizaInfo(OutputBuffer,OutputBufferLength);
			Irp->IoStatus.Information = OutputBufferLength;
		}
		break;
	case IOCTL_CRD_OPERATION_SENDINPUT:
		{
			PTransferSendInput msg = (PTransferSendInput)InputBuffer;
			UINT nRet = RNtUserSendInput_Proxy(msg->nInputs, msg->pInputs, msg->cbSize);
			if (nRet == 0)
			{
				status = STATUS_UNSUCCESSFUL;
			}
			else
			{
				status = STATUS_SUCCESS;
			}
			Irp->IoStatus.Information = nRet;
		}
		break;
	case IOCTL_CRD_OPERATION_FINDWINDOWEX:
		{
			PTransferFindWindowEx msg = (PTransferFindWindowEx)InputBuffer;
			HWND hwnd = RNtUserFindWindowEx_Proxy(msg->hwndParent,msg->hwndChild,(PUNICODE_STRING)msg->strClassName,(PUNICODE_STRING)msg->strWindowName,msg->dwType);
			OutputBufferLength = sizeof(HWND);
			if(hwnd)
			{
				*(HWND *)OutputBuffer = hwnd;
				status = STATUS_SUCCESS;
			}
			else
			{
				status = STATUS_UNSUCCESSFUL;
			}
			Irp->IoStatus.Information = OutputBufferLength;
		}
		break;
	case IOCTL_CRD_OPERATION_OPENPROCESS:
		{
			HANDLE handle = NULL;
			status = NtOpenProcessEx(InputBuffer,&handle);
			OutputBufferLength = sizeof(HANDLE);
			*(HANDLE *)OutputBuffer = handle;
			Irp->IoStatus.Information = OutputBufferLength;
		}
		break;
	default:
		status = STATUS_INVALID_DEVICE_REQUEST;
		Irp->IoStatus.Information = 0;
		break;
	}

	Irp->IoStatus.Status = status;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}

VOID CRD_DriverUnload(
					  IN PDRIVER_OBJECT		DriverObject
					  )
{
	//KeReleaseMutex(&g_GlobleMutex, FALSE);

	FreeAll();

	PDEVICE_OBJECT pdoNextDeviceObj = pdoGlobalDrvObj->DeviceObject;
	IoDeleteSymbolicLink(&usSymlinkName);

	// Delete all the device objects
	while(pdoNextDeviceObj)
	{
		PDEVICE_OBJECT pdoThisDeviceObj = pdoNextDeviceObj;
		pdoNextDeviceObj = pdoThisDeviceObj->NextDevice;
		IoDeleteDevice(pdoThisDeviceObj);
	}
}

#ifdef __cplusplus
extern "C" {
#endif
	NTSTATUS DriverEntry(
		IN OUT PDRIVER_OBJECT   DriverObject,
		IN PUNICODE_STRING      RegistryPath
		)
	{
		PDEVICE_OBJECT pdoDeviceObj = 0;
		NTSTATUS status = STATUS_UNSUCCESSFUL;
		pdoGlobalDrvObj = DriverObject;

		// Create the device object.
		if(!NT_SUCCESS(status = IoCreateDevice(
			DriverObject,
			0,
			&usDeviceName,
			FILE_DEVICE_UNKNOWN,
			FILE_DEVICE_SECURE_OPEN,
			FALSE,
			&pdoDeviceObj
			)))
		{
			// Bail out (implicitly forces the driver to unload).
			return status;
		};

		// Now create the respective symbolic link object
		if(!NT_SUCCESS(status = IoCreateSymbolicLink(
			&usSymlinkName,
			&usDeviceName
			)))
		{
			IoDeleteDevice(pdoDeviceObj);
			return status;
		}

		KeInitializeMutex(&g_GlobleMutex, 0);
		
		InitIDT();
		// NOTE: You need not provide your own implementation for any major function that
		//       you do not want to handle. I have seen code using DDKWizard that left the
		//       *empty* dispatch routines intact. This is not necessary at all!
		DriverObject->MajorFunction[IRP_MJ_CREATE] = CRD_DispatchCreate;
		DriverObject->MajorFunction[IRP_MJ_CLOSE] = CRD_DispatchCreateClose;
		DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = CRD_DispatchDeviceControl;
		DriverObject->DriverUnload = CRD_DriverUnload;

		return STATUS_SUCCESS;
	}
#ifdef __cplusplus
}; // extern "C"
#endif
