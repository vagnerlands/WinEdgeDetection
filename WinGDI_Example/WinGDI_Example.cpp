// WinGDI_Example.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include <Windows.h>
#include "BitmapHandler.h"
#include "dibstruc.h"
#include "CCommonTypes.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#endif
CDib* pDib;		// DIB structure where the actuel drawing is implementted
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
CBitmapHandler* bmpHnd;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PSTR szCmdLine, int iCmdShow)
{
	static char szAppName[] = "WinGDI Example";
	HWND        hwnd;
	MSG         msg;
	WNDCLASSEX  wndclass;


	bmpHnd = new CBitmapHandler();
	bmpHnd->load("c:/fotos/adam.bmp");
	bmpHnd->canny_edge_detection();
	bmpHnd->save("out.bmp");
	wndclass.cbSize = sizeof(wndclass);
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	//wndclass.hbrBackground = (HBRUSH) GetStockObject (WHITE_BRUSH) ;
	wndclass.hbrBackground = NULL;
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppName;
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	RegisterClassEx(&wndclass);

	hwnd = CreateWindow(szAppName,         // window class nam
		"Image processing - Real Time - by Avatar ",     // window caption 
		WS_OVERLAPPEDWINDOW,     // window style 
		CW_USEDEFAULT,           // initial x position 
		CW_USEDEFAULT,           // initial y position 
		bmpHnd->getBmpHeader().width,           // initial x size 
		bmpHnd->getBmpHeader().height,           // initial y size
		NULL,                    // parent window handle
		NULL,                    // window menu handle
		hInstance,               // program instance handle
		NULL);		             // creation parameters

	ShowWindow(hwnd, iCmdShow);
	UpdateWindow(hwnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	delete bmpHnd;
	bmpHnd = NULL;

	return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	static int cxClient, cyClient;
	static HDC hdcMem;
	static HGDIOBJ hBtmp;

	RECT rc;
	HDC         hdc;
	PAINTSTRUCT ps;
	int i;

	switch (iMsg)
	{
	case WM_CREATE:
		hdc = GetDC(hwnd);
		hdcMem = CreateCompatibleDC(hdc);
		ReleaseDC(hwnd, hdc);
		
		return 0;
	case WM_SIZE:
		hdc = GetDC(hwnd);
		cxClient = bmpHnd->getBmpHeader().width; //LOWORD(lParam);
		cyClient = bmpHnd->getBmpHeader().height; //HIWORD(lParam);
		if (!pDib == NULL)
		{
			pDib->m_hBitmap = SelectObject(hdcMem, hBtmp);
			delete pDib;
			pDib = NULL;
		}
		// recreates the DIB object with the new screen size
		pDib = new CDib(hdc, cxClient, cyClient, 16);
		hBtmp = SelectObject(hdcMem, pDib->m_hBitmap);
		ReleaseDC(hwnd, hdc);

		return 0;
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		rc.left = 0; rc.top = 0; rc.right = cxClient; rc.bottom = cyClient;
		bmpHnd->canny_edge_detection();
		// draws the cub.
		for (int iX = rc.left; iX < rc.right; iX++)
		{
			for (int iY = rc.top; iY < rc.bottom; iY++)
			{
				switch (bmpHnd->getCurrentState())
				{
					case Types::EIMAGESTATE_CLEAN:
						pDib->Pixel(iX, iY, bmpHnd->getPixelAt(iX, rc.bottom - iY));
						break;
					case Types::EIMAGESTATE_GAUSSIAN:
						pDib->Pixel(iX, iY, bmpHnd->getOutputPixelAt(iX, rc.bottom - iY));
						break;
					case Types::EIMAGESTATE_DISTANCE:
						pDib->Pixel(iX, iY, bmpHnd->getDistancePixelAt(iX, rc.bottom - iY));
						break;
					case Types::EIMAGESTATE_CANNY:
						pDib->Pixel(iX, iY, bmpHnd->getOutputPixelAt(iX, rc.bottom - iY));
						break;
					default:
						break;
				}
				
			}
		}
		
		FillRect(hdc, &rc, (HBRUSH)GetStockObject(GRAY_BRUSH));
		RECT textBox;
		textBox.bottom = 40;
		textBox.top = 0;
		textBox.left = 0;
		textBox.right = 100;
		DrawText(hdcMem, bmpHnd->getObjectText(), -1, &textBox, DT_SINGLELINE | DT_NOCLIP);
		// blits the pic. onto screen 
		BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom,
			hdcMem, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);
		EndPaint(hwnd, &ps);
		return 0;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_HOME:
			bmpHnd->increaseMinThreshold(1);
			break;
		case VK_END:
			bmpHnd->increaseMinThreshold(-1);
			break;
		case VK_DIVIDE:
			bmpHnd->increaseMaxThreshold(1);
			break;
		case VK_MULTIPLY:
			bmpHnd->increaseMaxThreshold(-1);
			break;
		case VK_UP:
			bmpHnd->increaseSigma(-0.1f);
			break;
		case VK_DOWN:
			bmpHnd->increaseSigma(0.1f);
			break;
		case VK_LEFT:
			bmpHnd->increaseCurrentState(-1);
			break;
		case VK_RIGHT:
			bmpHnd->increaseCurrentState(1);
			break;
		case VK_ESCAPE:
			delete bmpHnd;
			bmpHnd = NULL;
			exit(1);
		default:
			break;
		}
		InvalidateRect(hwnd, NULL, false);
		break;
	case WM_ERASEBKGND:
		return 0;

	case WM_DESTROY:

		//delete  pCub;
		DeleteDC(hdcMem);
		delete pDib;

		PostQuitMessage(0);
		return 0;
	default:
		break;
	}

	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}

