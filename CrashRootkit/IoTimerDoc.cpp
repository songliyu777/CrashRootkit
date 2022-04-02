// IoTimerDoc.cpp : 实现文件
//

#include "stdafx.h"
#include "CrashRootkit.h"
#include "IoTimerDoc.h"
#include "TaskManager.h"

// CIoTimerDoc

IMPLEMENT_DYNCREATE(CIoTimerDoc, CDocument)

CIoTimerDoc::CIoTimerDoc()
{
}

BOOL CIoTimerDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	CTaskManager::GetInstance()->AddDocument(this);
	CTaskManager::GetInstance()->CreateTaskUI(L"DoIoTimerList",CRConsole::DoIoTimerList,this);
	return TRUE;
}

CIoTimerDoc::~CIoTimerDoc()
{
	m_itiv.clear();
}


BEGIN_MESSAGE_MAP(CIoTimerDoc, CDocument)
END_MESSAGE_MAP()


// CIoTimerDoc 诊断

#ifdef _DEBUG
void CIoTimerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

#ifndef _WIN32_WCE
void CIoTimerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif
#endif //_DEBUG

#ifndef _WIN32_WCE
// CIoTimerDoc 序列化

void CIoTimerDoc::Serialize(CArchive& ar)
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
#endif

IoTimerInfoVector & CIoTimerDoc::GetIoTimerInfoVector()
{
	return m_itiv;
}
// CIoTimerDoc 命令
