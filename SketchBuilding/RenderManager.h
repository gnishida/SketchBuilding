#pragma once

#include "glew.h"
#include <vector>
#include <QMap>
#include "Vertex.h"
#include "ShadowMapping.h"
#include "SketchyRenderingBuffer.h"
#include "GLUtils.h"
#include <boost/shared_ptr.hpp>

class GeometryObject {
public:
	GLuint vao;
	GLuint vbo;
	std::vector<Vertex> vertices;
	bool vaoCreated;
	bool vaoOutdated;

public:
	GeometryObject();
	GeometryObject(const std::vector<Vertex>& vertices);
	void addVertices(const std::vector<Vertex>& vertices);
	void createVAO();
};

class RenderManager {
public:
	static enum { RENDERING_MODE_REGULAR = 0, RENDERING_MODE_WIREFRAME, RENDERING_MODE_LINE, RENDERING_MODE_SKETCHY };

public:
	GLuint program;
	QMap<QString, QMap<GLuint, GeometryObject> > objects;
	QMap<QString, GLuint> textures;

	bool useShadow;
	ShadowMapping shadow;

	SketchyRenderingBuffer rb;

	int renderingMode;
	float depthSensitivity;
	float normalSensitivity;
	bool useThreshold;
	float threshold;

public:
	RenderManager();

	void init(const std::string& vertex_file, const std::string& geometry_file, const std::string& fragment_file, bool useShadow, int shadowMapSize = 4096);
	void addFaces(const std::vector<boost::shared_ptr<glutils::Face> >& faces);
	void addObject(const QString& object_name, const QString& texture_file, const std::vector<Vertex>& vertices);
	void removeObjects();
	void removeObject(const QString& object_name);
	void centerObjects();
	void renderAll();
	void renderAllExcept(const QString& object_name);
	void render(const QString& object_name);
	void updateShadowMap(GLWidget3D* glWidget3D, const glm::vec3& light_dir, const glm::mat4& light_mvpMatrix);
	void resize(int width, int height);


private:
	GLuint loadTexture(const QString& filename);
};

