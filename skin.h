#ifndef SKIN_H
#define SKIN_H

wchar_t *GetSkinList();
int GetSkinPauseImg(const wchar_t *imeskina, wchar_t *path, UINT pathlen);
int GetSkinFigureImg(const wchar_t *imeskina, wchar_t *path, UINT pathlen);
int GetSkinBackground(const wchar_t *imeskina, wchar_t *path, UINT pathlen);

#endif