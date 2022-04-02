#pragma once
#include "Transfer.h"
#include "CRConsole.h"
// CKernelModuleDoc 文档

class CKernelModuleDoc : public CDocument
{
	friend CRConsole;
public:
	CKernelModuleDoc();
	virtual ~CKernelModuleDoc();
	DECLARE_DYNCREATE(CKernelModuleDoc)
#ifndef _WIN32_WCE
	virtual void Serialize(CArchive& ar);   // 为文档 I/O 重写
#endif
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

	ModuleInfoVector & GetModuleInfoVector();
	void ClearModuleInfoVector();

protected:
	virtual BOOL OnNewDocument();

	DECLARE_MESSAGE_MAP()
protected:
	ModuleInfoVector m_miv;
};
