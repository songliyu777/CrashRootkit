#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include <ntddkbd.h>
#include <ntddmou.h>
#ifdef __cplusplus
}; // extern "C"
#endif

#include "MyWinNT.h"
#define _CRD_
#include "Transfer.h"

typedef VOID (*KeyboardClassServiceCallback) (
									PDEVICE_OBJECT  DeviceObject,
									PKEYBOARD_INPUT_DATA  InputDataStart,
									PKEYBOARD_INPUT_DATA  InputDataEnd,
									PULONG  InputDataConsumed
									);

typedef VOID (*MouseClassServiceCallback) (
									 PDEVICE_OBJECT  DeviceObject,
									 PMOUSE_INPUT_DATA  InputDataStart,
									 PMOUSE_INPUT_DATA  InputDataEnd,
									 PULONG  InputDataConsumed
									);

NTSTATUS SimulateInput(PTransferMsg msg);