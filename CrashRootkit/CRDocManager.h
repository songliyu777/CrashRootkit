#pragma once
#include "afxwin.h"

class CCRDocManager : public CDocManager
{
public:
	CCRDocManager(void);
	~CCRDocManager(void);
	virtual void OnFileNew();
};
