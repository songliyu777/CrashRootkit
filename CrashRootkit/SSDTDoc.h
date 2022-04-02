#pragma once
#include "Transfer.h"
#include "CRConsole.h"

class CSSDTDoc :
	public CDocument
{
	friend CRConsole;
protected: // �������л�����
	CSSDTDoc();
	DECLARE_DYNCREATE(CSSDTDoc)
	// ����
	BOOL m_bIsShowAll;
	// ����
public:
	BOOL IsSSDTShadow();
	BOOL IsShowAll();
	void SetShowMode(BOOL showall);
	// ��д
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

	SSDTInfoVector & GetSSDTInfoVector();

	// ʵ��
public:
	virtual ~CSSDTDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	SSDTInfoVector m_siv;
	// ���ɵ���Ϣӳ�亯��
protected:
	DECLARE_MESSAGE_MAP()
};
