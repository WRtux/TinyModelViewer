#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef unsigned char byte, uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long long ulong;

typedef unsigned short wchar;

extern char stringBuffer[];
extern wchar wstringBuffer[];

extern uint stringConvertWideBuffer(const char *str, int s, wchar **bufp);
extern wchar *stringConvertWideBufferS(const char *str);
extern uint stringConvertWide(const char *str, int s, wchar **bufp);
extern wchar *stringConvertWideS(const char *str);
extern uint stringConvertMultiBuffer(const wchar *wstr, int len, char **bufp);
extern char *stringConvertMultiBufferS(const wchar *wstr);
extern uint stringConvertMulti(const wchar *wstr, int len, char **bufp);
extern char *stringConvertMultiS(const wchar *wstr);
extern uint stringACPConvertMultiBuffer(const char *str, int s, char **bufp);
extern char *stringACPConvertMultiBufferS(const char *str);

typedef void GDIPImage;

extern bool imageInit(void);
extern bool imageTerminate(void);
extern GDIPImage *imageLoadFile(const char *fn);
extern bool imageDispose(GDIPImage *himg);
extern bool imageGetSize(GDIPImage *himg, uint *wp, uint *hp);
extern byte (*imageGetPixels(GDIPImage *himg, byte (*buf)[3]))[3];

typedef void IOFile;

#define MESSAGE_INFO 0x40
#define MESSAGE_WARNING 0x20
#define MESSAGE_ERROR 0x10
#define MESSAGE_MODAL 0x12000

extern uint initProcess(char ***argsp);
extern bool uchdir(const char *fp);
extern IOFile *ufopen(const char *fn, const char *m);
extern void messageBox(const char *tt, const char *txt, uint typ);
