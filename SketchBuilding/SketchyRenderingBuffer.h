#pragma once

#include <glew.h>
#include <QGLWidget>

class SketchyRenderingBuffer {
public:
public:
	int width;
	int height;

	int programId;

	uint fbo;
	uint textureNormal;
	uint textureDepth;

public:
	SketchyRenderingBuffer();

	void init(int programId, int textureNormalIndex, int textureDepthIndex, int width, int height);
	void update(int width, int height);
	void pass1();
	void pass2();
};

