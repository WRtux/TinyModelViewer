#include "helper.h"
#include <stdio.h>
#include <windows.h>

int __ms_vsnprintf(char *buf, size_t cnt, const char *fmt, va_list argp) {
	return vsnprintf(buf, cnt, fmt, argp);
} // Support static linking

extern void SetProcessDPIAware(void); // Support old Windows header
uint initProcess(char ***argsp) {
	if (LOBYTE(GetVersion()) < 0x06) {
		messageBox("Windows outdated.", NULL, MESSAGE_WARNING);
	} else {
		SetProcessDPIAware();
	}
	if (argsp == NULL)
		return 0;
	int cnt = 0;
	wchar **wargs = CommandLineToArgvW(GetCommandLineW(), &cnt);
	*argsp = (cnt > 0) ? malloc(sizeof(char*) * cnt) : NULL;
	if (cnt > 0 && *argsp == NULL)
		abort();
	for (int i = 0; i < cnt; i++) {
		if (stringConvertMultiBuffer(wargs[i], -1, &(*argsp)[i]) == 0)
			abort();
	}
	LocalFree(wargs);
	return cnt;
}

void *zalloc(uint s) {
	void *p = malloc(s);
	if (p == NULL)
		return NULL;
	return memset(p, 0, s);
}

void vfree(void **ps, uint cnt) {
	for (uint i = 0; i < cnt; i++) {
		free(ps[i]);
	}
	free(ps);
}

bool uchdir(const char *fp) {
	return SetCurrentDirectoryW(stringConvertWideBufferS(fp));
}

IOFile *ufopen(const char *fn, const char *m) {
	wchar *wm;
	stringConvertWideBuffer(m, -1, &wm);
	FILE *f = _wfopen(stringConvertWideBufferS(fn), wm);
	if (wm != NULL)
		free(wm);
	return f;
}

void messageBox(const char *txt, const char *tt, uint typ) {
	wchar *wtt;
	stringConvertWideBuffer(tt, -1, &wtt);
	MessageBoxW(NULL, stringConvertWideBufferS(txt), wtt, typ);
	if (wtt != NULL)
		free(wtt);
}
