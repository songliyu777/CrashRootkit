#pragma once

enum SimulateEvent
{
	MOUSEMOVE,
	MOUSELEFTCLICK,
	MOUSERIGHTCLICK,
	KEYPRESS,
	KEYRELEASE,
	KEYSMESSAGE
};

class CSimulate
{
public:
	CSimulate(void);
	~CSimulate(void);

	void MouseMove(int x, int y);
	void MouseLeftClick();
	void MouseRightClick();
	void KeyPress(WORD key);
	void KeyRelease(WORD key);
	void KeysMessage(CString msg);
private:
	void SendAscii(wchar_t data, BOOL shift);
	void SendUnicode(wchar_t data);
};
