#pragma once

#define _CRD_
#include "Transfer.h"

NTSTATUS SetKeException(PTransferMsg msg);

NTSTATUS DoKiDispatchExceptionHook_XP();

NTSTATUS DoKiDispatchExceptionHook_WIN7();

VOID SetKiUserExceptionDispatcherAddress(ULONG_PTR BaseAddress);

ULONG_PTR GetKiUserExceptionDispatcherAddress();

NTSTATUS DoKeExceptionHook();