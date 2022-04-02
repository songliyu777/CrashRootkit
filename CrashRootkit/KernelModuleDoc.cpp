// KernelModuleDoc.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "CrashRootkit.h"
#include "KernelModuleDoc.h"
#include "CRConsole.h"
#include "TaskManager.h"


// CKernelModuleDoc

IMPLEMENT_DYNCREATE(CKernelModuleDoc, CDocument)

CKernelModuleDoc::CKernelModuleDoc()
{
}

BOOL CKernelModuleDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	CTaskManager::GetInstance()->AddDocument(this);
	CTaskManager::GetInstance()->CreateTaskUI(L"DoKernelModulesList",CRConsole::DoKernelModulesList,this);
	return TRUE;
}

CKernelModuleDoc::~CKernelModuleDoc()
{
	ClearModuleInfoVector();
}


BEGIN_MESSAGE_MAP(CKernelModuleDoc, CDocument)
END_MESSAGE_MAP()


// CKernelModuleDoc ���

#ifdef _DEBUG
void CKernelModuleDoc::AssertValid() const
{
	CDocument::AssertValid();
}

#ifndef _WIN32_WCE
void CKernelModuleDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif
#endif //_DEBUG

#ifndef _WIN32_WCE
// CKernelModuleDoc ���л�

void CKernelModuleDoc::Serialize(CArchive& ar)
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


void CKernelModuleDoc::ClearModuleInfoVector()
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

ModuleInfoVector & CKernelModuleDoc::GetModuleInfoVector()
{
	return m_miv;
};

// CKernelModuleDoc ����
