#pragma once

class CSM
{
public:
	CSM(Shader* shader, UINT width = 0, UINT height = 0);
	~CSM();

	void Update();
	
	void Pass(UINT val) { pass = val; }
	void Set();
	void UpdateVolume();
	//void SetSamplerForShader(Shader* shader);
	ID3D11ShaderResourceView* SRV() { return srv; }
	ID3D11DepthStencilView* DSV() { return dsv; }
private:
	void CreateDepthStencil();
	void CreateSampler();
	void BoundSphere(float nearplane, float farplane,Vector3& boundCenter,float& radius);
	bool NeedUpdate(const Matrix& shadowV, int idx, const Vector3& newCenter, Vector3& offset);
private:
	struct CascadeDesc
	{
		Matrix ViewProjection[3];
		Vector2 MapSize;
		float Bias = -0.0006f;
		float padding;
	} casDesc;

	struct DirShadowDesc
	{
		Matrix ToShadowSpace;
		Vector4 ToCascadeOffsetX;
		Vector4 ToCascadeOffsetY;
		Vector4 ToCascadeScale;
	} shadowDesc;

private:
	float CSMradius[4];
	float shadowBoundRadius = 0;
	Vector3 cascadeBoundCenter[3];
	float cascadeBoundRadius[3];
private:
	UINT pass;
	Shader * shader;
	UINT width, height;
	ConstantBuffer* buffer;
	ID3DX11EffectConstantBuffer* sBuffer;
	ConstantBuffer* shadowbuffer;
	ID3DX11EffectConstantBuffer* sShadowBuffer;

	/* 조명을 받을 중심 좌표 */
	Vector3 position;
	Vector3 c_pos;

private:
	UINT CascadeSize;

	ID3D11Texture2D* backBuffer;
	ID3D11DepthStencilView* dsv;
	ID3D11ShaderResourceView* srv;
	ID3D11SamplerState* samplerState;
};