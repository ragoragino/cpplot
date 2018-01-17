#pragma once
#define UNICODE
#define _UNICODE

#include <windows.h>
#include "Bitmap.h"
#include "Header.h"

#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Gdi32.lib")

void CreateImage(HWND hwnd, HDC hdc);

static wchar_t g_szClassName[] = L"Plot";

void Paint(HDC hDC, HWND hwnd)
{
	HPEN hBluePen = CreatePen(PS_SOLID, 5, RGB(0, 0, 0));
	HPEN hPen = (HPEN)SelectObject(hDC, hBluePen);

	RECT rcClient;
	GetClientRect(hwnd, &rcClient);

	Rectangle(hDC, rcClient.left + 10, rcClient.top + 10, rcClient.right - 10, rcClient.bottom - 10);

	DeleteObject(hBluePen);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	
	case WM_PAINT:
	{
		PAINTSTRUCT ps;

		HDC hdc = BeginPaint(hwnd, &ps);

		Paint(hdc, hwnd);
		CreateImage(hwnd, hdc);
		EndPaint(hwnd, &ps);
	}
	break;

	case WM_KEYDOWN:
	{
		if (wParam == VK_ESCAPE)
		{
			PostQuitMessage(0);
		}
	}
	break;

	case WM_CLOSE:
	{
		DestroyWindow(hwnd);
	}
	break;

	case WM_DESTROY:
	{
		PostQuitMessage(0);
	}
	break;

	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	return 0;
}

int InitializeWindow()
{
	WNDCLASSEX wc;
	HWND hwnd;
	MSG Msg;
	HINSTANCE hInstance = GetModuleHandle(NULL);

	// Registering the Window
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = g_szClassName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wc.style = CS_HREDRAW | CS_VREDRAW;

	if (!RegisterClassEx(&wc))
	{
		printf("Call to RegisterClassEx failed!");
		return 1;
	}

	// Creating the Window
	hwnd = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		g_szClassName,
		L"Plot",
		NULL,
		CW_USEDEFAULT, CW_USEDEFAULT, 600, 600,
		NULL, NULL, hInstance, NULL);

	if (!hwnd)
	{
		printf("Call to CreateWindow failed!");
		return 1;
	}

	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);
	
	// The message loop
	while (GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	return 0;
}

void CreateImage(HWND hwnd, HDC hdc)
{
	HDC memDC = CreateCompatibleDC(hdc);

	RECT rcClient;
	GetClientRect(hwnd, &rcClient);

	int nWidth = rcClient.right - rcClient.left;
	int nHeight = rcClient.bottom - rcClient.top;

	HBITMAP bmp = CreateCompatibleBitmap(hdc, nWidth, nHeight);
	SelectObject(memDC, bmp);
	BitBlt(memDC, 0, 0, nWidth, nHeight, hdc, 0, 0, SRCCOPY);
	PBITMAPINFO pbi = CreateBitmapInfoStruct(hwnd, bmp);

	CreateBMPFile(hwnd, L"file.bmp", pbi, bmp, hdc);

	DeleteObject(bmp);
	DeleteDC(memDC);
}