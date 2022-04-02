// QueueUserAPC退出线程.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ConsoleDll.h"

VOID WINAPI APCFunc(ULONG_PTR dwParam)
{
	ExitThread(0);
}

CONSOLEDLL_API VOID TerminalThreadEx(HANDLE hThread)
{
	QueueUserAPC(APCFunc,hThread,NULL);
	Sleep(25);
}
