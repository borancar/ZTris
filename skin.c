#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>	// samo za glup naèin provjere da li postoji fajla

#ifdef WINCE
#	define SKINSDIR L"Program Files/PPC_ZTetris/Skins/"
#else
#	define SKINSDIR L"Skins/"
#endif

wchar_t *GetSkinList() {
	HANDLE fh;
	WIN32_FIND_DATA ffd;
	wchar_t *result;
	UINT sz=1, psz;
	size_t sl1;

	ffd.cFileName[0] = 0;

	fh = FindFirstFile(SKINSDIR L"*", &ffd);
	if (fh == INVALID_HANDLE_VALUE) return NULL;

	result=(wchar_t*)malloc(sz*sizeof(wchar_t));
	if (!result) return NULL;
	*result=0;

	do {
		if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || *ffd.cFileName=='.') continue;

		psz=sz;
		if ((sl1=wcslen(ffd.cFileName))>MAX_PATH) {
			free(result);
			return NULL;
		}
		sz+=(UINT)sl1+1;
		result=(wchar_t*)realloc(result, sz*2);
		wcsncpy(result+psz-1, ffd.cFileName, sl1+1);
		result[sz-1]=0;
	} while (FindNextFile(fh, &ffd));
	FindClose(fh);
	return result;
}

int SkinFileExists(const wchar_t *fname) {
	WIN32_FIND_DATA ffd;
	HANDLE fh;
	int r=0;

	if ((fh=FindFirstFile(fname, &ffd))!=INVALID_HANDLE_VALUE) {
		r = !(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
		FindClose(fh);
	}
	return r;
}

/* returna
	-1 ako nema fajle
	-2 ako nema skina
	-3 ako pathlen nije dovoljan */
int GetSkinBackground(const wchar_t *imeskina, wchar_t *path, UINT pathlen) {
	if (wcschr(imeskina, L'\\')) return -2;
	if (wcslen(wcsncpy(path, SKINSDIR, pathlen)) > pathlen ||
		wcslen(wcsncat(path, imeskina, pathlen)) > pathlen ||
		wcslen(wcsncat(path, L"/pozadina.bmp", pathlen)) > pathlen) return -3;
	if (!SkinFileExists(path)) return -1;

	return 0;
}

/* returna
	-1 ako nema fajle
	-2 ako nema skina
	-3 ako pathlen nije dovoljan */
int GetSkinFigureImg(const wchar_t *imeskina, wchar_t *path, UINT pathlen) {
	if (wcschr(imeskina, L'\\')) return -2;
	if (wcslen(wcsncpy(path, SKINSDIR, pathlen)) > pathlen ||
		wcslen(wcsncat(path, imeskina, pathlen)) > pathlen ||
		wcslen(wcsncat(path, L"/figure.bmp", pathlen)) > pathlen) return -3;
	if (!SkinFileExists(path)) return -1;

	return 0;
}

/* returna
	-1 ako nema fajle
	-2 ako nema skina
	-3 ako pathlen nije dovoljan */
int GetSkinPauseImg(const wchar_t *imeskina, wchar_t *path, UINT pathlen) {
	if (wcschr(imeskina, L'\\')) return -2;
	if (wcslen(wcsncpy(path, SKINSDIR, pathlen)) > pathlen ||
		wcslen(wcsncat(path, imeskina, pathlen)) > pathlen ||
		wcslen(wcsncat(path, L"/pauza.bmp", pathlen)) > pathlen) return -3;
	if (!SkinFileExists(path)) return -1;

	return 0;
}