#pragma once

class OBBCollider : public Collider
{
private:
	struct Obb;
public:
	OBBCollider();
	~OBBCollider();

	virtual void Initalize() override;
	virtual void Update() override;
	virtual void Render() override; //���� �׸����� ��

	// ���߿� ���̴����� ���� ��� �Ұ���.
	virtual bool IsIntersect(Collider* other, const UINT& inst = 0, const UINT& oinst = 0) override;
	virtual bool IsIntersectRay(Vector3& position, Vector3& direction, float& dist, const UINT& inst = 0) override;

	inline const Vector3& GetMinRound(const UINT& inst = 0) ;
	inline const Vector3& GetMaxRound(const UINT& inst = 0) ;

private:
	//void SetObb(const UINT& inst = 0);
	bool RayInterSection(Vector3& rayPos, Vector3& rayDir, Vector3& minV, Vector3& maxV, float& dist);

private:
	//��ġ ���, �浹�Ҿ� 1, 2
	//bool SperatingPlane(Vector3 position, Vector3& direction, Obb& box1, Obb& box2);
	//bool CollisionOBB_OBB(Obb& box1, Obb& box2);

private:
	/* �浹 ���� ���� ������ */
	//struct Obb
	//{
	//	Vector3 Position;

	//	Vector3 AxisX;
	//	Vector3 AxisY;
	//	Vector3 AxisZ;

	//	Vector3 HalfSize;
	//};

	//struct ObbCol_Info
	//{
	//	Obb obb;
	//	//Rotation ���� ��츸 ����Ұ�.
	//	Vector3 MaxRound;
	//	Vector3 MinRound;
	//};

	//vector<ObbCol_Info> obbInfo;
private:
	Vector3 lines[8];

	// �浹 �˻��
private:
	// ������� ������ ���κ��ط� obb ������ ȹ�� ����
	struct CS_ObbInputDesc
	{
		Matrix data;
	};
	struct CS_ObbOutputDesc
	{
		int bCollsion;
		Vector3 MaxRound;
		Vector3 MinRound;
		float dist;
	};
	CS_ObbInputDesc*	csInput = NULL;
	CS_ObbOutputDesc*	csOutput = NULL;

private:
	void ComputeCollider();
	void CreateComputeBuffer();

private:
	StructuredBuffer* computeBuffer;
	ConstantBuffer* csConstBuffer;
};

