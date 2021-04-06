#pragma once
#include "kd_tree.h"


//操作符重载
std::ostream& operator <<(std::ostream &os, const std::vector<Point> &P)
{
	for (auto &i : P)
		std::cout << i.x << " " << i.y << " " << i.z << std::endl;
	//cout << endl;
	os.flush();
	return os;
}
std::ostream& operator <<(std::ostream &os, const Point &P)
{

	std::cout << P.x << " " << P.y << " " << P.z;
	//cout << endl;
	os.flush();
	return os;
}


std::vector<std::vector<float> > Transpose(std::vector<Point> Matrix)
{
	unsigned row = (unsigned)Matrix.size();
	unsigned col = 3;
	std::vector<float> tmp_x, tmp_y, tmp_z;
	for (auto p : Matrix)
	{
		tmp_x.push_back(p.x);
		tmp_y.push_back(p.y);
		tmp_z.push_back(p.z);
	}
	std::vector<std::vector<float> > Trans;
	Trans.push_back(tmp_x);
	Trans.push_back(tmp_y);
	Trans.push_back(tmp_z);
	return Trans;
}

float findMiddleValue(std::vector<float> vec)
{
	sort(vec.begin(), vec.end());
	auto pos = vec.size() / 2;
	return vec[pos];
}

float computeVar(std::vector<float> A)
{

	float sum = (float)std::accumulate(A.begin(), A.end(), 0);
	float mean = sum / A.size();

	float accum = 0;
	for (auto &i : A)
		accum += (i - mean)*(i - mean);
	float var = accum / (A.size() - 1);
	return var;
}

int findSplitAttribute(std::vector<std::vector<float> > transData)
{
	std::vector<int> vars;
	for (auto i : transData)
		vars.push_back(computeVar(i));
	auto biggest = std::max_element(std::begin(vars), std::end(vars));
	int idx = std::distance(std::begin(vars), biggest);
	return idx;
}
void buildKdTree(KdTree* tree, std::vector<Point> data, unsigned depth)
{

	//样本的数量
	int samplesNum = data.size();

	//终止条件
	if (samplesNum == 0)
	{
		return;
	}
	if (samplesNum == 1)
	{
		tree->root = data[0];
		return;
	}
	//样本的维度
	std::vector<std::vector<float> > transData = Transpose(data);
	//选择切分属性
	//unsigned splitAttribute = depth % k;
	unsigned splitAttribute = findSplitAttribute(transData);
	std::vector<float> splitAttributeValues = transData[splitAttribute];
	//选择切分值
	float splitValue = findMiddleValue(splitAttributeValues);
	//cout << "splitValue" << splitValue  << endl;

	// 根据选定的切分属性和切分值，将数据集分为两个子集
	std::vector<Point> subset1;
	std::vector<Point> subset2;
	for (unsigned i = 0; i < samplesNum; ++i)
	{
		if (splitAttributeValues[i] == splitValue)
			tree->root = data[i];
		else
		{
			if (splitAttributeValues[i] < splitValue)
				subset1.push_back(data[i]);
			else
				subset2.push_back(data[i]);
		}
	}


	tree->attribute = splitAttribute;
	if (!subset1.empty())
	{
		tree->leftChild = new KdTree;
		tree->leftChild->parent = tree;
		buildKdTree(tree->leftChild, subset1, depth + 1);
	}
	if (!subset2.empty())
	{
		tree->rightChild = new KdTree;
		tree->rightChild->parent = tree;
		buildKdTree(tree->rightChild, subset2, depth + 1);
	}
}

void printKdTree(KdTree *tree, unsigned depth)
{
	for (unsigned i = 0; i < depth; ++i)
		cout << "\t";

	cout << tree->root << endl;
	if (tree->leftChild == NULL && tree->rightChild == NULL)//叶子节点
		return;
	else //非叶子节点
	{
		if (tree->leftChild != NULL)
		{
			for (unsigned i = 0; i < depth + 1; ++i)
				cout << "\t";
			cout << " left:";
			printKdTree(tree->leftChild, depth + 1);
		}

		cout << endl;
		if (tree->rightChild != NULL)
		{
			for (unsigned i = 0; i < depth + 1; ++i)
				cout << "\t";
			cout << "right:";
			printKdTree(tree->rightChild, depth + 1);
		}
		cout << endl;
	}
}

void getKdTreeValue(KdTree *tree, std::vector<Point> &out, std::vector<int> &idx)
{
	idx.push_back(tree->ClusterIdx);
	out.push_back(tree->root);
	if (tree->leftChild == NULL && tree->rightChild == NULL)//叶子节点
		return;
	else //非叶子节点
	{
		if (tree->leftChild != NULL)
			getKdTreeValue(tree->leftChild, out, idx);
		if (tree->rightChild != NULL)
			getKdTreeValue(tree->rightChild, out, idx);
	}
}
bool searchUndefinedIdx(KdTree *tree, KdTree* & newTree)
{
	//if (tree == NULL)
	//	return false;
	if (tree->ClusterIdx == -1)
	{
		newTree = tree;
		return true;
	}
	if (tree->leftChild == NULL && tree->rightChild == NULL)//叶子节点
		return false;
	//else
	//	return searchUndefinedIdx(tree->leftChild, newTree) | 
	//	searchUndefinedIdx(tree->rightChild, newTree);
	bool left = false;
	bool right = false;
	if (tree->leftChild != NULL)
		left = searchUndefinedIdx(tree->leftChild, newTree);
	if (tree->rightChild != NULL)
		right = searchUndefinedIdx(tree->rightChild, newTree);
	return (right | left);

}
float measureDistance(Point point1, Point point2, unsigned method)
{
	switch (method)
	{
	case 0://欧氏距离
	{
		float res = pow((point1.x - point2.x), 2) + pow((point1.y - point2.y), 2) + pow((point1.z - point2.z), 2);
		return sqrt(res);
	}
	case 1://曼哈顿距离
	{

		float res = abs(point1.x - point2.x) + abs(point1.y - point2.y) + abs(point1.z - point2.z);
		return res;
	}
	case 2://点云水平距离
	{
		float res = pow((point1.x - point2.x), 2) + pow((point1.y - point2.y), 2) + 0.1*pow((point1.z - point2.z), 2);
		return sqrt(res);
	}
	default:
	{
		cerr << "Invalid method!!" << endl;
		return -1;
	}
	}
}

void searchRadiusNeighbor(KdTree *tree, Point goal, float radius, std::vector<Point> &out,bool first)
{
	if (tree == nullptr)
		return;
	KdTree* currentTree = tree;
	//这里的 currentNearest分割点的坐标
	Point currentNearest = currentTree->root;
	// 点云自定义距离
	float currentDistance = measureDistance(goal, currentNearest, 2);
	//如果距离符合，输出
	/*if (currentTree->isRemoved && first)
		return;*/
	first = true;
	if (currentDistance <= radius)
	{		
		if(!currentTree->isRemoved )
			out.push_back(currentNearest);
		currentTree->isRemoved = true;
	}
		
	// 是否为叶节点
	if (!currentTree->isLeaf())
	{
		// 当前分割属性
		unsigned index = currentTree->attribute;
		float districtDistance = -1;
		switch (index)
		{
		case 0:
		{
			districtDistance = goal.x - currentTree->root.x;
			break;
		}
		case 1:
		{
			districtDistance = goal.y - currentTree->root.y;
			break;
		}case 2:
		{
			districtDistance = goal.z - currentTree->root.z;
			break;
		}
		default:
			break;
		}
		if (districtDistance == -1)
			cout << "出错districtDistance == -1" << endl;
		if (abs(districtDistance) <= radius)
		{
			if (currentTree->leftChild != NULL)
				searchRadiusNeighbor(currentTree->leftChild, goal, radius, out,first);
			if (currentTree->rightChild != NULL)
				searchRadiusNeighbor(currentTree->rightChild, goal, radius, out,first);
		}
		else
		{
			if (currentTree->rightChild != NULL && districtDistance < 0)
				searchRadiusNeighbor(currentTree->leftChild, goal, radius, out,first);
			else if (currentTree->rightChild != NULL && districtDistance > 0)
				searchRadiusNeighbor(currentTree->rightChild, goal, radius, out,first);
		}
	}
}

void undoRemoved(KdTree *tree, Point goal)
{
	KdTree* currentTree = tree;
	if (tree == nullptr)
		return;
	//这里的 currentNearest分割点的坐标
	Point currentNearest = currentTree->root;

	if (currentTree->root == goal)
	{
		currentTree->isRemoved = false;
	}

	// 是否为叶节点
	if (!currentTree->isLeaf())
	{
		// 当前分割属性
		unsigned index = currentTree->attribute;
		float districtDistance = -1;
		switch (index)
		{
		case 0:
		{
			districtDistance = goal.x - currentTree->root.x;
			break;
		}
		case 1:
		{
			districtDistance = goal.y - currentTree->root.y;
			break;
		}case 2:
		{
			districtDistance = goal.z - currentTree->root.z;
			break;
		}
		default:
			break;
		}
		if (districtDistance == -1)
			cout << "出错districtDistance == -1" << endl;
		if (abs(districtDistance) <= 0)
		{
			if (currentTree->leftChild != NULL)
				undoRemoved(currentTree->leftChild, goal);
			if (currentTree->rightChild != NULL)
				undoRemoved(currentTree->rightChild, goal);
		}
		else
		{
			if (currentTree->rightChild != NULL && districtDistance < 0)
				undoRemoved(currentTree->leftChild, goal);
			else if (currentTree->rightChild != NULL && districtDistance > 0)
				undoRemoved(currentTree->rightChild, goal);
		}
	}
}

void searchRadiusNeighborConditional(KdTree *tree, KdTree *target_tree, float radius, std::vector<KdTree*> &out)
{
	KdTree* currentTree = tree;
	Point currentNearest = currentTree->root;
	Point goal = target_tree->root;

	// 点云自定义距离
	if (currentTree->ClusterIdx == -1)
	{
		float currentDistance = measureDistance(goal, currentNearest, 0);
		if (currentDistance <= radius)
		{
			out.push_back(currentTree);
			currentTree->ClusterIdx = target_tree->ClusterIdx;
		}
	}


	// 是否为叶节点
	if (!currentTree->isLeaf())
	{
		// 当前分割属性
		unsigned index = currentTree->attribute;
		float districtDistance = goal[index] - currentTree->root[index];

		if (abs(districtDistance) <= radius)
		{
			if (currentTree->leftChild != NULL)
				searchRadiusNeighborConditional(currentTree->leftChild, target_tree, radius, out);
			if (currentTree->rightChild != NULL)
				searchRadiusNeighborConditional(currentTree->rightChild, target_tree, radius, out);
		}
		else
		{
			if (currentTree->leftChild != NULL && districtDistance < 0)
				searchRadiusNeighborConditional(currentTree->leftChild, target_tree, radius, out);
			else if (currentTree->rightChild != NULL && districtDistance > 0)
				searchRadiusNeighborConditional(currentTree->rightChild, target_tree, radius, out);
		}
	}
}