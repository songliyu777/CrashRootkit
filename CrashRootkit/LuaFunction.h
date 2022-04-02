#pragma once

extern "C"{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

void RegisterFunction(lua_State *L);

bool IsFindRegisterFunction(char * buf);

void lua_printex(const char *fmt,...);

static int lua_registerex(lua_State *L);

static int lua_print(lua_State *L);

static int lua_print_on(lua_State *L);

static int lua_print_off(lua_State *L);

/********************对应控制函数********************/
static int lua_DoKernelHookCheck(lua_State *L);

static int lua_DoProcessesList(lua_State *L);

static int lua_DoModulesList(lua_State *L);

static int lua_DoThreadsList(lua_State *L);

static int lua_DoHookCheckList(lua_State *L);

static int lua_DoKernelModulesList(lua_State *L);

static int lua_DoSSDTList(lua_State *L);

static int lua_DoSSDTShadowList(lua_State *L);

static int lua_DoDpcTimerList(lua_State *L);

static int lua_DoIoTimerList(lua_State *L);

static int lua_DoAntiProtectHook_1(lua_State *L);

static int lua_DoAntiProtectHook_2(lua_State *L);

static int lua_GetProcAddressByName(lua_State *L);

static int lua_DoStopIoTimer(lua_State *L);

static int lua_DoKillDpcTimer(lua_State *L);

static int lua_DoKernelHookRecover(lua_State *L);

static int lua_DoTerminateThread(lua_State *L);

static int lua_WriteProcessMemory(lua_State *L);

static int lua_KdDisableDebugger(lua_State *L);

static int lua_KdEnableDebugger(lua_State *L);

static int lua_HideKernelDebugger(lua_State *L);

static int lua_MonitorDriverLoad(lua_State *L);

static int lua_AddTargetProcess(lua_State *L);

static int lua_RecoverObject(lua_State *L);

static int lua_AddProtectProcess(lua_State *L);

static int lua_AddTargetKernelModule(lua_State *L);

static int lua_AddFilterDrvIo(lua_State *L);

static int lua_DoTerminateProcess(lua_State *L);

static int lua_HideProcessModule(lua_State *L);

static int lua_HideProcessMemory(lua_State *L);

static int lua_SetKeSpeed(lua_State *L);

static int lua_SetKeException(lua_State *L);

static int lua_Exit(lua_State *L);

static int lua_CreateProcess(lua_State *L);

static int lua_Statistics(lua_State *L);

static int lua_AddSeparateProcess(lua_State *L);

static int lua_DoSuspendProcess(lua_State *L);

static int lua_DoResumeProcess(lua_State *L);

static int lua_Simulate(lua_State *L);

static int lua_Shutdown(lua_State *L);

static int lua_HLogin(lua_State *L);

static int lua_Binding(lua_State *L);

static int lua_CheckThreadState(lua_State *L);

static int lua_HideKernelModule(lua_State *L);

static int lua_ModifyKernelMemory(lua_State *L);