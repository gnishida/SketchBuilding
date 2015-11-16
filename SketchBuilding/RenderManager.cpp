#include "RenderManager.h"
#include <iostream>
#include "Shader.h"
#include <QImage>
#include <QGLWidget>

GeometryObject::GeometryObject() {
	vaoCreated = false;
	vaoOutdated = true;
}

GeometryObject::GeometryObject(const std::vector<Vertex>& vertices) {
	this->vertices = vertices;
	vaoCreated = false;
	vaoOutdated = true;
}

void GeometryObject::addVertices(const std::vector<Vertex>& vertices) {
	this->vertices.insert(this->vertices.end(), vertices.begin(), vertices.end());
	vaoOutdated = true;
}

/**
 * Create VAO according to the vertices.
 */
void GeometryObject::createVAO() {
	// VAOが作成済みで、最新なら、何もしないで終了
	if (vaoCreated && !vaoOutdated) return;

	if (!vaoCreated) {
		// create vao and bind it
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		// create VBO and tranfer the vertices data to GPU buffer
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		vaoCreated = true;
	} else {
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
	}

	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

	// configure the attributes in the vao
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, drawEdge));
		
	// unbind the vao
	glBindVertexArray(0); 
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	vaoOutdated = false;
}

RenderManager::RenderManager() {
}

void RenderManager::init(const std::string& vertex_file, const std::string& geometry_file, const std::string& fragment_file, bool useShadow, int shadowMapSize) {
	this->useShadow = useShadow;
	renderingMode = RENDERING_MODE_REGULAR;
	depthSensitivity = 1.0f; // 6000.0f;
	normalSensitivity = 1.0f;
	useThreshold = true;
	threshold = 0.25f;

	// init glew
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		std::cout << "Error: " << glewGetErrorString(err) << std::endl;
	}

	// load shaders
	Shader shader;
	if (geometry_file.empty()) {
		program = shader.createProgram(vertex_file, fragment_file);
	} else {
		program = shader.createProgram(vertex_file, geometry_file, fragment_file);
	}
	glUseProgram(program);

	// ダミーのtexture idを作成する。
	// これにより、実際に使われるtexture idは1以上の値となる
	GLuint texId;
	glGenTextures(1, &texId);

	shadow.init(program, shadowMapSize, shadowMapSize);
	rb.init(program, 4, 5, 100, 100);
}

void RenderManager::addFaces(const std::vector<glutils::Face>& faces) {
	for (int i = 0; i < faces.size(); ++i) {
		addObject(faces[i].name.c_str(), faces[i].texture.c_str(), faces[i].vertices);
	}
}

void RenderManager::addObject(const QString& object_name, const QString& texture_file, const std::vector<Vertex>& vertices) {
	GLuint texId;
	
	if (texture_file.length() > 0) {
		// テクスチャファイルがまだ読み込まれていない場合は、ロードする
		if (!textures.contains(texture_file)) {
			texId = loadTexture(texture_file);
			textures[texture_file] = texId;
		} else {
			texId = textures[texture_file];
		}
	} else {
		texId = 0;
	}

	if (objects.contains(object_name)) {
		if (objects[object_name].contains(texId)) {
			objects[object_name][texId].addVertices(vertices);
		} else {
			objects[object_name][texId] = GeometryObject(vertices);
		}
	} else {
		objects[object_name][texId] = GeometryObject(vertices);
	}
}

void RenderManager::removeObjects() {
	for (auto it = objects.begin(); it != objects.end(); ++it) {
		removeObject(it.key());
	}
	objects.clear();
}

void RenderManager::removeObject(const QString& object_name) {
	for (auto it = objects[object_name].begin(); it != objects[object_name].end(); ++it) {
		glDeleteBuffers(1, &it->vbo);
		glDeleteVertexArrays(1, &it->vao);
	}

	objects[object_name].clear();
}

void RenderManager::centerObjects() {
	glm::vec3 minPt((std::numeric_limits<float>::max)(), (std::numeric_limits<float>::max)(), (std::numeric_limits<float>::max)());
	glm::vec3 maxPt = -minPt;

	// もとのサイズを計算
	for (auto it = objects.begin(); it != objects.end(); ++it) {
		for (auto it2 = it.value().begin(); it2 != it.value().end(); ++it2) {
			for (int k = 0; k < it2->vertices.size(); ++k) {
				minPt.x = std::min(minPt.x, it2->vertices[k].position.x);
				minPt.y = std::min(minPt.y, it2->vertices[k].position.y);
				minPt.z = std::min(minPt.z, it2->vertices[k].position.z);
				maxPt.x = std::max(maxPt.x, it2->vertices[k].position.x);
				maxPt.y = std::max(maxPt.y, it2->vertices[k].position.y);
				maxPt.z = std::max(maxPt.z, it2->vertices[k].position.z);
			}
		}
	}

	glm::vec3 center = (maxPt + minPt) * 0.5f;

	float size = std::max(maxPt.x - minPt.x, std::max(maxPt.y - minPt.y, maxPt.z - minPt.z));
	float scale = 1.0f / size;

	// 単位立方体に入るよう、縮尺・移動
	for (auto it = objects.begin(); it != objects.end(); ++it) {
		for (auto it2 = it.value().begin(); it2 != it.value().end(); ++it2) {
			for (int k = 0; k < it2->vertices.size(); ++k) {
				it2->vertices[k].position = (it2->vertices[k].position - center) * scale;
			}
		}
	}
}

void RenderManager::renderAll() {
	if (renderingMode == RENDERING_MODE_REGULAR || renderingMode == RENDERING_MODE_WIREFRAME) {
		glClearColor(0.443, 0.439, 0.458, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_TEXTURE_2D);

		for (auto it = objects.begin(); it != objects.end(); ++it) {
			render(it.key());
		}
	} else if (renderingMode == RENDERING_MODE_LINE || RENDERING_MODE_SKETCHY) {
		rb.pass1();
		for (auto it = objects.begin(); it != objects.end(); ++it) {
			render(it.key());
		}
		rb.pass2();
		for (auto it = objects.begin(); it != objects.end(); ++it) {
			render(it.key());
		}
	}
}

void RenderManager::renderAllExcept(const QString& object_name) {
	for (auto it = objects.begin(); it != objects.end(); ++it) {
		if (it.key() == object_name) continue;

		render(it.key());
	}
}

void RenderManager::render(const QString& object_name) {
	for (auto it = objects[object_name].begin(); it != objects[object_name].end(); ++it) {
		GLuint texId = it.key();
		
		// vaoを作成
		it->createVAO();

		if (texId > 0) {
			// テクスチャなら、バインドする
			glBindTexture(GL_TEXTURE_2D, texId);
			glUniform1i(glGetUniformLocation(program, "textureEnabled"), 1);
			glUniform1i(glGetUniformLocation(program, "tex0"), 0);
		} else {
			glUniform1i(glGetUniformLocation(program, "textureEnabled"), 0);
		}

		if (useShadow) {
			glUniform1i(glGetUniformLocation(program, "useShadow"), 1);
		} else {
			glUniform1i(glGetUniformLocation(program, "useShadow"), 0);
		}

		if (renderingMode == RENDERING_MODE_REGULAR) {
			glUniform1i(glGetUniformLocation(program, "renderingMode"), 1);
		} else if (renderingMode == RENDERING_MODE_WIREFRAME) {
			glUniform1i(glGetUniformLocation(program, "renderingMode"), 2);
		} else {
			if (renderingMode == RENDERING_MODE_LINE) {
				glUniform1i(glGetUniformLocation(program, "renderingMode"), 3);
			} else {
				glUniform1i(glGetUniformLocation(program, "renderingMode"), 4);
			}
			glUniform1f(glGetUniformLocation(program, "depthSensitivity"), depthSensitivity);
			glUniform1f(glGetUniformLocation(program, "normalSensitivity"), normalSensitivity);
			glUniform1i(glGetUniformLocation(program, "useThreshold"), useThreshold ? 1 : 0);
			glUniform1f(glGetUniformLocation(program, "threshold"), threshold);
		}


		// 描画
		glBindVertexArray(it->vao);
		glDrawArrays(GL_TRIANGLES, 0, it->vertices.size());

		glBindVertexArray(0);
	}
}

void RenderManager::updateShadowMap(GLWidget3D* glWidget3D, const glm::vec3& light_dir, const glm::mat4& light_mvpMatrix) {
	if (useShadow) {
		shadow.update(glWidget3D, light_dir, light_mvpMatrix);
	}
}

void RenderManager::resize(int width, int height) {
	rb.update(width, height);
}

GLuint RenderManager::loadTexture(const QString& filename) {
	QImage img;
	if (!img.load(filename)) {
		printf("ERROR: loading %s\n",filename.toUtf8().data());
		return INT_MAX;
	}

	QImage GL_formatted_image;
	GL_formatted_image = QGLWidget::convertToGLFormat(img);
	if (GL_formatted_image.isNull()) {
		printf("ERROR: GL_formatted_image\n");
		return INT_MAX;
	}

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, GL_formatted_image.width(), GL_formatted_image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, GL_formatted_image.bits());
	glGenerateMipmap(GL_TEXTURE_2D);

	return texture;
}

