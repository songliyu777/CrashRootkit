// QueueUserAPC�˳��߳�.cpp : �������̨Ӧ�ó������ڵ㡣
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
