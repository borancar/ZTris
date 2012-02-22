#include <windows.h>
#include <stdlib.h>
#include "resource.h"
#include "igra.h"
#include "settings.h"
#include "skin.h"
#include "rang.h"
#include "about.h"

const wchar_t APPNAME[] = L"ZTris";
const wchar_t SETTINGSFNAME[] = L".\\ZTris.ini";

int_settings_t int_settings[] = {	{L"nivo", 1},
									{L"pocetnih_linija", 0},
									{L"prikazuj_iduceg", 1},
									{L"prikazuj_podatke", 1},
									{L"pada_iz_sredine", 0},
									{L"okreci_kao_sat", 0},
									{NULL, 0}};
str_settings_t str_settings[] = {	{L"ime", L"Ime"},
									{L"skin", L""},
									{NULL, {0}}};

scores_t scores;

int LoadSettings() {
#ifndef WINCE
	int_settings_t *is;
	str_settings_t *ss;

	for (is=int_settings;is->ime;++is) {
		is->value = GetPrivateProfileInt(APPNAME, is->ime, is->value, SETTINGSFNAME);
	}

	for (ss=str_settings;ss->ime;++ss) {
		GetPrivateProfileString(APPNAME, ss->ime, ss->value, ss->value, MAXSETTINGSSTRLEN, SETTINGSFNAME);
	}

	if (!GetPrivateProfileStruct(APPNAME, L"rang", &scores, sizeof(scores)/sizeof(wchar_t), SETTINGSFNAME)) {
		scores.n=0;
	}
#endif
	return 1;
}

int SaveSettings() {
#ifndef WINCE
	int_settings_t *is;
	str_settings_t *ss;
	wchar_t s[10];

	for (is=int_settings;is->ime;++is) {
		snwprintf(s, sizeof(s)/sizeof(wchar_t), L"%d", is->value);
		if (!WritePrivateProfileString(APPNAME, is->ime, s, SETTINGSFNAME)) return 0;
	}

	for (ss=str_settings;ss->ime;++ss) {
		if (!WritePrivateProfileString(APPNAME, ss->ime, ss->value, SETTINGSFNAME)) return 0;
	}

	if (!WritePrivateProfileStruct(APPNAME, L"rang", &scores, sizeof(scores)/sizeof(wchar_t), SETTINGSFNAME)) return 0;
#endif
	return 1;
}

/* returna
	1 - dodan
	0 - nije dodan */
int AddToRang(const wchar_t *ime, int bodova) {
	int i;
	for (i=0;i<scores.n;++i) {
		if (bodova > scores.imena[i].bodova ||
			(bodova == scores.imena[i].bodova && wcscmp(ime, scores.imena[i].ime) < 0))
			break;
	}
	if (i>=MAXLJUDIURANGU) return 0;

	if (i<MAXLJUDIURANGU-1) {
		memmove(&scores.imena[i+1], &scores.imena[i], sizeof(scores.imena[0])*(MAXLJUDIURANGU-i-1));
	}
	wcsncpy(scores.imena[i].ime, ime, MAXSETTINGSSTRLEN);
	scores.imena[i].bodova = bodova;
	
	if (scores.n < MAXLJUDIURANGU) ++scores.n;

	return 1;
}

INT_PTR CALLBACK SettingsDialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	wchar_t s[10], *sknlst, *t;
	UINT tmpi;
	int i;

	switch (uMsg) {
		case WM_INITDIALOG:
			// ime
			SetDlgItemText(hWnd, IDC_IME, str_settings[ss_name].value);
			// nivo
			for (i=1;i<=MAXLEVELS;++i) {
				snwprintf(s, sizeof(s)/sizeof(wchar_t), L"%d", i);
				SendDlgItemMessage(hWnd, IDC_LEVEL, CB_ADDSTRING, 0, (LPARAM)s);
			}
			SetDlgItemInt(hWnd, IDC_LEVEL, int_settings[is_level].value, FALSE);

			// inicijalnih redaka
			for (i=0;i<VPOLJA;++i) {
				snwprintf(s, sizeof(s)/sizeof(wchar_t), L"%d", i);
				SendDlgItemMessage(hWnd, IDC_LINES, CB_ADDSTRING, 0, (LPARAM)s);
			}
			SendDlgItemMessage(hWnd, IDC_LINES, CB_SETCURSEL, int_settings[is_lines].value, 0);

			// skin
			sknlst = GetSkinList();
			SendDlgItemMessage(hWnd, IDC_SKIN, CB_ADDSTRING, 0, (LPARAM) L"\\internal\\");
			if (!sknlst || !*sknlst) {
				SendDlgItemMessage(hWnd, IDC_SKIN, CB_SETCURSEL, 0, 0);
				EnableWindow(GetDlgItem(hWnd, IDC_SKIN), FALSE);
			} else {
				for (t=sknlst; *t; t+=wcslen(t)+1) {
					SendDlgItemMessage(hWnd, IDC_SKIN, CB_ADDSTRING, 0, (LPARAM)t);
				}
				SendDlgItemMessage(hWnd, IDC_SKIN, CB_SETCURSEL, 0, 0);
				SendDlgItemMessage(hWnd, IDC_SKIN, CB_SELECTSTRING, -1, (LPARAM)str_settings[ss_skin].value);
			}
			if (sknlst) free(sknlst);

			// prikazuj iduæu figuru
			CheckDlgButton(hWnd, IDC_CHK_SHOWNEXT, int_settings[is_shownext].value?BST_CHECKED:BST_UNCHECKED);

			// prikazuj podatke
			CheckDlgButton(hWnd, IDC_CHK_SHOWDATA, int_settings[is_showdata].value?BST_CHECKED:BST_UNCHECKED);

			// pada iz sredine
			CheckDlgButton(hWnd, IDC_CHK_MIDFALL, int_settings[is_midfall].value?BST_CHECKED:BST_UNCHECKED);

			// okreci kao sat
			CheckDlgButton(hWnd, IDC_CHK_ROTCW, int_settings[is_rotcw].value?BST_CHECKED:BST_UNCHECKED);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
					tmpi=GetDlgItemInt(hWnd, IDC_LEVEL, NULL, FALSE);
					if (tmpi==0 || tmpi>MAXLEVELS) {
						MessageBox(hWnd, L"Nema toliko nivoa!", NULL, MB_OK | MB_ICONERROR);
						break;
					}
					int_settings[is_level].value = tmpi;
					int_settings[is_lines].value = GetDlgItemInt(hWnd, IDC_LINES, NULL, FALSE);

					GetDlgItemText(hWnd, IDC_SKIN, str_settings[ss_skin].value, MAXSETTINGSSTRLEN);
					GetDlgItemText(hWnd, IDC_IME, str_settings[ss_name].value, MAXSETTINGSSTRLEN);
					if (!*str_settings[ss_name].value) {
						MessageBox(hWnd, L"Ime ne smije biti prazno!", NULL, MB_OK | MB_ICONERROR);
						break;
					}

					int_settings[is_shownext].value = IsDlgButtonChecked(hWnd, IDC_CHK_SHOWNEXT)==BST_CHECKED;
					int_settings[is_showdata].value = IsDlgButtonChecked(hWnd, IDC_CHK_SHOWDATA)==BST_CHECKED;
					int_settings[is_midfall].value = IsDlgButtonChecked(hWnd, IDC_CHK_MIDFALL)==BST_CHECKED;
					int_settings[is_rotcw].value = IsDlgButtonChecked(hWnd, IDC_CHK_ROTCW)==BST_CHECKED;

					EndDialog(hWnd, 1);
					break;
				case IDCANCEL:
					EndDialog(hWnd, 0);
					break;

				case IDC_VIEWRANG:
					ShowRang(hWnd);
					break;

				case IDC_CLEARRANG:
					ClearRang(hWnd);
					break;

#ifdef WINCE
				case IDC_ABOUT:
					ShowAboutBox(hWnd);
					break;
#endif
			}
			break;

		case WM_CLOSE:
			EndDialog(hWnd, 0);
			break;

		default:
			return FALSE;
	}
	return TRUE;
}

int ShowSettings() {
	return (int)DialogBox(GetModuleHandle(NULL), (LPCWSTR)IDD_SETTINGS, NULL, SettingsDialogProc);
}