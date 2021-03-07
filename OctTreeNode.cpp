#pragma once

#include "OctTreeNode.h"

OctTreeTriangle::OctTreeTriangle(std::vector<triangle> obj) {
	objlist = obj;
}

double OctTreeTriangle::getmin(int axis) {
	double curmin = 1e10;
	int len = objlist.size();
	for (int i = 0; i < len; i++) {
		if (objlist[i].A[axis] < curmin)
			curmin = objlist[i].A[axis];
		if (objlist[i].B[axis] < curmin)
			curmin = objlist[i].B[axis];
		if (objlist[i].C[axis] < curmin)
			curmin = objlist[i].C[axis];
	}
	return curmin;
}

double OctTreeTriangle::getmax(int axis) {
	double curmax = -1e10;
	int len = objlist.size();
	for (int i = 0; i < len; i++) {
		if (objlist[i].A[axis] > curmax)
			curmax = objlist[i].A[axis];
		if (objlist[i].B[axis] > curmax)
			curmax = objlist[i].B[axis];
		if (objlist[i].C[axis] > curmax)
			curmax = objlist[i].C[axis];
	}
	return curmax;
}

void OctTreeTriangle::set_aligned_box() {
	Vector3 min, max;
	for (int i = 0; i < 3; i++) {
		min[i] = getmin(i);
		max[i] = getmax(i);
	}
	box.setMinimum(min);
	box.setMaximum(max);
}

void OctTreeTriangle::set_aligned_box(Vector3 min, Vector3 max) {
	box.setMinimum(min);
	box.setMaximum(max);
}

void OctTreeTriangle::split() {
	if (objlist.size() <= threashold) {
		return;
	}

	std::vector<std::vector<triangle>> child_objlists;
	child_objlists.resize(8);
	Vector3 middle = box.getCenter();
	Ogre::AxisAlignedBox::Corners corners = box.getAllCorners();

	int A_count, B_count, C_count, final_count;
	for (int i = 0; i < objlist.size(); i++) {
		A_count = B_count = C_count = 0;

		if (objlist[i].A[0] >= middle[0])
			A_count += 1;
		if (objlist[i].A[1] >= middle[1])
			A_count += 2;
		if (objlist[i].A[2] >= middle[2])
			A_count += 4;

		if (objlist[i].B[0] >= middle[0])
			B_count += 1;
		if (objlist[i].B[1] >= middle[1])
			B_count += 2;
		if (objlist[i].B[2] >= middle[2])
			B_count += 4;

		if (objlist[i].C[0] >= middle[0])
			C_count += 1;
		if (objlist[i].C[1] >= middle[1])
			C_count += 2;
		if (objlist[i].C[2] >= middle[2])
			C_count += 4;

		final_count = std::min(std::min(A_count, B_count), C_count);

		switch (final_count) {
		case 0:child_objlists[0].push_back(objlist[i]);
			break;
		case 1:child_objlists[3].push_back(objlist[i]);
			break;
		case 2:child_objlists[1].push_back(objlist[i]);
			break;
		case 3:child_objlists[2].push_back(objlist[i]);
			break;
		case 4:child_objlists[6].push_back(objlist[i]);
			break;
		case 5:child_objlists[7].push_back(objlist[i]);
			break;
		case 6:child_objlists[5].push_back(objlist[i]);
			break;
		case 7:child_objlists[4].push_back(objlist[i]);
			break;
		}
	}

	for (int i = 0; i < 8; i++) {
		Vector3 MIN, MAX;
		for (int j = 0; j < 3; j++) {
			MIN[j] = std::min(corners[i][j], middle[j]);
			MAX[j] = std::max(corners[i][j], middle[j]);
		}
		childlist[i] = new OctTreeTriangle(child_objlists[i]);
		child_objlists[i].clear();
		childlist[i]->set_aligned_box(MIN, MAX);
	}
	child_objlists.clear();
	objlist.clear();
	for (int i = 0; i < 8; i++) {
		childlist[i]->split();
	}
}

bool OctTreeTriangle::is_leaf() {
	if (childlist[1] != NULL)
		return false;
	else return true;
}