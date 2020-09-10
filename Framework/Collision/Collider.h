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
		// �ƹ��� �����ص� ��ġ Ʈ�������� �ʼ� ����.
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
	// collider �� NULL ���¸� ���� �ƴϸ� ��ȣ
	// �̰� ȣ�� �Ҷ����� ����� �ʱ�ȭ�ȴ�.
	// �浹���� �ٷ� Ȯ���ϸ� ��.
	void ComputeColliderTest(const UINT& tech, const UINT& pass, Collider * colB =NULL);

protected:
	// Collider 2���� ���� �浹�ϴ� ��� I/O data�� 2���� �ʿ�.
	// ���� B�� ������ �� ����� ����
	void CSColliderTestB();

	void CreateComputeBuffer();

protected:
	struct CS_InputDesc
	{
		Matrix data;
	};
	//TODO: ���� ������ �浹�� Ȯ�ι���?
	struct CS_OutputDesc
	{
		int Collision = 0;
		float Dist = FLT_MAX;			// ���콺���̿��� ���� ����� ���̳� ��ü���� �Ÿ�.
		int ClosestNum = -1;
		int Frustum = 0;
	};
	CS_InputDesc*	csInput = NULL;
	CS_OutputDesc*	csOutput = NULL;

	Shader* csShader;
	StructuredBuffer* computeBuffer = NULL;

///////////////////////////////////////////////////////////////////////////////
// DebugLine ��
protected:
	void RenderLine(Vector3& start, Vector3& end);
	void RenderLine(Vector3& start, Vector3& end, float r, float g, float b);

	void RenderLine(float x, float y, float z, float x2, float y2, float z2);
	void RenderLine(float x, float y, float z, float x2, float y2, float z2, Color& color);
	void RenderLine(float x, float y, float z, float x2, float y2, float z2, float r, float g, float b);

	void RenderLine(Vector3& start, Vector3& end, Color& color);

private:
	Shader* shader;
	// ���¿� ���� �ν��Ͻ� ����. �ݶ��̴� ��ü�� ���ڷ�.
	UINT drawCount;

	PerFrame* perFrame;
	Transform* ctransform;
	
	UINT vCount;
	VertexBuffer* vertexBuffer;
	VertexColor* vertices;
protected:
	UINT lineCount;
};

