#include <GL/freeglut.h>

void drawTriangle(const float vs[3][3], const float vns[3][3], const float vts[3][2]) {
//	double vn[3][3] = {
//		{vs[0][0] - vs[1][0], vs[0][1] - vs[1][1], vs[0][2] - vs[1][2]},
//		{vs[0][0] - vs[2][0], vs[0][1] - vs[2][1], vs[0][2] - vs[2][2]}
//	};
//	vn[2][0] = vn[0][1] * vn[1][2] - vn[0][2] * vn[1][1];
//	vn[2][1] = vn[1][0] * vn[0][2] - vn[1][2] * vn[0][0];
//	vn[2][2] = vn[1][1] * vn[0][0] - vn[1][0] * vn[0][1];
//	double d = sqrt(vn[2][0] * vn[2][0] + vn[2][1] * vn[2][1] + vn[2][2] * vn[2][2]);
//	vn[2][0] /= d, vn[2][1] /= d, vn[2][2] /= d;
	glBegin(GL_TRIANGLE_STRIP);
	for (int i = 0; i < 3; i++) {
		glNormal3fv(vns[i]);
		glTexCoord2f(vts[i][0], 1.0 - vts[i][1]);
		glVertex3fv(vs[i]);
	}
	glEnd();
}
