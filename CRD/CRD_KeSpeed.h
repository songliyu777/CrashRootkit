#pragma once

#define _CRD_
#include "Transfer.h"

NTSTATUS DoKeSpeedHook();

NTSTATUS SetKeSpeed(PTransferMsg msg);

NTSTATUS KeUpdateSystemTime_Proxy();

LARGE_INTEGER KeQueryPerformanceCounter_Proxy(OUT PLARGE_INTEGER PerformanceFrequency);

typedef LARGE_INTEGER (*KEQUERYPERFORMANCECOUNTER)(OUT PLARGE_INTEGER PerformanceFrequency);