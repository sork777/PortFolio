#pragma once

enum class CollsionType
{
	None,
	OBB,
	Sphere,
	Capsule,
};

class Collider
{
public:
	Collider(Transform * transform=NULL, Transform * init =NULL);
	~Collider();

	virtual void Update() = 0;
	virtual void Render(Color color) = 0; //선을 그릴려면 콜

	virtual bool IsIntersect(Collider* other) = 0;
	virtual bool IsIntersectRay(Vector3& position, Vector3& direction, float& dist)= 0;

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

public:
	inline const CollsionType& GetCollisionType() { return type; }
	inline void SetUsingCollider(const bool& bUse) { bUsingCollider = bUse; }
	inline const bool& GetUsingCollider() { return bUsingCollider; }

	inline Transform * GetTransform() { return transform; }
	inline Transform * GetInit() { return init; }
	
	inline const Vector3& GetSelectPos() { return SelPos; }

protected:
	Vector3 Cross(Vector3& vec1, Vector3& vec2);

protected:
	bool bUsingCollider;
	Transform *	init;
	Transform * transform;

	Vector3 SelPos;
	CollsionType type;
};

