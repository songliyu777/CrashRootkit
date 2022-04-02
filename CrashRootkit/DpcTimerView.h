#pragma once
#include "afxcmn.h"
#include "DpcTimerDoc.h"


// CDpcTimerView ������ͼ

class CDpcTimerView : public CFormView
{
	DECLARE_DYNCREATE(CDpcTimerView)

protected:
	CDpcTimerView();           // ��̬������ʹ�õ��ܱ����Ĺ��캯��
	virtual ~CDpcTimerView();

public:
	enum { IDD = IDD_DPCTIMERVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif
	CDpcTimerDoc* GetDocument() const;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()

	afx_msg void OnContextMenu(CPoint point);
	
public:
	CListCtrl m_cDpcTimerTable;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnInitialUpdate();
	void InitTable();
	afx_msg void OnNMRClickDpctimertable(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnUpdateUI(CCmdUI* pCui);
	afx_msg void OnRefresh();
	afx_msg void OnKillDpcTimer();
};


