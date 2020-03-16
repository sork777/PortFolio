#pragma once

class Mesh
{
public:
	typedef VertexTextureNormalTangent MeshVertex;

public:
	Mesh();
	virtual ~Mesh();

	void Update();
	void Render(const UINT& drawCount);

	void SetShader(Shader* shader);

public:
	inline Shader* GetShader() { return shader; }
	inline void Pass(const UINT& val) { pass = val; }
	inline void Tech(const UINT& val) { tech = val; }

	inline void Topology(D3D11_PRIMITIVE_TOPOLOGY val) { topology = val; }
	inline void Tessellation(const bool& bTess) { this->bTess = bTess; }
	inline void Displacement(const float& dist) { displacement = dist < 0.0f ? 0.0f : dist; }

protected:
	virtual void Create() = 0;

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
	IndexBuffer* indexBuffer = NULL;
	IndexBuffer* indexTessBuffer = NULL;

	MeshVertex* vertices;
	UINT* indices;
	UINT* TessIndices;

	UINT vertexCount, indexCount,tessICount;
};