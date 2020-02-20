#pragma once

class Mesh
{
public:
	typedef VertexTextureNormalTangent MeshVertex;

public:
	Mesh();
	virtual ~Mesh();

	void SetShader(Shader* shader);
	void Pass(UINT val) { pass = val; }
	void Tech(UINT val) { tech = val; }

	void Update();
	void Render(UINT drawCount);

	void Topology(D3D11_PRIMITIVE_TOPOLOGY val) { topology = val; }

	void Tessellation(const bool& bTess) { this->bTess = bTess; }
	void Displacement(const float& dist) { displacement = dist < 0.0f ? 0.0f : dist; }
protected:
	virtual void Create() = 0;
	void CreateTess();

private:
	bool bTess = false;
	float displacement = 0.2f;

protected:
	Shader* shader;
	D3D11_PRIMITIVE_TOPOLOGY topology;
	UINT pass = 0;
	UINT tech = 0;

	PerFrame* perFrame = NULL;

	VertexBuffer* vertexBuffer = NULL;
	//VertexBuffer* vertexTesBuffer = NULL;
	IndexBuffer* indexBuffer = NULL;
	IndexBuffer* indexTessBuffer = NULL;

	MeshVertex* vertices;
	//MeshVertex* tessVertices;
	UINT* indices;
	UINT* TessIndices;

	UINT vertexCount, indexCount,tessICount;
};