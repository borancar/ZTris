#ifndef TETRIS_H
#define TETRIS_H

#define VELBLOKA 20
#define NFIGURA 7
#define SFIGURA 4
#define RFIGURA 4
#define TIMER 1

#ifndef WINCE
#define SPOLJA 15
#define VPOLJA 25
#else
#define SPOLJA 12
#define VPOLJA 12
#endif

#define TET_ODABERI_NIVO	10001
#define TET_RESET			10002
#define TET_BODOVI			10003
#define TET_POCETNI_REDOVI	10004
#define TET_IDUCA			10005
#define TET_BODOVI_RETKA	10006
#define TET_PADA_SA_SREDINE	10007
#define TET_START			10008
#define TET_STOP			10009
#define TET_ROTACIJA		10010
#define TET_PAUSE			10011
#define TET_IDUCA_PROZOR    10012
#define TET_BODLEVEL_PROZOR 10013
#define TET_SLIKA_POZADINA	10014
#define TET_SLIKA_FIGURE	10015
#define TET_SLIKA_PAUZE		10016
#define TET_GAMEOVE_HANDLER	10017
#define ROTACIJA_CW			100091
#define ROTACIJA_CCW		100092

#define MAXLEVELS			10

HWND tetris_window(HINSTANCE hInstance, int x, int y, DWORD style, DWORD ex_style, HWND parent);
void ShowAboutBox(HWND parent);

#endif