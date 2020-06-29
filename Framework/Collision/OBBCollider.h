#pragma once

class OBBCollider : public Collider
{
private:
	struct Obb;
public:
	OBBCollider(Transform * transform = NULL, Transform * init = NULL);
	~OBBCollider();

	virtual void Update() override;
	virtual void Render(Color color) override; //선을 그릴려면 콜

	// 나중에 쉐이더에서 통합 계산 할거임.
	virtual bool IsIntersect(Collider* other) override;
	virtual bool IsIntersectRay(Vector3& position, Vector3& direction, float& dist) override;

	inline const Vector3& GetMinRound() { return MinRound; }
	inline const Vector3& GetMaxRound() { return MaxRound; }

private:
	void SetObb();
	bool RayInterSection(Vector3& rayPos, Vector3& rayDir, Vector3& minV, Vector3& maxV, float& dist);

private:
	//위치 평면, 충돌할애 1, 2
	bool SperatingPlane(Vector3 position, Vector3& direction, Obb& box1, Obb& box2);
	bool CollisionOBB_OBB(Obb& box1, Obb& box2);

private:
	/* 충돌 정보 저장 데이터 */
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

	Vector3 MaxRound, MinRound;	//Rotation 없는 경우만 사용할것.
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
//	virtual void Render(const int& draw = -1) override; //선을 그릴려면 콜
//	virtual void Property(const UINT& inst = 0) override;
//
//	// 나중에 쉐이더에서 통합 계산 할거임.
//	virtual void RayIntersect(Vector3& position, Vector3& direction) override;
//
//	const Vector3& GetMinRound(const UINT& inst = 0) ;
//	const Vector3& GetMaxRound(const UINT& inst = 0) ;
//
//	const bool& IsIntersectRay(OUT float& dist, const UINT& inst = 0);
//
//private:
//	//cube의 라인
//	Vector3 lines[8];
//
//	// 충돌 검사용
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
//	// 월드행렬 있으면 내부분해로 obb 데이터 획득 가능
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
