#pragma once

#ifdef _WIN32_WCE
#error "Windows CE ��֧�� CMDIChildWnd��"
#endif 

// ���в������ CSplitterFrame ���

class CSplitterFrame : public CMDIChildWndEx
{
	DECLARE_DYNCREATE(CSplitterFrame)
protected:
	CSplitterFrame();           // ��̬������ʹ�õ��ܱ����Ĺ��캯��
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


