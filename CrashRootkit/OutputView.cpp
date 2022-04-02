// OutputView.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "CrashRootkit.h"
#include "OutputView.h"
#include "LuaEngine.h"

// COutputView

IMPLEMENT_DYNCREATE(COutputView, CEditView)

COutputView::COutputView()
{
	CLuaEngine::GetInstance()->m_outputview = this;
}

COutputView::~COutputView()
{
}

BEGIN_MESSAGE_MAP(COutputView, CEditView)
	ON_COMMAND(ID_CLEAROUTPUT, &COutputView::OnClearOutput)
	ON_UPDATE_COMMAND_UI(ID_CLEAROUTPUT, &COutputView::OnUpdateUI)
END_MESSAGE_MAP()


// COutputView ���

#ifdef _DEBUG
void COutputView::AssertValid() const
{
	CEditView::AssertValid();
}

#ifndef _WIN32_WCE
void COutputView::Dump(CDumpContext& dc) const
{
	CEditView::Dump(dc);
}
#endif
#endif //_DEBUG

void COutputView::OnClearOutput()
{
	DeleteContents();
}

void COutputView::AddOutputString(CString str)
{
	CString ostr;
	GetWindowText(ostr);
	ostr += str;
	int iPos = 0; //�ָ��λ��     
	int iNums = 0; //�ָ��������     
	CString strTemp = ostr;
	while (iPos != -1)     
	{         
		iPos = strTemp.ReverseFind(L'\n');  
		if (iPos == -1)         
		{
			strTemp = ostr;
			break;
		}
		iNums++;
		if(iNums>100)
		{
			strTemp = ostr.Mid(iPos);
			break;
		}
		strTemp.Delete(iPos,1);
	} 
	SetWindowText(strTemp);
	GetEditCtrl().LineScroll(GetEditCtrl().GetLineCount());
}

void COutputView::OnUpdateUI(CCmdUI* pCui)
{
	pCui->Enable(TRUE);
}
// COutputView ��Ϣ�������
