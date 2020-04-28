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
	virtual void Render() override; //선을 그릴려면 콜

	// 나중에 쉐이더에서 통합 계산 할거임.
	virtual bool IsIntersect(Collider* other, const UINT& inst = 0, const UINT& oinst = 0) override;
	virtual bool IsIntersectRay(Vector3& position, Vector3& direction, float& dist, const UINT& inst = 0) override;

	inline const Vector3& GetMinRound(const UINT& inst = 0) ;
	inline const Vector3& GetMaxRound(const UINT& inst = 0) ;

private:
	//void SetObb(const UINT& inst = 0);
	bool RayInterSection(Vector3& rayPos, Vector3& rayDir, Vector3& minV, Vector3& maxV, float& dist);

private:
	//위치 평면, 충돌할애 1, 2
	//bool SperatingPlane(Vector3 position, Vector3& direction, Obb& box1, Obb& box2);
	//bool CollisionOBB_OBB(Obb& box1, Obb& box2);

private:
	/* 충돌 정보 저장 데이터 */
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
	//	//Rotation 없는 경우만 사용할것.
	//	Vector3 MaxRound;
	//	Vector3 MinRound;
	//};

	//vector<ObbCol_Info> obbInfo;
private:
	Vector3 lines[8];

	// 충돌 검사용
private:
	// 월드행렬 있으면 내부분해로 obb 데이터 획득 가능
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

