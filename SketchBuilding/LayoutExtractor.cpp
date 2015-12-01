#include "LayoutExtractor.h"
#include <iostream>

std::pair<int, std::vector<float> > LayoutExtractor::extractFacadePattern(int width, int height, const std::vector<std::vector<glm::vec2> >& strokes, const glutils::Face& face, const glm::mat4& mvpMatrix) {
	// list up y coordinates
	std::vector<float> y_coordinates;
	for (auto stroke : strokes) {
		if (stroke.size() <= 1) continue;

		float y = (stroke[0].y + stroke.back().y) * 0.5f;

		y_coordinates.push_back(y);

	}

	// project the face to the image plane, and find the y coordinates of the bottom and top lines
	float bottom_y_screen = std::numeric_limits<float>::max();
	float top_y_screen = 0.0f;
	float bottom_y = 0.0f;
	float top_y = 0.0f;
	for (int i = 0; i < face.vertices.size(); ++i) {
		glm::vec4 projectedPt = mvpMatrix * glm::vec4(face.vertices[i].position, 1);
		float y = (projectedPt.y / projectedPt.w + 1.0) * 0.5 * height;
		if (y < bottom_y_screen) {
			bottom_y_screen = y;
			bottom_y = face.vertices[i].position.y;
		}
		if (y > top_y_screen) {
			top_y_screen = y;
			top_y = face.vertices[i].position.y;
		}
	}

	// add bottom and top
	y_coordinates.insert(y_coordinates.begin(), bottom_y_screen);
	y_coordinates.push_back(top_y_screen);

	// order the lines by Y
	std::sort(y_coordinates.begin(), y_coordinates.end());

	std::vector<float> y_coordinates_revised;
	for (int i = 0; i < y_coordinates.size(); ++i) {
		bool tooClose = false;

		if (i > 0 && i < y_coordinates.size() - 1) {
			// check if there is any other y coordinate that is close to this y
			for (int j = 0; j < y_coordinates.size(); ++j) {
				if (i == j) continue;
				if (fabs(y_coordinates[i] - y_coordinates[j]) < 2.0f) {
					tooClose = true;
					break;
				}
			}
		}

		if (!tooClose) {
			y_coordinates_revised.push_back(y_coordinates[i]);
		}
	}
	y_coordinates = y_coordinates_revised;

	////////////////////// DEBUG ///////////////////////
	for (auto y_coord : y_coordinates) {
		std::cout << y_coord << std::endl;
	}
	////////////////////// DEBUG ///////////////////////

	// compute the intervals
	std::vector<float> intervals;
	for (int i = 0; i < y_coordinates.size() - 1; ++i) {
		intervals.push_back(y_coordinates[i + 1] - y_coordinates[i]);
	}

	// count the num of ledges
	int num_ledges = 0;
	for (int i = 0; i < intervals.size(); ++i) {
		if (intervals[i] < 14) num_ledges++;
	}

	std::vector<float> ret;

	if (num_ledges == 0) {
		// A* pattern (i.e., same height for every floor)
		float total = 0.0f;
		for (int i = 0; i < intervals.size() - 1; ++i) {
			total += intervals[i];
		}
		
		// compute #floors
		float avg_height = total / (intervals.size() - 1);
		int num_floors = (top_y_screen - bottom_y_screen) / avg_height + 0.5f;
		float floor_height = (top_y_screen - bottom_y_screen) / num_floors;

		// return the actual height of the floors
		ret.push_back(floor_height / (top_y_screen - bottom_y_screen));
		return std::make_pair(0, ret);
	}
	else if (num_ledges == 1) {
		// A B C* pattern (B is a ledge)
		float ground_height = intervals[0];
		float ledge_height = intervals[1];

		int num_floors = 2;
		if (intervals.size() >= 3) {
			num_floors = (top_y_screen - bottom_y_screen - ground_height - ledge_height) / intervals[2] + 0.5f;
		}
		float floor_height = (top_y_screen - bottom_y_screen - ground_height - ledge_height) / num_floors;

		//float scale = (top_y - bottom_y) / (ground_height + ledge_height + floor_height * num_floors);

		ret.push_back(floor_height / (top_y_screen - bottom_y_screen));
		ret.push_back(ground_height / (top_y_screen - bottom_y_screen));
		ret.push_back(ledge_height / (top_y_screen - bottom_y_screen));
		return std::make_pair(1, ret);
	} else if (num_ledges > 1) {
		// {AB}* pattern (B is a ledge)
		int num_floors = (top_y_screen - bottom_y_screen) / (intervals[0] + intervals[1]) + 0.5f;

		float floor_height = (top_y_screen - bottom_y_screen) / num_floors / (intervals[0] + intervals[1]) * intervals[0];
		float ledge_height = (top_y_screen - bottom_y_screen) / num_floors / (intervals[0] + intervals[1]) * intervals[1];

		ret.push_back(floor_height / (top_y_screen - bottom_y_screen));
		ret.push_back(ledge_height / (top_y_screen - bottom_y_screen));
		return std::make_pair(2, ret);
	}
	else {
		return std::make_pair(3, ret);
	}
}

std::pair<int, std::vector<float> > LayoutExtractor::extractFloorPattern(int width, int height, const std::vector<std::vector<glm::vec2> >& strokes, const glutils::Face& face, const glm::mat4& mvpMatrix) {
	std::vector<glutils::BoundingBox> bboxes;

	const float threshold = 3.0f;

	for (auto stroke : strokes) {
		bool belongToBox = false;

		for (int i = 0; i < bboxes.size(); ++i) {
			if (bboxes[i].contains(stroke[0], threshold) || bboxes[i].contains(stroke.back(), threshold)) {
				// this stroke belongs to this bounding box, so update the bounding box accordingly.
				belongToBox = true;
				for (auto pt : stroke) {
					bboxes[i].addPoint(pt);
				}
				break;
			}
		}

		if (!belongToBox) {
			// create a new bbox
			glutils::BoundingBox bbox(stroke);
			bboxes.push_back(bbox);
		}
	}

	// for now, we assume that the bounding boxes are already ordered horizontally.


	// project the face to the image plane, and find the y coordinates of the bottom and top lines
	float bottom_screen = std::numeric_limits<float>::max();
	float top_screen = 0.0f;
	float left_screen = std::numeric_limits<float>::max();
	float right_screen = 0.0f;

	float bottom = 0.0f;
	float top = 0.0f;
	float left = 0.0f;
	float right = 0.0f;

	for (int i = 0; i < face.vertices.size(); ++i) {
		glm::vec4 projectedPt = mvpMatrix * glm::vec4(face.vertices[i].position, 1);
		float x = (projectedPt.x / projectedPt.w + 1.0) * 0.5 * width;
		if (x < left_screen) {
			left_screen = x;
			left = face.vertices[i].position.x;
		}
		if (x > right_screen) {
			right_screen = x;
			right = face.vertices[i].position.x;
		}

		float y = (projectedPt.y / projectedPt.w + 1.0) * 0.5 * height;
		if (y < bottom_screen) {
			bottom_screen = y;
			bottom = face.vertices[i].position.y;
		}
		if (y > top_screen) {
			top_screen = y;
			top = face.vertices[i].position.y;
		}
	}

	float horizontal_scale = (right - left) / (right_screen - left_screen);
	float vertical_scale = (top - bottom) / (top_screen - bottom_screen);

	// widths
	std::vector<float> widths;
	for (int i = 0; i < bboxes.size(); ++i) {
		widths.push_back(bboxes[i].maxPt.x - bboxes[i].minPt.x);
	}

	std::vector<float> ret;

	if (widths.size() <= 1 || fabs(widths[1] - widths[0]) < 0.3f) {
		// A* pattern (i.e., every window has the same width)
		float total_width = 0.0f;
		for (int i = 0; i < widths.size(); ++i) {
			total_width += widths[i];
		}
		float avg_width = total_width / widths.size();

		// extract parameter values
		float bottom_margin_total = 0.0f;
		float top_margin_total = 0.0f;
		for (int i = 0; i < bboxes.size(); ++i) {
			bottom_margin_total += bboxes[i].minPt.y - bottom_screen;
			top_margin_total += top_screen - bboxes[i].maxPt.y;
		}
		float bottom_margin = bottom_margin_total / bboxes.size();
		float top_margin = top_margin_total / bboxes.size();

		float padding = (bboxes[1].minPt.x - bboxes[0].maxPt.x) * 0.5f;
		float margin = (bboxes[0].minPt.x - left_screen) / padding;

		ret.push_back(bottom_margin * vertical_scale);
		ret.push_back(top_margin * vertical_scale);
		ret.push_back(padding * horizontal_scale);
		ret.push_back(margin * horizontal_scale);
		return std::make_pair(0, ret);
	}
	else {
		// {AB}* Pattern (i.e., every two windows have the same size)
		float bottom_margin1 = bboxes[0].minPt.y - bottom_screen;
		float top_margin1 = top_screen - bboxes[0].maxPt.y;
		float bottom_margin2 = bboxes[1].minPt.y - bottom_screen;
		float top_margin2 = top_screen - bboxes[1].maxPt.y;
		float padding = (bboxes[1].minPt.x - bboxes[0].maxPt.x) * 0.5f;
		float margin = (bboxes[0].minPt.x - left_screen) / padding;

		ret.push_back(bottom_margin1 * vertical_scale);
		ret.push_back(top_margin1 * vertical_scale);
		ret.push_back(bottom_margin2 * vertical_scale);
		ret.push_back(top_margin2 * vertical_scale);
		ret.push_back(padding * horizontal_scale);
		ret.push_back(margin * horizontal_scale);

		return std::make_pair(1, ret);
	}
}