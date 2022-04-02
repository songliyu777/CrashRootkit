#pragma once

#include "Transfer.h"
#include "CRConsole.h"
// CIoTimerDoc �ĵ�

class CIoTimerDoc : public CDocument
{
	friend CRConsole;
	DECLARE_DYNCREATE(CIoTimerDoc)

public:
	CIoTimerDoc();
	virtual ~CIoTimerDoc();
#ifndef _WIN32_WCE
	virtual void Serialize(CArchive& ar);   // Ϊ�ĵ� I/O ��д
#endif
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

	IoTimerInfoVector & GetIoTimerInfoVector();

protected:
	virtual BOOL OnNewDocument();

	DECLARE_MESSAGE_MAP()
protected:
	IoTimerInfoVector m_itiv;
};
