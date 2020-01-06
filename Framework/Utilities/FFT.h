#pragma once
////////////////////////////////////////////////////////////////////////////////
// Common constants
////////////////////////////////////////////////////////////////////////////////
#define TWO_PI 6.283185307179586476925286766559

#define FFT_DIMENSIONS 3U
#define FFT_PLAN_SIZE_LIMIT (1U << 27)

#define FFT_FORWARD -1
#define FFT_INVERSE 1
#define COHERENCY_GRANULARITY 128

class FFT
{
public:
	FFT();
	FFT(Shader* shader);
	~FFT();

	void Radix008A(ID3D11UnorderedAccessView* pUAV_Dst,
		ID3D11ShaderResourceView* pSRV_Src,
		UINT thread_count,
		UINT istride);

	void Create_Plan(UINT slices);
	void Destroy_Plan();

	void CreateCBuffers_512x512(UINT slices);
	void fft_512x512_c2c(ID3D11UnorderedAccessView* pUAV_Dst,
		ID3D11ShaderResourceView* pSRV_Dst,
		ID3D11ShaderResourceView* pSRV_Src);
	void RenderFFT();
	void SaveFile(UINT num);
private:
	struct CB_Structure
	{
		UINT thread_count;
		UINT ostride;
		UINT istride;
		UINT pstride;
		float phase_base;

		Vector3 Padding;
	};

private:
	Shader* shader;
	UINT passCS = 0;
	UINT passCS2 = 1;

	UINT slices;
	// More than one array can be transformed at same time

	// For 512x512 config, we need 6 constant buffers
	CB_Structure CB_Descs[6];
	ConstantBuffer* RadixCBuffers[6];

	// Temporary buffers
	ID3D11Buffer* tmpBuffer;
	ID3D11UnorderedAccessView* tmpUAV;
	ID3D11ShaderResourceView* tmpSRV;

	Render2D* renderSrvs[6];
	StructuredBuffer* textSB;
};

