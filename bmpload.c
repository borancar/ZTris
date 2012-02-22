#include <windows.h>
#include <stdio.h>

#ifdef WINCE

void calcWidthBytes (BITMAP *bmpData) {
	unsigned int bpl = bmpData->bmWidth * bmpData->bmBitsPixel;
	bmpData->bmWidthBytes = (bpl & 3) ? ((bpl >> 3) + 1) : (bpl >> 3);

	/*if (bmpData->bmWidthBytes & 1)
		bmpData->bmWidthBytes++;*/
	bmpData->bmWidthBytes+=(bmpData->bmWidthBytes & 1);
}

HBITMAP LoadFromFile (const wchar_t *filename) {
	BITMAP bmpData;
	FILE *fjl;
	BITMAPFILEHEADER bfh;
	BITMAPINFO bi;
	HBITMAP hBmp = NULL;
	void *newptr = NULL;
	unsigned int sz;

	fjl = _wfopen(filename, L"rb");
	if (!fjl)
		return NULL;

	fread (&bfh, sizeof(BITMAPFILEHEADER), 1, fjl);
	if (bfh.bfType != 'MB') {
		fclose (fjl);
		return NULL;
	}

	fread (&bi, sizeof(bi), 1, fjl);
	hBmp = CreateDIBSection (NULL, &bi, DIB_RGB_COLORS, &newptr, NULL, 0);
	if (!newptr) {
		if (hBmp) DeleteObject (hBmp);
		fclose (fjl);
		return NULL;
	}

	bmpData.bmWidth = bi.bmiHeader.biWidth;
	bmpData.bmBitsPixel = bi.bmiHeader.biBitCount;
	calcWidthBytes (&bmpData);
	sz = bmpData.bmWidthBytes * abs(bi.bmiHeader.biHeight);
	fseek (fjl, bfh.bfOffBits, SEEK_SET);
	fread (newptr, sz, 1, fjl);
	fclose (fjl);

	return hBmp;
}

#else

HBITMAP LoadFromFile (const wchar_t *filename) {
	return (HBITMAP)LoadImage(NULL, filename, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
}

#endif
