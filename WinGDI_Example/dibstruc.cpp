/******************************************************************************

Module:  dbstruc.cpp
Notices: 
 Encapsulate a 16 color DIB. do not use color table. 1 px. = 1 word.
 For details look at MSDN help.
 It is used to draw pixel/ by pixel and then will be blited 
 on the screen.

Copyright (c) 1999 - 2000
coded by Angel Komarov

******************************************************************************/

#include <assert.h>
#include "dibstruc.h"

//--------------------------------------------------------------------
// The bitmap has a maximum of 2^16 colors.
//  biCompression = BI_RGB
//	biPlanes = 1
// So that the bmiColors member is NULL. 
// Each WORD in the bitmap array represents a single pixel
//--------------------------------------------------------------------
CDib::CDib(HDC hdc, int width, int height, int BitsPerPixel)
{
	m_BitsPerPixel = BitsPerPixel;
	m_biWidth = width;
	m_biHeight = height;
	m_wc = (int) ((width) / 2); 
	m_wh = (int) ((height) / 2); 
	m_Pitch = (width*BitsPerPixel/8+3)&~3; // not in use!
	// set up BITMAPINFOHEADER structure.
	int heapspace = sizeof(BITMAPINFOHEADER);
	HANDLE heap = GetProcessHeap();
	BITMAPINFO* BitmapInfo = (BITMAPINFO*) HeapAlloc(heap, 0, heapspace);
	assert( BitmapInfo != NULL);
	BitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	BitmapInfo->bmiHeader.biWidth = width;
	BitmapInfo->bmiHeader.biHeight = -height;
	BitmapInfo->bmiHeader.biPlanes = 1;
	BitmapInfo->bmiHeader.biBitCount = BitsPerPixel;
	BitmapInfo->bmiHeader.biCompression = BI_RGB;
	BitmapInfo->bmiHeader.biSizeImage = 0;
	BitmapInfo->bmiHeader.biXPelsPerMeter = 0;
	BitmapInfo->bmiHeader.biYPelsPerMeter = 0;
	BitmapInfo->bmiHeader.biClrUsed = 0;
	BitmapInfo->bmiHeader.biClrImportant = 0;
	// call the API
	m_hBitmap = CreateDIBSection(hdc,
								BitmapInfo,
								DIB_RGB_COLORS,
								&m_ppvBits,
								NULL,
								0);
	HeapFree(heap, 0, BitmapInfo);
}

//--------------------------------------------------------------------
// Delete DIB.
//--------------------------------------------------------------------
CDib::~CDib()
{
	DeleteObject(m_hBitmap);
}

//--------------------------------------------------------------------
// Draw pixel by pixel in the DIB structure 
// This function is used in all 2D graphiks primitives .
//--------------------------------------------------------------------
void CDib::Pixel(int x, int y, COLORREF color)
{
	// swap pointer
	WORD* bits;

	x = max(x, 0);
	x = min(x, m_biWidth-1);
	y = max(y, 0);
	y = min(y, m_biHeight-1);

	int r = GetRValue(color);
	int g = GetGValue(color);
	int b = GetBValue(color);

	switch (m_BitsPerPixel)
	{
		case 8:
			bits = (WORD*)m_ppvBits;
			*(bits + y*m_biWidth + x) = (WORD)(
				(((GetRValue(color)) & (31 << 3)) << 7) |
				(((GetGValue(color)) & (31 << 3)) << 2) |
				(((GetBValue(color)) & (31 << 3)) >> 3));
			break;
		case 16:
			bits = (WORD*)m_ppvBits;
			//*(bits + y*m_biWidth + x) = (WORD)(r|g|b);
		
			*(bits + y*m_biWidth + x) = (WORD)(
				(((color) & (31 << 3)) << 7) |
				(((color) & (31 << 3)) << 2) |
				(((color) & (31 << 3)) >> 3));
		
			break;
		default:
			break;
	}
}
