#pragma once


class QuadTreeNode
{
public:
	QuadTreeNode();
	virtual ~QuadTreeNode();

	void Render(Color color);
	void SetParent(QuadTreeNode* parent) { Parent = parent; }
	QuadTreeNode* GetParent() { return Parent; }
	bool HasChilds() { return !children.empty(); }

	void SetCollider(OBBCollider * collider);
	OBBCollider* GetCollider() { return collider; }
	void AddChild(QuadTreeNode* child);
	vector< QuadTreeNode*>& GetChildren() { return children; }
	QuadTreeNode* GetPickedNode(Vector3& rayPos, Vector3& rayDir, float& minDist);
	QuadTreeNode* GetPickedNode(Vector3& position);

private:
	OBBCollider* collider;	//픽킹 충돌처리용
	Vector2 BoundsY;	//터레인 높이 측정용

	QuadTreeNode* Parent;
	vector< QuadTreeNode*> children;
};
///////////////////////////////////////////////////////////////////////////////

class QuadTree
{
	friend QuadTreeNode;
public:
	QuadTree(QuadTreeNode* Root);
	virtual ~QuadTree();
	
	void Render(Color color);
	QuadTreeNode* GetPickedNode(Vector3& rayPos, Vector3& rayDir);
	QuadTreeNode* GetPickedNode(Vector3& position);
public:
	QuadTreeNode* RootNode;
};

