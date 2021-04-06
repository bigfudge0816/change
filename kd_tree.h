#pragma once
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <cmath>
#include <numeric>
#include <time.h>
#include <glm/glm.hpp>
#include <random>
using namespace std;

struct Point {

	Point(float xx, float yy, float zz) :x(xx), y(yy), z(zz) {};
	Point() :x(0), y(0), z(0) {};
	float x;
	float y;
	float z;
	bool operator == (const Point &p)const
	{
		return (x == p.x && y == p.y && z == p.z);
	}
	float operator [](int i)
	{
		if (i > 5)
			std::cout << "colorPoint[i]" << std::endl;
		switch (i)
		{
		case 0:
			return x;
		case 1:
			return y;
		case 2:
			return z;
		default:
			break;
		}
		return 0;
	}
};
//操作符重载
std::ostream& operator <<(std::ostream &os, const std::vector<Point> &P);
std::ostream& operator <<(std::ostream &os, const Point &P);

class KdTree {
public:
	Point root;
	KdTree* parent;
	KdTree* leftChild;
	KdTree* rightChild;
	int ClusterIdx;

	int attribute;
	bool isRemoved;
	//默认构造函数
	KdTree()
	{
		parent = NULL;
		leftChild = NULL;
		rightChild = NULL;
		ClusterIdx = -1;
		isRemoved = false;
	}

	//判断kd树是否只是一个叶子结点
	bool isLeaf()
	{
		return (rightChild == NULL && leftChild == NULL);
	}
	//判断是否是树的根结点
	bool isRoot()
	{
		return (parent == NULL);
	}
	//判断该子kd树的根结点是否是其父kd树的左结点
	bool isLeft()
	{
		return parent->leftChild->root == root;
	}
	//判断该子kd树的根结点是否是其父kd树的右结点
	bool isRight()
	{
		return parent->rightChild->root == root;
	}

};

std::vector<std::vector<float> > Transpose(std::vector<Point> Matrix);

float findMiddleValue(std::vector<float> vec);

float computeVar(std::vector<float> A);

int findSplitAttribute(std::vector<std::vector<float> > transData);
void buildKdTree(KdTree* tree, std::vector<Point> data, unsigned depth);

void printKdTree(KdTree *tree, unsigned depth);

void getKdTreeValue(KdTree *tree, std::vector<Point> &out, std::vector<int> &idx);
bool searchUndefinedIdx(KdTree *tree, KdTree* & newTree);
float measureDistance(Point point1, Point point2, unsigned method);

void searchRadiusNeighbor(KdTree *tree, Point goal, float radius, std::vector<Point> &out,bool first);

void undoRemoved(KdTree *tree, Point goal);

void searchRadiusNeighborConditional(KdTree *tree, KdTree *target_tree, float radius, std::vector<KdTree*> &out);