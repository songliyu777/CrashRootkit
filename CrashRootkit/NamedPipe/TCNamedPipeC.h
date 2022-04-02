#pragma once

#include "TCNamedPipeMessage.h"

class CTCNamedPipeC
{
	static CTCNamedPipeC * m_instance;
private:
	static HANDLE m_hPipe;
	HANDLE m_hReceive;
	DWORD m_lpThreadId;
public:
	CTCNamedPipeC();
	~CTCNamedPipeC();
	static CTCNamedPipeC * GetInstance();
	bool CreateNamedPipeConnect();
	BOOL SendData(TCMessage msg);
};