#pragma once

class Shadow
{
public:
	Shadow(Shader* shader, Vector3& position = Vector3(0,0,0), float radius=500.0f, UINT width = 512, UINT height = 512);
	~Shadow();

	void Set();

	ID3D11ShaderResourceView* SRV() { return renderTarget->SRV(); }
	void Position(Vector3& pos) { c_pos = pos; }
private:
	void UpdateVolume();

private:
	struct Desc
	{
		Matrix View;
		Matrix Projection;

		Vector2 MapSize;
		float Bias = -0.0006f;

		UINT Quality=0;
	} desc;

private:
	Shader * shader;
	UINT width, height;

	ConstantBuffer* buffer;
	ID3DX11EffectConstantBuffer* sBuffer;

	RenderTarget* renderTarget;
	DepthStencil* depthStencil;
	Viewport* viewport;

	/* 조명을 받을 중심 좌표 */
	Vector3 position;		
	Vector3 c_pos;		
	float radius;

	ID3D11SamplerState* samplerState;
};