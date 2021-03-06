#pragma once

class Atmosphere : public Renderer
{
public:
	Atmosphere(Shader* shader);
	~Atmosphere();
	
	void Update();
	void PreRender();
	void Render(bool bRTV=true);
	void PostRender();

private:
	struct ScatterDesc
	{
		Vector3 WaveLength = Vector3(0.65f, 0.57f, 0.475f);
		int SampleCount = 8;

		Vector3 InvWaveLength;
		float StarIntensity;

		Vector3 WaveLengthMie;
		float MoonAlpha;
	} scatterDesc;

	struct CloudDesc
	{
		float Tiles = 1.5f;
		float Cover = 0.005f;
		float Sharpness = 0.405f;
		float Speed = 0.05f;
	} cloudDesc;
private:
	bool realTime;
	float timeFactor;

	float theta, prevTheta;

	class Scattering* scattering;
	ConstantBuffer* scatterBuffer;
	ID3DX11EffectConstantBuffer* sScatterBuffer; 

	class Cloud* cloud;
	ConstantBuffer* cloudBuffer;
	ID3DX11EffectConstantBuffer* sCloudBuffer;

	class Moon* moon;

	RenderTarget* atmoTarget;
	DepthStencil* depthStencil;

public:
	ID3D11ShaderResourceView* GetAtmoSRV() { return atmoTarget->SRV(); }
};

