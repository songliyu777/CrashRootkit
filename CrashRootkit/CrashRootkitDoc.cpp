
// CrashRootkitDoc.cpp : CCrashRootkitDoc 类的实现
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


// CCrashRootkitDoc 构造/析构

CCrashRootkitDoc::CCrashRootkitDoc()
{
	// TODO: 在此添加一次性构造代码

}

CCrashRootkitDoc::~CCrashRootkitDoc()
{
}

BOOL CCrashRootkitDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	CTaskManager::GetInstance()->AddDocument(this);
	// TODO: 在此添加重新初始化代码
	// (SDI 文档将重用该文档)

	return TRUE;
}




// CCrashRootkitDoc 序列化

void CCrashRootkitDoc::Serialize(CArchive& ar)
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
void CCrashRootkitDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CCrashRootkitDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CCrashRootkitDoc 命令

BOOL CCrashRootkitDoc::SaveModified()
{
	// TODO: 在此添加专用代码和/或调用基类

	return TRUE;//CDocument::SaveModified();
}
