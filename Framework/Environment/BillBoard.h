#pragma once
#define MAX_INSTANCE 4096

class BillBoard :public Renderer
{
public:
	BillBoard(Shader* shader, vector<wstring>& textureNames);
	~BillBoard();

	void FixedY(bool val) { fixedY = val; }
	void Add(Vector3& position, Vector2& scale, UINT texNum);

	Vector3 GetPositionFromIdx(UINT index) { return vertices[index].Position; }
	Vector2 GetScaleFromIdx(UINT index) { return vertices[index].Scale; }
	//UINT GetTexNumFromIdx(UINT index) { return vertices[index].TexNum; }

	UINT GetSize() { return vertices.size(); }
	void DeleteVertex(UINT index) { vertices.erase(vertices.begin() + index); }
	void ResizeBuffer();
	void Clear();

	void Update();
	void Render();
private:
	struct VertexScale
	{
		Vector3 Position;
		Vector2 Scale;
	};
private:
	bool fixedY;

	vector<VertexScale> vertices;

	TextureArray* textures;
//////////////////////////////////////////////////////////////////////////////

/* �ν��Ͻ� */
public:
	UINT Push();
	/* �ش� ��ġ�� ���� ���� �Լ� */
	void Pop(UINT index);	
	Transform* GetInstTransform(UINT index);
	UINT GetInstTexNum(UINT index);
	UINT GetCount() { return drawCount; }
	void SetInstTex(UINT index, UINT texNum);
private:
	UINT drawCount;

	Transform* transforms[MAX_INSTANCE];

	//Instancing�� ������
	struct InstDesc
	{
		Matrix world;
		UINT TexNum;
	};
	InstDesc instDesc[MAX_INSTANCE];
	//������ �ִ� ����
	VertexBuffer* instanceBuffer;	//�ν��Ͻ̿� ����

	bool bInstancing;
//////////////////////////////////////////////////////////////////////////////
};

