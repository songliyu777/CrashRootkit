// CProgressCtrlEx ͷ?ļ?

#pragma once

// CProgressCtrlEx
#define PBS_GLIDING 0x8

class CProgressCtrlEx : public CProgressCtrl
{
	DECLARE_DYNAMIC(CProgressCtrlEx)

public:
	CProgressCtrlEx();
	virtual ~CProgressCtrlEx();

	typedef enum _ALIGN_TEXT_
	{
		ALIGN_LEFT = 0x00,
		ALIGN_CENTER,
		ALIGN_RIGHT
	}ALIGN_TEXT;

public:
	inline void SetBarColor(COLORREF clr)
	{
		m_clrBarColor = clr;
	}

	inline COLORREF GetBarColor()
	{
		return m_clrBarColor;
	}

	inline void SetBarBkColor(COLORREF clr)
	{
		m_clrBarBkColor = clr;
	}

	inline COLORREF GetBarBkColor()
	{
		return m_clrBarBkColor;
	}

	inline void SetTextColor(COLORREF clr)
	{
		m_clrTextColor = clr;
	}

	inline COLORREF GetTextColor()
	{
		return m_clrTextColor;
	}

	inline void SetTextBkColor(COLORREF clr)
	{
		m_clrTextBkColor = clr;
	}

	inline COLORREF GetTextBkColor()
	{
		return m_clrTextBkColor;
	}

	inline void SetTextAlign(ALIGN_TEXT AlignText)
	{
		m_AlignText = AlignText;
	}

	inline ALIGN_TEXT GetTextAlign()
	{
		return m_AlignText;
	}

	inline void ShowPercent(BOOL bShowPercent = TRUE)
	{
		m_bShowPercent = bShowPercent;
	}

	inline BOOL IsShowPercent()
	{
		return m_bShowPercent;
	}
	
	VOID SetStyle(DWORD style);
protected:
	COLORREF m_clrBarColor;
	COLORREF m_clrBarBkColor;
	COLORREF m_clrTextColor;
	COLORREF m_clrTextBkColor;
	ALIGN_TEXT m_AlignText;
	BOOL m_bShowPercent;
	CFont m_font; // Only for vertical style
	DWORD m_dstartpos;
	BOOL m_direction;

protected:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();

	DECLARE_MESSAGE_MAP()
};