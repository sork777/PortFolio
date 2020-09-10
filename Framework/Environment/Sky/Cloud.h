#pragma once

class Cloud
{
public:
	Cloud(Shader* shader);
	~Cloud();

	void Update();
	void Render(bool bGlow = false);
	void PostRender();
	
	void WireCloud() { ImGui::Checkbox("WireCloud", &bwire); }
private:
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
	float skyTop = 7.5f;
	float skyBottom = -0.5f;
	//float multi = 3.0f;
	bool bwire = false;

	VertexBuffer* domeVertexBuffer;
	IndexBuffer* domeIndexBuffer;
	UINT domeVertexCount;
	UINT domeIndexCount;
};