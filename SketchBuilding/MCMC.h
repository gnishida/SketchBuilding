#pragma once

#include <opencv2/opencv.hpp>
#include "CGA.h"

class GLWidget3D;

class MCMC {
public:
	cga::CGA system;
	//cga::Grammar grammar;
	GLWidget3D* glWidget;
	std::vector<boost::shared_ptr<glutils::Face> > faces;

public:
	MCMC(GLWidget3D* glWidget);

	void optimize(cga::Grammar& grammar, const cv::Mat& image, float threshold, int maxIters, float offset_z, std::vector<float>& params);
	//static void convertParams(const std::vector<float>& params, const std::vector<std::pair<float, float> >& ranges, float& offset_x, float& offset_y, float& object_width, float& object_depth, std::vector<float>& params2);

private:
	float distanceTransform(cga::Grammar& grammar, const cv::Mat& distMap, const std::vector<float>& params, float offset_z, int count);
	QImage renderImage(cga::Grammar& grammar, const std::vector<float>& params, float offset_z);
};

