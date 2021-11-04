#include "helper.h"
#include <windows.h>
#include <GL/gl.h>

HFONT interfaceFont = NULL;

bool vglhInitFont(void) {
	NONCLIENTMETRICSW *buf = malloc(sizeof(NONCLIENTMETRICSW));
	if (buf == NULL)
		return false;
	buf->cbSize = sizeof(NONCLIENTMETRICSW);
	if (!SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICSW), buf, 0)) {
		free(buf);
		return false;
	}
	if (interfaceFont != NULL)
		DeleteObject(interfaceFont);
	interfaceFont = CreateFontIndirectW(&buf->lfStatusFont);
	free(buf);
	return interfaceFont != NULL;
}

bool vglhGetViewport(int *lp, int *tp, int *rp, int *bp) {
	if (lp == NULL && tp == NULL && rp == NULL && bp == NULL)
		return false;
	int buf[4];
	glGetError();
	glGetIntegerv(GL_VIEWPORT, buf);
	if (glGetError() != GL_NO_ERROR)
		return false;
	if (lp != NULL)
		*lp = buf[0];
	if (tp != NULL)
		*tp = buf[1];
	if (rp != NULL)
		*rp = buf[0] + buf[2];
	if (bp != NULL)
		*bp = buf[1] + buf[3];
	return true;
}

bool vglhTextConfig(VGLHWindow *hwnd, uint aln, uint clr) {
	HDC hdc = GetDC(hwnd);
	if (hdc == NULL)
		return false;
	if (interfaceFont == NULL && !vglhInitFont())
		return false;
	if (SelectObject(hdc, interfaceFont) == NULL)
		return false;
	SetTextAlign(hdc, aln);
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, clr);
	return true;
}

bool vglhDrawText(VGLHWindow *hwnd, const char *txt, uint x, uint y) {
	HDC hdc = GetDC(hwnd);
	if (hdc == NULL)
		return false;
	wchar *wstr = stringConvertWideBufferS(txt);
	if (wstr == NULL)
		return false;
	return TextOutW(hdc, x, y, wstr, wcslen(wstr));
}
