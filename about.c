#include <windows.h>
#include "resource.h"

INT_PTR CALLBACK AboutDialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_INITDIALOG: {
			wchar_t verstr[18];
			HINSTANCE myi;
			HRSRC hrsc;
			HGLOBAL hgl;
			void *verdata, *vd;
			UINT len;
			DWORD s;
			VS_FIXEDFILEINFO *ffi;
			wchar_t *str;

			myi=GetModuleHandle(NULL);

			if (!(hrsc = FindResource(myi, (LPCTSTR)VS_VERSION_INFO, RT_VERSION))) break;
			if (!(hgl = LoadResource(myi, hrsc))) break;

			if (!(verdata = LockResource(hgl))) {
#ifndef WINCE
				FreeResource(hgl);
#endif
				break;
			}

			s=SizeofResource(myi, hrsc);
			vd=(void*)malloc(s);
			memcpy(vd, verdata, s);
			if (!VerQueryValue(vd, L"\\", (LPVOID*)&ffi, &len)) goto fri;

			snwprintf(verstr, 18, L"%d.%d.%d.%d.",	HIWORD(ffi->dwProductVersionMS),
															LOWORD(ffi->dwProductVersionMS),
															HIWORD(ffi->dwProductVersionLS),
															LOWORD(ffi->dwProductVersionLS));
			SetDlgItemText(hWnd, IDC_S_VERSION, verstr);

			if (!VerQueryValue(vd, L"\\StringFileInfo\\080004b0\\Comments", (LPVOID*)&str, &len)) goto fri;
			SetDlgItemText(hWnd, IDC_S_DESC, str);

fri:
			free(vd);
#ifndef WINCE
			UnlockResource(hgl);
			FreeResource(hgl);
#endif
			break;
			}

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDCANCEL:
				case IDOK:
					EndDialog(hWnd, 0);
					break;
			}

			break;

		default:
			return FALSE;
	}

	return TRUE;
}

void ShowAboutBox(HWND parent) {
	DialogBox(GetModuleHandle(NULL), (LPCWSTR)IDD_ABOUT, parent, AboutDialogProc);
}
