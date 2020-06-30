#pragma once

class CapsuleCollider :	public Collider
{
public:
	CapsuleCollider();
	CapsuleCollider(Shader * shader, Shader * cs);
	~CapsuleCollider();

	// Collider을(를) 통해 상속됨
	virtual void Update() override;
	virtual void Render(const int& draw = -1) override;
	virtual void Property(const UINT& inst = 0) override;

	virtual void RayIntersect(Vector3& position, Vector3& direction) override;

private:
	virtual void CreateComputeBuffer() override;

};

