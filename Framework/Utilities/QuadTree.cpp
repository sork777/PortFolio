#include "Framework.h"
#include "QuadTree.h"



QuadTreeNode::QuadTreeNode()
	:collider(NULL)
{
}

QuadTreeNode::~QuadTreeNode()
{
	SafeDelete(collider);

	for (QuadTreeNode* child : children)
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
	//충돌 통과 시 실행
	if (collider->IsIntersectRay(rayPos, rayDir, dist))
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
			if (picked != NULL)
				resultNode = picked;
		}
	}

	return resultNode;
}

QuadTreeNode * QuadTreeNode::GetPickedNode(Vector3 & position)
{
	QuadTreeNode* resultNode = NULL;
	Vector3 colmax = collider->GetMaxRound();
	Vector3 colmin = collider->GetMinRound();
	//충돌 통과 시 실행

	if (colmax.x > position.x && colmin.x < position.x &&
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
}


QuadTree::~QuadTree()
{
	SafeDelete(RootNode);
}

void QuadTree::Render(Color color)
{
	RootNode->Render(color);
}

QuadTreeNode* QuadTree::GetPickedNode(Vector3& rayPos, Vector3& rayDir, OUT float& minDist)
{
	float dist = FLT_MAX;
	QuadTreeNode* result = RootNode->GetPickedNode(rayPos, rayDir, dist);
	minDist = dist;
	return result;
}

QuadTreeNode * QuadTree::GetPickedNode(Vector3 & position)
{
	return RootNode->GetPickedNode(position);
}

//
//
//
//
//QuadTreeNode::QuadTreeNode(QuadTree* tree)
//	:tree(tree)
//{
//	collider = tree->collider;
//	colInst = collider->GetSize();
//}
//
//QuadTreeNode::~QuadTreeNode()
//{
//
//	for(QuadTreeNode* child:children)
//		SafeDelete(child);
//	children.shrink_to_fit();
//}
//
//void QuadTreeNode::AddChild(QuadTreeNode* child)
//{
//	children.emplace_back(child);
//}
//
//const bool & QuadTreeNode::IsIntersect()
//{
//	// TODO: 여기에 반환 구문을 삽입합니다.
//	return collider->IsCollision( colInst);
//}
//
//QuadTreeNode* QuadTreeNode::GetPickedNode(OUT float& minDist)
//{
//	QuadTreeNode* resultNode = NULL;
//
//	float dist;
//	//충돌 통과 시 실행
//	if (collider->IsIntersectRay(dist,colInst))
//	{
//		//충돌 했는데 자식 없으면 최종 노드로 내보냄
//		if (children.size() < 1)
//		{
//			if (minDist > dist)
//			{
//				minDist = dist;
//				return this;				
//			}
//		}
//		for (QuadTreeNode* child : children)
//		{
//			QuadTreeNode* picked = child->GetPickedNode(minDist);
//			if(picked != NULL)
//				resultNode=picked;
//		}
//	}
//
//	return resultNode;
//}
//
//QuadTreeNode * QuadTreeNode::GetPickedNode(Vector3 & position)
//{
//	QuadTreeNode* resultNode = NULL;
//	Vector3 colmax = collider->GetMaxRound(colInst);
//	Vector3 colmin = collider->GetMinRound(colInst);
//	//충돌 통과 시 실행
//	
//	if (colmax.x>position.x && colmin.x < position.x &&
//		colmax.z > position.z && colmin.z < position.z)
//	{
//		//충돌 했는데 자식 없으면 최종 노드로 내보냄
//		if (children.size() < 1)
//		{
//			return this;
//		}
//		for (QuadTreeNode* child : children)
//		{
//			resultNode = child->GetPickedNode(position);
//			if (resultNode != NULL)
//				return resultNode;
//		}
//	}
//	return NULL;
//}
//
/////////////////////////////////////////////////////////////////////////////////
//
//QuadTree::QuadTree()	
//{
//	collider = new OBBCollider();
//}
//
//
//QuadTree::~QuadTree()
//{
//	SafeDelete(RootNode);
//	SafeDelete(collider);
//}
//
//void QuadTree::Update()
//{
//	collider->Update();
//}
//
//void QuadTree::Render()
//{
//	//RootNode->Render();
//	collider->SetDebugModeOn();
//	collider->Render();
//	//collider->SetDebugModeOff();
//}
//
//QuadTreeNode* QuadTree::GetPickedNode(Vector3& rayPos, Vector3& rayDir, OUT float& minDist)
//{
//	float dist = FLT_MAX;
//	collider->RayIntersect(rayPos, rayDir);
//	QuadTreeNode* result = RootNode->GetPickedNode(dist);
//	minDist = dist;
//	return result;
//}
//
//QuadTreeNode * QuadTree::GetPickedNode(Vector3 & position)
//{
//	return RootNode->GetPickedNode(position);
//}
