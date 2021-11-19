#include "helper.h"
#include <stdio.h>
#include <windows.h>

int __ms_vsnprintf(char *buf, size_t cnt, const char *fmt, va_list argp) {
	return vsnprintf(buf, cnt, fmt, argp);
} // Support static linking

const void *const nullish = NULL;

uint initProcess(char ***argsp) {
	if (LOBYTE(GetVersion()) >= 0x06) {
		HMODULE hmod = LoadLibraryA("user32.dll");
		if (hmod == NULL)
			ExitProcess(GetLastError());
		GetProcAddress(hmod, "SetProcessDPIAware")();
		FreeLibrary(hmod);
	} else {
		messageBox("Windows outdated.", NULL, MESSAGE_WARNING);
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

uint ufget(IOFile *f, char **strp) {
	ushort lim;
	if (fread(&lim, sizeof(ushort), 1, f) < 1) {
		*strp = NULL;
		return 0;
	}
	*strp = malloc(lim);
	if (*strp == NULL)
		return fseek(f, SEEK_CUR, lim) == 0 ? lim : 0;
	if (fread(*strp, lim, 1, f) < 1) {
		free(*strp);
		*strp = NULL;
		return 0;
	}
	return lim;
}

void messageBox(const char *txt, const char *tt, uint typ) {
	wchar *wtt;
	stringConvertWideBuffer(tt, -1, &wtt);
	MessageBoxW(NULL, stringConvertWideBufferS(txt), wtt, typ);
	if (wtt != NULL)
		free(wtt);
}
