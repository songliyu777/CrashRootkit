#pragma once
#include "Transfer.h"
#include "CRConsole.h"

class CSSDTDoc :
	public CDocument
{
	friend CRConsole;
protected: // 仅从序列化创建
	CSSDTDoc();
	DECLARE_DYNCREATE(CSSDTDoc)
	// 属性
	BOOL m_bIsShowAll;
	// 操作
public:
	BOOL IsSSDTShadow();
	BOOL IsShowAll();
	void SetShowMode(BOOL showall);
	// 重写
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

	SSDTInfoVector & GetSSDTInfoVector();

	// 实现
public:
	virtual ~CSSDTDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	SSDTInfoVector m_siv;
	// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()
};
