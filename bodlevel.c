#include <windows.h>
#include "igra.h"
#include "bodlevel.h"

#define IDS_BODOVI 9001
#define IDS_LEVEL 9002

int default_prolaz[MAXLEVELS+1] = {0, 100, 300, 600, 1000, 1500, 2100, 2800, 3600, 4500, 5500};

int *prolaz = 0;

LRESULT CALLBACK BodlevelProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch(Msg)
	{
		case WM_CREATE:
			CreateWindow(L"STATIC", L"", WS_VISIBLE | WS_CHILD, 0, 0, 80, 40, hWnd, (HMENU)IDS_LEVEL, GetModuleHandle(NULL), 0);
			CreateWindow(L"STATIC", L"", WS_VISIBLE | WS_CHILD, 0, 40, 80, 40, hWnd, (HMENU)IDS_BODOVI, GetModuleHandle(NULL), 0);

			prolaz = default_prolaz;
			break;

		case WM_ACTIVATE:
			SetActiveWindow(GetParent(hWnd));
			break;

		case WM_CTLCOLORSTATIC:
			{
				HDC hDC;

				hDC = (HDC) wParam;

				SetTextColor(hDC, RGB(0,0,0));
				SetBkMode(hDC, TRANSPARENT);
			}
            return (LRESULT) GetStockObject(WHITE_BRUSH);

		case LEVEL_PROLAZ:
			prolaz = (int *) wParam;
			break;

		case BODLEVEL_POKAZI:
			{
				static int bodovi, level;

				level = ((int) wParam) != -1 ? (int) wParam : level;
				bodovi = ((int) lParam) != -1 ? (int) lParam : bodovi;

				SetDlgItemInt(hWnd, IDS_LEVEL, level, TRUE);
				SetDlgItemInt(hWnd, IDS_BODOVI, bodovi, TRUE);


				if(level <= 10)
					if(bodovi > prolaz[level])
						SendMessage(GetParent(hWnd), TET_ODABERI_NIVO, (WPARAM) (level+1), 0);
			}
			break;

		default:
			return DefWindowProc(hWnd, Msg, wParam, lParam);
	}

	return 0;
}

HWND bodlevel_window(HINSTANCE hInstance, int x, int y, DWORD style, DWORD ex_style, HWND parent)
{
	WNDCLASS wc;
	HWND hWnd;
	RECT bodlevel = {0, 0, SFIGURA*VELBLOKA, RFIGURA*VELBLOKA};

	wc.cbWndExtra = 0;
	wc.cbClsExtra = 0;
	wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = NULL;
	wc.hInstance = hInstance;
	wc.lpfnWndProc = BodlevelProc;
	wc.lpszClassName = L"BodlevelWindow";
	wc.lpszMenuName = NULL;
	wc.style = 0;

	if(!RegisterClass(&wc))
		return NULL;

	AdjustWindowRectEx(&bodlevel, style, FALSE, ex_style);

	hWnd = CreateWindowEx(ex_style, L"BodlevelWindow", L"Nivo i bodovi", style, x, y,
		bodlevel.right - bodlevel.left, bodlevel.bottom - bodlevel.top, parent, NULL, hInstance, 0);

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	return hWnd;}
