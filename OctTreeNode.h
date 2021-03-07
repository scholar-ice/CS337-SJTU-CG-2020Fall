#pragma once
#include "Ogre.h"
#include "OgreApplicationContext.h"
#include "OgreInput.h"
#include "OgreRTShaderSystem.h"
#include "OgreRectangle2D.h"
#include "Ogre.h"
#include <vector>
#include <iostream>
#include <queue>
#include <string>
#include <Windows.h>
#include <fstream>

using namespace Ogre;
using namespace OgreBites;

//����ƬԪ
struct triangle {
	Vector3 A, B, C;
	Vector2 tA, tB, tC;
	Vector3 center;
};

//��׶ ���������У����λ�ã�������Ϸ�����������������z��/Զƽ������꣬�����(aspect=width/height)���ӽǴ�С(fovy)
struct frustum {
	Vector3 camPos;
	Vector3 camUp;
	Vector3 lookAt;
	Real zNear, zFar;
	Real aspect, fovy;
};

class OctTreeTriangle {
public:
	OctTreeTriangle(std::vector<triangle> obj);
	void set_aligned_box();
	void set_aligned_box(Vector3 min, Vector3 max);
	void split();
	bool is_leaf();

	Ogre::AxisAlignedBox box;
	OctTreeTriangle* childlist[8];
	std::vector<triangle> objlist;
private:
	double getmin(int axis);
	double getmax(int axis);

	int threashold = 50;
};