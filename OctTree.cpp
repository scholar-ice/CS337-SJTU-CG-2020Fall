#include "OctTree.h"

OctTree::OctTree(const char* path) {
	read_obj(path);
	root = new OctTreeTriangle(all_triangles);
}

OctTree::~OctTree() {
	delete root;
}

void OctTree::split() {
	std::cout << "Model splitting starts." << std::endl;
	clock_t c1 = clock();
	root->set_aligned_box();
	root->split();
	clock_t c2 = clock();
	std::cout << "Model splitting ends. Using time is: " << double((c2 - c1)) / CLOCKS_PER_SEC << std::endl;
}

Vector3 OctTree::get_min() {
	return root->box.getMinimum();
}

Vector3 OctTree::get_max() {
	return root->box.getMaximum();
}

void OctTree::read_obj(const char* path) {
	FILE* fp = fopen(path, "r");
	if (fp == NULL) {
		return;
	}
	std::cout << "Loading obj to Memory... " << '\n';
	std::string s;
	std::vector<Vector3> vert;
	std::vector<Vector2> texture;
	while (!feof(fp)) {
		char s = fgetc(fp);
		char c[100];
		if (s == '#' || s == 'm' || s == 'u') {
			fscanf(fp, "%[^\n]", c);
			continue;
		}
		if (s == 'v' && fgetc(fp) == ' ') {
			Vector3 v;
			fscanf(fp, "%f %f %f", &v[0], &v[1], &v[2]);
			vert.push_back(v);
		}
		else if (s == 'v') {
			Vector2 t;
			fscanf(fp, "%f %f", &t[0], &t[1]);
			t[1] = 1 - t[1];
			texture.push_back(t);
		}
		else if (s == 'f') {
			Vector3 vi, ti;
			fscanf(fp, "%f/%f %f/%f %f/%f", &vi[0], &ti[0], &vi[1], &ti[1], &vi[2], &ti[2]);
			triangle tri;
			tri.A = vert[vi[0] - 1];
			tri.B = vert[vi[1] - 1];
			tri.C = vert[vi[2] - 1];
			tri.tA = texture[ti[0] - 1];
			tri.tB = texture[ti[1] - 1];
			tri.tC = texture[ti[2] - 1];
			tri.center[0] = (tri.A[0] + tri.B[0] + tri.C[0]) / 3.0;
			tri.center[1] = (tri.A[1] + tri.B[1] + tri.C[1]) / 3.0;
			tri.center[2] = (tri.A[2] + tri.B[2] + tri.C[2]) / 3.0;
			all_triangles.push_back(tri);
		}
	}
	std::cout << "obj Loaded successfully!" << '\n' << "size of obj is" << all_triangles.size() << std::endl;
}

std::vector<triangle> OctTree::get_all() {
	return all_triangles;
}

void OctTree::calculate_plane(Vector3 point1, Vector3 point2, Vector3 point3, Vector3 judgePoint, double* coef) {
	coef[0] = (point2[1] - point1[1]) * (point3[2] - point1[2]) - (point2[2] - point1[2]) * (point3[1] - point1[1]);	//A = (y2 - y1)*(z3 - z1) - (z2 -z1)*(y3 - y1)
	coef[1] = (point3[0] - point1[0]) * (point2[2] - point1[2]) - (point2[0] - point1[0]) * (point3[2] - point1[2]);	//B = (x3 - x1)*(z2 - z1) - (x2 - x1)*(z3 - z1)
	coef[2] = (point2[0] - point1[0]) * (point3[1] - point1[1]) - (point3[0] - point1[0]) * (point2[1] - point1[1]);	//C = (x2 - x1)*(y3 - y1) - (x3 - x1)*(y2 - y1)
	coef[3] = (-1) * (coef[0] * point1[0] + coef[1] * point1[1] + coef[2] * point1[2]);									//D = -(A * x1 + B * y1 + C * z1)
	if (coef[0] * judgePoint[0] + coef[1] * judgePoint[1] + coef[2] * judgePoint[2] + coef[3] > 0)
		coef[4] = 1;
	else if (coef[0] * judgePoint[0] + coef[1] * judgePoint[1] + coef[2] * judgePoint[2] + coef[3] < 0)
		coef[4] = -1;
	else
		coef[4] = 0;
}

void OctTree::get_frustem_coef(frustum* frus, double** coef) {
	Vector3 points[8];				//8顶点 前左上、前右上、前左下、前右下、后左上、后右上、后左下、后右下
	Vector3 centers[2];				//2中心 前平面中心 后平面中心
	frus->camUp.normalise();		//coef 6平面 上、下、左、右、前、后 ax+by+cz+d=0 abcd分别对应0-3

	Vector3 viewDir = frus->lookAt - frus->camPos; //视线方向
	viewDir.normalise();

	Vector3 camRight;				//指向相机右侧的向量
	camRight[0] = viewDir[1] * frus->camUp[2] - viewDir[2] * frus->camUp[1];
	camRight[1] = viewDir[2] * frus->camUp[0] - viewDir[0] * frus->camUp[2];
	camRight[2] = viewDir[0] * frus->camUp[1] - viewDir[1] * frus->camUp[0];
	camRight.normalise();

	centers[0] = frus->camPos + viewDir * frus->zNear;
	centers[1] = frus->camPos + viewDir * frus->zFar;
	points[0] = centers[0] + frus->camUp * frus->zNear * tan(frus->fovy / 2) - camRight * frus->zNear * tan(frus->fovy / 2) * frus->aspect;
	points[1] = centers[0] + frus->camUp * frus->zNear * tan(frus->fovy / 2) + camRight * frus->zNear * tan(frus->fovy / 2) * frus->aspect;
	points[2] = centers[0] - frus->camUp * frus->zNear * tan(frus->fovy / 2) - camRight * frus->zNear * tan(frus->fovy / 2) * frus->aspect;
	points[3] = centers[0] - frus->camUp * frus->zNear * tan(frus->fovy / 2) + camRight * frus->zNear * tan(frus->fovy / 2) * frus->aspect;
	points[4] = centers[1] + frus->camUp * frus->zFar * tan(frus->fovy / 2) - camRight * frus->zFar * tan(frus->fovy / 2) * frus->aspect;
	points[5] = centers[1] + frus->camUp * frus->zFar * tan(frus->fovy / 2) + camRight * frus->zFar * tan(frus->fovy / 2) * frus->aspect;
	points[6] = centers[1] - frus->camUp * frus->zFar * tan(frus->fovy / 2) - camRight * frus->zFar * tan(frus->fovy / 2) * frus->aspect;
	points[7] = centers[1] - frus->camUp * frus->zFar * tan(frus->fovy / 2) + camRight * frus->zFar * tan(frus->fovy / 2) * frus->aspect;
	calculate_plane(points[0], points[4], points[5], points[6], coef[0]); //上
	calculate_plane(points[2], points[6], points[7], points[4], coef[1]); //下
	calculate_plane(points[0], points[4], points[6], points[7], coef[2]); //左
	calculate_plane(points[1], points[5], points[7], points[4], coef[3]); //右
	calculate_plane(points[0], points[1], points[2], points[4], coef[4]); //前
	calculate_plane(points[4], points[5], points[6], points[0], coef[5]); //后
}

void OctTree::is_points_in_frustum(double** coef, std::vector<Vector3> point, bool** inner) {
	int point_num = point.size();

	//inner[i][0]表示第i个点是否位于这个视锥内	(1->在内部，0->不在内部)
	//inner[i][1-6]表示第i个点是否在第i个平面内	(1->在内部，0->不在内部)
	for (int j = 0; j < point_num; j++) {
		inner[j][0] = 1;
		for (int i = 0; i < 6; i++) {
			if ((coef[i][0] * point[j][0] + coef[i][1] * point[j][1] + coef[i][2] * point[j][2] + coef[i][3]) * coef[i][4] <= 0) {
				inner[j][0] = 0;
				inner[j][i + 1] = 0;
			}
			else {
				inner[j][i + 1] = 1;
			}
		}
	}
}

std::vector<triangle> OctTree::search(frustum* frus, double** coef) {
	get_frustem_coef(frus, coef);

	return search(root, coef, 0);
}

std::vector<triangle> OctTree::search(OctTreeTriangle* node, double** coef, int mode) {
	// 单线程 最终返回的ans内包含前序遍历的所有面片
	// mode=1		：记录所有面片
	// mode=2		：不查找
	// 其他(mode=0)	：查找所有子节点
	std::vector<triangle> res;
	if (node == NULL || mode == 2)
		return res;
	else if (mode == 1) {
		if (node->is_leaf()) {
			res.insert(res.end(), node->objlist.begin(), node->objlist.end());
			return res;
		}
		std::vector<triangle> child_res;
		for (int i = 0; i < 8; i++) {
			child_res = search(node->childlist[i], coef, 1);
			res.insert(res.end(), child_res.begin(), child_res.end());
		}
		return res;
	}

	//组合出AlignedBox的八个顶点points，同时计算视锥平面参数coef，确定这个AlignedBox是否整体在视锥内部(用inners保存)
	//默认认为整体在内部(mod=1)
	int mod = 1;

	//声明所有需要的变量：inners，points，coef以及中间量corner
	//计算视锥参数coef，获取八个顶点，随后判断inners
	bool** inners;
	inners = new bool* [8];
	for (int i = 0; i < 8; i++)
		inners[i] = new bool[7];
	
	std::vector<Vector3> points;
	points.resize(8);

	Vector3 corner[2];
	corner[0] = node->box.getMinimum();
	corner[1] = node->box.getMaximum();

	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			for (int k = 0; k < 2; k++) {
				points[i * 4 + j * 2 + k][0] = corner[i][0];
				points[i * 4 + j * 2 + k][1] = corner[j][1];
				points[i * 4 + j * 2 + k][2] = corner[k][2];
			}
		}
	}
	is_points_in_frustum(coef, points, inners);

	//首先判断AlignedBox是否是完全位于视锥内(即是否保留mod=1)
	for (int i = 0; i < 8; i++)
		if (!inners[i][0])
			mod = 0;
	
	//如果不是完全位于视锥内，再判断是否部分位于视锥内(mod=2:完全位于视锥外)
	if (mod == 0) {
		for (int i = 1; i < 7; i++) {
			mod = 2;
			for (int j = 0; j < 8; j++)
				if (inners[j][i])
					mod = 0;
			if (mod == 2)
				break;
		}
	}
	for (int i = 0; i < 8; i++)
		delete[] inners[i];
	delete[] inners;

	std::vector<triangle> child_res;
	for (int i = 0; i < 8; i++) {
		child_res = search(node->childlist[i], coef, mod);
		res.insert(res.end(), child_res.begin(), child_res.end());
	}
	return res;
}
