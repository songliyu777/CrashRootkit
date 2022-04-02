#include "stdafx.h"
#include "TCNamedPipeC.h"
#include "Log.h"

const char *pStrPipeNameR = "\\\\.\\pipe\\NamePipe_CRK_C";

HANDLE CTCNamedPipeC::m_hPipe = NULL;
CTCNamedPipeC * CTCNamedPipeC::m_instance = NULL;

CTCNamedPipeC g_TCNamedPipe;

CTCNamedPipeC::CTCNamedPipeC()
{
	m_instance = this;
	m_hPipe = NULL;
	m_hReceive = NULL;
	m_lpThreadId = 0;
}


CTCNamedPipeC * CTCNamedPipeC::GetInstance()
{
	return m_instance;
}

CTCNamedPipeC::~CTCNamedPipeC()
{
	if(m_hReceive)
	{
		TerminateThread(m_hReceive,0);
		m_hReceive = NULL;
	}
	if(m_hPipe)
	{
		CloseHandle(m_hPipe);
		m_hPipe = NULL;
	}
}

bool CTCNamedPipeC::CreateNamedPipeConnect()
{
	if (!WaitNamedPipeA(pStrPipeNameR, NMPWAIT_WAIT_FOREVER))
	{
		PrintDbgString(L"[%d]连接服务端命名管道失败\r\n",GetCurrentProcessId());
		return false;
	}
	m_hPipe = CreateFileA(pStrPipeNameR, GENERIC_READ | GENERIC_WRITE, 0,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(!m_hPipe || m_hPipe==INVALID_HANDLE_VALUE)
	{
		PrintDbgString(L"[%d]打开服务端命名管道失败\r\n",GetCurrentProcessId());
		return false;
	}
	PrintDbgString(L"[%d]服务端连接成功\r\n",GetCurrentProcessId());
	return true;
}

BOOL CTCNamedPipeC::SendData(TCMessage msg)
{
	DWORD dwLen;
	return WriteFile(m_hPipe, &msg, sizeof(TCMessage), &dwLen, NULL);
}