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
//����������
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
	//Ĭ�Ϲ��캯��
	KdTree()
	{
		parent = NULL;
		leftChild = NULL;
		rightChild = NULL;
		ClusterIdx = -1;
		isRemoved = false;
	}

	//�ж�kd���Ƿ�ֻ��һ��Ҷ�ӽ��
	bool isLeaf()
	{
		return (rightChild == NULL && leftChild == NULL);
	}
	//�ж��Ƿ������ĸ����
	bool isRoot()
	{
		return (parent == NULL);
	}
	//�жϸ���kd���ĸ�����Ƿ����丸kd��������
	bool isLeft()
	{
		return parent->leftChild->root == root;
	}
	//�жϸ���kd���ĸ�����Ƿ����丸kd�����ҽ��
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