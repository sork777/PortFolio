#pragma once


class SSAO
{
public:
	SSAO();
	SSAO(Shader* shader);
	~SSAO();

	void RenderSSAO();
	void Pass(UINT passMini,UINT passAO)
	{
		this->passMini = passMini;
		this->passAO = passAO;
	}
	void Compute(ID3D11ShaderResourceView* DepthSRV, ID3D11ShaderResourceView* NormalsSRV);

	void SetParameters(int SSAOSampRadius, float Radius) {
		this->SSAOSampRadius = SSAOSampRadius;
		this->Radius = Radius; 
	}
	ID3D11ShaderResourceView* GetSSAOSRV() { return SSAO_SRV; }
	ID3D11ShaderResourceView* GetMiniDepthSRV() { return miniDepthSRV; }

private:
	void Initialize();
	void DownscaleDepth(ID3D11ShaderResourceView * DepthSRV, ID3D11ShaderResourceView * NormalsSRV);
	void ComputeSSAO();
	
private:
	struct TDownscaleDesc
	{
		UINT Width;
		UINT Height;
		float HorResRcp;
		float VerResRcp;

		Vector4 ProjParams;
		Matrix ViewMatrix;

		float OffsetRadius;
		float Radius;
		float MaxDepth;
		float padding;
	} desc;
	ConstantBuffer* cBuffer;
private:
	Shader* shader;
	UINT passMini = 0;
	UINT passAO = 1;

	UINT width;
	UINT height;
	int SSAOSampRadius=10;
	float Radius=25.0f;
	
private:
	// SSAO values for usage with the directional light
	ID3D11Texture2D* SSAO_RT;
	ID3D11UnorderedAccessView* SSAO_UAV;
	ID3D11ShaderResourceView* SSAO_SRV;

	// Downscaled depth buffer (1/4 size)
	ID3D11Buffer* miniDepthBuffer;
	ID3D11UnorderedAccessView* miniDepthUAV;
	ID3D11ShaderResourceView* miniDepthSRV;
private:
	Render2D* renderSrvs[2];
	StructuredBuffer* CSminiDepthbuffer;
};

