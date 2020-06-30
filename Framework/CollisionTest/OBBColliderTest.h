#pragma once


class OBBColliderTest : public ColliderTest
{
private:
	struct Obb;
public:
	OBBColliderTest();
	OBBColliderTest(Shader* shader, Shader* cs);
	~OBBColliderTest();

	virtual void Initalize() override;
	virtual void Update() override;
	virtual void Render(const int& draw = -1) override; //선을 그릴려면 콜
	virtual void Property(const UINT& inst = 0) override;

	// 나중에 쉐이더에서 통합 계산 할거임.
	virtual void RayIntersect(Vector3& position, Vector3& direction) override;

	const Vector3& GetMinRound(const UINT& inst = 0) ;
	const Vector3& GetMaxRound(const UINT& inst = 0) ;

	const bool& IsIntersectRay(OUT float& dist, const UINT& inst = 0);

private:
	//cube의 라인
	Vector3 lines[8];

	// 충돌 검사용
private:
	struct RayDesc
	{
		Vector3 position;
		float padding;
		Vector3 direction;
		float padding2;
	}rayDesc;

	ConstantBuffer* rayCB = NULL;
private:
	// 월드행렬 있으면 내부분해로 obb 데이터 획득 가능
	
	struct CS_ObbOutputDesc
	{
		int Intersect;
		Vector3 MaxRound;
		Vector3 MinRound;
		float Dist;
	};
	CS_ObbOutputDesc* csObbOutput = NULL;
	StructuredBuffer* obbCSBuffer = NULL;
	
private:
	virtual void CreateComputeBuffer() override;

};

