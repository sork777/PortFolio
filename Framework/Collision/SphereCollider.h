#pragma once

class SphereCollider :	public Collider
{
public:
	SphereCollider();
	~SphereCollider();

	// Collider을(를) 통해 상속됨
	virtual void Update() override;
	virtual void Render(Color color) override;
	virtual bool IsIntersect(Collider * other) override;
	virtual bool IsIntersectRay(Vector3 & position, Vector3 & direction, float & dist) override;
};

