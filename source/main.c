#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <png.h>
#include <GL/freeglut.h>

typedef unsigned char byte, uchar;
typedef unsigned short ushort;
typedef unsigned int uint;

extern void drawTriangle(const float vs[3][3], const float vns[3][3], const float vts[3][2]);

const float ambientLight[] = {0.2, 0.2, 0.2, 1.0};
const float directedLight[] = {0.8, 0.8, 0.8, 1.0};

typedef struct {
	float vertices[3][3];
	float normals[3][3];
	float textureBinds[3][2];
} triangle_model;

typedef struct {
	uint id;
	ushort width, height;
	const byte (*pixels)[3];
} texture_model;

typedef struct {
	float color[3];
	float specularColor[3];
	texture_model *texture;
	uint triangleCount;
	triangle_model *triangles;
} component_model;

typedef struct {
	uint textureCount;
	texture_model *textures;
	uint componentCount;
	component_model *components;
} model;

model *currentModel;

texture_model *loadTextureImage(const char *fn, texture_model *tex) {
	FILE *fp = fopen(fn, "rb");
	if (fp == NULL)
		return NULL;
	if (tex == NULL)
		tex = malloc(sizeof(texture_model));
	png_struct *ps = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	png_info *pi = png_create_info_struct(ps);
	png_init_io(ps, fp);
	png_read_png(ps, pi,
		PNG_TRANSFORM_EXPAND | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_STRIP_ALPHA, NULL);
	fclose(fp);
	uint w, h;
	png_get_IHDR(ps, pi, &w, &h, NULL, NULL, NULL, NULL, NULL);
	tex->width = w;
	tex->height = h;
	tex->pixels = malloc(3 * w * h);
	byte (**pxss)[3] = (byte(**)[3])png_get_rows(ps, pi);
	for (uint i = 0; i < h; i++) {
		memcpy((byte(*)[3])&tex->pixels[w * i], pxss[i], 3 * w);
	}
	png_destroy_read_struct(&ps, &pi, NULL);
	return tex;
}

model *loadModel(const char *fn, model *mod) {
	FILE *fp = fopen(fn, "rb");
	if (fp == NULL)
		return NULL;
	if (mod == NULL)
		mod = malloc(sizeof(model));
	struct {
		uint header;
		ushort textureCount;
		ushort componentCount;
	} buf;
	fread(&buf, sizeof(buf), 1, fp);
	if (buf.header != 0x4A424FFF)
		return NULL;
	mod->textureCount = buf.textureCount;
	mod->textures = malloc(sizeof(texture_model) * buf.textureCount);
	for (ushort i = 0; i < buf.textureCount; i++) {
		ushort len;
		fread(&len, sizeof(ushort), 1, fp);
		char *fn = malloc(len + 1);
		fread(fn, len, 1, fp);
		fn[len] = '\0';
		loadTextureImage(fn, &mod->textures[i]);
		free(fn);
	}
	mod->componentCount = buf.componentCount;
	mod->components = malloc(sizeof(component_model) * buf.componentCount);
	for (ushort i = 0; i < buf.componentCount; i++) {
		struct {
			float color[3];
			float specularColor[3];
			ushort textureIndex;
			ushort counts[3];
		} buf;
		component_model *comp = &mod->components[i];
		fread(&buf, sizeof(buf), 1, fp);
		memcpy(&comp->color, &buf.color, sizeof(float[3]));
		memcpy(&comp->specularColor, &buf.specularColor, sizeof(float[3]));
		comp->texture = &mod->textures[buf.textureIndex];
		struct {
			float (*vertices)[3];
			float (*normals)[3];
			float (*textureBinds)[2];
		} vi;
		vi.vertices = malloc(sizeof(float[3]) * buf.counts[0]);
		fread(vi.vertices, sizeof(float[3]), buf.counts[0], fp);
		vi.normals = malloc(sizeof(float[3]) * buf.counts[1]);
		fread(vi.normals, sizeof(float[3]), buf.counts[1], fp);
		vi.textureBinds = malloc(sizeof(float[2]) * buf.counts[2]);
		fread(vi.textureBinds, sizeof(float[2]), buf.counts[2], fp);
		fread(&buf.counts, sizeof(ushort[3]), 1, fp);
		comp->triangleCount = buf.counts[2];
		comp->triangles = malloc(sizeof(triangle_model) * buf.counts[2]);
		for (ushort j = 0; j < buf.counts[2]; j++) {
			triangle_model *tri = &comp->triangles[j];
			ushort iss[3][3];
			fread(iss, sizeof(ushort[3][3]), 1, fp);
			for (uchar k = 0; k < 3; k++) {
				memcpy(tri->vertices[k], vi.vertices[iss[0][k]], sizeof(float[3]));
				memcpy(tri->normals[k], vi.normals[iss[1][k]], sizeof(float[3]));
				memcpy(tri->textureBinds[k], vi.textureBinds[iss[2][k]], sizeof(float[2]));
			}
		}
		free(vi.vertices);
		free(vi.normals);
		free(vi.textureBinds);
	}
	fclose(fp);
	return mod;
}

void initTexture(texture_model *tex) {
	glGenTextures(1, &tex->id);
	glBindTexture(GL_TEXTURE_2D, tex->id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex->width, tex->height, 0, GL_RGB, GL_UNSIGNED_BYTE, tex->pixels);
	free((byte(*)[3])tex->pixels);
	tex->pixels = NULL;
//	glDeleteTextures(1, &id);
}

void init(uint argcnt, const char **args) {
	chdir(argcnt == 2 ? args[1] : "model/");
	currentModel = loadModel("model.dat", NULL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glClearColor(ambientLight[0], ambientLight[1], ambientLight[2], ambientLight[3]);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);
	glEnable(GL_COLOR_MATERIAL);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_TEXTURE_2D);
	for (ushort i = 0; i < currentModel->textureCount; i++) {
		initTexture(&currentModel->textures[i]);
	}
}

void reshape(int w, int h) {
	int min = (w <= h) ? w : h;
	double dx = 0.1 / 2 * w / min, dy = 0.1 / 2 * h / min;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-dx, dx, -dy, dy, 0.075, 1000.0);
}

void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0, 12.0, 20.0, 0.0, 10.0, 0.0, 0.0, 1.0, 0.0);
	glEnable(GL_LIGHT0);
	float pos[] = {0.0, 0.0, 10.0, 0.0};
	glLightfv(GL_LIGHT0, GL_POSITION, pos);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, directedLight);
	glLightfv(GL_LIGHT0, GL_SPECULAR, directedLight);
	glRotated(180.0, 0.0, 1.0, 0.0);
	for (ushort i = 0; i < currentModel->componentCount; i++) {
		component_model *comp = &currentModel->components[i];
		glColor3fv(comp->color);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, comp->specularColor);
		glBindTexture(GL_TEXTURE_2D, comp->texture->id);
		for (ushort j = 0; j < comp->triangleCount; j++) {
			triangle_model *tri = &comp->triangles[j];
			drawTriangle(tri->vertices, tri->normals, tri->textureBinds);
		}
	}
	glFlush();
	glutSwapBuffers();
}

int main(int argcnt, char **args) {
	glutInit(&argcnt, args);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(600, 600);
	glutCreateWindow("RenderModel");
	init(argcnt, (const char**)args);
	glutReshapeFunc(&reshape);
	glutDisplayFunc(&display);
	glutMainLoop();
	return 0;
}
