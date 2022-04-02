#include "StdAfx.h"
#include "Task.h"
#include "ConsoleDll.h"

CTask::CTask(CString sTask,LPTHREAD_START_ROUTINE lpStartAddress,LPVOID lpParameter,CProgressDlg * lpProgressdlg)
{
	m_hTask = NULL;
	m_sTask = sTask;
	m_lpStartAddress = lpStartAddress;
	m_lpThreadId = 0;
	m_lpParameter = lpParameter;
	m_lpProgressdlg = lpProgressdlg;
	m_bIsEnd = FALSE;
	m_result = 0;
}

CTask::~CTask(void)
{

}

BOOL CTask::Start()
{
#ifdef FASTEST
	m_lpStartAddress(this);
	return TRUE;
#else
	if(m_hTask)
	{
		return TRUE;
	}
	m_hTask = CreateThread(NULL,0,m_lpStartAddress,this,0,&m_lpThreadId);
	if(m_hTask)
	{
		return TRUE;
	}
	return FALSE
#endif

}

BOOL CTask::Stop()
{
	if(m_hTask && !m_bIsEnd)
	{
		TerminateThread(m_hTask,0);
		CloseHandle(m_hTask);
		m_bIsEnd = TRUE;
		return TRUE;
	}
	return FALSE;
}

VOID CTask::End()
{
	if(m_hTask)
	{
		CloseHandle(m_hTask);
	}
	m_bIsEnd = TRUE;
}

VOID CTask::Wait()
{
	WaitForSingleObject(m_hTask,INFINITE);
	GetExitCodeThread(m_hTask,&m_result);
}

LPVOID CTask::GetParameter()
{
	return m_lpParameter;
}

CString CTask::GetTaskName()
{
	return m_sTask;
}

VOID CTask::SetValue(DWORD value)
{
	if(m_lpProgressdlg)
	{
		while(!m_lpProgressdlg->GetSafeHwnd())
		{
			Sleep(10);
		}
		m_lpProgressdlg->m_value = value;
	}
}

BOOL CTask::IsEnd()
{
	return m_bIsEnd;
}

DWORD CTask::GetResult()
{
	return m_result;
}

BOOL CTask::HasUI()
{
	return m_lpProgressdlg ? true : false;
}