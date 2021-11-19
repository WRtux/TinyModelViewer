#include "helper.h"
#include <windows.h>

#define WSTRING_BUFFER_LENGTH 0x400
#define STRING_BUFFER_SIZE (WSTRING_BUFFER_LENGTH * 2)

char stringBuffer[STRING_BUFFER_SIZE];
wchar wstringBuffer[WSTRING_BUFFER_LENGTH];

uint stringConvertWideBuffer(const char *str, int s, wchar **bufp) {
	if (str == NULL || s == 0) {
		wstringBuffer[0] = L'\0';
		if (bufp != NULL)
			*bufp = NULL;
		return 0;
	}
	int len = MultiByteToWideChar(CP_UTF8, 0, str, s, wstringBuffer, WSTRING_BUFFER_LENGTH);
	if (s > 0 && len < WSTRING_BUFFER_LENGTH || s == -1 && len == 0)
		wstringBuffer[len] = L'\0';
	if (bufp != NULL) {
		*bufp = (len > 0) ? malloc(sizeof(wchar) * len) : NULL;
		if (*bufp != NULL) {
			memcpy(*bufp, wstringBuffer, sizeof(wchar) * len);
		} else {
			len = 0;
		}
	}
	return len;
}
wchar *stringConvertWideBufferS(const char *str) {
	if (str == NULL)
		return NULL;
	int len = MultiByteToWideChar(CP_UTF8, 0, str, -1, wstringBuffer, WSTRING_BUFFER_LENGTH);
	return len > 0 ? wstringBuffer : NULL;
}

uint stringConvertWide(const char *str, int s, wchar **bufp) {
	if (str == NULL || s == 0) {
		*bufp = NULL;
		return 0;
	}
	int len = MultiByteToWideChar(CP_UTF8, 0, str, s, NULL, 0);
	*bufp = (len > 0) ? malloc(sizeof(wchar) * len) : NULL;
	return *bufp && MultiByteToWideChar(CP_UTF8, 0, str, s, *bufp, len);
}
wchar *stringConvertWideS(const char *str) {
	if (str == NULL)
		return NULL;
	int len = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
	wchar *wstr = (len > 0) ? malloc(sizeof(wchar) * len) : NULL;
	if (wstr != NULL)
		MultiByteToWideChar(CP_UTF8, 0, str, -1, wstr, len);
	return wstr;
}

uint stringConvertMultiBuffer(const wchar *wstr, int len, char **bufp) {
	if (wstr == NULL || len == 0) {
		stringBuffer[0] = '\0';
		if (bufp != NULL)
			*bufp = NULL;
		return 0;
	}
	int s = WideCharToMultiByte(CP_UTF8, 0, wstr, len, stringBuffer, STRING_BUFFER_SIZE, NULL, NULL);
	if (len > 0 && s < STRING_BUFFER_SIZE || len == -1 && s == 0)
		stringBuffer[s] = '\0';
	if (bufp != NULL) {
		*bufp = (s > 0) ? malloc(sizeof(char) * s) : NULL;
		if (*bufp != NULL) {
			memcpy(*bufp, stringBuffer, sizeof(char) * s);
		} else {
			s = 0;
		}
	}
	return s;
}
char *stringConvertMultiBufferS(const wchar *wstr) {
	if (wstr == NULL)
		return NULL;
	int s = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, stringBuffer, STRING_BUFFER_SIZE, NULL, NULL);
	return s > 0 ? stringBuffer : NULL;
}

uint stringConvertMulti(const wchar *wstr, int len, char **bufp) {
	if (wstr == NULL || len == 0) {
		*bufp = NULL;
		return 0;
	}
	int s = WideCharToMultiByte(CP_UTF8, 0, wstr, len, NULL, 0, NULL, NULL);
	*bufp = (s > 0) ? malloc(sizeof(char) * s) : NULL;
	return *bufp && WideCharToMultiByte(CP_UTF8, 0, wstr, len, *bufp, s, NULL, NULL);
}
char *stringConvertMultiS(const wchar *wstr) {
	if (wstr == NULL)
		return NULL;
	int s = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	char *str = (s > 0) ? malloc(sizeof(wchar) * s) : NULL;
	if (str != NULL)
		WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, s, NULL, NULL);
	return str;
}

uint stringACPConvertMultiBuffer(const char *str, int s, char **bufp) {
	if (str == NULL || s == 0) {
		stringBuffer[0] = '\0';
		if (bufp != NULL)
			*bufp = NULL;
		return 0;
	}
	int len = MultiByteToWideChar(CP_ACP, 0, str, s, wstringBuffer, WSTRING_BUFFER_LENGTH);
	if (len > 0)
		len = WideCharToMultiByte(CP_UTF8, 0, wstringBuffer, len, stringBuffer, STRING_BUFFER_SIZE, NULL, NULL);
	if (s > 0 && len < STRING_BUFFER_SIZE || s == -1 && len == 0)
		stringBuffer[len] = '\0';
	if (bufp != NULL) {
		*bufp = (len > 0) ? malloc(sizeof(char) * len) : NULL;
		if (*bufp != NULL) {
			memcpy(*bufp, stringBuffer, sizeof(char) * len);
		} else {
			len = 0;
		}
	}
	return len;
}
char *stringACPConvertMultiBufferS(const char *str) {
	if (str == NULL)
		return NULL;
	int len = MultiByteToWideChar(CP_ACP, 0, str, -1, wstringBuffer, WSTRING_BUFFER_LENGTH);
	if (len > 0)
		len = WideCharToMultiByte(CP_UTF8, 0, wstringBuffer, len, stringBuffer, STRING_BUFFER_SIZE, NULL, NULL);
	return len > 0 ? stringBuffer : NULL;
}
