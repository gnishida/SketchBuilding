#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "Scene.h"

class Material {
private:
	int type;
	glm::vec4 color;
	std::string texture;

public:
	Material() : type(0) {}
	Material(const glm::vec4& color) : type(1), color(color) {}
	Material(const std::string& texture) : type(2), texture(texture) {}

	bool equals(const Material& other);
	std::string to_string();
};

class OBJWriter {
protected:
	OBJWriter() {}

public:
	static void write(const std::vector<sc::SceneObject>& objects, const std::string& filename);
};

