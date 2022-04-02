#ifdef __cplusplus
extern "C" {
#endif
#include <ntddk.h>
#include <string.h>
#ifdef __cplusplus
}; // extern "C"
#endif

#include "KeSysFun.h"
#include "CRD_KeSpeed.h"
#include "CRD_Hook.h"
#include "Log.h"

//ULONG_PTR GetHookAddress_KeUpdateSystemTime()
//{
//	FARPROC funadr = (FARPROC)KeUpdateSystemTime; 
//	FARPROC ofunadr = GetOrignalNtKrFunctionByName("KeUpdateSystemTime");
//	if(funadr && ofunadr)
//	{
//		if(RtlCompareMemory(funadr,ofunadr,5)==5)
//		{
//			return funadr;
//		}
//		else
//		{
//
//		}
//	}
//	return NULL;
//}

// 变速基数
const ULONG g_dwSpeedBase = 100;
// 变速数值 (200 / 100 = 2倍速,驱动加载后,默认为2倍速)
ULONG g_dwSpeed_X = 100;


ULONG_PTR OrignalKeUpdateSystemTime_Jmp = 0;
ULONG_PTR OrignalKeQueryPerformanceCounter_Jmp = 0;

NTSTATUS __declspec(naked) KeUpdateSystemTime_Proxy()
{
	_asm
	{
		mul g_dwSpeed_X
		div g_dwSpeedBase
		mov ecx,0FFDF0000h
		jmp OrignalKeUpdateSystemTime_Jmp
	}
}

LARGE_INTEGER __declspec(naked) OriginalKeQueryPerformanceCounter(OUT PLARGE_INTEGER PerformanceFrequency)
{
	_asm
	{
		mov     edi,edi
		push    ebp
		mov     ebp,esp
		jmp OrignalKeQueryPerformanceCounter_Jmp
	}
}


LARGE_INTEGER g_liPreReturnCounter = {0};
LARGE_INTEGER g_liPreOriginalCounter = {0};

LARGE_INTEGER KeQueryPerformanceCounter_Proxy(OUT PLARGE_INTEGER PerformanceFrequency)
{
	LARGE_INTEGER liResult;
	LARGE_INTEGER liCurrent;

	KEQUERYPERFORMANCECOUNTER function = (KEQUERYPERFORMANCECOUNTER)GetRealFunctionAddress("KeQueryPerformanceCounter");
	if(function)
	{
		liCurrent = function(PerformanceFrequency);
		if(g_liPreOriginalCounter.QuadPart == 0)
		{
			g_liPreOriginalCounter.QuadPart = liCurrent.QuadPart;
		}
		if(g_liPreReturnCounter.QuadPart == 0)
		{
			g_liPreReturnCounter.QuadPart = liCurrent.QuadPart;
		}
		// 上次返回时间 + Round((当前返回时间 - 上次正常时间) * Power(2,倍数));
		liResult.QuadPart = g_liPreReturnCounter.QuadPart + (liCurrent.QuadPart - g_liPreOriginalCounter.QuadPart) * g_dwSpeed_X / g_dwSpeedBase;
		// 保存当前的原始数值
		g_liPreOriginalCounter.QuadPart = liCurrent.QuadPart;
		// 保持返回值
		g_liPreReturnCounter.QuadPart = liResult.QuadPart;
	}
	return liResult;
}

NTSTATUS DoKeSpeedHook()
{
	OrignalKeUpdateSystemTime_Jmp = (ULONG_PTR)KeUpdateSystemTime + 5;
	OrignalKeQueryPerformanceCounter_Jmp = (ULONG_PTR)KeQueryPerformanceCounter + 5;
	if(SetDynamicInLineHook("KeUpdateSystemTime",NULL,5,(ULONG_PTR)KeUpdateSystemTime_Proxy,INLINE_HOOK_TYPE_JMP))
	{
		SetDynamicInLineHook("KeQueryPerformanceCounter",(ULONG_PTR)KeQueryPerformanceCounter,5,(ULONG_PTR)KeQueryPerformanceCounter_Proxy,INLINE_HOOK_TYPE_JMP);
	}
	return STATUS_SUCCESS;
}

NTSTATUS SetKeSpeed(PTransferMsg msg)
{
	DoKeSpeedHook();
	g_dwSpeed_X = *PULONG(msg->buff);
	g_liPreOriginalCounter.QuadPart = 0;
	g_liPreReturnCounter.QuadPart = 0;
	return STATUS_SUCCESS;
}