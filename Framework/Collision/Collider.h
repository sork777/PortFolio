#pragma once

#define MAX_COLLISION 65536

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
	Collider(Transform * transform = NULL, Transform * init = NULL);
	~Collider();

	virtual void Update() = 0;
	virtual void Render(Color color) = 0; //

	virtual bool IsIntersect(Collider* other) = 0;
	virtual bool IsIntersectRay(Vector3& position, Vector3& direction, float& dist) = 0;

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
	bool bUsingCollider;
	Transform *	init;
	Transform * transform;

	Vector3 SelPos;
	CollsionType type;
};

//TODO: CS계산 재도전 필요.

//class Collider
//{
//public:
//	Collider();
//	//쉐이더 공유를 위함
//	Collider(Shader* shader, Shader* cs);
//	virtual ~Collider();
//
//	virtual void Initalize();
//	virtual void Update() abstract;
//	virtual void Render(const int& draw = -1);
//	virtual void Property(const UINT& inst = 0) abstract;
//
//	virtual void RayIntersect(Vector3& position, Vector3& direction) abstract;
//
//public:
//	const UINT& GetSize() { return colInfos.size(); }
//	void AddInstance(Transform * transform =NULL,Transform * init=NULL);
//	void DelInstance(const UINT& inst = 0);
//
//	Transform * GetTransform(const UINT& inst = 0);
//	Transform * GetInit(const UINT& inst = 0);
//
//	void SetColliderOn(const UINT& inst = 0);
//	void SetColliderOff(const UINT& inst = 0);
//	const bool& IsCollisionOn(const UINT& inst = 0);
//	const bool& IsCollision(const UINT& inst = 0);
//
//public:
//	inline const CollsionType& GetCollisionType() { return type; }
//	inline void SetDebugModeOn()	{ bDebugMode = true; }
//	inline void SetDebugModeOff()	{ bDebugMode = false; }
//
//protected:
//	struct Col_Info
//	{
//		bool bColliderOn;
//		// 아무리 생각해도 위치 트랜스폼은 필수 같음.
//		Transform *	init;
//		Transform * transform;
//		Col_Info() {
//			bColliderOn = true;
//		}
//	};
//
//	vector<Col_Info*> colInfos;
//
//	Color norColor = Color(0, 1, 0, 1);
//	Color colColor = Color(1, 0, 0, 1);
//
//protected:
//	bool bDebugMode;
//
//	CollsionType type;
//
//public:
//	virtual void ComputeCollider(const UINT& tech, const UINT& pass, Collider * colB =NULL);
//
//protected:
//	// Collider 2개가 서로 충돌하는 경우 I/O data가 2개씩 필요.
//	// 그중 B로 지정될 애 계산할 영역
//
//	virtual void CSColliderB();
//	// collider 가 NULL 상태면 셀프 아니면 상호
//	// 이거 호출 할때마다 결과값 초기화된다.
//	// 충돌여부 바로 확인하면 됨.
//	virtual void CreateComputeBuffer();
//
//protected:
//	struct CS_InputDesc
//	{
//		Matrix data;
//	};
//	//TODO: 상대방 여러개 충돌시 확인법은?
//	struct CS_OutputDesc
//	{
//		int Collision;
//		Vector3 Padding;
//	};
//	CS_InputDesc*	csInput = NULL;
//	CS_OutputDesc*	csOutput = NULL;
//
//	Shader* csShader;
//	StructuredBuffer* computeBuffer = NULL;
//
/////////////////////////////////////////////////////////////////////////////////
//// DebugLine 용
//protected:
//	void RenderLine(Vector3& start, Vector3& end);
//	void RenderLine(Vector3& start, Vector3& end, float r, float g, float b);
//
//	void RenderLine(float x, float y, float z, float x2, float y2, float z2);
//	void RenderLine(float x, float y, float z, float x2, float y2, float z2, Color& color);
//	void RenderLine(float x, float y, float z, float x2, float y2, float z2, float r, float g, float b);
//
//	void RenderLine(Vector3& start, Vector3& end, Color& color);
//
//private:
//	Shader* shader;
//	// 형태에 따른 인스턴스 값임. 콜라이더 자체는 각자로.
//	UINT drawCount;
//
//	PerFrame* perFrame;
//	Transform* ctransform;
//	
//	UINT vCount;
//	VertexBuffer* vertexBuffer;
//	VertexColor* vertices;
//protected:
//	UINT lineCount;
//};
//
