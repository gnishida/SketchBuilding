#pragma once

#include <glew.h>
#include <glm/glm.hpp>

class Camera {
public:
	Camera();

	void mousePress(int mouse_x, int mouse_y);
	void rotate(int mouse_x, int mouse_y);
	void zoom(float delta);
	void move(int mouse_x, int mouse_y);
	float f() { return _f; }
	float aspect() { return _aspect; }
	void updatePMatrix(int width,int height);
	void updateMVPMatrix();
	glm::vec3 cameraPosInWorld();
	glm::vec3 cameraViewDir();

public:
	float fovy;
	float _f;
	float _aspect;
	glm::vec3 pos;
	float xrot;
	float yrot;
	float zrot;

	glm::vec2 mouse_pos;

public:
	glm::mat4 mvMatrix;
	glm::mat4 pMatrix;
	glm::mat4 mvpMatrix;
};

