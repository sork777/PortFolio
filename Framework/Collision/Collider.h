#pragma once

#define MAX_COLLISION 65536
//#define MAX_COLLISION 1024

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
	Collider();
	virtual ~Collider();

	virtual void Initalize();
	virtual void Update();
	virtual void Render(const int& draw = -1);
	virtual void Property(const UINT& inst = 0) abstract;

	virtual void RayIntersect(Vector3& position, Vector3& direction) abstract;

public:
	const UINT& GetSize() { return colInfos.size(); }
	void AddInstance(Transform * transform =NULL,Transform * init=NULL);
	void DelInstance(const UINT& inst = 0);

	Transform * GetTransform(const UINT& inst = 0);
	Transform * GetInit(const UINT& inst = 0);

	void SetCollisionOn(const UINT& inst = 0);
	void SetCollisionOff(const UINT& inst = 0);
	const bool& IsCollisionOn(const UINT& inst = 0);
	const bool& IsCollision(const UINT& inst = 0);

public:
	const CollsionType& GetCollisionType() { return type; }
	void SetDebugMode(const bool& bDebug)	{ bDebugMode = bDebug; }
	void SetDefferedMode(const bool& bDeffered)	{ bDefferedMode = bDeffered; }

	void SetFrustum(Frustum* frustum) { this->frustum = frustum; }
protected:
	struct Col_Info
	{
		bool bCollisionOn;
		// 아무리 생각해도 위치 트랜스폼은 필수 같음.
		Transform * transform;
		Col_Info() {
			bCollisionOn = true;
		}
	};

	vector<Col_Info*> colInfos;

	Color norColor = Color(0, 1, 0, 1);
	Color colColor = Color(1, 0, 0, 1);

protected:
	bool bDebugMode;
	bool bDefferedMode;
	bool bChangeCount;

	CollsionType type;
	Frustum* frustum =NULL;

public:
	// collider 가 NULL 상태면 셀프 아니면 상호
	// 이거 호출 할때마다 결과값 초기화된다.
	// 충돌여부 바로 확인하면 됨.
	void ComputeColliderTest(const UINT& tech, const UINT& pass, Collider * colB =NULL);

protected:
	// Collider 2개가 서로 충돌하는 경우 I/O data가 2개씩 필요.
	// 그중 B로 지정될 애 계산할 영역
	void CSColliderTestB();

	void CreateComputeBuffer();

protected:
	struct CS_InputDesc
	{
		Matrix data;
	};
	//TODO: 상대방 여러개 충돌시 확인법은?
	struct CS_OutputDesc
	{
		int Collision = 0;
		float Dist = FLT_MAX;			// 마우스레이에서 가장 가까운 값이나 객체간의 거리.
		int ClosestNum = -1;
		int Frustum = 0;
	};
	CS_InputDesc*	csInput = NULL;
	CS_OutputDesc*	csOutput = NULL;

	Shader* csShader;
	StructuredBuffer* computeBuffer = NULL;

///////////////////////////////////////////////////////////////////////////////
// DebugLine 용
protected:
	void RenderLine(Vector3& start, Vector3& end);
	void RenderLine(Vector3& start, Vector3& end, float r, float g, float b);

	void RenderLine(float x, float y, float z, float x2, float y2, float z2);
	void RenderLine(float x, float y, float z, float x2, float y2, float z2, Color& color);
	void RenderLine(float x, float y, float z, float x2, float y2, float z2, float r, float g, float b);

	void RenderLine(Vector3& start, Vector3& end, Color& color);

private:
	Shader* shader;
	// 형태에 따른 인스턴스 값임. 콜라이더 자체는 각자로.
	UINT drawCount;

	PerFrame* perFrame;
	Transform* ctransform;
	
	UINT vCount;
	VertexBuffer* vertexBuffer;
	VertexColor* vertices;
protected:
	UINT lineCount;
};

