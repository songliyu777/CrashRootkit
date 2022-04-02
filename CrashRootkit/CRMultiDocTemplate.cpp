#include "StdAfx.h"
#include "CRMultiDocTemplate.h"

CCRMultiDocTemplate::~CCRMultiDocTemplate(void)
{
}

CCRMultiDocTemplate::CCRMultiDocTemplate(UINT nIDResource, CRuntimeClass* pDocClass, CRuntimeClass* pFrameClass, CRuntimeClass* pViewClass)
	: CMultiDocTemplate(nIDResource, pDocClass, pFrameClass, pViewClass)
{

}

void CCRMultiDocTemplate::SetDefaultTitle(CDocument* pDocument)
{
	CString strDocName;
	if (GetDocString(strDocName, CDocTemplate::docName) &&
		!strDocName.IsEmpty())
	{
		//TCHAR szNum[16];
		//_stprintf_s(szNum, _countof(szNum), _T("%d"), m_nUntitledCount+1);
		//strDocName += szNum;
	}
	else
	{
		// use generic 'untitled' - ignore untitled count
		ENSURE(strDocName.LoadString(AFX_IDS_UNTITLED));
	}
	pDocument->SetTitle(strDocName);
}