#pragma once
#include "ProgressDlg.h"

class CTask
{
public:
	CTask(CString sTask,LPTHREAD_START_ROUTINE lpStartAddress,LPVOID lpParameter,CProgressDlg * lpProgressdlg = NULL);
	~CTask(void);
	BOOL Start();
	BOOL Stop();
	VOID Wait();
	VOID End();
	VOID SetValue(DWORD value);
	BOOL IsEnd();
	LPVOID GetParameter();
	CString GetTaskName();
	DWORD GetResult();
	BOOL HasUI();
protected:
	HANDLE m_hTask;
	DWORD m_lpThreadId;
	CString m_sTask;
	LPVOID m_lpParameter;
	LPTHREAD_START_ROUTINE m_lpStartAddress;
	BOOL m_bIsEnd;
	CProgressDlg * m_lpProgressdlg;
	DWORD m_result;
};
