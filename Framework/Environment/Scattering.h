#pragma once

class Scattering
{
public:
	Scattering(Shader* shader);
	~Scattering();

	void Update();

	void PreRender();
	void Render();
	void PostRender();

private:
	void CreateQuad();
	void CreateDome();
private:
	const UINT width, height;
	UINT domeCount;

	Shader* shader;
	Render2D* render2D;

	RenderTarget* mieTarget, *rayleighTarget;
	DepthStencil* depthStencil;
	Viewport* viewport;

	VertexBuffer* vertexBuffer;
	
	ID3DX11EffectShaderResourceVariable* sRayleighSRV;
	ID3DX11EffectShaderResourceVariable* sMieSRV;

	//UINT domeCount;
	VertexBuffer* domeVertexBuffer;
	IndexBuffer* domeIndexBuffer;
	UINT domeVertexCount;
	UINT domeIndexCount;
};