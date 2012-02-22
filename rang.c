#include <windows.h>
#include "resource.h"
#include "settings.h"

int ClearRang(HWND parent) {
	int i;
	i = MessageBox(parent, L"Obrisati rang?", L"Brisanje ranga", MB_YESNO | MB_ICONQUESTION) == IDYES;

	if (i) scores.n = 0;

	return i;
}

INT_PTR CALLBACK RangDialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	wchar_t tmpstr[MAXSETTINGSSTRLEN+100];
	int i;

	switch (uMsg) {
		case WM_INITDIALOG:
			for (i=0;i<scores.n;++i) {
				snwprintf(tmpstr, MAXSETTINGSSTRLEN+100, L"%4d %s", scores.imena[i].bodova, scores.imena[i].ime);
				SendDlgItemMessage(hWnd, IDC_RANGLIST, LB_ADDSTRING, 0, (LPARAM)tmpstr);
			}
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDCANCEL:
				case IDOK:
					EndDialog(hWnd, 0);
					break;
				case IDC_CLEARRANG:
					if (ClearRang(hWnd)) {
						SendDlgItemMessage(hWnd, IDC_RANGLIST, LB_RESETCONTENT, 0, 0);
					}
					break;
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

int ShowRang(HWND parent) {
	return (int)DialogBox(GetModuleHandle(NULL), (LPCWSTR)IDD_RANG, parent, RangDialogProc);
}