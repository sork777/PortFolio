#pragma once

class OBBCollider : public Collider
{
private:
	struct Obb;
public:
	OBBCollider(Transform * transform = NULL, Transform * init = NULL);
	~OBBCollider();

	virtual void Update() override;
	virtual void Render(Color color) override; //���� �׸����� ��

	// ���߿� ���̴����� ���� ��� �Ұ���.
	virtual bool IsIntersect(Collider* other) override;
	virtual bool IsIntersectRay(Vector3& position, Vector3& direction, float& dist) override;

	inline const Vector3& GetMinRound() { return MinRound; }
	inline const Vector3& GetMaxRound() { return MaxRound; }

private:
	void SetObb();
	bool RayInterSection(Vector3& rayPos, Vector3& rayDir, Vector3& minV, Vector3& maxV, float& dist);

private:
	//��ġ ���, �浹�Ҿ� 1, 2
	bool SperatingPlane(Vector3 position, Vector3& direction, Obb& box1, Obb& box2);
	bool CollisionOBB_OBB(Obb& box1, Obb& box2);

private:
	/* �浹 ���� ���� ������ */
	struct Obb
	{
		Vector3 Position;

		Vector3 AxisX;
		Vector3 AxisY;
		Vector3 AxisZ;

		Vector3 HalfSize;
	} obb;
private:
	Vector3 lines[8];

	Vector3 MaxRound, MinRound;	//Rotation ���� ��츸 ����Ұ�.
};
//#pragma once
//
//class OBBCollider : public Collider
//{
//private:
//	struct Obb;
//public:
//	OBBCollider();
//	OBBCollider(Shader* shader, Shader* cs);
//	~OBBCollider();
//
//	virtual void Initalize() override;
//	virtual void Update() override;
//	virtual void Render(const int& draw = -1) override; //���� �׸����� ��
//	virtual void Property(const UINT& inst = 0) override;
//
//	// ���߿� ���̴����� ���� ��� �Ұ���.
//	virtual void RayIntersect(Vector3& position, Vector3& direction) override;
//
//	const Vector3& GetMinRound(const UINT& inst = 0) ;
//	const Vector3& GetMaxRound(const UINT& inst = 0) ;
//
//	const bool& IsIntersectRay(OUT float& dist, const UINT& inst = 0);
//
//private:
//	//cube�� ����
//	Vector3 lines[8];
//
//	// �浹 �˻��
//private:
//	struct RayDesc
//	{
//		Vector3 position;
//		float padding;
//		Vector3 direction;
//		float padding2;
//	}rayDesc;
//
//	ConstantBuffer* rayCB = NULL;
//private:
//	// ������� ������ ���κ��ط� obb ������ ȹ�� ����
//	
//	struct CS_ObbOutputDesc
//	{
//		int Intersect;
//		Vector3 MaxRound;
//		Vector3 MinRound;
//		float Dist;
//	};
//	CS_ObbOutputDesc* csObbOutput = NULL;
//	StructuredBuffer* obbCSBuffer = NULL;
//	
//private:
//	virtual void CreateComputeBuffer() override;
//
//};
//
