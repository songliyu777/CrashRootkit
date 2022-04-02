#pragma once
#include "Task.h"
#include "Util.h"

class CTaskManager
{
	static CTaskManager * m_instance;
public:
	CTaskManager(void);
	virtual ~CTaskManager(void);

	static CTaskManager * CTaskManager::GetInstance();

	DWORD CreateTaskUI(CString sTask,LPTHREAD_START_ROUTINE lpStartAddress,LPVOID lpParameter,BOOL bUnknow = TRUE);

	DWORD CreateTask(CString sTask,LPTHREAD_START_ROUTINE lpStartAddress,LPVOID lpParameter);

	BOOL CheckTaskExist(CString sTask);

	VOID AddDocument(CDocument* pNewDoc);
	CDocument* GetDocument(const CRuntimeClass* pClass,POSITION * ppos = NULL);
protected:
	CLockUtil m_lock;
	CList<CTask*,CTask*> m_TaskList;
	CList<CDocument*,CDocument*> m_DocumentList;
	DWORD m_lpThreadId;
	HANDLE m_hTaskManager;
	static DWORD WINAPI ManagerThread(LPVOID lpThreadParameter);
	VOID CreateTaskManagerThread();
	VOID ClearTaskList();
};
