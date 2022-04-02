#pragma once
#include "afxwin.h"

class CCRMultiDocTemplate : public CMultiDocTemplate
{
public:
	CCRMultiDocTemplate(UINT nIDResource, CRuntimeClass* pDocClass,
		CRuntimeClass* pFrameClass, CRuntimeClass* pViewClass);
	~CCRMultiDocTemplate(void);

	virtual void SetDefaultTitle(CDocument* pDocument);
};
