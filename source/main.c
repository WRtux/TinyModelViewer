#include "helper.h"
#include <stdio.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>
#include <GL/glfw3.h>

// Replace glfw3native to avoid including Windows header
extern VGLHWindow *glfwGetWin32Window(GLFWwindow* wnd);
extern void *glfwGetWGLContext(GLFWwindow* wnd);

float cameraPosition[3] = {0.0, 10.0, 25.0};
float cameraOrientation[2] = {0.0, -5.0};

const float ambientLight[] = {0.2, 0.2, 0.2, 1.0};
const float directedLight[] = {0.8, 0.8, 0.8, 1.0};

float lightPosition[4] = {0.0, 20.0, 20.0, 1.0};

VGLHModel *currentModel = NULL;

float transformMatrix[4][4];

byte interactMode = ' ';

byte dragMode = ' ';
float dragDistance = 0.0;

void initData(uint argcnt, char **args) {
	if (!imageInit()) {
		messageBox("GDI+ error.", NULL, MESSAGE_ERROR);
		abort();
	}
	currentModel = vglhLoadModel(argcnt == 2 ? args[1] : "model.dat", NULL);
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
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glGetFloatv(GL_MODELVIEW_MATRIX, (float*)transformMatrix);
	if (currentModel != NULL) {
		for (uint i = 0; i < currentModel->textureCount; i++) {
			vglhCreateTexture(&currentModel->textures[i]);
		}
	}
	interactMode = 'V';
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
		glRotatef(-cameraOrientation[1], 1.0, 0.0, 0.0);
		glRotatef(-cameraOrientation[0], 0.0, 1.0, 0.0);
		glTranslatef(-cameraPosition[0], -cameraPosition[1], -cameraPosition[2]);
		glEnable(GL_LIGHT0);
		glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, directedLight);
		glLightfv(GL_LIGHT0, GL_SPECULAR, directedLight);
	} {
		glBindTexture(GL_TEXTURE_2D, 0);
		glColor3f(0.4, 0.4, 0.4);
		glBegin(GL_QUADS);
			glNormal3f(0.0, 1.0, 0.0);
			glVertex3f(-10.0, -0.1, -10.0);
			glVertex3f(10.0, -0.1, -10.0);
			glVertex3f(10.0, -0.1, 10.0);
			glVertex3f(-10.0, -0.1, 10.0);
		glEnd();
		glBegin(GL_LINES);
			glColor3f(1.0, 0.2, 0.2);
			glVertex3f(0.0, 0.0, 0.0);
			glVertex3f(10.0, 0.0, 0.0);
			glColor3f(0.2, 1.0, 0.2);
			glVertex3f(0.0, -0.1, 0.0);
			glVertex3f(0.0, 20.0, 0.0);
			glColor3f(0.2, 0.2, 1.0);
			glVertex3f(0.0, 0.0, 0.0);
			glVertex3f(0.0, 0.0, 10.0);
		glEnd();
		glPushMatrix();
		glMultMatrixf((float*)transformMatrix);
		if (currentModel != NULL)
			vglhDrawModel(currentModel);
		glPopMatrix();
	}
	glFlush();
	glfwSwapBuffers(wnd);
	{
		VGLHWindow *hwnd = glfwGetWin32Window(wnd);
		vglhTextConfig(hwnd, ALIGN_TOP | ALIGN_LEFT, 0x66FFFF);
		char *tt = (interactMode == 'V' ? "View Mode" :
			interactMode == 'F' ? "Free Control Mode" :
			interactMode == 'E' ? "Component Edit Mode" : "Unknown Mode");
		vglhDrawText(hwnd, tt, l, t);
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

void inputKey(GLFWwindow *wnd, int k, int id, int st, int m) {
	if (st == GLFW_PRESS || st == GLFW_REPEAT) {
		switch (interactMode) {
		case 'V':
		case 'F':
		case 'E': {
			bool f = false;
			float d[] = {0.0, 0.0, 0.0};
			switch (k) {
			case GLFW_KEY_A:
				f = true;
				d[0] = -1.0;
				break;
			case GLFW_KEY_D:
				f = true;
				d[0] = 1.0;
				break;
			case GLFW_KEY_W:
				f = true;
				d[2] = -1.0;
				break;
			case GLFW_KEY_S:
				f = true;
				d[2] = 1.0;
				break;
			case GLFW_KEY_Q:
				f = true;
				d[1] = 1.0;
				break;
			case GLFW_KEY_E:
				f = true;
				d[1] = -1.0;
				break;
			}
			if (f) {
				glMatrixMode(GL_MODELVIEW);
				glPushMatrix();
				glLoadIdentity();
				glRotatef(cameraOrientation[0], 0.0, 1.0, 0.0);
				if (interactMode != 'F')
					glRotatef(cameraOrientation[1], 1.0, 0.0, 0.0);
				float buf[4][4] = {{d[0], d[1], d[2], 1.0}};
				glMultMatrixf((float*)buf);
				glGetFloatv(GL_MODELVIEW_MATRIX, (float*)buf);
				for (uint i = 0; i < 3; i++) {
					cameraPosition[i] += buf[0][i];
				}
				glPopMatrix();
				display(wnd);
			}
			break;
		}
		}
	}
	if (st == GLFW_PRESS) {
		if (interactMode == 'V' || interactMode == 'F') {
			switch (k) {
			case GLFW_KEY_L:
				lightPosition[3] = (lightPosition[3] < 0.8 ? 1.0 : 0.0);
			case GLFW_KEY_F:
				for (uint i = 0; i < 3; i++) {
					lightPosition[i] = cameraPosition[i];
				}
				display(wnd);
				break;
			}
		}
	}
}

void inputClick(GLFWwindow *wnd, int butn, int st, int m) {
	if (st == GLFW_RELEASE && dragDistance < 4.0) {
		switch (interactMode) {
		case 'V':
			switch (butn) {
			case GLFW_MOUSE_BUTTON_LEFT:
				interactMode = 'F';
				glfwSetInputMode(wnd, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				display(wnd);
				break;
			case GLFW_MOUSE_BUTTON_RIGHT:
			case GLFW_MOUSE_BUTTON_MIDDLE:
				interactMode = 'E';
				display(wnd);
				break;
			}
			break;
		case 'F':
			switch (butn) {
			case GLFW_MOUSE_BUTTON_LEFT:
				interactMode = 'V';
				glfwSetInputMode(wnd, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				display(wnd);
				break;
			case GLFW_MOUSE_BUTTON_MIDDLE:
				interactMode = 'E';
				glfwSetInputMode(wnd, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				display(wnd);
				break;
			}
			break;
		case 'E':
			switch (butn) {
			case GLFW_MOUSE_BUTTON_MIDDLE:
				interactMode = 'V';
				display(wnd);
				break;
			}
			break;
		}
	}
	if (st == GLFW_PRESS) {
		if (dragMode == ' ') {
			dragMode = (butn == GLFW_MOUSE_BUTTON_LEFT ? 'L' :
				butn == GLFW_MOUSE_BUTTON_RIGHT ? 'R' :
				butn == GLFW_MOUSE_BUTTON_MIDDLE ? 'M' : 'U');
			dragDistance = 0.0;
		} else {
			dragMode = 'X';
		}
	} else if (st == GLFW_RELEASE) {
		dragMode = ' ';
		dragDistance = 0.0;
	}
}

void inputMove(GLFWwindow *wnd, double x, double y) {
	static double lx, ly;
	double dx = x - lx, dy = y - ly;
	dragDistance += (dx >= 0.0 ? dx : -dx) + (dy >= 0.0 ? dy : -dy);
	switch (interactMode) {
	case 'V':
		switch (dragMode) {
		case 'L':
			cameraOrientation[0] -= dx * 180 / 1000;
			cameraOrientation[1] -= dy * 180 / 1000;
			cameraOrientation[1] = min(max(cameraOrientation[1], -90.0), 90.0);
			display(wnd);
			break;
		case 'R': {
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();
			glRotatef(cameraOrientation[0], 0.0, 1.0, 0.0);
			glRotatef(cameraOrientation[1], 1.0, 0.0, 0.0);
			float buf[4][4] = {{-dx / 50, dy / 50, 0.0, 1.0}};
			glMultMatrixf((float*)buf);
			glGetFloatv(GL_MODELVIEW_MATRIX, (float*)buf);
			for (uint i = 0; i < 3; i++) {
				cameraPosition[i] += buf[0][i];
			}
			glPopMatrix();
			display(wnd);
			break;
		}
		}
		break;
	case 'F':
		cameraOrientation[0] -= dx * 180 / 1000;
		cameraOrientation[1] -= dy * 180 / 1000;
		cameraOrientation[1] = min(max(cameraOrientation[1], -90.0), 90.0);
		display(wnd);
		break;
	case 'E':
		switch (dragMode) {
		case 'L': {
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();
			glRotatef(cameraOrientation[0], 0.0, 1.0, 0.0);
			glRotatef(cameraOrientation[1], 1.0, 0.0, 0.0);
			float buf[4][4] = {{dx / 50, -dy / 50, 0.0, 1.0}};
			glMultMatrixf((float*)buf);
			glGetFloatv(GL_MODELVIEW_MATRIX, (float*)buf);
			glLoadIdentity();
			glTranslatef(buf[0][0], buf[0][1], buf[0][2]);
			glMultMatrixf((float*)transformMatrix);
			glGetFloatv(GL_MODELVIEW_MATRIX, (float*)transformMatrix);
			glPopMatrix();
			display(wnd);
			break;
		} case 'R':
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadMatrixf((float*)transformMatrix);
			glRotatef(dx * 180 / 1000, 0.0, 1.0, 0.0);
			glGetFloatv(GL_MODELVIEW_MATRIX, (float*)transformMatrix);
			glPopMatrix();
			display(wnd);
			break;
		}
		break;
	}
	lx = x, ly = y;
}

void inputScroll(GLFWwindow *wnd, double dx, double dy) {
	dragDistance += (dx >= 0.0 ? dx : -dx) + (dy >= 0.0 ? dy : -dy);
	switch (interactMode) {
	case 'V':
	case 'F': {
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		glRotatef(cameraOrientation[0], 0.0, 1.0, 0.0);
		glRotatef(cameraOrientation[1], 1.0, 0.0, 0.0);
		float buf[4][4] = {{-dx, 0.0, -dy, 1.0}};
		glMultMatrixf((float*)buf);
		glGetFloatv(GL_MODELVIEW_MATRIX, (float*)buf);
		cameraPosition[0] += buf[0][0];
		cameraPosition[1] += buf[0][1];
		cameraPosition[2] += buf[0][2];
		glPopMatrix();
		display(wnd);
		break;
	} case 'E':
		switch (dragMode) {
		case ' ': {
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadMatrixf((float*)transformMatrix);
			float s = 1.0 + (dx + dy) * 0.05;
			glScalef(s, s, s);
			glGetFloatv(GL_MODELVIEW_MATRIX, (float*)transformMatrix);
			glPopMatrix();
			display(wnd);
			break;
		} case 'L': {
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();
			glRotatef(cameraOrientation[0], 0.0, 1.0, 0.0);
			glRotatef(cameraOrientation[1], 1.0, 0.0, 0.0);
			float buf[4][4] = {{dx, 0.0, dy, 1.0}};
			glMultMatrixf((float*)buf);
			glGetFloatv(GL_MODELVIEW_MATRIX, (float*)buf);
			glLoadIdentity();
			glTranslatef(buf[0][0], buf[0][1], buf[0][2]);
			glMultMatrixf((float*)transformMatrix);
			glGetFloatv(GL_MODELVIEW_MATRIX, (float*)transformMatrix);
			glPopMatrix();
			display(wnd);
			break;
		} case 'R':
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadMatrixf((float*)transformMatrix);
			glRotatef(-dy * 7.5, 1.0, 0.0, 0.0);
			glRotatef(dx * 7.5, 0.0, 0.0, 1.0);
			glGetFloatv(GL_MODELVIEW_MATRIX, (float*)transformMatrix);
			glPopMatrix();
			display(wnd);
			break;
		}
		break;
	}
}

void start(void) {
	char **args;
	uint argcnt = initProcess(&args);
	initData(argcnt, args);
	for (uint i = 0; i < argcnt; i++) {
		free(args[i]);
	}
	free(args);
	GLFWwindow *wnd = initGLFW();
	glfwMakeContextCurrent(wnd);
	initGraphics();
	glfwSetFramebufferSizeCallback(wnd, &resize);
	glfwSetWindowRefreshCallback(wnd, &display);
	glfwSetKeyCallback(wnd, &inputKey);
	glfwSetMouseButtonCallback(wnd, &inputClick);
	glfwSetCursorPosCallback(wnd, &inputMove);
	glfwSetScrollCallback(wnd, &inputScroll);
	while (!glfwWindowShouldClose(wnd)) {
		glfwWaitEvents();
	}
	glfwDestroyWindow(wnd);
	glfwTerminate();
	exit(0);
}
