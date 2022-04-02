#pragma once

#include "TCNamedPipeMessage.h"

class TCServiceConsole
{
	static TCServiceConsole * m_instance;
public:
	TCServiceConsole(void);
	~TCServiceConsole(void);

	static TCServiceConsole * GetInstance();
	
	bool StartService();

	static BOOL WINAPI DispatchTCMessage(PTCMessage msg);

	static BOOL WINAPI ConnectSuccessCallBack();

	BOOL SendMessage(TCMessage msg);
};
