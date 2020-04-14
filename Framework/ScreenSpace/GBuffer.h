#pragma once

class GBuffer
{
public:
	GBuffer(Shader* shader);
	~GBuffer();

	void Update();
	void Render();
	void Pass(UINT val) { pass = val; }
	void Tech(UINT val) { tech = val; }
	
	void SetRTVs();
	ID3D11ShaderResourceView* GetNormalSrv() {return targetNormal->SRV();}
	ID3D11ShaderResourceView* GetDepthSrv() {return depthStencil->SRV();}

	ID3D11DepthStencilView* GetDepthDsv() {return depthStencil->DSV();}
	ID3D11DepthStencilView* GetDepthOnlyDsv() {return depthStencil->OnlyDSV();}
	
	void RenderGBuffers();
private:
	PerFrame* perframe;
private:
	Shader* shader;
	UINT pass = 0;
	UINT tech = 0;

	Vector4 perspertive;

	RenderTarget * targetColor;
	RenderTarget * targetNormal;
	RenderTarget * targetDepth;
	RenderTarget * targetSpec;
	RenderTarget * targetEmissve;
	RenderTarget * targetMetalLoughcoef;

	DepthStencil* depthStencil;
	Viewport* viewport;

	Render2D* DepNorDifSpe[5];
};

