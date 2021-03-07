#pragma once

#include "OctTreeNode.h"

using namespace Ogre;
using namespace OgreBites;

class OctTree {
public:
	OctTree(const char* path);
	~OctTree();
	std::vector<triangle> search(OctTreeTriangle* node, double** coef,int mode);
	std::vector<triangle> search(frustum* frus, double** coef);
	std::vector<triangle> get_all();
	void split();
	Vector3 get_min();
	Vector3 get_max();
private:
	OctTreeTriangle* root;
	std::vector<triangle> all_triangles;
	void read_obj(const char* path); //引用了助教提供的源码
	void calculate_plane(Vector3 point1, Vector3 point2, Vector3 point3, Vector3 judgePoint, double* coef); //引用了助教提供的源码
	void get_frustem_coef(frustum* frus, double** coef);
	void is_points_in_frustum(double** coef, std::vector<Vector3> point, bool** inner); //引用了助教提供的源码
};