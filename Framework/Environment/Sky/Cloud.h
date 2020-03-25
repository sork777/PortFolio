#pragma once

class Cloud
{
public:
	Cloud(Shader* shader);
	~Cloud();

	void Update();
	void Render(bool bGlow = false);
	void PostRender();

private:
	void CreateDome();
	void CreatePlane();
private:
	Shader * shader;

	VertexBuffer * vertexBuffer;

	ID3D11Texture2D* texture;
	ID3D11ShaderResourceView* srv;

	ID3DX11EffectShaderResourceVariable* sSRV;

	Render2D* render2D;
	Texture* cloud;
	UINT domeCount=32;
	UINT planeRes = 32;
	float skyTop = 10.00f;
	float skyBottom = -3.00f;
	float multi = 10.0f;

	VertexBuffer* domeVertexBuffer;
	IndexBuffer* domeIndexBuffer;
	UINT domeVertexCount;
	UINT domeIndexCount;
};