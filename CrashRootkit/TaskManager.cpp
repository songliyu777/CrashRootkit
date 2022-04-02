#include "StdAfx.h"
#include "TaskManager.h"
#include "Task.h"
#include "ConsoleDll.h"
#include "ProgressDlg.h"

CTaskManager * CTaskManager::m_instance = NULL;

CTaskManager g_taskmanager;

CTaskManager::CTaskManager(void)
{
	assert(!m_instance);
	m_instance = this;
	m_lpThreadId = 0;
	m_hTaskManager = NULL;
	CreateTaskManagerThread();
}

CTaskManager::~CTaskManager(void)
{
	assert(m_instance);
	m_instance = NULL;
	if(m_hTaskManager)
	{
		TerminateThread(m_hTaskManager,0);
		CloseHandle(m_hTaskManager);
	}
	ClearTaskList();
	m_DocumentList.RemoveAll();
}

CTaskManager * CTaskManager::GetInstance()
{
	return m_instance;
}

DWORD CTaskManager::CreateTaskUI(CString sTask,LPTHREAD_START_ROUTINE lpStartAddress,LPVOID lpParameter,BOOL bUnknow)
{
#ifdef FASTEST
	CTask task(sTask,lpStartAddress,lpParameter,NULL);
	task.Start();
#else 
	CProgressDlg dlg;
	dlg.SetUnknow(bUnknow);
	m_lock.Lock(); 
	CTask task(sTask,lpStartAddress,lpParameter,&dlg);
	dlg.m_pTask = &task;
	m_TaskList.AddTail(&task);
	m_lock.Unlock();
	dlg.DoModal();
#endif
	return task.GetResult();

}

DWORD CTaskManager::CreateTask(CString sTask,LPTHREAD_START_ROUTINE lpStartAddress,LPVOID lpParameter)
{
	m_lock.Lock(); 
	CTask * pTask = new CTask(sTask,lpStartAddress,lpParameter,NULL);
	m_TaskList.AddTail(pTask);
	m_lock.Unlock();
	return pTask->GetResult();
}

BOOL CTaskManager::CheckTaskExist(CString sTask)
{
	BOOL Exist = FALSE;
	m_lock.Lock();
	POSITION pos = m_TaskList.GetHeadPosition();
	while(pos != NULL)   
	{   
		CTask* pTask = m_TaskList.GetNext(pos);
		if(!pTask->GetTaskName().Compare(sTask))
		{
			Exist = TRUE;
			break;
		}
	}
	m_lock.Unlock();
	return Exist;
}

DWORD WINAPI CTaskManager::ManagerThread(LPVOID lpThreadParameter)
{
	CTaskManager * pTaskManager = (CTaskManager *)lpThreadParameter;
	while(1)
	{
		pTaskManager->m_lock.Lock();
		POSITION pos = pTaskManager->m_TaskList.GetHeadPosition();
		while(pos != NULL)   
		{   
			CTask* pTask = pTaskManager->m_TaskList.GetNext(pos);
			if(!pTask->IsEnd() && pTask->Start())
			{
				if(pTask->HasUI())
				{
					pTask->Wait();
					pTask->End();
				}
			}
		}
		pTaskManager->ClearTaskList();
		pTaskManager->m_lock.Unlock();
		Sleep(10);
	}
	return 0;
}

VOID CTaskManager::CreateTaskManagerThread()
{
	m_hTaskManager = CreateThread(NULL,0,ManagerThread,this,0,&m_lpThreadId);
}

VOID CTaskManager::ClearTaskList()
{
	POSITION pos = m_TaskList.GetHeadPosition();
	POSITION pos2;
	while(pos != NULL)   
	{   
		pos2 = pos;
		CTask* pTask = m_TaskList.GetNext(pos);
		if(pTask->IsEnd())
		{
			m_TaskList.RemoveAt(pos2);
			if(!pTask->HasUI())
			{
				delete pTask;
			}
		}
	}
}

VOID CTaskManager::AddDocument(CDocument* pNewDoc)
{
	POSITION pos;
	pos = m_DocumentList.GetHeadPosition();
	while(pos != NULL)   
	{   
		CDocument* pDoc = m_DocumentList.GetNext(pos);
		if(pDoc==pNewDoc)
		{
			return;
		}
	}
	m_DocumentList.AddTail(pNewDoc);
}

CDocument* CTaskManager::GetDocument(const CRuntimeClass* pClass,POSITION * ppos)
{
	POSITION pos;
	if(ppos==NULL)
	{
		pos = m_DocumentList.GetHeadPosition();
	}
	else
	{
		if(*ppos==NULL)
		{
			pos = m_DocumentList.GetHeadPosition();
		}
		else
		{
			pos = *ppos;
		}
	}
	while(pos != NULL)   
	{   
		CDocument* pDoc = m_DocumentList.GetNext(pos);
		if(pDoc->IsKindOf(pClass))
		{
			if(ppos!=NULL)
				*ppos = pos;
			return pDoc;
		}
	}
	return NULL;
}


