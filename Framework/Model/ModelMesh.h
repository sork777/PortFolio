#pragma once

class ModelBone
{
public:
	friend class Model;

private:
	ModelBone();
	~ModelBone();

public:
	const int& Index() { return index; }
	const int& ParentIndex() { return parentIndex; }

	wstring Name() { return name; }

	Matrix& Transform() { return transform; }
	void Transform(Matrix& matrix) { transform = matrix; }

	ModelBone* Parent() { return parent; }
	vector<ModelBone *>& Childs() { return childs; }

private:
	int index;
	wstring name;

	int parentIndex;
	ModelBone* parent;

	Matrix transform;
	vector<ModelBone *> childs;
};

///////////////////////////////////////////////////////////////////////////////

class ModelMesh
{
public:
	friend class Model;

private:
	ModelMesh();
	~ModelMesh();

	void Binding(Model* model);
public:
	void Pass(UINT val) { pass = val; }
	void Tech(UINT val) { tech = val; }
	void SetShader(Shader* shader);
	
	void Update();
	void Render(UINT drawCount);

	wstring Name() { return name; }

	int BoneIndex() { return boneIndex; }
	class ModelBone* Bone() { return bone; }


	void TransformsSRV(ID3D11ShaderResourceView* srv);

	UINT GetVertexCount() { return vertexCount; }
	Model::ModelVertex* GetVertices() { return vertices; }

private:

	struct BoneDesc
	{
		UINT Index;
		float Padding[3];
	} boneDesc;

private:
	wstring name;

	Shader* shader =NULL;
	UINT tech = 0;
	UINT pass = 0;

	PerFrame* perFrame = NULL;


	wstring materialName = L"";
	Material* material = NULL;

	int boneIndex;
	class ModelBone* bone;

	VertexBuffer* vertexBuffer;
	UINT vertexCount;
	Model::ModelVertex* vertices;

	IndexBuffer* indexBuffer;
	UINT indexCount;
	UINT* indices;

	ConstantBuffer* boneBuffer;
	ID3DX11EffectConstantBuffer* sBoneBuffer;

	ID3D11ShaderResourceView* transformsSRV = NULL;
	ID3DX11EffectShaderResourceVariable* sTransformsSRV;
};