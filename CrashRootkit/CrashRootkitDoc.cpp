
// CrashRootkitDoc.cpp : CCrashRootkitDoc ���ʵ��
//

#include "stdafx.h"
#include "CrashRootkit.h"
#include "CrashRootkitDoc.h"
#include "TaskManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCrashRootkitDoc

IMPLEMENT_DYNCREATE(CCrashRootkitDoc, CDocument)

BEGIN_MESSAGE_MAP(CCrashRootkitDoc, CDocument)
END_MESSAGE_MAP()


// CCrashRootkitDoc ����/����

CCrashRootkitDoc::CCrashRootkitDoc()
{
	// TODO: �ڴ����һ���Թ������

}

CCrashRootkitDoc::~CCrashRootkitDoc()
{
}

BOOL CCrashRootkitDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	CTaskManager::GetInstance()->AddDocument(this);
	// TODO: �ڴ�������³�ʼ������
	// (SDI �ĵ������ø��ĵ�)

	return TRUE;
}




// CCrashRootkitDoc ���л�

void CCrashRootkitDoc::Serialize(CArchive& ar)
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
void CCrashRootkitDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CCrashRootkitDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CCrashRootkitDoc ����

BOOL CCrashRootkitDoc::SaveModified()
{
	// TODO: �ڴ����ר�ô����/����û���

	return TRUE;//CDocument::SaveModified();
}
