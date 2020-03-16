#pragma once

class CapsuleCollider :	public Collider
{
public:
	CapsuleCollider();
	~CapsuleCollider();

	// Collider��(��) ���� ��ӵ�
	virtual void Update() override;
	virtual void Render(Color color) override;
	virtual bool IsIntersect(Collider * other) override;
	virtual bool IsIntersectRay(Vector3 & position, Vector3 & direction, float & dist) override;
};

