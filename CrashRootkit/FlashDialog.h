#pragma once

void ShowFlashDialog();

void SetHistance(HINSTANCE g_hInst);

VOID InitPicturePath();

void ShowPicture();

void CloseFlashDialog();

void UpdateValue(int val);

void DrawGradientV( HDC hdc, COLORREF co1, COLORREF co2, RECT& DrawRect);

void SetLanguage(DWORD language);

DWORD GetLanguage();