// ScriptEditView.cpp : 实现文件
//

#include "stdafx.h"
#include "CrashRootkit.h"
#include "ScriptEditView.h"
#include "LuaEngine.h"
#include "Log.h"


// CScriptEditView

IMPLEMENT_DYNCREATE(CScriptEditView, CEditView)

CScriptEditView::CScriptEditView()
{
	CLuaEngine::GetInstance()->m_scriptview = this;
}

CScriptEditView::~CScriptEditView()
{
}

BEGIN_MESSAGE_MAP(CScriptEditView, CEditView)
	ON_COMMAND(ID_RUNSCRIPT, &CScriptEditView::OnRunScript)
	ON_UPDATE_COMMAND_UI(ID_RUNSCRIPT, &CScriptEditView::OnUpdateUI)
END_MESSAGE_MAP()


// CScriptEditView 诊断

#ifdef _DEBUG
void CScriptEditView::AssertValid() const
{
	CEditView::AssertValid();
}

#ifndef _WIN32_WCE
void CScriptEditView::Dump(CDumpContext& dc) const
{
	CEditView::Dump(dc);
}
#endif
#endif //_DEBUG

void CScriptEditView::OnRunScript()
{
	GetWindowTextA(this->m_hWnd,CLuaEngine::GetInstance()->GetScriptBuff(),MAXSCRIPTBUFF);
	CLuaEngine::GetInstance()->RunScript();
}

void CScriptEditView::OnUpdateUI(CCmdUI* pCui)
{
	pCui->Enable(TRUE);
}
// CScriptEditView 消息处理程序
