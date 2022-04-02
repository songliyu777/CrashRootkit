#include "StdAfx.h"
#include "TCServiceConsole.h"
#include "TCNamedPipeS.h"
#include "TCNamedPipeC.h"
#include "Log.h"
#include "Login.h"
#ifdef RELEASE_VERSION
#include "VMProtectSDK.h"
#endif

#include <string.h>
using namespace std;

TCServiceConsole * TCServiceConsole::m_instance = NULL;

TCServiceConsole g_TCServiceConsole;

WCHAR g_wTCMainPath[MAX_PATH] = {0};

TCServiceConsole::TCServiceConsole(void)
{
	m_instance = this;
}

TCServiceConsole::~TCServiceConsole(void)
{
}

TCServiceConsole * TCServiceConsole::GetInstance()
{
	return m_instance;
}

bool TCServiceConsole::StartService()
{
	CTCNamedPipeS::GetInstance()->SetConnectSuccessCallBack(&TCServiceConsole::ConnectSuccessCallBack);
	return CTCNamedPipeS::GetInstance()->CreateTCNamedPipe(&TCServiceConsole::DispatchTCMessage);	
}

BOOL WINAPI TCServiceConsole::DispatchTCMessage(PTCMessage msg)
{
#ifdef RELEASE_VERSION
	VMProtectBegin("DispatchTCMessage");
#endif
	switch(msg->MsgType)
	{
	case MSG_SCRIPT:
		{
			string script = (char *)msg->buff;
			script.resize(msg->size);
			if(CLogin::GetInstance()->RunScript(script.c_str()))
			{
				TCServiceConsole::GetInstance()->SendMessage(*msg);
			}
#ifdef RELEASE_VERSION
			CLogin::GetInstance()->RunScript("KdDisableDebugger();");
#endif

		}
		break;
	}
#ifdef RELEASE_VERSION
	VMProtectEnd();
#endif
	return TRUE;
}

BOOL WINAPI TCServiceConsole::ConnectSuccessCallBack()
{
	return TRUE;
}

BOOL TCServiceConsole::SendMessage(TCMessage msg)
{
	return CTCNamedPipeC::GetInstance()->SendData(msg);
}