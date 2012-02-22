#include <windows.h>
#include <stdlib.h>
#include "resource.h"
#include "bmpload.h"
#include "igra.h"
#include "iduca.h"
#include "bodlevel.h"
#include "rang.h"
#include "about.h"

#ifdef WINCE
#define GetWindowLongPtr GetWindowLong
#define SetWindowLongPtr SetWindowLong
#define GWLP_USERDATA GWL_USERDATA

#define SLOWEST	700

#else

#define SLOWEST 500

#endif

typedef struct _tag_tetris_data {
	wchar_t *tet_caption;
	int (*rotiraj)(struct _tag_tetris_data *data, int r, int s);
	int polje[VPOLJA+2][SPOLJA+2];
	int figura[RFIGURA][SFIGURA];
	int rfigure, sfigure, tfigure, rotfigure, gover;
	int bodovi, bodovi_retka, iduca, zapuni_redove, level;
	int pad_sa_sredine;
	DWORD spid;
	char paused;
	void (*game_over_handler)(int bodovi);

	HDC hdcPause;
	HDC hdcFigure;
	HDC hdcBuffer;
	HDC hdcBackground;

	HWND iduca_prozor;
	HWND bodlevel_prozor;

	BITMAP bmPoz, bmPause;
} tetris_data;

#define tet_caption L"ZTris"

void crtaj_polje(tetris_data *data);
void crtaj_figuru(tetris_data *data, int r, int s);

void brisi_polje(tetris_data *data);
int dodaj_figuru(tetris_data *data, HWND hWnd, int r, int s);

void ucitaj_figuru(tetris_data *data, int polje_fig[RFIGURA][SFIGURA], int fig);
void odaberi_figuru(tetris_data *data, int fig);
int rot_figuru_ccw(tetris_data *data, int r, int s);
int rot_figuru_cw(tetris_data *data, int r, int s);
void game_over(tetris_data *data, HWND hWnd);
void ShowAboutBox(HWND parent);

int figure[RFIGURA][NFIGURA+1][SFIGURA] = {
	{{0,0,0,0}, {0,1,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
	{{0,0,0,0}, {0,1,0,0}, {0,1,1,0}, {0,1,0,0}, {0,0,1,0}, {1,1,0,0}, {0,0,1,1}, {0,1,0,0}},
	{{0,0,0,0}, {0,1,0,0}, {0,1,1,0}, {0,1,0,0}, {0,0,1,0}, {0,1,1,0}, {0,1,1,0}, {1,1,1,0}},
	{{0,0,0,0}, {0,1,0,0}, {0,0,0,0}, {0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
};
int rot_figura[NFIGURA+1] = {0, 1, 0, 3, 3, 1, 1, 3};

void brisi_polje(tetris_data *data) {
	int i, j;

	for(i = 0; i < VPOLJA + 2; i++)
		for(j = 0; j < SPOLJA + 2; j++)
			data->polje[i][j] = (i == 0 || i == VPOLJA+1 || j == 0 || j == SPOLJA+1);

	for(i = 0; i < data->zapuni_redove; i++)
		for(j = 1; j <= SPOLJA; j++)
			data->polje[VPOLJA - i][j] = (rand() % 2) * (NFIGURA + 1);
}

void pokazi_iducu(tetris_data *data)
{
	int iduca_figura[RFIGURA][SFIGURA];

	if(!data->iduca_prozor) return;

	ucitaj_figuru(data, iduca_figura, data->iduca);

	SendMessage(data->iduca_prozor, IDUCA_FIGURA, (WPARAM) data->hdcFigure, (LPARAM) iduca_figura); 
}

void ClearBuffer(tetris_data *data) {
	BitBlt(data->hdcBuffer, 0, 0, SPOLJA*VELBLOKA, VPOLJA*VELBLOKA, NULL, 0, 0, PATCOPY);

	BitBlt(data->hdcBuffer, (SPOLJA*VELBLOKA - data->bmPoz.bmWidth)/2, (VPOLJA*VELBLOKA - data->bmPoz.bmHeight)/2, data->bmPoz.bmWidth, data->bmPoz.bmHeight, data->hdcBackground, 0, 0, SRCCOPY);
}

void crtaj_polje(tetris_data *data)
{
	int i, j;

	ClearBuffer(data);

	for(i = 0; i < VPOLJA+1; i++)
		for(j = 0; j < SPOLJA+1; j++)
			if(data->polje[i+1][j+1])
			{
				BitBlt(data->hdcBuffer, j*VELBLOKA, i*VELBLOKA, VELBLOKA, VELBLOKA, data->hdcFigure, data->polje[i+1][j+1]*VELBLOKA, VELBLOKA, SRCAND);
				BitBlt(data->hdcBuffer, j*VELBLOKA, i*VELBLOKA, VELBLOKA, VELBLOKA, data->hdcFigure, data->polje[i+1][j+1]*VELBLOKA, 0, SRCPAINT);
			}
}

void ucitaj_figuru(tetris_data *data, int polje_fig[RFIGURA][SFIGURA], int fig)
{
	int i, j;

	for(i = 0; i < RFIGURA; i++)
		for(j = 0; j < SFIGURA; j++)
			polje_fig[i][j] = figure[i][fig][j] * fig;
}

int rot_figuru_ccw(tetris_data *data, int r, int s)
{
	int nova_figura[RFIGURA][SFIGURA];
	int i, j;

	if(data->rotfigure < rot_figura[data->tfigure])
		for(i = 0; i < RFIGURA; i++)
			for(j = 0; j < SFIGURA; j++)
				nova_figura[i][j] = data->figura[j][3-i];
	else
		ucitaj_figuru(data, nova_figura, data->tfigure);

	for(i = 0; i < RFIGURA; i++)
		for(j = 0; j < SFIGURA; j++)
			if(nova_figura[i][j] && data->polje[r+i+1][s+j+1]) return 0;

	for(i = 0; i < RFIGURA; i++)
		for(j = 0; j < SFIGURA; j++)
			data->figura[i][j] = nova_figura[i][j];

	data->rotfigure = (data->rotfigure + 1) % (rot_figura[data->tfigure]+1);

	return 1;
}

int rot_figuru_cw(tetris_data *data, int r, int s)
{
	int nova_figura[RFIGURA][SFIGURA];
	int i, j;

	if(data->rotfigure < rot_figura[data->tfigure])
		for(i = 0; i < RFIGURA; i++)
			for(j = 0; j < SFIGURA; j++)
				nova_figura[i][j] = data->figura[3-j][i];
	else
		ucitaj_figuru(data, nova_figura, data->tfigure);

	for(i = 0; i < RFIGURA; i++)
		for(j = 0; j < SFIGURA; j++)
			if(nova_figura[i][j] && data->polje[r+i+1][s+j+1]) return 0;

	for(i = 0; i < RFIGURA; i++)
		for(j = 0; j < SFIGURA; j++)
			data->figura[i][j] = nova_figura[i][j];

	data->rotfigure = (data->rotfigure + 1) % (rot_figura[data->tfigure]+1);

	return 1;
}

int stavi_figuru(tetris_data *data, int r, int s)
{
	int i, j;

	for(i = 0; i < RFIGURA; i++)
		for(j = 0; j < SFIGURA; j++)
			if(data->polje[r+1+i][s+1+j] && data->figura[i][j]) return 0;

	return 1;
}

void odaberi_figuru(tetris_data *data, int fig)
{
	if (data->pad_sa_sredine) {
		data->sfigure = (SPOLJA - SFIGURA)/2;
	} else {
		data->sfigure = rand() % (SPOLJA-SFIGURA);
	}
	data->rfigure = 0;
	data->rotfigure = 0;
	data->tfigure = fig;
	ucitaj_figuru(data, data->figura, fig);
}

void crtaj_figuru(tetris_data *data, int r, int s)
{
	int i, j;

	for(i = 0; i < RFIGURA; i++)
		for(j = 0; j < SFIGURA; j++)
			if(data->figura[i][j])
			{
				BitBlt(data->hdcBuffer, (s+j)*VELBLOKA, (r+i)*VELBLOKA, VELBLOKA, VELBLOKA, data->hdcFigure, (data->figura[i][j])*VELBLOKA, VELBLOKA, SRCAND);
				BitBlt(data->hdcBuffer, (s+j)*VELBLOKA, (r+i)*VELBLOKA, VELBLOKA, VELBLOKA, data->hdcFigure, (data->figura[i][j])*VELBLOKA, 0, SRCPAINT);
			}
}

void PrikaziBodove(tetris_data *data, HWND hWnd) {
	SendMessage(data->bodlevel_prozor, BODLEVEL_POKAZI, (WPARAM) data->level, (LPARAM) data->bodovi);
}

int dodaj_figuru(tetris_data *data, HWND hWnd, int r, int s)
{
	int i, j, redak_pun;
	RECT izmjena;

	for(i = 0; i < RFIGURA; i++)
		for(j = 0; j < SFIGURA; j++)
			data->polje[r+1+i][s+1+j] = data->figura[i][j] ? data->figura[i][j] : data->polje[r+1+i][s+1+j];

	for(i = VPOLJA; i > 0; i--)
	{
		redak_pun = 1;

		for(j = 1; j <= SPOLJA; j++)
			redak_pun = redak_pun && data->polje[i][j];

		if(redak_pun)
		{
			int k, l;
			data->bodovi+=data->bodovi_retka*data->level;
			PrikaziBodove(data, hWnd);
			InvalidateRect(hWnd, NULL, FALSE);
			for(k = i; k > 1; k--)
				for(l = 1; l <= SPOLJA; l++)
					data->polje[k][l] = data->polje[k-1][l];

			i++;

			InvalidateRect(hWnd, NULL, TRUE);
		}
	}

	data->tfigure = data->iduca;
	data->iduca = rand() % NFIGURA + 1;
	odaberi_figuru(data, data->tfigure);
	data->rfigure--;

	pokazi_iducu(data);

	if(!stavi_figuru(data, data->rfigure, data->sfigure))
	{
		if(!stavi_figuru(data, data->rfigure+1, data->sfigure))
		{
			if(data->tfigure == 1) data->rfigure++;
			return 0;
		}

		data->rfigure++;
	}

	izmjena.left = data->sfigure * VELBLOKA;
	izmjena.right = (data->sfigure + SFIGURA) * VELBLOKA;
	izmjena.top = data->rfigure * VELBLOKA;
	izmjena.bottom = (data->rfigure + RFIGURA) * VELBLOKA;

	InvalidateRect(hWnd, &izmjena, FALSE);

	return 1;
}

void game_over(tetris_data *data, HWND hWnd)
{
	data->gover = 1;
	crtaj_polje(data);
	crtaj_figuru(data, data->rfigure, data->sfigure);
	InvalidateRect(hWnd, NULL, FALSE);
	odaberi_figuru(data, 0);
	KillTimer(hWnd, TIMER);
	if (data->game_over_handler) data->game_over_handler(data->bodovi);
}

void PokreniTimer(tetris_data *data, HWND hWnd) {
	int spid;

	spid = SLOWEST - (data->level-1)*50;

	if (spid < 15) spid = 15;

	data->spid = spid;

	KillTimer(hWnd, TIMER);
	SetTimer(hWnd, TIMER, spid, 0);

	data->paused=0;
}

void LoadPauseImg(tetris_data *data, const wchar_t *path) {
	HBITMAP hbmPause;

	if (!path) {
		hbmPause = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_PAUZA));
	} else {
		hbmPause = LoadFromFile(path);
	}

	GetObject(hbmPause, sizeof(BITMAP), &data->bmPause);

	DeleteObject(SelectObject(data->hdcPause, hbmPause));
	DeleteObject(hbmPause);
}

void LoadFigureImg(tetris_data *data, const wchar_t *path) {
	HBITMAP hbmFigure;

	if (!path) {
		hbmFigure = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_FIGURE));
	} else {
		hbmFigure = LoadFromFile(path);
	}

	DeleteObject(SelectObject(data->hdcFigure, hbmFigure));
	DeleteObject(hbmFigure);
}

void LoadBGImg(tetris_data *data, const wchar_t *path) {
	HBITMAP hbmBG;

	if (!path) {
		hbmBG = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_POZADINA));
	} else {
		hbmBG = LoadFromFile(path);
	}

	GetObject(hbmBG, sizeof(BITMAP), &data->bmPoz);

	DeleteObject(SelectObject(data->hdcBackground, hbmBG));
	DeleteObject(hbmBG);
}

LRESULT CALLBACK TetrisWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	static DWORD start;
	HDC hDC;
	RECT izmjena;
	PAINTSTRUCT ps;
	tetris_data *data;
	char proc;

	data = (tetris_data *) (LONG_PTR)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	switch(Msg)
	{
		case WM_ACTIVATE:
			{
				BOOL fActive;

				fActive = LOWORD(wParam);

				if(fActive == WA_ACTIVE || fActive == WA_CLICKACTIVE)
					SendMessage(hWnd, TET_PAUSE, 0, 0);

				if(fActive == WA_INACTIVE && !data->gover)
					SendMessage(hWnd, TET_PAUSE, 1, 0);
			}
			break;

		case TET_ODABERI_NIVO:
			{
				data->level = (int)wParam;
				if(data->bodlevel_prozor)
					SendMessage(data->bodlevel_prozor, BODLEVEL_POKAZI, (WPARAM) data->level, (LPARAM) data->bodovi);
				PokreniTimer(data, hWnd);
			}
			break;

		case TET_IDUCA_PROZOR:
			data->iduca_prozor = (HWND) wParam;
			break;

		case TET_BODLEVEL_PROZOR:
			data->bodlevel_prozor = (HWND) wParam;
			break;

		case TET_RESET:
			break;

		case TET_BODOVI:
			return data->bodovi;

		case TET_BODOVI_RETKA:
			data->bodovi_retka = (int)wParam;
			break;

		case TET_POCETNI_REDOVI:
			data->zapuni_redove = (int)wParam;
			brisi_polje(data);
			break;

		case TET_PADA_SA_SREDINE:
			data->pad_sa_sredine = !!wParam;
			break;

		case TET_ROTACIJA:
			if(wParam == ROTACIJA_CW)
				data->rotiraj = rot_figuru_cw;
			else if(wParam == ROTACIJA_CCW)
				data->rotiraj = rot_figuru_ccw;
			break;

		case TET_START:
			data->tfigure = rand() % NFIGURA + 1;
			data->iduca = rand() % NFIGURA + 1;

			pokazi_iducu(data);

			odaberi_figuru(data, data->tfigure);

			brisi_polje(data);

			if(data->bodlevel_prozor)
				SendMessage(data->bodlevel_prozor, BODLEVEL_POKAZI, (WPARAM) data->level, (LPARAM) data->bodovi);

			crtaj_polje(data);
			crtaj_figuru(data, data->rfigure, data->sfigure);
			break;

		case TET_STOP:
			KillTimer(hWnd, TIMER);
			break;

		case TET_IDUCA:
			return data->iduca;

		case TET_PAUSE:
			if (data->paused && !wParam) {
				PokreniTimer(data, hWnd);
				crtaj_polje(data);
				crtaj_figuru(data, data->rfigure, data->sfigure);
			} else if (!data->paused && wParam) {
				KillTimer(hWnd, TIMER);
				data->paused=1;
				ClearBuffer(data);

				BitBlt(data->hdcBuffer,
						(VELBLOKA*SPOLJA-data->bmPause.bmWidth)/2, (VELBLOKA*VPOLJA-data->bmPause.bmHeight)/2,
						data->bmPause.bmWidth, data->bmPause.bmHeight, data->hdcPause, 0, 0, SRCCOPY);
			}

			izmjena.left=0;
			izmjena.top=0;
			izmjena.right=VELBLOKA*SPOLJA;
			izmjena.bottom=VELBLOKA*VPOLJA;

			InvalidateRect(hWnd, &izmjena, FALSE);
			break;

		case TET_SLIKA_POZADINA:
			LoadBGImg(data, (wchar_t*)wParam);
			break;

		case TET_SLIKA_FIGURE:
			LoadFigureImg(data, (wchar_t*)wParam);
			break;

		case TET_SLIKA_PAUZE:
			LoadPauseImg(data, (wchar_t*)wParam);
			break;

		case TET_GAMEOVE_HANDLER:
			data->game_over_handler = (void(*)(int))wParam;
			break;

		case WM_LBUTTONDOWN:
#ifdef WINCE
			SendMessage(hWnd, TET_PAUSE, !data->paused, 0);
#else
			SendMessage(hWnd, TET_PAUSE, 0, 0);
#endif
			break;

#ifndef WINCE
		case WM_MOVING:
			{
				RECT *pos, offset;

				if(!data->gover && !data->paused)
					SendMessage(hWnd, TET_PAUSE, 1, 0);

				pos = (RECT *) lParam;

				SetWindowPos(data->iduca_prozor, HWND_NOTOPMOST, pos->right, pos->top, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);

				GetWindowRect(data->iduca_prozor, &offset);

				SetWindowPos(data->bodlevel_prozor, HWND_NOTOPMOST, offset.left, offset.bottom, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
			}
			break;
#endif

		case WM_MOVE:
			{
				RECT offset;
				
				if(!data->gover && !data->paused)
					SendMessage(hWnd, TET_PAUSE, 1, 0);
			
				GetWindowRect(hWnd, &offset);

				SetWindowPos(data->iduca_prozor, HWND_NOTOPMOST, offset.right, offset.top, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);

				GetWindowRect(data->iduca_prozor, &offset);

				SetWindowPos(data->bodlevel_prozor, HWND_NOTOPMOST, offset.left, offset.bottom, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
			
				InvalidateRect(data->iduca_prozor, NULL, FALSE);
			}
			break;

		case WM_CREATE:
			{
				CREATESTRUCT *ccs;
				HBITMAP hbm = NULL;
				HDC hdcWindow = NULL;
				
				ccs = (CREATESTRUCT *) lParam;

				SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG)(LONG_PTR)ccs->lpCreateParams);

				data = ccs->lpCreateParams;

				hdcWindow = GetDC(hWnd);

				data->hdcBuffer = CreateCompatibleDC(hdcWindow);
				DeleteObject(SelectObject(data->hdcBuffer, CreateCompatibleBitmap(hdcWindow, SPOLJA * VELBLOKA, VPOLJA * VELBLOKA)));

				data->hdcPause = CreateCompatibleDC(hdcWindow);
				data->hdcFigure = CreateCompatibleDC(hdcWindow);
				data->hdcBackground = CreateCompatibleDC(hdcWindow);

				LoadBGImg(data, NULL);
				LoadFigureImg(data, NULL);
				LoadPauseImg(data, NULL);

				ReleaseDC(hWnd, hdcWindow);

				data->bodovi_retka = 10;
				data->level=1;
			}
			break;

		case WM_KEYDOWN:
			proc = 1;

			switch (wParam) {
				case 'P':
					if(!data->gover)
						SendMessage(hWnd, TET_PAUSE, !data->paused, 0);
					break;

				case 'A': {
					char biopauz;
					biopauz = data->paused;
					SendMessage(hWnd, TET_PAUSE, 1 && !data->gover, 0);
					ShowAboutBox(hWnd);
					SendMessage(hWnd, TET_PAUSE, biopauz && !data->gover, 0);
					break;
					}

				case 'R': {
					char biopauz;
					biopauz = data->paused;
					SendMessage(hWnd, TET_PAUSE, 1, 0);
					ShowRang(hWnd);
					SendMessage(hWnd, TET_PAUSE, biopauz, 0);
					break;
					}

				case VK_ESCAPE:
					SendMessage(hWnd, WM_CLOSE, 0, 0);
					break;

				default:
					proc=0;
					break;
			}

			if (proc || data->gover || data->paused) break;

			switch(wParam)
			{
				case VK_LEFT:
					if(stavi_figuru(data, data->rfigure, data->sfigure-1))
					{
						data->sfigure--;

						izmjena.left = data->sfigure * VELBLOKA;
						izmjena.right = (data->sfigure + (SFIGURA + 1)) * VELBLOKA;
						izmjena.top = data->rfigure;
						izmjena.bottom = (data->rfigure + RFIGURA) * VELBLOKA;
						
						InvalidateRect(hWnd, &izmjena, FALSE);
					}
					break;

				case VK_RIGHT:
					if(stavi_figuru(data, data->rfigure, data->sfigure+1))
					{
						data->sfigure++;

						izmjena.left = (data->sfigure - 1) * VELBLOKA;
						izmjena.right = (data->sfigure + SFIGURA) * VELBLOKA;
						izmjena.top = data->rfigure;
						izmjena.bottom = (data->rfigure + RFIGURA) * VELBLOKA;
						
						InvalidateRect(hWnd, &izmjena, FALSE);
					}
					break;

				case VK_DOWN:
					if(stavi_figuru(data, data->rfigure+1, data->sfigure))
					{
						data->rfigure++;

						izmjena.left = data->sfigure * VELBLOKA;
						izmjena.right = (data->sfigure + SFIGURA) * VELBLOKA;
						izmjena.top = data->rfigure;
						izmjena.bottom = (data->rfigure + RFIGURA) * VELBLOKA;
					
						InvalidateRect(hWnd, &izmjena, FALSE);					
					}
					else
						if(!dodaj_figuru(data, hWnd, data->rfigure, data->sfigure))
							game_over(data, hWnd);

					break;

#ifndef WINCE
				case VK_SPACE:
#else
				case VK_RETURN:
#endif
					{
						izmjena.left = data->sfigure * VELBLOKA;
						izmjena.right = (data->sfigure + SFIGURA) * VELBLOKA;
						izmjena.top = data->rfigure * VELBLOKA;
						izmjena.bottom = (data->rfigure + RFIGURA) * VELBLOKA;

						InvalidateRect(hWnd, &izmjena, FALSE);

						for(; stavi_figuru(data, data->rfigure+1, data->sfigure); data->rfigure++);

						izmjena.top = data->rfigure * VELBLOKA;
						izmjena.bottom = (data->rfigure + RFIGURA) * VELBLOKA;

						InvalidateRect(hWnd, &izmjena, FALSE);

						if(!dodaj_figuru(data, hWnd, data->rfigure, data->sfigure))
							game_over(data, hWnd);
					}
					break;

				case VK_UP:
					if(data->rotiraj(data, data->rfigure, data->sfigure))
					{
						izmjena.left = data->sfigure * VELBLOKA;
						izmjena.right = (data->sfigure + SFIGURA) * VELBLOKA;
						izmjena.top = data->rfigure * VELBLOKA;
						izmjena.bottom = (data->rfigure + RFIGURA) * VELBLOKA;

						InvalidateRect(hWnd, &izmjena, FALSE);
					}
					break;
			}

			if (!data->paused) {
				crtaj_polje(data);
				crtaj_figuru(data, data->rfigure, data->sfigure);
			}

#ifdef WINCE
			UpdateWindow(hWnd);
#endif		

			if(GetTickCount() - start <= data->spid)
				break;

		case WM_TIMER:
			{
				if(stavi_figuru(data, data->rfigure+1, data->sfigure))
					data->rfigure++;
				else
					if(!dodaj_figuru(data, hWnd, data->rfigure, data->sfigure))
					{
						game_over(data, hWnd);
						break;
					}

				izmjena.left = data->sfigure * VELBLOKA;
				izmjena.right = (data->sfigure + SFIGURA) * VELBLOKA;
				izmjena.top = data->rfigure-2;
				izmjena.bottom = (data->rfigure + (RFIGURA+1)) * VELBLOKA;

				InvalidateRect(hWnd, &izmjena, FALSE);

				crtaj_polje(data);
				crtaj_figuru(data, data->rfigure, data->sfigure);
				
				if(data->bodlevel_prozor)
					SendMessage(data->bodlevel_prozor, BODLEVEL_POKAZI, (WPARAM) data->level, (LPARAM) data->bodovi);

				start = GetTickCount();

#ifdef WINCE
				UpdateWindow(hWnd);
#endif		
			}
			break;

		case WM_PAINT:
			hDC = BeginPaint(hWnd, &ps);

			BitBlt(hDC, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right - ps.rcPaint.left,
				ps.rcPaint.bottom - ps.rcPaint.top, data->hdcBuffer, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);
			
			EndPaint(hWnd, &ps);
			break;

		case WM_DESTROY:
			DestroyWindow(data->iduca_prozor);
			DestroyWindow(data->bodlevel_prozor);
			DeleteDC(data->hdcBuffer);
			DeleteDC(data->hdcFigure);
			DeleteDC(data->hdcPause);
			KillTimer(hWnd, TIMER);
			GlobalFree((HGLOBAL)(LONG_PTR)GetWindowLongPtr(hWnd, GWLP_USERDATA));
			PostQuitMessage(EXIT_SUCCESS);
			break;

		default:
			return DefWindowProc(hWnd, Msg, wParam, lParam);
	}

	return 0;
}

HWND tetris_window(HINSTANCE hInstance, int x, int y, DWORD style, DWORD ex_style, HWND parent)
{
#ifdef WINCE
	WNDCLASS wcex;
#else
	WNDCLASSEX wcex;
#endif
	HWND hWnd;
	RECT field = {0, 0, SPOLJA*VELBLOKA, VPOLJA*VELBLOKA};
	tetris_data *new_tetris_data = 0;

	srand((unsigned int)GetTickCount());

#ifndef WINCE
	wcex.cbSize = sizeof(WNDCLASSEX);
#endif
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = sizeof(tetris_data *);
	wcex.hbrBackground = (HBRUSH) ((COLOR_WINDOW) + 1);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hIcon = LoadIcon(hInstance, (LPCWSTR)IDI_ICON1);
#ifndef WINCE
	wcex.hIconSm = LoadImage(hInstance, (LPCWSTR)IDI_ICON1, IMAGE_ICON, 16, 16, 0);
#endif
	wcex.hInstance = hInstance;
	wcex.lpfnWndProc = TetrisWndProc;
	wcex.lpszClassName = L"TetrisField";
	wcex.lpszMenuName = NULL;
#ifdef WINCE
	wcex.style = 0;
#else
	wcex.style = CS_OWNDC;
#endif

#ifdef WINCE
	if(!RegisterClass(&wcex))
#else
	if(!RegisterClassEx(&wcex))
#endif
	{
		MessageBox(NULL, L"RegisterClassEx() failed!", L"Error", MB_OK | MB_ICONERROR);

		return 0;
	}

	new_tetris_data = (tetris_data *) GlobalAlloc(GPTR, sizeof(tetris_data));

	AdjustWindowRectEx(&field, style, FALSE, ex_style);

	hWnd = CreateWindowEx(ex_style, L"TetrisField", tet_caption, style, x, y, field.right - field.left, field.bottom - field.top, parent, NULL, hInstance, (LPVOID) new_tetris_data);

	ShowWindow(hWnd, SW_SHOW);

	return hWnd;
}
