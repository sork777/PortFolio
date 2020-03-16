#include "Framework.h"
#include "QuadTree.h"



QuadTreeNode::QuadTreeNode()
	:collider(NULL)
{
}

QuadTreeNode::~QuadTreeNode()
{
	SafeDelete(collider);

	for(QuadTreeNode* child:children)
		SafeDelete(child);
	children.shrink_to_fit();
}

void QuadTreeNode::Render(Color color)
{
	if (children.size() < 1)
	{
		collider->Render(color);
	}
	else
	{
		for (QuadTreeNode* child : children)
			child->Render(color);
	}
}

void QuadTreeNode::SetCollider(OBBCollider * collider)
{
	this->collider = collider;
}
void QuadTreeNode::AddChild(QuadTreeNode* child)
{
	children.emplace_back(child);
}

QuadTreeNode* QuadTreeNode::GetPickedNode(Vector3& rayPos, Vector3 & rayDir, float& minDist)
{
	QuadTreeNode* resultNode = NULL;

	collider->Update();
	float dist;
	//�浹 ��� �� ����
	if (collider->IsIntersectRay(rayPos, rayDir, dist))
	{
		//�浹 �ߴµ� �ڽ� ������ ���� ���� ������
		if (children.size() < 1)
		{
			if (minDist > dist)
			{
				minDist = dist;
				return this;				
			}
		}
		for (QuadTreeNode* child : children)
		{
			QuadTreeNode* picked = child->GetPickedNode(rayPos, rayDir, minDist);
			if(picked != NULL)
				resultNode=picked;
		}
	}

	return resultNode;
}

QuadTreeNode * QuadTreeNode::GetPickedNode(Vector3 & position)
{
	QuadTreeNode* resultNode = NULL;
	Vector3 colmax = collider->GetMaxRound();
	Vector3 colmin = collider->GetMinRound();
	//�浹 ��� �� ����
	
	if (colmax.x>position.x && colmin.x < position.x &&
		colmax.z > position.z && colmin.z < position.z)
	{
		//�浹 �ߴµ� �ڽ� ������ ���� ���� ������
		if (children.size() < 1)
		{
			return this;
		}
		for (QuadTreeNode* child : children)
		{
			resultNode = child->GetPickedNode(position);
			if (resultNode != NULL)
				return resultNode;
		}
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////

QuadTree::QuadTree(QuadTreeNode* Root)
	:RootNode(Root)
{
}


QuadTree::~QuadTree()
{
	SafeDelete(RootNode);
}

void QuadTree::Render(Color color)
{
	RootNode->Render(color);
}

QuadTreeNode* QuadTree::GetPickedNode(Vector3& rayPos, Vector3& rayDir)
{
	float dist = FLT_MAX;

	return RootNode->GetPickedNode(rayPos, rayDir,dist);
}

QuadTreeNode * QuadTree::GetPickedNode(Vector3 & position)
{
	return RootNode->GetPickedNode(position);
}
