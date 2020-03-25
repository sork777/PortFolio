#pragma once

class Collider;
/*
	�浹 �ڽ� ���� ������.
	���º��� �����°� �³�...?
*/

class CollisionComponent :public ObjectBaseComponent
{
public:
	CollisionComponent();
	~CollisionComponent();

	// ObjectBaseComponent��(��) ���� ��ӵ�
	virtual void Update() override;
	virtual void Render() override;
	virtual bool Property() override;
	virtual Transform * GetTransform(const UINT & instance = 0) override;

	virtual void AddInstanceData() override;
	virtual void DelInstanceData(const UINT& instance) override;

//	inline virtual const UINT& GetInstSize() override { return colliders.size(); }
private:
	vector<Collider*> colliders;
};

