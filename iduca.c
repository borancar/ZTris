#include <windows.h>
#include "igra.h"
#include "iduca.h"

HDC hdcIduca = NULL;

#define FIGURA(i,j) (figura[(i)*SFIGURA+(j)])

LRESULT CALLBACK IducaProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HDC hDC;
	PAINTSTRUCT ps;

	switch(Msg)
	{
		case WM_ACTIVATE:
			SetActiveWindow(GetParent(hWnd));
			break;

		case IDUCA_FIGURA:
			{
				HDC hdcFigura;
				int *figura;
				int i, j;

				hdcFigura = (HDC) wParam;
				figura = (int *) lParam;

				BitBlt(hdcIduca, 0, 0, SFIGURA*VELBLOKA, RFIGURA*VELBLOKA, 0, 0, 0, PATCOPY);

				for(i = 0; i < RFIGURA; i++)
					for(j = 0; j < SFIGURA; j++)
					{
						BitBlt(hdcIduca, j*VELBLOKA, i*VELBLOKA, VELBLOKA, VELBLOKA, hdcFigura, FIGURA(i, j)*VELBLOKA, VELBLOKA, SRCAND);
						BitBlt(hdcIduca, j*VELBLOKA, i*VELBLOKA, VELBLOKA, VELBLOKA, hdcFigura, FIGURA(i, j)*VELBLOKA, 0, SRCPAINT);
					}

				InvalidateRect(hWnd, NULL, TRUE);
			}
			break;

		case WM_PAINT:
			hDC = BeginPaint(hWnd, &ps);

			if(!hdcIduca)
			{
				hdcIduca = CreateCompatibleDC(NULL);
				DeleteObject(SelectObject(hdcIduca, CreateCompatibleBitmap(hDC, SFIGURA*VELBLOKA, RFIGURA*VELBLOKA)));
			}

			BitBlt(hDC, ps.rcPaint.left, ps.rcPaint.top,
				ps.rcPaint.right - ps.rcPaint.left,
				ps.rcPaint.bottom - ps.rcPaint.top,
				hdcIduca, 0, 0, SRCCOPY);

			EndPaint(hWnd, &ps);
			break;

		case WM_DESTROY:
			DeleteDC(hdcIduca);
			break;

		default:
			return DefWindowProc(hWnd, Msg, wParam, lParam);
	}

	return 0;
}

HWND iduca_window(HINSTANCE hInstance, int x, int y, DWORD style, DWORD ex_style, HWND parent)
{
	WNDCLASS wc;
	HWND hWnd;
	RECT iduca = {0, 0, SFIGURA*VELBLOKA, RFIGURA*VELBLOKA};

	wc.cbWndExtra = 0;
	wc.cbClsExtra = 0;
	wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = NULL;
	wc.hInstance = hInstance;
	wc.lpfnWndProc = IducaProc;
	wc.lpszClassName = L"IducaWindow";
	wc.lpszMenuName = NULL;
	wc.style = 0;

	if(!RegisterClass(&wc))
		return NULL;

	AdjustWindowRectEx(&iduca, style, FALSE, ex_style);

	hWnd = CreateWindowEx(ex_style, L"IducaWindow", L"Iduća figura", style, x, y,
		iduca.right - iduca.left, iduca.bottom - iduca.top, parent, NULL, hInstance, 0);

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	return hWnd;
}

