#pragma once

class PerFrame;
class Transform;

class Renderer
{
public:
	Renderer(Shader* shader);
	Renderer(wstring shaderFile);

	virtual ~Renderer();

	Shader* GetShader() { return shader; }

	UINT& Pass() { return pass; }
	UINT& Tech() { return tech; }
	void Pass(UINT val) { pass = val; }
	void Tech(UINT val) { tech = val; }

	virtual void Update();
	virtual void Render();

	void SetShader(Shader* shader);
	Transform* GetTransform() { return transform; }

private:
	void Initialize();

protected:
	void Topology(D3D11_PRIMITIVE_TOPOLOGY val) { topology = val; }

protected:
	Shader* shader;
	Transform * transform;

	VertexBuffer* vertexBuffer;
	IndexBuffer* indexBuffer;

	UINT vertexCount;
	UINT indexCount;

private:
	bool bCreateShader;

	D3D11_PRIMITIVE_TOPOLOGY topology;
	UINT pass;
	UINT tech;

	PerFrame* perFrame;
};