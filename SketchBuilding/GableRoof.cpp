#include "GableRoof.h"
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/create_straight_skeleton_2.h>
#include "GLUtils.h"
#include "Polygon.h"
#include "CGA.h"
#include "GeneralObject.h"

typedef CGAL::Exact_predicates_inexact_constructions_kernel K ;
typedef K::Point_2 KPoint;
typedef CGAL::Polygon_2<K> Polygon_2 ;
typedef CGAL::Straight_skeleton_2<K> Ss ;
typedef boost::shared_ptr<Ss> SsPtr ;

namespace cga {

GableRoof::GableRoof(const std::string& name, const glm::mat4& pivot, const glm::mat4& modelMat, const std::vector<glm::vec2>& points, float angle, const glm::vec3& color) {
	this->_name = name;
	this->_removed = false;
	this->_pivot = pivot;
	this->_modelMat = modelMat;
	this->_points = points;
	this->_angle = angle;
	this->_color = color;
}

boost::shared_ptr<Shape> GableRoof::clone(const std::string& name) const {
	boost::shared_ptr<Shape> copy = boost::shared_ptr<Shape>(new GableRoof(*this));
	copy->_name = name;
	return copy;
}

void GableRoof::comp(const std::map<std::string, std::string>& name_map, std::vector<boost::shared_ptr<Shape> >& shapes) {
	Polygon_2 poly;
	for (int i = 0; i < _points.size(); ++i) {
		poly.push_back(KPoint(_points[i].x, _points[i].y));
	}

	// You can pass the polygon via an iterator pair
	SsPtr iss = CGAL::create_interior_straight_skeleton_2(poly);

	std::map<int, glm::vec2> new_pts;
	std::map<int, int> num_edges;

	int count = 0;
	for (auto face = iss->faces_begin(); face != iss->faces_end(); ++face, ++count) {
		num_edges[count] = 0;
		{
			auto edge0 = face->halfedge();
			auto edge = edge0;
			do {
				num_edges[count]++;
			} while ((edge = edge->next()) != edge0);
		}

		if (num_edges[count] == 3) {
			auto edge0 = face->halfedge();
			auto edge = edge0;
			glm::vec2 p0, p1, p2;

			do {
				auto head = edge->vertex();
				auto tail = edge->opposite()->vertex();

				if (edge->is_bisector()) {
					if (edge->is_inner_bisector()) { // 外側に接続されていない分割線
					} else { // 外側と接続されている分割線
						p2 = glm::vec2(head->point().x(), head->point().y());
						new_pts[head->id()] = (p0 + p1) * 0.5f; // to be fixed
					}
				} else { // 一番外側のボーダー
					p0 = glm::vec2(tail->point().x(), tail->point().y());
					p1 = glm::vec2(head->point().x(), head->point().y());
				}
			} while ((edge = edge->next()) != edge0);
		}
	}

	count = 0;
	for (auto face = iss->faces_begin(); face != iss->faces_end(); ++face, ++count) {
		// 各faceについて、ポリゴンを生成する
		auto edge0 = face->halfedge();
		auto edge = edge0;

		// 最初のエッジを保存する
		glm::vec2 p0, p1;
		bool first = true;

		glm::vec3 prev_p;

		do {
			auto head = edge->vertex();
			auto tail = edge->opposite()->vertex();

			if (first) {
				p0 = glm::vec2(tail->point().x(), tail->point().y());
				p1 = glm::vec2(head->point().x(), head->point().y());
				first = false;

				prev_p = glm::vec3(p1, 0);
			} else {
				glm::vec2 p2 = glm::vec2(head->point().x(), head->point().y());

				if (p2 != p0) {
					// p2の高さを計算
					float z = glutils::distance(p0, p1, p2) * tanf(_angle * M_PI / 180.0f);

					if (new_pts.find(head->id()) != new_pts.end()) {
						p2 = new_pts[head->id()];
					}

					// vertical faces
					if (num_edges[count] == 3 && name_map.find("side") != name_map.end() && name_map.at("side") != "NIL") {
						glm::vec2 v1(1, 0);
						glm::vec2 v2 = glm::normalize(p1 - p0);
						float theta = acos(glm::dot(v1, v2));
						if (v1.x * v2.y - v1.y * v2.x < 0) {
							theta = -theta;
						}

						glm::mat4 mat = glm::rotate(glm::rotate(glm::translate(glm::mat4(), glm::vec3(p0, 0)), theta, glm::vec3(0, 0, 1)), M_PI * 0.5f, glm::vec3(1, 0, 0));
						glm::mat4 inv = glm::inverse(mat);

						// この辺り、正しくない。to be fixed!!!!!
						std::vector<glm::vec2> pts2d;
						pts2d.push_back(glm::vec2(0, 0));
						pts2d.push_back(glm::vec2(inv * glm::vec4(prev_p, 1)));
						pts2d.push_back(glm::vec2(pts2d[1].x * 0.5, z));

						shapes.push_back(boost::shared_ptr<Shape>(new Polygon(name_map.at("side"), _pivot, _modelMat * mat, pts2d, _color, _texture)));
					} else if (num_edges[count] > 3 && name_map.find("top") != name_map.end() && name_map.at("top") != "NIL") {
						std::vector<glm::vec3> pts3d;
						std::vector<glm::vec3> normals;
						pts3d.push_back(glm::vec3(p0, 0));
						pts3d.push_back(prev_p);
						pts3d.push_back(glm::vec3(p2, z));
						glm::vec3 n = glm::cross(pts3d[1] - pts3d[0], pts3d[2] - pts3d[0]);
						normals.push_back(n);
						normals.push_back(n);
						normals.push_back(n);
						shapes.push_back(boost::shared_ptr<Shape>(new GeneralObject(name_map.at("top"), _pivot, _modelMat, pts3d, normals, _color)));
					}
	
					prev_p = glm::vec3(p2, z);
				}
			}
		} while ((edge = edge->next()) != edge0);
	}
}

void GableRoof::generateGeometry(std::vector<glutils::Face>& faces, float opacity) const {
	std::vector<Vertex> vertices;

	Polygon_2 poly;
	for (int i = 0; i < _points.size(); ++i) {
		poly.push_back(KPoint(_points[i].x, _points[i].y));
	}

	// You can pass the polygon via an iterator pair
	SsPtr iss = CGAL::create_interior_straight_skeleton_2(poly);

	std::map<int, glm::vec2> new_pts;

	for (auto face = iss->faces_begin(); face != iss->faces_end(); ++face) {
		int numEdges = 0;
		{
			auto edge0 = face->halfedge();
			auto edge = edge0;
			do {
				numEdges++;
			} while ((edge = edge->next()) != edge0);
		}

		if (numEdges == 3) {
			auto edge0 = face->halfedge();
			auto edge = edge0;
			glm::vec2 p0, p1, p2;

			do {
				auto head = edge->vertex();
				auto tail = edge->opposite()->vertex();

				if (edge->is_bisector()) {
					if (edge->is_inner_bisector()) { // 外側に接続されていない分割線
					} else { // 外側と接続されている分割線
						p2 = glm::vec2(head->point().x(), head->point().y());
						new_pts[head->id()] = (p0 + p1) * 0.5f; // to be fixed
					}
				} else { // 一番外側のボーダー
					p0 = glm::vec2(tail->point().x(), tail->point().y());
					p1 = glm::vec2(head->point().x(), head->point().y());
				}
			} while ((edge = edge->next()) != edge0);
		}
	}

	for (auto face = iss->faces_begin(); face != iss->faces_end(); ++face) {
		// 各faceについて、ポリゴンを生成する
		auto edge0 = face->halfedge();
		auto edge = edge0;

		// 最初のエッジを保存する
		glm::vec2 p0, p1;
		bool first = true;

		glm::vec3 prev_p;

		do {
			auto head = edge->vertex();
			auto tail = edge->opposite()->vertex();

			if (first) {
				p0 = glm::vec2(tail->point().x(), tail->point().y());
				p1 = glm::vec2(head->point().x(), head->point().y());
				first = false;

				prev_p = glm::vec3(p1, 0);
			} else {
				glm::vec2 p2 = glm::vec2(head->point().x(), head->point().y());

				if (p2 != p0) {
					// p2の高さを計算
					float z = glutils::distance(p0, p1, p2) * tanf(_angle * M_PI / 180.0f);

					if (new_pts.find(head->id()) != new_pts.end()) {
						p2 = new_pts[head->id()];
					}

					// 三角形を作成
					glm::vec3 v0 = glm::vec3(_pivot * _modelMat * glm::vec4(p0, 0, 1));
					glm::vec3 v1 = glm::vec3(_pivot * _modelMat * glm::vec4(prev_p, 1));
					glm::vec3 v2 = glm::vec3(_pivot * _modelMat * glm::vec4(p2, z, 1));

					glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

					vertices.push_back(Vertex(v0, normal, glm::vec4(_color, opacity)));
					vertices.push_back(Vertex(v1, normal, glm::vec4(_color, opacity)));
					vertices.push_back(Vertex(v2, normal, glm::vec4(_color, opacity)));
	
					prev_p = glm::vec3(p2, z);
				}
			}
		} while ((edge = edge->next()) != edge0);
	}

	faces.push_back(glutils::Face(_name, vertices));
}

}
