#include "MyWinNT.h"
#define _CRD_
#include "Transfer.h"

VOID ImageCreateMon (IN PUNICODE_STRING  FullImageName, IN HANDLE  ProcessId, IN PIMAGE_INFO  ImageInfo);

NTSTATUS HideKernelDebugger(PTransferMsg msg);

NTSTATUS MonitorDriverLoad(PTransferMsg msg);

NTSTATUS UnMonitorDL();

NTSTATUS UnHideKD();

NTSTATUS SetSynchronizaEvent(HANDLE hEvent);

NTSTATUS FreeSynchronizaEvent();