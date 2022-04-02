#include "StdAfx.h"
#include "CRDocManager.h"

CCRDocManager::CCRDocManager(void)
{
}

CCRDocManager::~CCRDocManager(void)
{
}

void CCRDocManager::OnFileNew()
{
	if (m_templateList.IsEmpty())
	{
		TRACE(traceAppMsg, 0, "Error: no document templates registered with CWinApp.\n");
		AfxMessageBox(AFX_IDP_FAILED_TO_CREATE_DOC);
		return;
	}

	CDocTemplate* pTemplate = NULL;
	POSITION pos =  m_templateList.GetHeadPosition();
	while(pos!=NULL)
	{
		pTemplate = (CDocTemplate*)m_templateList.GetNext(pos);
		pTemplate->OpenDocumentFile(NULL);
	}

	ASSERT(pTemplate != NULL);
	ASSERT_KINDOF(CDocTemplate, pTemplate);

}