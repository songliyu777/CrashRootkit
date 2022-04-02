#pragma once

#include "Transfer.h"
#include "CRConsole.h"
// CDpcTimerDoc 文档

class CDpcTimerDoc : public CDocument
{
	friend CRConsole;
	DECLARE_DYNCREATE(CDpcTimerDoc)

public:
	CDpcTimerDoc();
	virtual ~CDpcTimerDoc();
#ifndef _WIN32_WCE
	virtual void Serialize(CArchive& ar);   // 为文档 I/O 重写
#endif
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

	DpcTimerInfoVector & GetDpcTimerInfoVector();

protected:
	virtual BOOL OnNewDocument();

	DECLARE_MESSAGE_MAP()
protected:
	DpcTimerInfoVector m_dtiv;
};
