#pragma once
class Collider
{
private:
	struct Obb;
public:
	Collider(Transform * transform=NULL, Transform * init =NULL);
	~Collider();

	void Update();
	void Render(Color color); //선을 그릴려면 콜

	void SetUsingCollider(const bool& bUse) { bUsingCollider = bUse; }
	const bool& GetUsingCollider() { return bUsingCollider; }

	bool IsIntersect(Collider* other);
	bool IsIntersect(Vector3& position, Vector3& direction, float& dist);

	Transform * GetTransform() { return transform; }
	Transform * GetInit() { return init; }
	Vector3& GetMinRound() { return MinRound; }
	Vector3& GetMaxRound() { return MaxRound; }
	Vector3& GetSelectPos() { return SelPos; }
	
	void ChangeTrans(Transform * transform) 
	{
		this->transform = transform; 
		Update();
	}
	void ChangeInit(Transform * init) 
	{ 
		this->init = init;
		Update();
	}
private:
	void SetObb();
	bool RayInterSection(Vector3& rayPos, Vector3& rayDir, Vector3& minV, Vector3& maxV, float& dist);
private:
	//위치 평면, 충돌할애 1, 2
	bool SperatingPlane(Vector3 position, Vector3& direction, Obb& box1, Obb& box2);
	bool Collision(Obb& box1, Obb& box2);
	Vector3 Cross(Vector3& vec1, Vector3& vec2);
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
	bool bUsingCollider;
	Transform *	init;
	Transform * transform;

	Vector3 lines[8];

	Vector3 MaxRound, MinRound,SelPos;	//Rotation 없는 경우만 사용할것.
};

