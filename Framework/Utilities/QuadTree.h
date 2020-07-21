#pragma once

class QuadTree;

class QuadTreeNode
{
public:
	QuadTreeNode(QuadTree* tree);
	virtual ~QuadTreeNode();

	//void Render();
	void SetParent(QuadTreeNode* parent) { Parent = parent; }
	QuadTreeNode* GetParent() { return Parent; }
	const bool& HasChilds() { return !children.empty(); }

	const UINT& GetColInst() { return colInst; }

	void AddChild(QuadTreeNode* child);
	vector< QuadTreeNode*>& GetChildren() { return children; }

	const bool& IsIntersect();

	QuadTreeNode* GetPickedNode(OUT float& minDist);
	QuadTreeNode* GetPickedNode(Vector3& position);

private:
	OBBCollider* collider;	//픽킹 충돌처리용, Tree에서 연결
	UINT colInst;
	Vector2 BoundsY;	//터레인 높이 측정용

	QuadTreeNode* Parent;
	vector< QuadTreeNode*> children;

	QuadTree* tree;
};
///////////////////////////////////////////////////////////////////////////////

class QuadTree
{
	friend QuadTreeNode;
public:
	QuadTree();
	virtual ~QuadTree();
	
	void Update();
	void Render();

	void SetRootNode(QuadTreeNode* Root);
	QuadTreeNode* GetPickedNode(Vector3& rayPos, Vector3& rayDir, OUT float& minDist);
	QuadTreeNode* GetPickedNode(Vector3& position);

	QuadTreeNode* GetRootNode() { return RootNode; }

	OBBCollider* GetCollider() { return collider; }

private:
	QuadTreeNode* RootNode;
	OBBCollider* collider;
};

