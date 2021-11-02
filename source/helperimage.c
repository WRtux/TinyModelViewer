#include "helper.h"
#include <windows.h>
#include <gdiplus/gdiplus.h>

static size_t gdipToken = 0;

bool imageInit(void) {
	GdiplusStartupInput param = {1, NULL, FALSE, FALSE};
	return GdiplusStartup(&gdipToken, &param, NULL) == Ok;
}

bool imageTerminate(void) {
	if (gdipToken == 0)
		return false;
	GdiplusShutdown(gdipToken);
	gdipToken = 0;
	return true;
}

GDIPImage *imageLoadFile(const char *fn) {
	GpBitmap *himg;
	if (GdipCreateBitmapFromFile(stringConvertWideBufferS(fn), &himg) != Ok)
		return NULL;
	return himg;
}

bool imageDispose(GDIPImage *himg) {
	return GdipDisposeImage(himg) == Ok;
}

bool imageGetSize(GDIPImage *himg, uint *wp, uint *hp) {
	if (wp == NULL && hp == NULL)
		return false;
	bool f = true;
	if (wp != NULL)
		f = (GdipGetImageWidth(himg, wp) == Ok);
	if (hp != NULL && f)
		f = (GdipGetImageHeight(himg, hp) == Ok);
	return f;
}

byte (*imageGetPixels(GDIPImage *himg, byte (*buf)[3]))[3] {
	uint w, h;
	uint res = GdipGetImageWidth(himg, &w) || GdipGetImageHeight(himg, &h);
	if (res != Ok)
		return NULL;
	Rect rect = {0, 0, w, h};
	byte (*nbuf)[3] = (buf != NULL) ? buf : malloc(3 * w * h);
	BitmapData dat = {w, h, 3 * w, PixelFormat24bppRGB, nbuf};
	if (nbuf == NULL)
		return NULL;
//	GdipBitmapConvertFormat(himg, PixelFormat24bppRGB, DitherTypeSolid, PaletteTypeCustom, NULL, 80.0);
	res = GdipBitmapLockBits(himg, &rect,
		ImageLockModeRead | ImageLockModeUserInputBuf, PixelFormat24bppRGB, &dat);
	if (res != Ok) {
		if (buf == NULL)
			free(nbuf);
		return NULL;
	}
	GdipBitmapUnlockBits(himg, &dat);
	return nbuf;
}
