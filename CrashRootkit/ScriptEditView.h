#pragma once


// CScriptEditView 视图

class CScriptEditView : public CEditView
{
	DECLARE_DYNCREATE(CScriptEditView)

protected:
	CScriptEditView();           // 动态创建所使用的受保护的构造函数
	virtual ~CScriptEditView();

public:
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif
	void OnRunScript();
	afx_msg void OnUpdateUI(CCmdUI* pCui);
protected:
	DECLARE_MESSAGE_MAP()
};


