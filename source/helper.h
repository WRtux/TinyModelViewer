#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef unsigned char byte, uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long long ulong;

typedef unsigned short wchar;

#define bcatch(exp,lbl); if (!(exp)) goto lbl;
#define pcatch(p,lbl); if (!*((p) || &nullish)) goto lbl;

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

typedef float VGLHModelPoint[3];
typedef float VGLHModelLine[2][3];
typedef struct {
	float vertices[3][3];
	float normals[3][3];
	float textureBinds[3][2];
} VGLHModelTriangle;

typedef struct {
	uint id;
	uint width, height;
	const byte (*pixels)[3];
} VGLHTexture;

typedef struct {
	float ambientColor[3];
	float diffuseColor[3];
	float specularColor[3];
	VGLHTexture *texture;
	uint pointCount;
	uint lineCount;
	uint triangleCount;
	VGLHModelPoint *points;
	VGLHModelLine *lines;
	VGLHModelTriangle *triangles;
} VGLHComponent;

typedef struct {
	uint textureCount;
	VGLHTexture *textures;
	uint componentCount;
	VGLHComponent *components;
} VGLHModel;

typedef void VGLHWindow;

#define ALIGN_TOP 0
#define ALIGN_BOTTOM 8
#define ALIGN_LEFT 0
#define ALIGN_CENTER 6
#define ALIGN_RIGHT 2

extern VGLHTexture *vglhLoadTextureImage(const char *fn, VGLHTexture *tex);
extern bool vglhCreateTexture(VGLHTexture *tex);
extern VGLHModel *vglhLoadModel(const char *fn, VGLHModel *mod);
extern void vglhDrawTriangle(const VGLHModelTriangle *tri);
extern bool vglhDrawComponent(const VGLHComponent *comp);
extern bool vglhDrawModel(const VGLHModel *mod);

extern bool vglhGetViewport(int *xp, int *yp, uint *wp, uint *hp);
extern bool vglhTextConfig(VGLHWindow *hwnd, uint aln, uint clr);
extern bool vglhDrawText(VGLHWindow *hwnd, const char *txt, uint x, uint y);

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

extern const void *const nullish;

extern uint initProcess(char ***argsp);
extern void *zalloc(uint s);
extern void vfree(void **ps, uint cnt);
extern bool uchdir(const char *fp);
extern IOFile *ufopen(const char *fn, const char *m);
extern uint ufget(IOFile *f, char **strp);
extern void messageBox(const char *tt, const char *txt, uint typ);
