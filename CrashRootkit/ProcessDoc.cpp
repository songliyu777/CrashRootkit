#include "StdAfx.h"
#include "ProcessDoc.h"
#include "ConsoleDll.h"
#include "Transfer.h"
#include "Task.h"
#include "TaskManager.h"

IMPLEMENT_DYNCREATE(CProcessDoc, CDocument)

BEGIN_MESSAGE_MAP(CProcessDoc, CDocument)
END_MESSAGE_MAP()

CProcessDoc::CProcessDoc(void)
{
	m_pCurrentProcessInfo = NULL;
}

CProcessDoc::~CProcessDoc(void)
{
	m_piv.clear();
	ClearModuleInfoVector();
	m_tiv.clear();
	ClearMemoryHookVector();
}

BOOL CProcessDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	CTaskManager::GetInstance()->AddDocument(this);
	CTaskManager::GetInstance()->CreateTaskUI(L"DoProcessesList",CRConsole::DoProcessesList,this);
	return TRUE;
}

ProcessInfoVector & CProcessDoc::GetProcessInfoVector()
{
	return m_piv;
}

ModuleInfoVector & CProcessDoc::GetModuleInfoVector()
{
	return m_miv;
}

ThreadInfoVector & CProcessDoc::GetThreadInfoVector()
{
	return m_tiv;
}

MemoryHookVector & CProcessDoc::GetMemoryHookVector()
{
	return m_mhv;
}

void CProcessDoc::ClearMemoryHookVector()
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

void CProcessDoc::ClearModuleInfoVector()
{
	ModuleInfoVector::iterator pos;
	for(pos=m_miv.begin();pos!=m_miv.end();pos++)
	{
		ModuleInfo mi = (*pos);
		if(mi.PEImage)
		{
			free(mi.PEImage);
		}
	}
	m_miv.clear();
}

void CProcessDoc::AddMemoryHook(PWCHAR ModulePathName,ULONG_PTR Address,PBYTE Origin,PBYTE Current,SIZE_T Length)
{
	MemoryHook mh;
	GetImageNameByPath(mh.ModuleName,ModulePathName);
	mh.Origin = (PBYTE)malloc(Length);
	memcpy(mh.Origin,Origin,Length);
	mh.Current = (PBYTE)malloc(Length);
	memcpy(mh.Current,Current,Length);
	mh.Address = Address;
	mh.Length = Length;
	m_mhv.push_back(mh);
}
// CProcessDoc 序列化

void CProcessDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: 在此添加存储代码
	}
	else
	{
		// TODO: 在此添加加载代码
	}
}


// CCrashRootkitDoc 诊断

#ifdef _DEBUG
void CProcessDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CProcessDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG