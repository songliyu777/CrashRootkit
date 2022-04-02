// DpcTimerDoc.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "CrashRootkit.h"
#include "DpcTimerDoc.h"
#include "CRConsole.h"
#include "TaskManager.h"


// CDpcTimerDoc

IMPLEMENT_DYNCREATE(CDpcTimerDoc, CDocument)

CDpcTimerDoc::CDpcTimerDoc()
{
}

BOOL CDpcTimerDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	CTaskManager::GetInstance()->AddDocument(this);
	CTaskManager::GetInstance()->CreateTaskUI(L"DoDpcTimerList",CRConsole::DoDpcTimerList,this);
	return TRUE;
}

CDpcTimerDoc::~CDpcTimerDoc()
{
	m_dtiv.clear();
}


BEGIN_MESSAGE_MAP(CDpcTimerDoc, CDocument)
END_MESSAGE_MAP()


// CDpcTimerDoc ���

#ifdef _DEBUG
void CDpcTimerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

#ifndef _WIN32_WCE
void CDpcTimerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif
#endif //_DEBUG

#ifndef _WIN32_WCE
// CDpcTimerDoc ���л�

void CDpcTimerDoc::Serialize(CArchive& ar)
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
#endif


// CDpcTimerDoc ����

DpcTimerInfoVector & CDpcTimerDoc::GetDpcTimerInfoVector()
{
	return m_dtiv;
}
