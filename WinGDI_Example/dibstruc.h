/******************************************************************************

Module:  dbstruc.h
Notices: 
 Encapsulate a 16 color DIB. do not use color table. 1 px. = 1 word.
 For details look at MSDN help.
 It is used to draw pixel/ by pixel and then will be blited 
 on the screen.

Copyright (c) 1999 - 2000
coded by Angel Komarov

******************************************************************************/


#ifndef __DIBSTRUC_H__
#define __DIBSTRUC_H__

#include <windows.h>

class CDib
{
protected:
public:
	int m_Pitch;	// not in use
	int m_biWidth, m_biHeight;
	int m_wc, m_wh;
	HGDIOBJ m_hBitmap; // handle to the newly created DIB
	void* m_ppvBits; // Pointer to a variable that receives 
					 // a pointer to the location of the DIB's bit values
	int m_BitsPerPixel; // color data type indicator
	CDib(HDC , int , int , int = 16);
	~CDib();
	void Pixel(int , int , COLORREF );
};


#endif