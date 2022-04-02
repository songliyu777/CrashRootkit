#include "StdAfx.h"
#include "KernelHookDoc.h"
#include "ConsoleDll.h"
#include "Task.h"
#include "TaskManager.h"

IMPLEMENT_DYNCREATE(CKernelHookDoc, CDocument)

BEGIN_MESSAGE_MAP(CKernelHookDoc, CDocument)
END_MESSAGE_MAP()

CKernelHookDoc::CKernelHookDoc(void)
{
}

CKernelHookDoc::~CKernelHookDoc(void)
{
	FreeMemoryHookVector();
}

void CKernelHookDoc::FreeMemoryHookVector()
{
	MemoryHookVector::iterator pos;
	for(pos=m_mhv.begin();pos!=m_mhv.end();pos++)
	{
		MemoryHook mh = (*pos);
		if(mh.Origin)
		{
			free(mh.Origin);
		}
		if(mh.Current)
		{
			free(mh.Current);
		}
	}
	m_mhv.clear();
}

MemoryHookVector & CKernelHookDoc::GetMemoryHookVector()
{
	return m_mhv;
}

BOOL CKernelHookDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	CTaskManager::GetInstance()->AddDocument(this);
	CTaskManager::GetInstance()->CreateTaskUI(L"DoKernelHookCheck",CRConsole::DoKernelHookCheck,this);

	return TRUE;
}

// CCrashRootkitDoc ���л�

void CKernelHookDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: �ڴ���Ӵ洢����
	}
	else
	{
		// TODO: �ڴ���Ӽ��ش���
	}
}


// CCrashRootkitDoc ���

#ifdef _DEBUG
void CKernelHookDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CKernelHookDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

