
// CrashRootkitDoc.h : CCrashRootkitDoc ��Ľӿ�
//


#pragma once


class CCrashRootkitDoc : public CDocument
{
protected: // �������л�����
	CCrashRootkitDoc();
	DECLARE_DYNCREATE(CCrashRootkitDoc)

// ����
public:

// ����
public:

// ��д
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// ʵ��
public:
	virtual ~CCrashRootkitDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// ���ɵ���Ϣӳ�亯��
protected:
	DECLARE_MESSAGE_MAP()
	virtual BOOL SaveModified();
};


