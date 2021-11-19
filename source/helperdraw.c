#include "helper.h"
#include <stdio.h>
#include <GL/gl.h>

VGLHTexture *vglhLoadTextureImage(const char *fn, VGLHTexture *tex) {
	VGLHTexture *ntex = (tex != NULL) ? tex : malloc(sizeof(VGLHTexture));
	if (ntex == NULL)
		return NULL;
	GDIPImage *himg = imageLoadFile(fn);
	bcatch(himg != NULL, c0);
	uint w, h;
	bcatch(imageGetSize(himg, &w, &h), c0);
	ntex->width = w;
	ntex->height = h;
	ntex->pixels = imageGetPixels(himg, NULL);
	bcatch(ntex->pixels != NULL, c0);
	imageDispose(himg);
	return ntex;
c0:	if (himg != NULL)
		imageDispose(himg);
	if (tex != NULL) {
		tex->id = 0;
		tex->width = 0, tex->height = 0;
		tex->pixels = NULL;
	} else {
		free(ntex);
	}
	return NULL;
}

bool vglhCreateTexture(VGLHTexture *tex) {
	glGetError();
	glGenTextures(1, &tex->id);
	glBindTexture(GL_TEXTURE_2D, tex->id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex->width, tex->height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, tex->pixels);
	return glGetError() == GL_NO_ERROR;
//	free((byte(*)[3])tex->pixels);
//	tex->pixels = NULL;
//	glDeleteTextures(1, &id);
}

VGLHModel *vglhLoadModel(const char *fn, VGLHModel *mod) {
	VGLHModel *nmod = (mod != NULL) ? mod : malloc(sizeof(VGLHModel));
	if (nmod == NULL)
		return NULL;
	memset(nmod, 0, sizeof(VGLHModel));
	IOFile *fp = ufopen(fn, "rb");
	bcatch(fp != NULL, c0);
	struct {
		uint header;
		ushort textureCount;
		ushort componentCount;
	} buf;
	bcatch(fread(&buf, sizeof(buf), 1, fp) == 1, c0);
	bcatch(buf.header == 0x4A424FFF, c0);
	nmod->textureCount = buf.textureCount;
	nmod->textures = calloc(buf.textureCount, sizeof(VGLHTexture));
	bcatch(nmod->textures != NULL, c0)
	for (ushort i = 0; i < buf.textureCount; i++) {
		ushort len;
		bcatch(fread(&len, sizeof(ushort), 1, fp) == 1, c0);
		bcatch(fread(stringBuffer, len, 1, fp) == 1, c0);
		stringBuffer[len] = '\0';
		vglhLoadTextureImage(stringBuffer, &nmod->textures[i]);
	}
	nmod->componentCount = buf.componentCount;
	nmod->components = calloc(buf.componentCount, sizeof(VGLHComponent));
	bcatch(nmod->components != NULL, c0);
	for (ushort i = 0; i < buf.componentCount; i++) {
		VGLHComponent *comp = &nmod->components[i];
		struct {
			float color[3];
			float specularColor[3];
			ushort textureIndex;
			ushort counts[3];
		} buf;
		bcatch(fread(&buf, sizeof(buf), 1, fp) == 1, c0);
		memcpy(&comp->ambientColor, &buf.color, sizeof(float[3]));
		memcpy(&comp->diffuseColor, &buf.color, sizeof(float[3])); //TODO
		memcpy(&comp->specularColor, &buf.specularColor, sizeof(float[3]));
		if (buf.textureIndex < nmod->textureCount)
			comp->texture = &nmod->textures[buf.textureIndex];
		float (*vs)[3] = malloc(sizeof(float[3]) * buf.counts[0]);
		float (*vns)[3] = malloc(sizeof(float[3]) * buf.counts[1]);
		float (*vts)[2] = malloc(sizeof(float[2]) * buf.counts[2]);
		bcatch(vs != NULL && vns != NULL && vts != NULL, c1);
		buf.counts[0] -= fread(vs, sizeof(float[3]), buf.counts[0], fp);
		buf.counts[1] -= fread(vns, sizeof(float[3]), buf.counts[1], fp);
		buf.counts[2] -= fread(vts, sizeof(float[2]), buf.counts[2], fp);
		bcatch(buf.counts[0] == 0 && buf.counts[1] == 0 && buf.counts[2] == 0, c1);
		bcatch(fread(&buf.counts, sizeof(ushort[3]), 1, fp) == 1, c1);
		comp->pointCount = buf.counts[0];
		comp->points = malloc(sizeof(VGLHModelPoint) * buf.counts[0]);
		bcatch(comp->points != NULL, c1);
		for (ushort j = 0; j < buf.counts[0]; j++) {
			ushort i;
			bcatch(fread(&i, sizeof(ushort), 1, fp) == 1, c1);
			memcpy(&comp->points[i], vs[i], sizeof(float[3]));
		}
		comp->lineCount = buf.counts[1];
		comp->lines = malloc(sizeof(VGLHModelLine) * buf.counts[1]);
		bcatch(comp->lines != NULL, c1);
		for (ushort j = 0; j < buf.counts[1]; j++) {
			ushort is[2];
			bcatch(fread(is, sizeof(ushort[2]), 1, fp) == 1, c1);
			memcpy(&comp->lines[i][0], vs[is[0]], sizeof(float[3]));
			memcpy(&comp->lines[i][1], vs[is[1]], sizeof(float[3]));
		}
		comp->triangleCount = buf.counts[2];
		comp->triangles = malloc(sizeof(VGLHModelTriangle) * buf.counts[2]);
		bcatch(comp->triangles != NULL, c1);
		for (ushort j = 0; j < buf.counts[2]; j++) {
			VGLHModelTriangle *tri = &comp->triangles[j];
			ushort iss[3][3];
			bcatch(fread(iss, sizeof(ushort[3][3]), 1, fp) == 1, c1);
			for (uchar k = 0; k < 3; k++) {
				memcpy(tri->vertices[k], vs[iss[0][k]], sizeof(float[3]));
				memcpy(tri->normals[k], vns[iss[1][k]], sizeof(float[3]));
				memcpy(tri->textureBinds[k], vts[iss[2][k]], sizeof(float[2]));
			}
		}
		free(vs);
		free(vns);
		free(vts);
		continue;
c1:		free(vs);
		free(vns);
		free(vts);
		goto c0;
	}
	fclose(fp);
	return nmod;
c0:	if (fp != NULL)
		fclose(fp);
	if (nmod->textures != NULL) {
		for (uint i = 0; i < nmod->textureCount; i++) {
			free((byte(*)[3])nmod->textures[i].pixels);
		}
		free(nmod->textures);
	}
	if (nmod->components != NULL) {
		for (uint i = 0; i < nmod->componentCount; i++) {
			free(nmod->components[i].points);
			free(nmod->components[i].lines);
			free(nmod->components[i].triangles);
		}
		free(nmod->components);
	}
	if (mod != NULL) {
		mod->textureCount = 0, mod->componentCount = 0;
		mod->textures = NULL, mod->components = NULL;
	} else {
		free(nmod);
	}
	return NULL;
}

void vglhDrawTriangle(const VGLHModelTriangle *tri) {
	glBegin(GL_TRIANGLE_STRIP);
	for (int i = 0; i < 3; i++) {
		glNormal3fv(tri->normals[i]);
		glTexCoord2f(tri->textureBinds[i][0], 1.0 - tri->textureBinds[i][1]);
		glVertex3fv(tri->vertices[i]);
	}
	glEnd();
}

bool vglhDrawComponent(const VGLHComponent *comp) {
	if (comp->triangles == NULL)
		return false;
	glGetError();
	glColor3fv(comp->ambientColor);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, comp->specularColor);
	if (comp->texture != NULL)
		glBindTexture(GL_TEXTURE_2D, comp->texture->id);
	if (glGetError() != GL_NO_ERROR)
		return false;
	for (uint j = 0; j < comp->triangleCount; j++) {
		vglhDrawTriangle(&comp->triangles[j]);
	}
	return true;
}

bool vglhDrawModel(const VGLHModel *mod) {
	if (mod->components == NULL)
		return false;
	bool f = true;
	for (uint i = 0; i < mod->componentCount; i++) {
		if (!vglhDrawComponent(&mod->components[i]))
			f = false;
	}
	return f;
}
