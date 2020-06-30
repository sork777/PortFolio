#pragma once

#define MAX_COLLISION 65536
//
//enum class CollsionType
//{
//	None,
//	OBB,
//	Sphere,
//	Capsule,
//};
class ColliderTest
{
public:
	ColliderTest();
	//���̴� ������ ����
	ColliderTest(Shader* shader, Shader* cs);
	virtual ~ColliderTest();

	virtual void Initalize();
	virtual void Update() abstract;
	virtual void Render(const int& draw = -1);
	virtual void Property(const UINT& inst = 0) abstract;

	virtual void RayIntersect(Vector3& position, Vector3& direction) abstract;

public:
	const UINT& GetSize() { return colInfos.size(); }
	void AddInstance(Transform * transform =NULL,Transform * init=NULL);
	void DelInstance(const UINT& inst = 0);

	Transform * GetTransform(const UINT& inst = 0);
	Transform * GetInit(const UINT& inst = 0);

	void SetColliderTestOn(const UINT& inst = 0);
	void SetColliderTestOff(const UINT& inst = 0);
	const bool& IsCollisionOn(const UINT& inst = 0);
	const bool& IsCollision(const UINT& inst = 0);

public:
	inline const CollsionType& GetCollisionType() { return type; }
	inline void SetDebugModeOn()	{ bDebugMode = true; }
	inline void SetDebugModeOff()	{ bDebugMode = false; }

protected:
	struct Col_Info
	{
		bool bColliderOn;
		// �ƹ��� �����ص� ��ġ Ʈ�������� �ʼ� ����.
		Transform *	init;
		Transform * transform;
		Col_Info() {
			bColliderOn = true;
		}
	};

	vector<Col_Info*> colInfos;

	Color norColor = Color(0, 1, 0, 1);
	Color colColor = Color(1, 0, 0, 1);

protected:
	bool bDebugMode;

	CollsionType type;

public:
	virtual void ComputeColliderTest(const UINT& tech, const UINT& pass, ColliderTest * colB =NULL);

protected:
	// ColliderTest 2���� ���� �浹�ϴ� ��� I/O data�� 2���� �ʿ�.
	// ���� B�� ������ �� ����� ����

	virtual void CSColliderTestB();
	// collider �� NULL ���¸� ���� �ƴϸ� ��ȣ
	// �̰� ȣ�� �Ҷ����� ����� �ʱ�ȭ�ȴ�.
	// �浹���� �ٷ� Ȯ���ϸ� ��.
	virtual void CreateComputeBuffer();

protected:
	struct CS_InputDesc
	{
		Matrix data;
	};
	//TODO: ���� ������ �浹�� Ȯ�ι���?
	struct CS_OutputDesc
	{
		int Collision;
		Vector3 Padding;
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

