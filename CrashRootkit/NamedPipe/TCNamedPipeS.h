#pragma once

#include "TCNamedPipeMessage.h"

typedef BOOL (WINAPI * ConnectSuccessCallBack)();

typedef BOOL (WINAPI * DISPATCHTCMESSAGE)(PTCMessage pMsg);

class CTCNamedPipeS
{
	static CTCNamedPipeS * m_instance;
	ConnectSuccessCallBack m_cscb;
	DISPATCHTCMESSAGE m_DispatchMessage;
private:
	static HANDLE m_hPipe;
	HANDLE m_hReceive;
	DWORD m_lpThreadId;
public:
	CTCNamedPipeS();
	~CTCNamedPipeS();
	static CTCNamedPipeS * GetInstance();
	bool CreateTCNamedPipe(DISPATCHTCMESSAGE DisaptchMessage);
	static DWORD WINAPI ReceiveThread(LPVOID lpThreadParameter);
	void SetConnectSuccessCallBack(ConnectSuccessCallBack cscb);
};