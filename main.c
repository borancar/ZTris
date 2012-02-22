#include <windows.h>
#include <commctrl.h>

#include "igra.h"
#include "iduca.h"
#include "settings.h"
#include "skin.h"
#include "bodlevel.h"

int level_advance[MAXLEVELS+1] = {0, 100, 300, 600, 1000, 1500, 2100, 2800, 3600, 4500, 5500};

void goverhandler(int bodovi) {
	AddToRang(str_settings[ss_name].value, bodovi);
	MessageBox(NULL, L"Game over!", L"Game over!", MB_OK | MB_ICONEXCLAMATION);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG Msg;
	HWND hwndTetris = NULL, hwndIduca = NULL, hwndBodlevel = NULL;
	RECT window;
	wchar_t bgimgpath_d[MAX_PATH], *bgimgpath=NULL;
	wchar_t figureimgpath_d[MAX_PATH], *figureimgpath=NULL;
	wchar_t pauseimgpath_d[MAX_PATH], *pauseimgpath=NULL;

	InitCommonControls();

	LoadSettings();
	if (!ShowSettings()) {
		return 0;
	}

	hwndTetris = tetris_window(hInstance, CW_USEDEFAULT, CW_USEDEFAULT, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, 0, NULL);
	GetWindowRect(hwndTetris, &window);
	if (int_settings[is_shownext].value) {
		hwndIduca = iduca_window(hInstance, window.right, window.top, WS_CHILD | WS_POPUP | WS_CAPTION, 0, hwndTetris); 
		GetWindowRect(hwndIduca, &window);
	} else {
		window.left = window.right;
		window.bottom = window.top;
	}
	if (int_settings[is_showdata].value) {
		hwndBodlevel = bodlevel_window(hInstance, window.left, window.bottom, WS_CHILD | WS_POPUP | WS_CAPTION, 0, hwndTetris);
	}

	if (GetSkinBackground(str_settings[ss_skin].value, bgimgpath_d, MAX_PATH) >= 0) bgimgpath=bgimgpath_d;
	if (GetSkinFigureImg(str_settings[ss_skin].value, figureimgpath_d, MAX_PATH) >= 0) figureimgpath=figureimgpath_d;
	if (GetSkinPauseImg(str_settings[ss_skin].value, pauseimgpath_d, MAX_PATH) >=0) pauseimgpath=pauseimgpath_d;

	SetActiveWindow(hwndTetris);
	SetForegroundWindow(hwndTetris);

	SendMessage(hwndBodlevel, LEVEL_PROLAZ, (WPARAM)level_advance, 0);

	SendMessage(hwndTetris, TET_SLIKA_POZADINA, (WPARAM)bgimgpath, 0);
	SendMessage(hwndTetris, TET_SLIKA_FIGURE, (WPARAM)figureimgpath, 0);
	SendMessage(hwndTetris, TET_SLIKA_PAUZE, (WPARAM)pauseimgpath, 0);
	SendMessage(hwndTetris, TET_IDUCA_PROZOR, (WPARAM)hwndIduca, 0);
	SendMessage(hwndTetris, TET_BODLEVEL_PROZOR, (WPARAM)hwndBodlevel, 0);
	SendMessage(hwndTetris, TET_PADA_SA_SREDINE, int_settings[is_midfall].value, 0);
	SendMessage(hwndTetris, TET_BODOVI_RETKA, 10, 0);
	SendMessage(hwndTetris, TET_ROTACIJA, int_settings[is_rotcw].value?ROTACIJA_CW:ROTACIJA_CCW, 0);
	SendMessage(hwndTetris, TET_POCETNI_REDOVI, int_settings[is_lines].value, 0);
	SendMessage(hwndTetris, TET_ODABERI_NIVO, int_settings[is_level].value, 0);
	SendMessage(hwndTetris, TET_GAMEOVE_HANDLER, (WPARAM)goverhandler, 0);
	SendMessage(hwndTetris, TET_START, 0, 0);

	while(GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	if (!SaveSettings()) {
		MessageBox(NULL, L"Ne mogu spremiti postavke!", NULL, MB_OK | MB_ICONWARNING);
	}

	return (int)Msg.wParam;
}
