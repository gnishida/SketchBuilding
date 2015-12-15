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

	// compute the intervals
	std::vector<float> intervals;
	float avg_intervals = 0.0f;
	for (int i = 0; i < y_coordinates.size() - 2; ++i) {
		intervals.push_back(y_coordinates[i + 1] - y_coordinates[i]);
		avg_intervals += y_coordinates[i + 1] - y_coordinates[i];
	}
	if (avg_intervals > 0) {
		avg_intervals /= intervals.size();
	} else {
		avg_intervals = top_y_screen - bottom_y_screen;
	}

	// count the num of ledges and recompute the average interval
	int num_ledges = 0;
	int num_ledges_consective = 0;
	int floor_consective_count = 0;
	bool ledge_consecutive = true;
	float total_intervals = 0;
	for (int i = 0; i < intervals.size(); ++i) {
		if (intervals[i] < avg_intervals * 0.7f) {
			num_ledges++;
			if (ledge_consecutive) {
				num_ledges_consective++;
				floor_consective_count = 0;
			}
		}
		else {
			total_intervals += intervals[i];
			floor_consective_count++;
			if (floor_consective_count >= 2) {
				ledge_consecutive = false;
			}
		}
	}
	avg_intervals = total_intervals / (intervals.size() - num_ledges);



	// scale
	float scale = (top_y - bottom_y) / (top_y_screen - bottom_y_screen);

	std::vector<float> ret;

	if (num_ledges == 0) {
		// A* pattern (i.e., same height for every floor)
		int num_floors = (top_y_screen - bottom_y_screen) / avg_intervals + 0.5f;
		float floor_height = (top_y_screen - bottom_y_screen) / num_floors;

		// return the actual height of the floors
		ret.push_back(floor_height * scale);
		return std::make_pair(0, ret);
	}
	else if (num_ledges == 1) {
		// A B C* pattern (B is a ledge)
		float ground_height = intervals[0];
		float ledge_height = intervals[1];

		int num_floors = (top_y_screen - bottom_y_screen - ground_height - ledge_height) / avg_intervals + 0.5f;
		float floor_height = (top_y_screen - bottom_y_screen - ground_height - ledge_height) / num_floors;

		//float scale = (top_y - bottom_y) / (ground_height + ledge_height + floor_height * num_floors);

		ret.push_back(floor_height * scale);
		ret.push_back(ground_height * scale);
		ret.push_back(ledge_height * scale);
		return std::make_pair(1, ret);
	}
	else if (num_ledges > 1 && num_ledges_consective == 1) {
		// ALB{AL}* pattern (L is a ledge)
		float ground_base = std::max(0.0f, intervals[0] - intervals[2]);

		float ledge_height;
		if (intervals.size() >= 5) {
			ledge_height = (intervals[1] + intervals[4]) * 0.5f;
		}
		else {
			ledge_height = intervals[1];
		}

		int num_floors = (top_y_screen - bottom_y_screen - ground_base + ledge_height * 2) / (ledge_height + intervals[2]);
		float floor_height = (top_y_screen - bottom_y_screen - ground_base + ledge_height * 2) / num_floors - ledge_height;
		
		ret.push_back(floor_height * scale);
		ret.push_back(ground_base * scale);
		ret.push_back(ledge_height * scale);
		return std::make_pair(4, ret);
	}
	else if (num_ledges > 1) {
		// {AL}* pattern (L is a ledge)
		int num_floors = (top_y_screen - bottom_y_screen) / (intervals[0] + intervals[1]) + 0.5f;

		float floor_height = (top_y_screen - bottom_y_screen) / num_floors / (intervals[0] + intervals[1]) * intervals[0];
		float ledge_height = (top_y_screen - bottom_y_screen) / num_floors / (intervals[0] + intervals[1]) * intervals[1];

		ret.push_back(floor_height * scale);
		ret.push_back(ledge_height * scale);
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
			if (bboxes[i].contains(stroke[0], 0) || bboxes[i].contains(stroke.back(), 0)) {
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

	float floor_height = 0.0f;
	float floor_width = 0.0f;

	for (int i = 0; i < face.vertices.size(); ++i) {
		glm::vec4 projectedPt = mvpMatrix * glm::vec4(face.vertices[i].position, 1);
		float x = (projectedPt.x / projectedPt.w + 1.0) * 0.5 * width;
		if (x < left_screen) {
			left_screen = x;
		}
		if (x > right_screen) {
			right_screen = x;
		}

		float y = (projectedPt.y / projectedPt.w + 1.0) * 0.5 * height;
		if (y < bottom_screen) {
			bottom_screen = y;
		}
		if (y > top_screen) {
			top_screen = y;
		}

		int next = (i + 1) % face.vertices.size();
		glm::vec3 v = face.vertices[next].position - face.vertices[i].position;
		if (fabs(glm::dot(glm::normalize(v), glm::vec3(0, 1, 0))) < 0.1f) {
			floor_width = glm::length(v);
		}
		if (fabs(glm::dot(glm::normalize(v), glm::vec3(0, 1, 0))) > 0.99f) {
			floor_height = glm::length(v);
		}
	}

	float horizontal_scale = floor_width / (right_screen - left_screen);
	float vertical_scale = floor_height / (top_screen - bottom_screen);

	// widths
	std::vector<float> widths;
	for (int i = 0; i < bboxes.size(); ++i) {
		widths.push_back(bboxes[i].maxPt.x - bboxes[i].minPt.x);
	}

	std::vector<float> ret;

	if (widths.size() == 1 && fabs(bboxes[0].minPt.y - bottom_screen) < 4) {
		// Entrance
		float entrance_width = widths[0];
		float top_margin = top_screen - bboxes[0].maxPt.y;

		ret.push_back(entrance_width * horizontal_scale);
		ret.push_back(top_margin * vertical_scale);
		return std::make_pair(2, ret);
	}
	else if (widths.size() <= 1 || fabs(widths[1] - widths[0]) < (widths[0] + widths[1]) * 0.3) {
		// A* pattern (i.e., every window has the same width)
		float total_width = 0.0f;
		for (int i = 0; i < widths.size(); ++i) {
			total_width += widths[i];
		}
		float window_width = total_width / widths.size();

		// extract parameter values
		float bottom_margin_total = 0.0f;
		float top_margin_total = 0.0f;
		for (int i = 0; i < bboxes.size(); ++i) {
			bottom_margin_total += bboxes[i].minPt.y - bottom_screen;
			top_margin_total += top_screen - bboxes[i].maxPt.y;
		}
		float bottom_margin = bottom_margin_total / bboxes.size();
		float top_margin = top_margin_total / bboxes.size();

		float padding;
		float margin;
		if (bboxes.size() >= 2) {
			padding = (bboxes[1].minPt.x - bboxes[0].maxPt.x) * 0.5f;
			margin = bboxes[0].minPt.x - left_screen - padding;
		}
		else {
			padding = (bboxes[0].minPt.x - left_screen) * 0.5f;
			margin = (bboxes[0].minPt.x - left_screen) * 0.5f;
		}

		ret.push_back(bottom_margin * vertical_scale);
		ret.push_back(margin * horizontal_scale);
		ret.push_back(padding * horizontal_scale);
		ret.push_back(top_margin * vertical_scale);
		ret.push_back(window_width * vertical_scale);
		return std::make_pair(0, ret);
	}
	else {
		// {AB}* Pattern (i.e., every two windows have the same size)
		float window_width1 = bboxes[0].sx();
		float window_width2 = bboxes[1].sx();
		float bottom_margin1 = bboxes[0].minPt.y - bottom_screen;
		float top_margin1 = top_screen - bboxes[0].maxPt.y;
		float bottom_margin2 = bboxes[1].minPt.y - bottom_screen;
		float top_margin2 = top_screen - bboxes[1].maxPt.y;
		float padding = (bboxes[1].minPt.x - bboxes[0].maxPt.x) * 0.5f;
		float margin = bboxes[0].minPt.x - left_screen - padding;

		ret.push_back(bottom_margin1 * vertical_scale);
		ret.push_back(bottom_margin2 * vertical_scale);
		ret.push_back(margin * horizontal_scale);
		ret.push_back(padding * horizontal_scale);
		ret.push_back(top_margin1 * vertical_scale);
		ret.push_back(top_margin2 * vertical_scale);
		ret.push_back(window_width1 * horizontal_scale);
		ret.push_back(window_width2 * horizontal_scale);
		return std::make_pair(1, ret);
	}
}