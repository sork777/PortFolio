#pragma once


class HDR_Tone
{
public:
	HDR_Tone();
	HDR_Tone(Shader* shader);
	~HDR_Tone();

	void Init();
	void Tech(UINT val) { tech = val; }
	ID3D11ShaderResourceView* GetDS1DSrv() { return ds1Dsrv; }
	ID3D11ShaderResourceView* GetAvgLumSrv() { return avgLumSRV; }
	// Entry point for post processing
	void PostProcessing(ID3D11DepthStencilView * dsv);
	void Set(ID3D11DepthStencilView* dsv);
	ID3D11RenderTargetView* GetRTV() { return renderTarget->RTV(); }
	void SetParameters(float middleGray, float white)
	{
		this->middleGray = middleGray; 
		this->white = white; 
	}

private:
	// fullSize의 HDR 이미지를 다운스케일
	void DownScale(ID3D11ShaderResourceView* HDRSRV);

	// Final pass that composites all the post processing calculations
	void FinalPass(ID3D11ShaderResourceView* HDRSRV);

private:
	RenderTarget* renderTarget;

	ID3D11DepthStencilView* oldDSV;
	ID3D11RenderTargetView* oldRTV;
private:
	struct DownScaleDesc
	{
		//다운된 해상도
		UINT Width;
		UINT Height;
		//다운된 이미지 픽셀수
		UINT TotalPixels;
		//첫패스 그룹수
		UINT GroupSize;
	}dsDesc;

	struct FinalPassDesc
	{
		float MiddleGray;
		float LuminanceWhiteSqr;
		Vector2 Padding;
	}fpDesc;

	ConstantBuffer* DSBuffer;
	ConstantBuffer*	FinalPassBuffer;
	ID3DX11EffectConstantBuffer* sDSBuffer;
	ID3DX11EffectConstantBuffer* sFinalPassBuffer;
private:
	Shader* shader;
	UINT tech = 0;
	UINT dsPass = 0;
	UINT avgPass = 1;
	UINT finalPass = 2;

	ID3D11Buffer* ds1Dbuffer;
	ID3D11UnorderedAccessView* ds1Duav;
	ID3D11ShaderResourceView* ds1Dsrv;

	// Average luminance
	ID3D11Buffer* avgLumBuffer;
	ID3D11UnorderedAccessView* avgLumUAV;
	ID3D11ShaderResourceView* avgLumSRV;

	UINT width;
	UINT height;
	UINT dsGroups;
	float middleGray;
	float white;
};

