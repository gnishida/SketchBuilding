#include "MCMC.h"
#include "Rectangle.h"
#include <time.h>
#include "GLWidget3D.h"

MCMC::MCMC(GLWidget3D* glWidget) {
	this->glWidget = glWidget;
}

void MCMC::optimize(cga::Grammar& grammar, const cv::Mat& image, float threshold, int maxIters, float offset_z, std::vector<float>& params) {
	time_t start = clock();

	// cretae a binary image
	cv::Mat image2;
	cv::threshold(image, image2, 254, 255, CV_THRESH_BINARY);

	//cv::imwrite("results/input.png", image2);

	// compute a distance map
	cv::Mat distMap;
	cv::distanceTransform(image2, distMap, CV_DIST_L2, 3);
	distMap.convertTo(distMap, CV_32F);
	
	int WIDTH = image.cols;
	int HEIGHT = image.rows;

	float delta = 0.1f;
	int stepsize = 10;

	int step_count = 0;
	float dist = std::numeric_limits<float>::max();
	for (int i = 0; i < maxIters; ++i) {
		///////////////////////////////// DEBUG ///////////////////////////////// 
		/*
		QImage hoge = renderImage(params, ranges);
		char filename[256];
		sprintf(filename, "results/image_%04d.png", i);
		hoge.save(filename);
		*/
		///////////////////////////////// DEBUG ///////////////////////////////// 
		
		int r = i % params.size();
		float old_value = params[r];

		// dist from the proposal 1
		params[r] = std::max(0.0f, old_value - delta);
		float dist1 = distanceTransform(grammar, distMap, params, offset_z, i);

		// dist from the proposal 2
		params[r] = std::min(1.0f, old_value + delta);
		float dist2 = distanceTransform(grammar, distMap, params, offset_z, i);

		if (dist1 <= dist2 && dist1 <= dist) {
			params[r] = old_value - delta;
			dist = dist1;
		}
		else if (dist2 <= dist1 && dist2 <= dist) {
			params[r] = old_value + delta;
			dist = dist2;
		}
		else {
			params[r] = old_value;
		}

		if (step_count >= stepsize) {
			step_count = 0;
			delta = std::max(0.01f, delta * 0.8f);
		}

		if (dist < threshold) break;
	}

	time_t end = clock();
	std::cout << "Duration: " << (double)(end - start) / CLOCKS_PER_SEC << "sec." << std::endl;
}

float MCMC::distanceTransform(cga::Grammar& grammar, const cv::Mat& distMap, const std::vector<float>& params, float offset_z, int count) {
	QImage ref_image = renderImage(grammar, params, offset_z);

	char filename[256];
	sprintf(filename, "results/image_%03d.png", count);
	//ref_image.save(filename);

	cv::Mat mat(ref_image.height(), ref_image.width(), CV_8UC4, ref_image.bits(), ref_image.bytesPerLine());
	cv::Mat grayMat;
	cv::cvtColor(mat, grayMat, CV_BGR2GRAY);

	// compute a distance map
	cv::Mat ref_distMap;
	cv::distanceTransform(grayMat, ref_distMap, CV_DIST_L2, 3);
	ref_distMap.convertTo(ref_distMap, CV_32F);

	char filename2[256];
	sprintf(filename2, "results/ref_distMap_%03d.png", count);
	//cv::imwrite(filename2, ref_distMap);
	
	// compute the squared difference
	cv::Mat tmp;
	cv::reduce((distMap - ref_distMap).mul(distMap - ref_distMap), tmp, 0, CV_REDUCE_SUM);
	cv::reduce(tmp, tmp, 1, CV_REDUCE_SUM);

	return tmp.at<float>(0, 0);
}

QImage MCMC::renderImage(cga::Grammar& grammar, const std::vector<float>& params, float offset_z) {
	std::vector<float> params2;

	float offset_x = params[0] * 16 - 8;
	float offset_y = params[1] * 16 - 8;
	float object_width = params[2] * 24 + 4;
	float object_depth = params[3] * 24 + 4;
	offset_x -= object_width * 0.5f;
	offset_y -= object_depth * 0.5f;

	for (int i = 4; i < params.size(); ++i) {
		params2.push_back(params[i]);
	}

	glWidget->renderManager.removeObjects();
	glWidget->renderManager.renderingMode = RenderManager::RENDERING_MODE_LINE;
	glUseProgram(glWidget->renderManager.program);

	// set camera
	/*
	glWidget->camera.xrot = 25.0f;
	glWidget->camera.yrot = -40.0f;
	glWidget->camera.zrot = 0.0f;
	glWidget->camera.pos = glm::vec3(0, 0, 40.0f);
	glWidget->camera.updateMVPMatrix();
	*/

	// lot
	cga::Rectangle* start = new cga::Rectangle("Start", "building", glm::translate(glm::rotate(glm::mat4(), -3.141592f * 0.5f, glm::vec3(1, 0, 0)), glm::vec3(offset_x, offset_y, offset_z)), glm::mat4(), object_width, object_depth, glm::vec3(1, 1, 1));
	system.stack.push_back(boost::shared_ptr<cga::Shape>(start));

	// generate geometry
	faces.clear();
	cga::CGA::setParamValues(grammar, params2, true);
	system.derive(grammar, true);
	system.generateGeometry(faces);
	for (int i = 0; i < faces.size(); ++i) {
		bool transparent = false;

		if (faces[i]->grammar_type != "building") {
			transparent = true;
		}

		if (transparent) {
			for (int j = 0; j < faces[i]->vertices.size(); ++j) {
				faces[i]->vertices[j].color.a = 0.5f;
			}
		}
		else {
			for (int j = 0; j < faces[i]->vertices.size(); ++j) {
				faces[i]->vertices[j].color.a = 1.0f;
			}
		}
	}
	glWidget->renderManager.addFaces(faces);

	// render
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);

	glUniform1i(glGetUniformLocation(glWidget->renderManager.program, "seed"), rand() % 100);

	// Model view projection行列をシェーダに渡す
	glUniformMatrix4fv(glGetUniformLocation(glWidget->renderManager.program, "mvpMatrix"), 1, GL_FALSE, &glWidget->camera.mvpMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(glWidget->renderManager.program, "mvMatrix"), 1, GL_FALSE, &glWidget->camera.mvMatrix[0][0]);

	// pass the light direction to the shader
	//glUniform1fv(glGetUniformLocation(renderManager.program, "lightDir"), 3, &light_dir[0]);
	//glUniform3f(glGetUniformLocation(renderManager->program, "lightDir"), light_dir.x, light_dir.y, light_dir.z);

	// obtain the image from the frame buffer
	glWidget->drawScene(0);
	return glWidget->grabFrameBuffer();
}

/*
void MCMC::convertParams(const std::vector<float>& params, const std::vector<std::pair<float, float> >& ranges, float& offset_x, float& offset_y, float& object_width, float& object_depth, std::vector<float>& params2) {
	params2.resize(params.size() - 4);
	for (int i = 0; i < params.size() - 4; ++i) {
		params2[i] = ranges[i].first + (ranges[i].second - ranges[i].first) * params[i];
	}

	int offset = params.size() - 4;
	offset_x = ranges[offset + 0].first + (ranges[offset + 0].second - ranges[offset + 0].first) * params[offset + 0];
	offset_y = ranges[offset + 1].first + (ranges[offset + 1].second - ranges[offset + 1].first) * params[offset + 1];
	object_width = ranges[offset + 2].first + (ranges[offset + 2].second - ranges[offset + 2].first) * params[offset + 2];
	object_depth = ranges[offset + 3].first + (ranges[offset + 3].second - ranges[offset + 3].first) * params[offset + 3];
}
*/