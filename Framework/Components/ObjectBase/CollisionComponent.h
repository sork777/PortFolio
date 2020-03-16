#pragma once

/*
	충돌 박스 갖고 있을것.
	형태별로 나누는게 맞나...?
*/

class CollisionComponent :public ObjectBaseComponent
{
public:
	CollisionComponent();
	~CollisionComponent();

	// ObjectBaseComponent을(를) 통해 상속됨
	virtual void Update() override;
	virtual void Render() override;
	virtual bool Property(const UINT & instance) override;
	virtual Transform * GetTransform(const UINT & instance = 0) override;

	virtual void AddInstanceData() override;
	virtual void DelInstanceData(const UINT& instance) override;

private:
	vector<Collider*> colliders;
};

