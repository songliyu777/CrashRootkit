#include "StdAfx.h"
#include "Simulate.h"
#include "ConsoleDll.h"

CSimulate::CSimulate(void)
{
}

CSimulate::~CSimulate(void)
{
}

void CSimulate::MouseMove(int x, int y)
{
	INPUT input;
	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;
	input.mi.dx = 65535 * x / (GetSystemMetrics(SM_CXSCREEN)-1) ;
	input.mi.dy = 65535 * y / (GetSystemMetrics(SM_CYSCREEN)-1) ;
	input.mi.time = GetTickCount();
	input.mi.dwExtraInfo = GetMessageExtraInfo();
	SendInputEx(1,&input,sizeof(INPUT));
}

void CSimulate::MouseLeftClick()
{
	INPUT input[2];
	input[0].type = INPUT_MOUSE;
	input[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	input[0].mi.dx = 0;
	input[0].mi.dy = 0;
	input[0].mi.mouseData = 0;
	input[0].mi.time = GetTickCount();
	input[0].mi.dwExtraInfo = GetMessageExtraInfo();
	SendInputEx(1,input,sizeof(INPUT));
	Sleep(500);
	input[1].type = INPUT_MOUSE;
	input[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
	input[1].mi.dx = 0;
	input[1].mi.dy = 0;
	input[1].mi.mouseData = 0;
	input[1].mi.time = GetTickCount();
	input[1].mi.dwExtraInfo = GetMessageExtraInfo();
	SendInputEx(1,input+1,sizeof(INPUT));
}

void CSimulate::MouseRightClick()
{
	INPUT input[2];
	input[0].type = INPUT_MOUSE;
	input[0].mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
	input[0].mi.dx = 0;
	input[0].mi.dy = 0;
	input[0].mi.mouseData = 0;
	input[0].mi.time = GetTickCount();
	input[0].mi.dwExtraInfo = GetMessageExtraInfo();
	SendInputEx(1,input,sizeof(INPUT));
	Sleep(500);
	input[1].type = INPUT_MOUSE;
	input[1].mi.dwFlags = MOUSEEVENTF_RIGHTUP;
	input[1].mi.dx = 0;
	input[1].mi.dy = 0;
	input[1].mi.mouseData = 0;
	input[1].mi.time = GetTickCount();
	input[1].mi.dwExtraInfo = GetMessageExtraInfo();
	SendInputEx(1,input+1,sizeof(INPUT));
}

void CSimulate::KeyPress(WORD key)
{
	INPUT input;
	input.type = INPUT_KEYBOARD;
	input.ki.wVk = key;
	input.ki.dwFlags = KEYEVENTF_EXTENDEDKEY;
	SendInputEx(1,&input,sizeof(INPUT));
}

void CSimulate::KeyRelease(WORD key)
{
	INPUT input;
	input.type = INPUT_KEYBOARD;
	input.ki.wVk = key;
	input.ki.dwFlags = KEYEVENTF_KEYUP;
	SendInputEx(1,&input,sizeof(INPUT));
}

void CSimulate::KeysMessage(CString msg)
{
	short vk;
	BOOL shift;

	USES_CONVERSION;
	wchar_t* data = T2W(msg.GetBuffer(0));
	int len = wcslen(data);

	for(int i=0;i<len;i++)
	{
		if (data[i]>=0 && data[i]<256) //ascii×Ö·û
		{
			vk = VkKeyScanW(data[i]);

			if (vk == -1)
			{
				SendUnicode(data[i]);
			}
			else
			{
				if (vk < 0)
				{
					vk = ~vk + 0x1;
				}

				shift = vk >> 8 & 0x1;

				if (GetKeyState(VK_CAPITAL) & 0x1)
				{
					if (data[i]>='a' && data[i]<='z' || data[i]>='A' && data[i]<='Z')
					{
						shift = !shift;
					}
				}

				SendAscii(vk & 0xFF, shift);
			}
		}
		else //unicode×Ö·û
		{
			SendUnicode(data[i]);
		}
	}
}

void CSimulate::SendAscii(wchar_t data, BOOL shift)
{
	INPUT input[2];
	memset(input, 0, 2 * sizeof(INPUT));

	//if (shift)
	//{
	//	input[0].type = INPUT_KEYBOARD;
	//	input[0].ki.wVk = VK_SHIFT;
	//	input[0].ki.dwFlags = KEYEVENTF_EXTENDEDKEY;
	//	SendInputEx(1, input, sizeof(INPUT));
	//	Sleep(500);
	//}

	input[0].type = INPUT_KEYBOARD;
	input[0].ki.wVk = data;
	input[0].ki.dwFlags = KEYEVENTF_EXTENDEDKEY;
	SendInputEx(1, input, sizeof(INPUT));

	Sleep(500);

	input[1].type = INPUT_KEYBOARD;
	input[1].ki.wVk = data;
	input[1].ki.dwFlags = KEYEVENTF_KEYUP;
	SendInputEx(1, input+1, sizeof(INPUT));

	//if (shift)
	//{
	//	Sleep(500);
	//	input[0].type = INPUT_KEYBOARD;
	//	input[0].ki.wVk = VK_SHIFT;
	//	input[0].ki.dwFlags = KEYEVENTF_KEYUP;
	//	SendInputEx(1, input, sizeof(INPUT));  
	//}
}

void CSimulate::SendUnicode(wchar_t data)
{
	INPUT input[2];
	memset(input, 0, 2 * sizeof(INPUT));

	input[0].type = INPUT_KEYBOARD;
	input[0].ki.wVk = 0;
	input[0].ki.wScan = data;
	input[0].ki.time = GetTickCount();
	input[0].ki.dwFlags = 0x4;//KEYEVENTF_UNICODE;
	Sleep(500);
	input[1].type = INPUT_KEYBOARD;
	input[1].ki.wVk = 0;
	input[1].ki.wScan = data;
	input[1].ki.time = GetTickCount();
	input[1].ki.dwFlags = KEYEVENTF_KEYUP | 0x4;//KEYEVENTF_UNICODE;

	SendInputEx(2, input, sizeof(INPUT));
}