#pragma once

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
	virtual bool Property(const UINT & instance) override;
	virtual Transform * GetTransform(const UINT & instance = 0) override;

	virtual void AddInstanceData() override;
	virtual void DelInstanceData(const UINT& instance) override;

private:
	vector<Collider*> colliders;
};

