#pragma once


// COutputView ��ͼ

class COutputView : public CEditView
{
	DECLARE_DYNCREATE(COutputView)

protected:
	COutputView();           // ��̬������ʹ�õ��ܱ����Ĺ��캯��
	virtual ~COutputView();

public:
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif
	
	void OnClearOutput();

public:
	void AddOutputString(CString str);
	afx_msg void OnUpdateUI(CCmdUI* pCui);
protected:
	DECLARE_MESSAGE_MAP()
};


