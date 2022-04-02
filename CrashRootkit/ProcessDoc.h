#pragma once
#include "afxwin.h"
#include "Transfer.h"
#include "CRConsole.h"

class CProcessDoc :
	public CDocument
{
	friend CRConsole;
public:
	CProcessDoc(void);
	virtual ~CProcessDoc(void);
	DECLARE_DYNCREATE(CProcessDoc)

public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

	ProcessInfoVector & GetProcessInfoVector();
	ModuleInfoVector & GetModuleInfoVector();
	ThreadInfoVector & GetThreadInfoVector();
	MemoryHookVector & GetMemoryHookVector();

	void ClearMemoryHookVector();
	void ClearModuleInfoVector();
	void AddMemoryHook(PWCHAR ModuleName,ULONG_PTR Address,PBYTE Origin,PBYTE Current,SIZE_T Length);
	// й╣ож
public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	ProcessInfoVector m_piv;
	ModuleInfoVector m_miv;
	ThreadInfoVector m_tiv;
	MemoryHookVector m_mhv;
	DECLARE_MESSAGE_MAP()
public:
	PProcessInfo m_pCurrentProcessInfo;
};
