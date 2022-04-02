#include "StdAfx.h"
#include "SSDTDoc.h"
#include "TaskManager.h"
#include "Transfer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCrashRootkitDoc

IMPLEMENT_DYNCREATE(CSSDTDoc, CDocument)

BEGIN_MESSAGE_MAP(CSSDTDoc, CDocument)
END_MESSAGE_MAP()

CSSDTDoc::CSSDTDoc(void)
{
	m_bIsShowAll = FALSE;
}

CSSDTDoc::~CSSDTDoc(void)
{
	m_siv.clear();
}

BOOL CSSDTDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	CTaskManager::GetInstance()->AddDocument(this);
	// TODO: 在此添加重新初始化代码
	// (SDI 文档将重用该文档)
	if(IsSSDTShadow())
	{
		CTaskManager::GetInstance()->CreateTaskUI(L"DoSSDTShadowList",CRConsole::DoSSDTShadowList,this);
	}
	else
	{
		CTaskManager::GetInstance()->CreateTaskUI(L"DoSSDTList",CRConsole::DoSSDTList,this);
	}

	return TRUE;
}


BOOL CSSDTDoc::IsSSDTShadow()
{
	if(GetTitle().Compare(_T("SSDT")))
	{
		return TRUE;
	}
	return FALSE;
}

BOOL CSSDTDoc::IsShowAll()
{
	return m_bIsShowAll;
}

void CSSDTDoc::SetShowMode(BOOL showall)
{
	m_bIsShowAll = showall;
}
// CCrashRootkitDoc 序列化

void CSSDTDoc::Serialize(CArchive& ar)
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
void CSSDTDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CSSDTDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CCrashRootkitDoc 命令

SSDTInfoVector & CSSDTDoc::GetSSDTInfoVector()
{
	return m_siv;
}