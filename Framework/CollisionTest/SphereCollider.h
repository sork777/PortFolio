#pragma once

class SphereCollider :	public Collider
{
public:
	SphereCollider();
	SphereCollider(Shader * shader, Shader * cs);
	~SphereCollider();

	// Collider을(를) 통해 상속됨
	virtual void Update() override;
	virtual void Render(const int& draw = -1) override;
	virtual void Property(const UINT& inst = 0) override;

	virtual void RayIntersect(Vector3& position, Vector3& direction) override;

private:
	virtual void CreateComputeBuffer() override;
};

