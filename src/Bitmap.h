#pragma once
#include "Header.h"

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

	Gdiplus::GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure

	pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL) { return -1;  }// Failure

	Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}

void CreateImage2(HWND hwnd, HDC hdc, wchar_t *dir, wchar_t *ext)
{
	// Initialize GDI+
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	// Create compatible DC and bitmaps
	HDC memDC = CreateCompatibleDC(hdc);

	RECT rcClient;
	GetClientRect(hwnd, &rcClient);
	int nWidth = rcClient.right - rcClient.left;
	int nHeight = rcClient.bottom - rcClient.top;

	HDC memdc = CreateCompatibleDC(hdc);
	HBITMAP hbitmap = CreateCompatibleBitmap(hdc, nWidth, nHeight);
	HBITMAP oldbmp = (HBITMAP)SelectObject(memdc, hbitmap);
	BitBlt(memdc, 0, 0, nWidth, nHeight, hdc, 0, 0, SRCCOPY | CAPTUREBLT);
	SelectObject(memdc, oldbmp);

	// Create the wchar_t holding the desired extension type
	// Ext needs to be null terminated -> this is a safety check
	unsigned int counter = 1;
	wchar_t *ext_copy = ext;
	while (*ext_copy++ != L'\0')
	{
		++counter;
		if (counter > MAX_EXTENSION_SIZE)
		{
			printf("ERROR: Encoder extension is too long. Either change the " 
				"extension or set the MAX_EXTENSION_SIZE macro\n");
			return;
		}
	}
	unsigned int clsid_buffer_size = 6 + counter;
	wchar_t *clsid_buff = new wchar_t[clsid_buffer_size];
	wcscpy_s(clsid_buff, 7, L"image/\0");
	wcscpy_s(clsid_buff + 6, counter, ext);

	// Get the encoder parameters
	CLSID pngClsid;
	HRESULT hresult = GetEncoderClsid(clsid_buff, &pngClsid);
	if (hresult < 0)
	{
		printf("ERROR: Given encoder is not installed.\n");
	}

	// Save the bitmap to file
	Gdiplus::Bitmap bitmap(hbitmap, NULL);
	Gdiplus::Status s = bitmap.Save(dir, &pngClsid);
	if (s)
	{
		printf("ERROR: An image could not be saved. Error status from "
			"Gdiplus::Bitmap::save: %d \n", s);
	}

	// Clean the objects
	DeleteObject(hbitmap);
	DeleteDC(memdc);

	delete[] clsid_buff;
}
