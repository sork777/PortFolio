#include "Framework.h"
#include "QuadTree.h"



QuadTreeNode::QuadTreeNode(QuadTree* tree)
	:tree(tree)
{
	collider = tree->collider;
	colInst = collider->GetSize();
}

QuadTreeNode::~QuadTreeNode()
{

	for(QuadTreeNode* child:children)
		SafeDelete(child);
	children.shrink_to_fit();
}

void QuadTreeNode::AddChild(QuadTreeNode* child)
{
	children.emplace_back(child);
}

const bool & QuadTreeNode::IsIntersect(Collider * other)
{
	// TODO: 여기에 반환 구문을 삽입합니다.
	return collider->IsIntersect(other, colInst);
}

QuadTreeNode* QuadTreeNode::GetPickedNode(Vector3& rayPos, Vector3 & rayDir, float& minDist)
{
	QuadTreeNode* resultNode = NULL;

	float dist;
	//충돌 통과 시 실행
	if (collider->IsIntersectRay(rayPos, rayDir, dist,colInst))
	{
		//충돌 했는데 자식 없으면 최종 노드로 내보냄
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
	Vector3 colmax = collider->GetMaxRound(colInst);
	Vector3 colmin = collider->GetMinRound(colInst);
	//충돌 통과 시 실행
	
	if (colmax.x>position.x && colmin.x < position.x &&
		colmax.z > position.z && colmin.z < position.z)
	{
		//충돌 했는데 자식 없으면 최종 노드로 내보냄
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
	collider = new OBBCollider();
}


QuadTree::~QuadTree()
{
	SafeDelete(RootNode);
	SafeDelete(collider);
}

void QuadTree::Update()
{
	collider->Update();
}

void QuadTree::Render()
{
	//RootNode->Render();
	collider->Render();
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
