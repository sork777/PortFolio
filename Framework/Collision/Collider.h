#pragma once

#define MAX_COLLISION 500

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
	virtual void Update() = 0;
	virtual void Render();

	virtual bool IsIntersect(Collider* other, const UINT& inst = 0, const UINT& oinst = 0) = 0;
	virtual bool IsIntersectRay(Vector3& position, Vector3& direction, float& dist, const UINT& inst = 0)= 0;

public:
	virtual void AddInstance(Transform * transform = NULL, Transform * init = NULL);
	virtual void DelInstance(const UINT& inst = 0);

	void SetColliderOn(const UINT& inst = 0);
	void SetColliderOff(const UINT& inst = 0);
	Transform * GetTransform(const UINT& inst = 0);
	Transform * GetInit(const UINT& inst = 0);
	
public:
	inline const CollsionType& GetCollisionType() { return type; }
	inline void SetDebugModeOn()	{ bDebugMode = true; }
	inline void SetDebugModeOff()	{ bDebugMode = false; }
	inline const Vector3& GetSelectPos() { return SelPos; }

	const UINT& GetSize() { return cmColInfos.size(); }

protected:
	struct CmCol_Info
	{
		bool bColliderOn;
		Transform *	init;
		Transform * transform;
	};
	Color norColor = Color(0, 1, 0, 1);
	Color colColor = Color(1, 0, 0, 1);

	vector<CmCol_Info> cmColInfos;

public:
	CmCol_Info& GetCmInfo(const UINT& inst = 0) { if (inst < cmColInfos.size())	return cmColInfos[inst]; }

protected:
	bool bDebugMode;

	//인스턴스 중에서 하나 찍히면 SelPos는 알아서 찍힐 거라 봄
	Vector3 SelPos;
	CollsionType type;

private:
	Shader* csShader;

///////////////////////////////////////////////////////////////////////////////
// DebugLine 용
public:
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
	VertexBuffer* vertexBuffer;
	VertexColor* vertices;

protected:
	UINT lineCount;
};

