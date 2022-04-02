#include "stdafx.h"
#include "resource.h"
#include "FlashDialog.h"
#include "process.h"
#include "ocidl.h"
#include "olectl.h"

#pragma warning(disable:4996)

HINSTANCE g_hInst = NULL;
HWND g_hDlg = NULL;
WCHAR szDriverImagePath[MAX_PATH] = {0};
int g_ProcValue = 0;
CHAR m_status[36] = {0};
DWORD g_dLanguage = 1;//0英语1中文 暂时这么弄的

void CenterDialog(HWND hDlg)
{
	HDC hdc;
	RECT rect;
	GetClientRect(hDlg,&rect);
	hdc = GetDC(GetDesktopWindow());
	int width = GetDeviceCaps(hdc,HORZRES);
	int height = GetDeviceCaps(hdc,VERTRES);
	int cliendwidth = (rect.right - rect.left);
	int cliendheight = (rect.bottom - rect.top);
	int x = (width - cliendwidth)/2;
	int y = (height - cliendheight)/2;
	MoveWindow(hDlg,x,y,cliendwidth,cliendheight,TRUE);
	SetWindowLong(hDlg,GWL_EXSTYLE, GetWindowLong(hDlg,GWL_EXSTYLE)^0x8000000); 
}

VOID CALLBACK TimerProc(
						HWND hwnd,
						UINT uMsg,
						UINT_PTR idEvent,
						DWORD dwTime
						)
{
	SendMessage(hwnd,WM_PAINT,0,0);
}

INT_PTR CALLBACK Flash(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		g_hDlg = hDlg;
		CenterDialog(hDlg);
		SetTimer(hDlg,0,25,TimerProc);
		return (INT_PTR)TRUE;
	case WM_PAINT:
		hdc = BeginPaint(hDlg, &ps);
		ShowPicture();
		EndPaint(hDlg, &ps);
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

void Show(void* param)
{
	DialogBox(g_hInst, MAKEINTRESOURCE(IDD_FLASH_DLG), NULL, Flash);
}

void ShowFlashDialog()
{
	_beginthread(Show,0,NULL);
}

void SetHistance(HINSTANCE hInst)
{
	g_hInst = hInst;
	InitPicturePath();
}

void CloseFlashDialog()
{
	if(g_hDlg)
		EndDialog(g_hDlg, IDOK);
}

VOID InitPicturePath()
{
	GetModuleFileName(NULL,szDriverImagePath,MAX_PATH); //得到当前模块路径
	for (int i=wcslen(szDriverImagePath)-1;i>=0;i--)
	{
		if (L'\\'==szDriverImagePath[i])
		{
			szDriverImagePath[i+1]='\0';
			break;
		}
	}

	wcscat(szDriverImagePath,L"Logo.jpg");
}


void InitStatusStr()
{
	memset(m_status,0,36);
	sprintf(m_status,"%s","更新中...");
}

void ShowPicture()
{
	HANDLE hFile = NULL;
	HDC hdc;
	DWORD dwFileSize,dwByteRead;
	BOOL bResult;
	RECT flashRect;
	RECT procRect;
	RECT textRect;
	HBRUSH hBrush;
	IPicture *pPic;
	IStream *pStm;
	LOGFONTA fontRect;

	HWND hFlash = GetDlgItem(g_hDlg,IDC_STATIC_FLASH);
	hdc = GetDC(hFlash);
	hFile = CreateFile(szDriverImagePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if(hFile!=INVALID_HANDLE_VALUE)
	{
		dwFileSize = GetFileSize(hFile,NULL);
		if(dwFileSize==0xffffffff)
		{
			return;
		}
	}

	HGLOBAL hGloable = GlobalAlloc(GMEM_MOVEABLE,dwFileSize);
	LPVOID pvData;

	if(hGloable == NULL)
	{
		return;
	}

	pvData = GlobalLock(hGloable);
	if(pvData == NULL)
	{
		return;
	}

	if(!ReadFile(hFile,pvData,dwFileSize,&dwByteRead,NULL))
	{
		return;
	}

	GlobalUnlock(hGloable);

	if(CreateStreamOnHGlobal(hGloable,TRUE,&pStm)!=S_OK)
	{
		return;
	}

	bResult = OleLoadPicture(pStm,dwFileSize,TRUE,IID_IPicture,(LPVOID *)&pPic);

	if(FAILED(bResult))
	{
		return;
	}

	GetClientRect(hFlash,&flashRect);


	OLE_XSIZE_HIMETRIC	hmHeight;
	OLE_XSIZE_HIMETRIC	hmWidth;
	pPic->get_Height(&hmHeight);
	pPic->get_Width(&hmWidth);

	LONG height = flashRect.bottom - flashRect.top;
	LONG width = flashRect.right - flashRect.left;

	HDC hMemDC = CreateCompatibleDC(hdc);     //创建内存DC
	HBITMAP hBitmap=::CreateCompatibleBitmap(hdc,width,height);//创建位图
	::SelectObject(hMemDC,hBitmap);   //把位图选进内存DC

	bResult = pPic->Render(hMemDC,-1,-1,width+1,height+1,0,hmHeight,hmWidth,-hmHeight,NULL);
	CloseHandle(hFile);
	pPic->Release();
	GlobalFree(hGloable);

	int bDist = 15;

	RoundRect(hMemDC,20,height - 40 + bDist,width - 20,height - 20 + bDist, 5, 5);

	procRect.left = 20 + 2;
	procRect.top = height - 40 + 2 + bDist;
	LONG proclen = g_ProcValue*(width - 42)/100;
	procRect.right = 20 + 2 + proclen;
	procRect.bottom = height - 20 -2 + bDist;

	hBrush = CreateSolidBrush(RGB(22,44,88));

	COLORREF cof1 = RGB(0,88,166);
	COLORREF cof2 = RGB(166,200,255);

	DrawGradientV(hMemDC,cof1,cof2,procRect);

	ZeroMemory(&fontRect,sizeof(LOGFONTA));
	fontRect.lfHeight=-14;						//字体的高度
	fontRect.lfWeight=FW_THIN;					//字体的粗细
	lstrcpyA(fontRect.lfFaceName,"宋体");

	HFONT hFont=CreateFontIndirectA(&fontRect); //创建字体
	HFONT hOld=(HFONT)SelectObject(hMemDC,hFont);//引用上面的字体


	SetBkMode(hMemDC,TRANSPARENT);
	SetTextColor(hMemDC,RGB(255,255,255));
	InitStatusStr();

	procRect.top -= 15;
	procRect.bottom -= 15;

	textRect = procRect;
	textRect.right = width - 42;

	DrawTextA(hMemDC,m_status,36,&textRect,0);

	procRect.top += 15;
	procRect.bottom += 15;

	if(g_ProcValue<55)
	{
		SetTextColor(hMemDC,RGB(0,88,166));
	}
	else
	{
		SetTextColor(hMemDC,RGB(255,255,255));
	}

	procRect.left = width/2 - 20;
	procRect.right = width - 42;

	procRect.top+=0;

	char showbuf[36] = {0};
	sprintf(showbuf,"%d%%",g_ProcValue);

	DrawTextA(hMemDC,showbuf,36,&procRect,0);

	::SelectObject(hMemDC,hOld);
	::DeleteObject(hFont);

	BitBlt(hdc,0,0,width,height,hMemDC,0,0,SRCCOPY);

	//删除
	::DeleteDC(hMemDC) ;      
	::DeleteObject(hBitmap); 

	ReleaseDC(hFlash,hdc);
}

void UpdateValue(int val)
{
	g_ProcValue = val;
}

#define GRADLEVEL 1

void DrawGradientV( HDC hdc, COLORREF co1, COLORREF co2, RECT& DrawRect )
{
	int r = GetRValue( co1 );
	int g = GetGValue( co1 );
	int b = GetBValue( co1 );

	int r2 = GetRValue( co2 );
	int g2 = GetGValue( co2 );
	int b2 = GetBValue( co2 );

	//计算宽,高
	int DrawRectWidth=DrawRect.right-DrawRect.left;
	int DrawRectHeight=DrawRect.bottom-DrawRect.top;

	if ( DrawRectWidth<=0)
		return;

	//初始化rect
	RECT rect={0,0,DrawRectWidth,GRADLEVEL};

	//准备GDI
	HDC hMemDC=CreateCompatibleDC(hdc);     //创建内存DC
	HBITMAP hBitmap=::CreateCompatibleBitmap(hdc,DrawRectWidth,DrawRectHeight);//创建位图
	::SelectObject(hMemDC,hBitmap);   //把位图选进内存DC
	HBRUSH hbr;


	for(int i = DrawRectHeight; i > 0; i -= GRADLEVEL ) 
	{
		//创建刷子
		hbr = CreateSolidBrush( RGB( r, g, b ) );
		FillRect( hMemDC, &rect, hbr );
		DeleteObject( hbr );

		//改变小正方体的位置
		rect.top += GRADLEVEL;
		rect.bottom += GRADLEVEL;

		//判断小正方体是否超界
		if( rect.bottom > DrawRect.bottom )
			rect.bottom = DrawRect.bottom;

		//改变颜色
		r += ( r2 - r + i / 2 ) / i * GRADLEVEL;
		g += ( g2 - g + i / 2 ) / i * GRADLEVEL;
		b += ( b2 - b + i / 2 ) / i * GRADLEVEL;
	}

	//内存DC映射到屏幕DC
	BitBlt(hdc,DrawRect.left,DrawRect.top,DrawRectWidth,DrawRectHeight,hMemDC,0,0,SRCCOPY);

	//删除
	::DeleteDC(hMemDC) ;      
	::DeleteObject(hBitmap); 
}

void SetLanguage(DWORD language)
{
	g_dLanguage = language;
}

DWORD GetLanguage()
{
	return g_dLanguage;
}
