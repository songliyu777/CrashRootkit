#pragma once
#include "LoginDialog.h"
//#ifndef RELEASE_VERSION
#define YYF001
//#endif
class CLogin
{
	CLoginDialog * pdlg;
	static CLogin * m_instance;
public:
	CLogin(void);
	~CLogin(void);
	static CLogin * CLogin::GetInstance();
	void Init();
	bool ShowLoginDialog();
	bool ValidateRegister();
	bool RunScript(const char * script);
	bool Statistics(const char * GameAccount,const char * RoleName,double Money);
	bool Hack();
	bool Binding(bool binding);
};
#ifdef SAMUELSONG
#define ACCOUNT		"x12xcx18x10x9x1fx1cx1x9x67"
#define PASSWORD	"x3x1x52x3x1x7x1x3x36"
#endif
#ifdef YYF000
#define ACCOUNT		"x0x1fx56x0x0x30"
#define PASSWORD	"x3x1x53x0x0x30"
#endif
#ifdef YYF001
#define ACCOUNT		"x0x1fx56x0x1x31"
#define PASSWORD	"x3x1x53x0x1x31"
#endif
#ifdef YYF002
#define ACCOUNT		"x0x1fx56x0x2x32"
#define PASSWORD	"x3x1x53x0x2x32"
#endif
#ifdef YYF003
#define ACCOUNT		"x0x1fx56x0x3x33"
#define PASSWORD	"x3x1x53x0x3x33"
#endif
#ifdef YYF004
#define ACCOUNT		"x0x1fx56x0x4x34"
#define PASSWORD	"x3x1x53x0x4x34"
#endif
#ifdef YYF005
#define ACCOUNT		"x0x1fx56x0x5x35"
#define PASSWORD	"x3x1x7x55x3x1x7x34"
#endif
#ifdef YYF006
#define ACCOUNT		"x0x1fx56x0x6x36"
#define PASSWORD	"x3x1x7x55x3x1x33"
#endif
#ifdef YYF008
#define ACCOUNT		"x0x1fx56x0x8x38"
#define PASSWORD	"x3x1x53x0x8x38"
#endif
#ifdef YYF009
#define ACCOUNT		"x0x1fx56x0x9x39"
#define PASSWORD	"x3x1x53x0x9x39"
#endif
#ifdef YYF010
#define ACCOUNT		"x0x1fx56x1x1x30"
#define PASSWORD	"x3x1x53x1x1x30"
#endif


