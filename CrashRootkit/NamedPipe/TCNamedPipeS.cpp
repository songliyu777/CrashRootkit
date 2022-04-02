#include "stdafx.h"
#include "TCNamedPipeS.h"
#include "TCNamedPipeC.h"
#include "Log.h"

const char *pStrPipeNameS = "\\\\.\\pipe\\NamePipe_CRK_S";

HANDLE CTCNamedPipeS::m_hPipe = NULL;
CTCNamedPipeS * CTCNamedPipeS::m_instance = NULL;

CTCNamedPipeS g_TCNamedPipe;

CTCNamedPipeS::CTCNamedPipeS()
{
	m_instance = this;
	m_hPipe = NULL;
	m_hReceive = NULL;
	m_lpThreadId = 0;
	m_DispatchMessage = NULL;
}

CTCNamedPipeS * CTCNamedPipeS::GetInstance()
{
	return m_instance;
}

CTCNamedPipeS::~CTCNamedPipeS()
{
	if(m_hReceive)
	{
		TerminateThread(m_hReceive,0);
		m_hReceive = NULL;
	}
	if(m_hPipe)
	{
		DisconnectNamedPipe(m_hPipe);  
		CloseHandle(m_hPipe);
		m_hPipe = NULL;
	}
}

bool CTCNamedPipeS::CreateTCNamedPipe(DISPATCHTCMESSAGE DisaptchMessage)
{
	m_DispatchMessage = DisaptchMessage;
	m_hPipe = CreateNamedPipeA(pStrPipeNameS, PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES, 0, 0, NMPWAIT_WAIT_FOREVER, 0);
	if(!m_hPipe || m_hPipe == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	m_hReceive = CreateThread(NULL,0,ReceiveThread,this,0,&m_lpThreadId);
	if(!m_hReceive || m_hReceive == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	return true;
}

DWORD WINAPI CTCNamedPipeS::ReceiveThread(LPVOID lpThreadParameter)
{
	CTCNamedPipeS * pTCNamedPipeS = (CTCNamedPipeS *)lpThreadParameter;
	bool bClientConnect = CTCNamedPipeC::GetInstance()->CreateNamedPipeConnect();
	if(ConnectNamedPipe(m_hPipe, NULL))//等待连接。
	{
		PrintDbgString(L"[%d]客户端连接成功\r\n",GetCurrentProcessId());
		Sleep(100);
		if(!bClientConnect)
		{
			CTCNamedPipeC::GetInstance()->CreateNamedPipeConnect();//发送用通道怕阻塞所以特别用个发送的
		}
		if(pTCNamedPipeS->m_cscb)
		{
			pTCNamedPipeS->m_cscb();
		}
		while(true)
		{
			TCMessage msg;
			DWORD dwLen;  
			if(ReadFile(m_hPipe, &msg, sizeof(TCMessage), &dwLen, NULL))
			{
				PrintDbgString(L"处理的客户端消息[%d]\r\n",msg.MsgType);
				if(pTCNamedPipeS->m_DispatchMessage)
				{
					pTCNamedPipeS->m_DispatchMessage(&msg);
				}
			}
			else
			{
				PrintDbgString(L"[%d]客户端断开连接\r\n",GetCurrentProcessId());
				DisconnectNamedPipe(m_hPipe);  
				if(ConnectNamedPipe(m_hPipe, NULL))
				{
					PrintDbgString(L"[%d]客户端连接成功\r\n",GetCurrentProcessId());
					Sleep(100);
					CTCNamedPipeC::GetInstance()->CreateNamedPipeConnect();
					if(pTCNamedPipeS->m_cscb)
					{
						pTCNamedPipeS->m_cscb();
					}
					continue;
				}
			}
		}
	}
	return 0;
}

void CTCNamedPipeS::SetConnectSuccessCallBack(ConnectSuccessCallBack cscb)
{
	m_cscb = cscb;
}