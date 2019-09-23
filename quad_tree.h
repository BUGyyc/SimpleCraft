//#include "stdafx.h"
#include <list>
using namespace std;
struct QuadAABB
{
public:
	int left;
	int top;
	int width;
	int height;
	int right;
	int bottom;
	QuadAABB()
	{
		left = right = top = bottom = width = height = 0;
	}
	QuadAABB(int left, int top, int width, int height)
	{
		this->left = left;
		this->top = top;
		this->width = width;
		this->height = height;
		this->right = left + width;
		this->bottom = top + height;
	}
	bool isContain(QuadAABB aabb)
	{
		return aabb.left >= left && aabb.right <= right && aabb.top >= top && aabb.bottom <= bottom;
	}
	bool isEqual(QuadAABB aabb)
	{
		return aabb.left == left && aabb.right == right && aabb.top == top && aabb.bottom == bottom;
	}
	bool isIntersect(QuadAABB aabb)
	{
		int nleft = aabb.left > left ? aabb.left : left;
		int nright = aabb.right < right ? aabb.right : right;
		int ntop = aabb.top > top ? aabb.top : top;
		int nbottom = aabb.bottom < bottom ? aabb.bottom : bottom;
		return nleft < nright && ntop < nbottom;
	}
};
template<typename T> class QuadTreeNode
{
public:
	static const int LT = 0;
	static const int RT = 1;
	static const int LB = 2;
	static const int RB = 3;
	list<T> dataList;
	list<QuadAABB> aabbList;
	QuadAABB aabb;
	QuadTreeNode(QuadAABB aabb) :childList(NULL) { this->aabb = aabb; }
	QuadTreeNode<T>** childList;

};
template<typename T> class QuadTree
{
private:
	int _maxDeep;
	QuadTreeNode<T>* _root;
private:
	void add(QuadTreeNode<T>* node, T data, QuadAABB aabb, int deep) //�ŵ����������ȫ����aabb��QuadTreeNode��
	{
		QuadAABB paabb = node->aabb;
		if (deep<_maxDeep)
		{
			//�����ӽڵ�
			if (node->childList == NULL)
			{
				node->childList = new QuadTreeNode<T>*[4];
				int nwidth = paabb.width / 2;
				int nheight = paabb.height / 2;
				node->childList[QuadTreeNode<T>::LT] = new QuadTreeNode<T>(QuadAABB(paabb.left, paabb.top, nwidth, nheight));
				node->childList[QuadTreeNode<T>::RT] = new QuadTreeNode<T>(QuadAABB(paabb.left + nwidth, paabb.top, nwidth, nheight));
				node->childList[QuadTreeNode<T>::LB] = new QuadTreeNode<T>(QuadAABB(paabb.left, paabb.top + nheight, nwidth, nheight));
				node->childList[QuadTreeNode<T>::RB] = new QuadTreeNode<T>(QuadAABB(paabb.left + nwidth, paabb.top + nheight, nwidth, nheight));
			}
			for (int i = 0; i<4; i++)
			{
				QuadTreeNode<T>* child = node->childList[i];
				if (child->aabb.isContain(aabb))
				{
					add(child, data, aabb, deep + 1);
					return;
				}
			}
		}
		node->dataList.push_back(data);
		node->aabbList.push_back(aabb);
	}
	void find(QuadTreeNode<T>* node, QuadAABB aabb, vector<T>& list, vector<QuadAABB>& aabbList) //�����к�aabb�ཻ��QuaTreeNode���T����list
	{
		if (node->dataList.size()>0)
		{
			list.insert(list.end(), node->dataList.begin(), node->dataList.end());
			aabbList.insert(aabbList.end(), node->aabbList.begin(), node->aabbList.end());
		}
		//�����ӽڵ�
		if (node->childList == NULL)
		{
			return;
		}
		//����ཻ���ͼ����ݹ�
		for (int i = 0; i<4; i++)
		{
			QuadTreeNode<T>* child = node->childList[i];
			if (child->aabb.isIntersect(aabb))
			{
				find(child, aabb, list, aabbList);
			}
		}
	}
public:
	QuadTree(int maxDeep = 31) //todo ���Ըĳɴ�һ��aabb
	{
		_maxDeep = maxDeep;
		int size = 2;//���Ϊ2
		for (int i = 2; i<_maxDeep; i++)
			size *= 2;
		QuadAABB aabb(-size / 2, -size / 2, size, size);
		_root = new QuadTreeNode<T>(aabb);
	}
	void add(T data, QuadAABB aabb)
	{
		add(_root, data, aabb, 1);
	}

	void find(QuadAABB aabb, vector<T>& list, bool accuracy = false) //�ҵ��Ľ��������һ���ཻ��ֻ�ǿ����ཻ
	{
		vector<QuadAABB> aabbList;
		find(_root, aabb, list, aabbList); //�����ҵ����ǿ����ཻ��T
		if (accuracy) //���ж�����Щ����ʵ�ཻ��T
		{
			for (int i = list.size() - 1; i >= 0; i--)
			{
				if (!aabbList[i].isIntersect(aabb))
				{
					list.erase(list.begin() + i);
				}
			}
		}
	}

};