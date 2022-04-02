#pragma once
#include "afxwin.h"
#include "Transfer.h"
#include "Task.h"
#include "CRConsole.h"

class CKernelHookDoc :
	public CDocument
{
	friend CRConsole;
public:
	CKernelHookDoc(void);
	virtual~CKernelHookDoc(void);
	DECLARE_DYNCREATE(CKernelHookDoc)

	MemoryHookVector & GetMemoryHookVector();
	void FreeMemoryHookVector();

public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

	// ʵ��
public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	MemoryHookVector m_mhv;
	// ���ɵ���Ϣӳ�亯��
protected:
	DECLARE_MESSAGE_MAP()
};
