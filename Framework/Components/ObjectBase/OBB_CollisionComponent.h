#pragma once

class Collider;
class OBBCollider;

class OBB_CollisionComponent :public ObjectBaseComponent
{
public:
	OBB_CollisionComponent();
	OBB_CollisionComponent(const OBB_CollisionComponent& obbComp);
	~OBB_CollisionComponent();

	// ObjectBaseComponent을(를) 통해 상속됨
	virtual void Update() override;
	virtual void Render() override;
	virtual bool Property(const UINT& instance = 0) override;


	virtual void AddInstanceData() override;
	virtual void DelInstanceData(const UINT& instance) override;
	virtual const UINT& GetInstSize() override;

	virtual Transform * GetTransform(const UINT & instance = 0) override;

private:
	//OBBCollider* colliders;
	vector<OBBCollider*> colliders;

	Color lineColor = Color(0, 1, 0, 1);
};

