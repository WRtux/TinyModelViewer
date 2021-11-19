#include "helper.h"
#include <stdio.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>
#include <GL/glfw3.h>

// Replace native to avoid including Windows header
extern VGLHWindow *glfwGetWin32Window(GLFWwindow* wnd);
extern void *glfwGetWGLContext(GLFWwindow* wnd);

const float ambientLight[] = {0.2, 0.2, 0.2, 1.0};
const float directedLight[] = {0.8, 0.8, 0.8, 1.0};

VGLHModel *currentModel;

void initData(uint argcnt, char **args) {
	if (!imageInit()) {
		messageBox("GDI+ error.", NULL, MESSAGE_ERROR);
		abort();
	}
	uchdir(argcnt == 2 ? args[1] : "model/");
	currentModel = vglhLoadModel("model.dat", NULL);
	imageTerminate();
}

GLFWwindow *initGLFW(void) {
	int i, j;
	glfwGetVersion(&i, &j, NULL);
	if (i < 3 || j < 1) {
		messageBox("GLFW version too low.", NULL, MESSAGE_ERROR | MESSAGE_MODAL);
		abort();
	}
	if (!glfwInit())
		abort();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_DOUBLEBUFFER, true);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_SRGB_CAPABLE, true);
	GLFWwindow *wnd = glfwCreateWindow(640, 480, "Tiny Model Viewer", NULL, NULL);
	if (wnd == NULL) {
		messageBox("Context error.", NULL, MESSAGE_ERROR | MESSAGE_MODAL);
		abort();
	}
	return wnd;
}

void initGraphics(void) {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_LIGHTING);
	glClearColor(ambientLight[0], ambientLight[1], ambientLight[2], ambientLight[3]);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);
	glEnable(GL_COLOR_MATERIAL);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_RESCALE_NORMAL);
	glEnable(GL_TEXTURE_2D);
	if (currentModel != NULL) {
		for (ushort i = 0; i < currentModel->textureCount; i++) {
			vglhCreateTexture(&currentModel->textures[i]);
		}
	}
}

void resize(GLFWwindow *wnd, int w, int h) {
	glViewport(0, 0, w, h);
}

void display(GLFWwindow *wnd) {
	if (glfwWindowShouldClose(wnd))
		return;
	int l, t;
	uint w, h;
	if (!vglhGetViewport(&l, &t, &w, &h)) {
		glfwSetWindowShouldClose(wnd, true);
		messageBox("Context error.", NULL, MESSAGE_ERROR | MESSAGE_MODAL);
		return;
	}
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	{
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		uint min = (w <= h) ? w : h;
		double dx = 0.1 / 2 * w / min, dy = 0.1 / 2 * h / min;
		glFrustum(-dx, dx, -dy, dy, 0.075, 1000.0);
	} {
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(0.0, 12.0, 20.0, 0.0, 10.0, 0.0, 0.0, 1.0, 0.0);
		glEnable(GL_LIGHT0);
		float pos[] = {0.0, 0.0, 10.0, 0.0};
		glLightfv(GL_LIGHT0, GL_POSITION, pos);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, directedLight);
		glLightfv(GL_LIGHT0, GL_SPECULAR, directedLight);
		glRotated(180.0, 0.0, 1.0, 0.0);
		if (currentModel != NULL)
			vglhDrawModel(currentModel);
	}
	glFlush();
	glfwSwapBuffers(wnd);
	{
		VGLHWindow *hwnd = glfwGetWin32Window(wnd);
		vglhTextConfig(hwnd, ALIGN_TOP | ALIGN_LEFT, 0x66FFFF);
		vglhDrawText(hwnd, "View Mode", l, t);
		vglhTextConfig(hwnd, ALIGN_BOTTOM | ALIGN_LEFT, 0x66FFFF);
		if (currentModel != NULL) {
			VGLHModel *mod = currentModel;
			uint cnt = 0;
			for (uint i = 0; i < mod->componentCount; i++) {
				cnt += mod->components[i].triangleCount;
			}
			sprintf(stringBuffer, "Comp.%d, tex.%d, tri.%d",
				mod->componentCount, mod->textureCount, cnt);
			vglhDrawText(hwnd, stringBuffer, l, t + h);
		} else {
			vglhDrawText(hwnd, "No model", l, t + h);
		}
	}
}

void start(void) {
	char **args;
	uint argcnt = initProcess(&args);
	initData(argcnt, args);
	for (uint i = 0; i < argcnt; i++) {
		free(args[0]);
	}
	free(args);
	GLFWwindow *wnd = initGLFW();
	glfwMakeContextCurrent(wnd);
	initGraphics();
	glfwSetFramebufferSizeCallback(wnd, &resize);
	glfwSetWindowRefreshCallback(wnd, &display);
	while (!glfwWindowShouldClose(wnd)) {
		glfwWaitEvents();
	}
	glfwDestroyWindow(wnd);
	glfwTerminate();
	exit(0);
}
