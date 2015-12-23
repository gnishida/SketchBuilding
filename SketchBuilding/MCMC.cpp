#include "MCMC.h"
#include "Rectangle.h"
#include <time.h>
#include "GLWidget3D.h"

MCMC::MCMC(GLWidget3D* glWidget) {
	this->glWidget = glWidget;
}

void MCMC::optimize(cga::Grammar& grammar, const cv::Mat& image, float threshold, int maxIters, float offset_z, std::vector<float>& params) {
	// cretae a binary image
	cv::Mat image2;
	cv::threshold(image, image2, 254, 255, CV_THRESH_BINARY);

	///////////////////////////////// DEBUG ///////////////////////////////// 
	//cv::imwrite("results/input.png", image2);
	///////////////////////////////// DEBUG ///////////////////////////////// 

	// compute a distance map
	cv::Mat distMap;
	cv::distanceTransform(image2, distMap, CV_DIST_L2, 3);
	distMap.convertTo(distMap, CV_32F);
	
	int WIDTH = image.cols;
	int HEIGHT = image.rows;

	float delta = 0.05f;
	int stepsize = 10;

	int step_count = 0;
	float dist = std::numeric_limits<float>::max();
	for (int i = 0; i < maxIters; ++i) {		
		int r = i % params.size();
		float old_value = params[r];

		///////////////////////////////// DEBUG ///////////////////////////////// 
		//std::cout << "Round: " << (i + 1) << " (" << (r+1) << "th param)" << std::endl;
		///////////////////////////////// DEBUG ///////////////////////////////// 

		// dist from the proposal 1
		float dist1 = std::numeric_limits<float>::max();
		if (old_value - delta >= 0.0f) {
			params[r] = old_value - delta;
			dist1 = distanceTransform(grammar, distMap, params, offset_z, i * 2);
			///////////////////////////////// DEBUG ///////////////////////////////// 
			//std::cout << "    proposal 1 (" << old_value << " -> " << params[r] << "): " << dist1 << std::endl;
			///////////////////////////////// DEBUG ///////////////////////////////// 
		}

		// dist from the proposal 2
		float dist2 = std::numeric_limits<float>::max();
		if (old_value + delta <= 1.0f) {
			params[r] = std::min(1.0f, old_value + delta);
			dist2 = distanceTransform(grammar, distMap, params, offset_z, i * 2 + 1);
			///////////////////////////////// DEBUG ///////////////////////////////// 
			//std::cout << "    proposal 2 (" << old_value << " -> " << params[r] << "): " << dist2 << std::endl;
			///////////////////////////////// DEBUG ///////////////////////////////// 
		}

		if (dist1 <= dist2 && dist1 <= dist) {
			params[r] = old_value - delta;
			dist = dist1;
			///////////////////////////////// DEBUG ///////////////////////////////// 
			//std::cout << "--> proposal 1" << std::endl;
			///////////////////////////////// DEBUG ///////////////////////////////// 
		}
		else if (dist2 <= dist1 && dist2 <= dist) {
			params[r] = old_value + delta;
			dist = dist2;
			///////////////////////////////// DEBUG ///////////////////////////////// 
			//std::cout << "--> proposal 2" << std::endl;
			///////////////////////////////// DEBUG ///////////////////////////////// 
		}
		else {
			params[r] = old_value;
			///////////////////////////////// DEBUG ///////////////////////////////// 
			//std::cout << "--> remain the current state" << std::endl;
			///////////////////////////////// DEBUG ///////////////////////////////// 
		}

		if (step_count >= stepsize) {
			step_count = 0;
			delta = std::max(0.01f, delta * 0.8f);
		}

		if (dist < threshold) break;
	}
}

float MCMC::distanceTransform(cga::Grammar& grammar, const cv::Mat& distMap, const std::vector<float>& params, float offset_z, int count) {
	QImage ref_image = renderImage(grammar, params, offset_z);

	cv::Mat mat(ref_image.height(), ref_image.width(), CV_8UC4, ref_image.bits(), ref_image.bytesPerLine());
	cv::Mat grayMat;
	cv::cvtColor(mat, grayMat, CV_BGR2GRAY);

	// resize the rendered image to 128x128
	int min_size = std::min(grayMat.cols, grayMat.rows);
	grayMat = grayMat(cv::Rect((grayMat.cols - min_size) * 0.5, (grayMat.rows - min_size) * 0.5, min_size, min_size));

	if (min_size > 512) {
		cv::resize(grayMat, grayMat, cv::Size(512, 512));
		cv::threshold(grayMat, grayMat, 250, 255, CV_THRESH_BINARY);

	}
	cv::resize(grayMat, grayMat, cv::Size(256, 256));
	cv::threshold(grayMat, grayMat, 250, 255, CV_THRESH_BINARY);
	cv::resize(grayMat, grayMat, cv::Size(128, 128));
	cv::threshold(grayMat, grayMat, 250, 255, CV_THRESH_BINARY);

	///////////////////////////////// DEBUG ///////////////////////////////// 
	//char filename[256];
	//sprintf(filename, "results/image_%03d.png", count);
	//cv::imwrite(filename, grayMat);
	///////////////////////////////// DEBUG ///////////////////////////////// 

	// compute a distance map
	cv::Mat ref_distMap;
	cv::distanceTransform(grayMat, ref_distMap, CV_DIST_L2, 3);
	ref_distMap.convertTo(ref_distMap, CV_32F);

	///////////////////////////////// DEBUG ///////////////////////////////// 
	//char filename2[256];
	//sprintf(filename2, "results/ref_distMap_%03d.png", count);
	//cv::imwrite(filename2, ref_distMap);
	///////////////////////////////// DEBUG ///////////////////////////////// 

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
	int prevMode = glWidget->renderManager.renderingMode;
	glWidget->renderManager.renderingMode = RenderManager::RENDERING_MODE_LINE;
	glUseProgram(glWidget->renderManager.programs["pass1"]);

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
	{
		glUseProgram(glWidget->renderManager.programs["pass1"]);

		glBindFramebuffer(GL_FRAMEBUFFER, glWidget->renderManager.fragDataFB);
		glClearColor(0.95, 0.95, 0.95, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, glWidget->renderManager.fragDataTex[0], 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, glWidget->renderManager.fragDataTex[1], 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, glWidget->renderManager.fragDataTex[2], 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, glWidget->renderManager.fragDataTex[3], 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, glWidget->renderManager.fragDepthTex, 0);

		// Set the list of draw buffers.
		GLenum DrawBuffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
		glDrawBuffers(4, DrawBuffers); // "3" is the size of DrawBuffers
		// Always check that our framebuffer is ok
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			printf("+ERROR: GL_FRAMEBUFFER_COMPLETE false\n");
			exit(0);
		}

		glUniformMatrix4fv(glGetUniformLocation(glWidget->renderManager.programs["pass1"], "mvpMatrix"), 1, false, &glWidget->camera.mvpMatrix[0][0]);
		glUniform3f(glGetUniformLocation(glWidget->renderManager.programs["pass1"], "lightDir"), glWidget->light_dir.x, glWidget->light_dir.y, glWidget->light_dir.z);
		glUniformMatrix4fv(glGetUniformLocation(glWidget->renderManager.programs["pass1"], "light_mvpMatrix"), 1, false, &glWidget->light_mvpMatrix[0][0]);

		glUniform1i(glGetUniformLocation(glWidget->renderManager.programs["pass1"], "shadowMap"), 6);
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, glWidget->renderManager.shadow.textureDepth);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glWidget->drawScene();
	}

	{
		glUseProgram(glWidget->renderManager.programs["line"]);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(1, 1, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glDisable(GL_DEPTH_TEST);
		glDepthFunc(GL_ALWAYS);

		glUniform2f(glGetUniformLocation(glWidget->renderManager.programs["line"], "pixelSize"), 1.0f / glWidget->width(), 1.0f / glWidget->height());
		glUniformMatrix4fv(glGetUniformLocation(glWidget->renderManager.programs["line"], "pMatrix"), 1, false, &glWidget->camera.pMatrix[0][0]);
		if (glWidget->renderManager.renderingMode == RenderManager::RENDERING_MODE_LINE) {
			glUniform1i(glGetUniformLocation(glWidget->renderManager.programs["line"], "useHatching"), 0);
		}
		else {
			glUniform1i(glGetUniformLocation(glWidget->renderManager.programs["line"], "useHatching"), 1);
		}

		glUniform1i(glGetUniformLocation(glWidget->renderManager.programs["line"], "tex0"), 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, glWidget->renderManager.fragDataTex[0]);

		glUniform1i(glGetUniformLocation(glWidget->renderManager.programs["line"], "tex1"), 2);
		glActiveTexture(GL_TEXTURE2);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, glWidget->renderManager.fragDataTex[1]);

		glUniform1i(glGetUniformLocation(glWidget->renderManager.programs["line"], "tex2"), 3);
		glActiveTexture(GL_TEXTURE3);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, glWidget->renderManager.fragDataTex[2]);

		glUniform1i(glGetUniformLocation(glWidget->renderManager.programs["line"], "tex3"), 4);
		glActiveTexture(GL_TEXTURE4);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, glWidget->renderManager.fragDataTex[3]);

		glUniform1i(glGetUniformLocation(glWidget->renderManager.programs["line"], "depthTex"), 8);
		glActiveTexture(GL_TEXTURE8);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, glWidget->renderManager.fragDepthTex);

		glUniform1i(glGetUniformLocation(glWidget->renderManager.programs["line"], "hatchingTexture"), 5);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_3D, glWidget->renderManager.hatchingTextures);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		glBindVertexArray(glWidget->renderManager.secondPassVAO);

		glDrawArrays(GL_QUADS, 0, 4);
		glBindVertexArray(0);
		glDepthFunc(GL_LEQUAL);
	}

	glWidget->renderManager.renderingMode = prevMode;

	// obtain the image from the frame buffer
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