#pragma once

#ifdef _WIN32_WCE
#error "Windows CE 不支持 CMDIChildWnd。"
#endif 

// 带有拆分器的 CSplitterFrame 框架

class CSplitterFrame : public CMDIChildWndEx
{
	DECLARE_DYNCREATE(CSplitterFrame)
protected:
	CSplitterFrame();           // 动态创建所使用的受保护的构造函数
	virtual ~CSplitterFrame();
	
	CSplitterWnd m_wndSplitter;
	BOOL m_bIsCreate;
protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);

	DECLARE_MESSAGE_MAP()

	afx_msg void OnSize(UINT nType, int cx, int cy);
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};


