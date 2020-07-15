#pragma once


class OBBColliderTest : public ColliderTest
{
private:
	struct Obb;
public:
	OBBColliderTest();
	~OBBColliderTest();

	virtual void Initalize() override;
	virtual void Update() override;
	virtual void Render(const int& draw = -1) override; //���� �׸����� ��
	virtual void Property(const UINT& inst = 0) override;

	// ���߿� ���̴����� ���� ��� �Ұ���.
	virtual void RayIntersect(Vector3& position = Vector3(0,0,0), Vector3& direction = Vector3(0, 0, 0)) override;

	const Vector3& GetMinRound(const UINT& inst = 0) ;
	const Vector3& GetMaxRound(const UINT& inst = 0) ;

	const bool& IsIntersectRay(OUT float& dist, const UINT& inst = 0);

private:
	//cube�� ����
	Vector3 lines[8];

	// �浹 �˻��
private:
	struct RayDesc
	{
		Vector3 position;
		float padding;
		Vector3 direction;
		float padding2;
	}rayDesc;

	ConstantBuffer* rayCB = NULL;

};

