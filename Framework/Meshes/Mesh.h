#pragma once

enum class MeshType
{
	None,
	Cylinder,
	Cube,
	Grid,
	Quad,
	Sphere
};

class Mesh
{
public:
	typedef VertexTextureNormalTangent MeshVertex;

public:
	Mesh();
	Mesh(const Mesh& mesh);
	virtual ~Mesh();

	void Update();
	void Render(const UINT& drawCount);

	void SetShader(Shader* shader);
	const MeshType& GetMeshType() const { return type; }

public:
	inline Shader* GetShader() { return shader; }
	inline void Pass(const UINT& val) { pass = val; }
	inline void Tech(const UINT& val) { tech = val; }

	inline void Topology(D3D11_PRIMITIVE_TOPOLOGY val) { topology = val; }

protected:
	virtual void Create() = 0;


protected:
	Shader* shader;
	MeshType type = MeshType::None;
	D3D11_PRIMITIVE_TOPOLOGY topology;
	UINT pass = 0;
	UINT tech = 0;

	PerFrame* perFrame = NULL;

	VertexBuffer* vertexBuffer = NULL;
	IndexBuffer* indexBuffer = NULL;

	MeshVertex* vertices;
	UINT* indices;

	UINT vertexCount, indexCount;
};