#pragma once

class SSLR
{
public:
	SSLR();
	SSLR(Shader* shader);
	~SSLR();

	void Init();
	void Pass(UINT val)
	{
		occPass = val + 0;
		rtPass = val + 1;
		combinePass = val + 2;
	}
	// Render the screen space light rays on top of the scene
	void Render(ID3D11ShaderResourceView* pMiniDepthSRV);
	
	void SSLRController();
public:
	ID3D11ShaderResourceView* GetOcclusionSRV() {		return OcclusionSRV;	}
private:
	// Prepare the occlusion map
	void PrepareOcclusion( ID3D11ShaderResourceView* pMiniDepthSRV);

	// Ray trace the occlusion map to generate the rays
	void RayTrace(const D3DXVECTOR2& vSunPosSS, const D3DXVECTOR3& vSunColor);

	// Combine the rays with the scene
	void Combine( ID3D11RenderTargetView* pLightAccumRTV);

private:
	struct OcculusionDesc
	{
		UINT Width;
		UINT Height;
		float pad[2];
	}OccDesc;

	struct RayTraceDesc
	{
		Vector2 SunPos;
		float InitDecay;
		float DistDecay;
		Vector3 RayColor;
		float MaxDeltaLen;
	}RTDesc;

	ConstantBuffer* occlussionBuffer;
	ID3DX11EffectConstantBuffer* sOcclussionBuffer;

	ConstantBuffer* rayTraceBuffer;
	ID3DX11EffectConstantBuffer* sRayTraceBuffer;
private:
	Shader* shader;
	UINT occPass = 0;
	UINT rtPass = 1;
	UINT combinePass = 2;

	UINT Width;
	UINT Height;
	float InitDecay=0.2f;
	float DistDecay=0.8f;
	float MaxDeltaLen=0.005f;
	float intensity=0.45f;

	ID3D11Texture2D* OcclusionTex;
	ID3D11UnorderedAccessView* OcclusionUAV;
	ID3D11ShaderResourceView* OcclusionSRV;

	ID3D11Texture2D* LightRaysTex;
	ID3D11RenderTargetView* LightRaysRTV;
	ID3D11ShaderResourceView* LightRaysSRV;

	bool showRTRes;
};

