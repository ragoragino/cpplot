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

PBITMAPINFO CreateBitmapInfoStruct(HWND hwnd, HBITMAP hBmp);
void CreateBMPFile(HWND hwnd, LPTSTR pszFile, PBITMAPINFO pbi,
	HBITMAP hBMP, HDC hDC);

void CreateImage(HWND hwnd, HDC hdc, wchar_t *dir)
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

	CreateBMPFile(hwnd, dir, pbi, bmp, hdc);

	DeleteObject(bmp);
	DeleteDC(memDC);
}

// Source for the following two function:
// https://msdn.microsoft.com/en-us/library/windows/desktop/dd145119(v=vs.85).aspx

PBITMAPINFO CreateBitmapInfoStruct(HWND hwnd, HBITMAP hBmp)
{
	BITMAP bmp;
	PBITMAPINFO pbmi;
	WORD    cClrBits;

	// Retrieve the bitmap color format, width, and height.  
	if (!GetObject(hBmp, sizeof(BITMAP), (LPSTR)&bmp))
	{
		printf("No information of HBITMAP received");
		return 0;
	}

	// Convert the color format to a count of bits.  
	cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel);
	if (cClrBits == 1)
		cClrBits = 1;
	else if (cClrBits <= 4)
		cClrBits = 4;
	else if (cClrBits <= 8)
		cClrBits = 8;
	else if (cClrBits <= 16)
		cClrBits = 16;
	else if (cClrBits <= 24)
		cClrBits = 24;
	else cClrBits = 32;

	// Allocate memory for the BITMAPINFO structure. (This structure  
	// contains a BITMAPINFOHEADER structure and an array of RGBQUAD  
	// data structures.)  

	if (cClrBits < 24)
	{
		pbmi = (PBITMAPINFO)LocalAlloc(LPTR,
			sizeof(BITMAPINFOHEADER) +
			sizeof(RGBQUAD) * (1 << cClrBits));
	}
	else
	{
		// There is no RGBQUAD array for these formats: 24-bit-per-pixel or 32-bit-per-pixel 
		pbmi = (PBITMAPINFO)LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER));
	}

	// Initialize the fields in the BITMAPINFO structure.  
	pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pbmi->bmiHeader.biWidth = bmp.bmWidth;
	pbmi->bmiHeader.biHeight = bmp.bmHeight;
	pbmi->bmiHeader.biPlanes = bmp.bmPlanes;
	pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel;
	if (cClrBits < 24)
	{
		pbmi->bmiHeader.biClrUsed = (1 << cClrBits);
	}

	// If the bitmap is not compressed, set the BI_RGB flag.  
	pbmi->bmiHeader.biCompression = BI_RGB;

	// Compute the number of bytes in the array of color  
	// indices and store the result in biSizeImage.  
	// The width must be DWORD aligned unless the bitmap is RLE 
	// compressed. 
	pbmi->bmiHeader.biSizeImage = ((pbmi->bmiHeader.biWidth * cClrBits + 31) & ~31) / 8
		* pbmi->bmiHeader.biHeight;
	// Set biClrImportant to 0, indicating that all of the device colors are important.  
	pbmi->bmiHeader.biClrImportant = 0;

	return pbmi;
}

void CreateBMPFile(HWND hwnd, LPTSTR pszFile, PBITMAPINFO pbi,
	HBITMAP hBMP, HDC hDC)
{
	HANDLE hf;                  // file handle  
	BITMAPFILEHEADER hdr;       // bitmap file-header  
	PBITMAPINFOHEADER pbih;     // bitmap info-header  
	LPBYTE lpBits;              // memory pointer  
	DWORD dwTotal;              // total count of bytes  
	DWORD cb;                   // incremental count of bytes  
	BYTE *hp;                   // byte pointer  
	DWORD dwTmp;

	pbih = (PBITMAPINFOHEADER)pbi;
	lpBits = (LPBYTE)GlobalAlloc(GMEM_FIXED, pbih->biSizeImage);

	if (!lpBits)
	{
		printf("Memory allocation unsuccesful!");
		return;
	}

	// Retrieve the color table (RGBQUAD array) and the bits (array of palette indices) from the DIB.  
	if (!GetDIBits(hDC, hBMP, 0, (WORD)pbih->biHeight, lpBits, pbi, DIB_RGB_COLORS))
	{
		printf("Device-independent bitmap could not be created!");
		return;
	}

	// Create the .BMP file.  
	hf = CreateFile(pszFile,
		GENERIC_READ | GENERIC_WRITE,
		(DWORD)0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		(HANDLE)NULL);
	if (hf == INVALID_HANDLE_VALUE)
	{
		printf("File could not be created!");
		return;
	}

	hdr.bfType = 0x4d42; // 0x42 = "B" 0x4d = "M"  

						 // Compute the size of the entire file.  
	hdr.bfSize = (DWORD)(sizeof(BITMAPFILEHEADER) +
		pbih->biSize + pbih->biClrUsed
		* sizeof(RGBQUAD) + pbih->biSizeImage);
	hdr.bfReserved1 = 0;
	hdr.bfReserved2 = 0;

	// Compute the offset to the array of color indices.  
	hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) +
		pbih->biSize + pbih->biClrUsed
		* sizeof(RGBQUAD);

	// Copy the BITMAPFILEHEADER into the .BMP file.  
	if (!WriteFile(hf, (LPVOID)&hdr, sizeof(BITMAPFILEHEADER),
		(LPDWORD)&dwTmp, NULL))
	{
		printf("Unable to write to the specified file!");
		return;
	}

	// Copy the BITMAPINFOHEADER and RGBQUAD array into the file.  
	if (!WriteFile(hf, (LPVOID)pbih, sizeof(BITMAPINFOHEADER)
		+ pbih->biClrUsed * sizeof(RGBQUAD),
		(LPDWORD)&dwTmp, (NULL)))
	{
		printf("Unable to write to the specified file!");
		return;
	}

	// Copy the array of color indices into the .BMP file.  
	dwTotal = cb = pbih->biSizeImage;
	hp = lpBits;
	if (!WriteFile(hf, (LPSTR)hp, (int)cb, (LPDWORD)&dwTmp, NULL))
	{
		printf("Unable to write to the specified file!");
		return;
	};

	// Close the .BMP file.  
	if (!CloseHandle(hf))
	{
		printf("Unable to close the file!");;
		return;
	}

	// Free memory.  
	GlobalFree((HGLOBAL)lpBits);
}