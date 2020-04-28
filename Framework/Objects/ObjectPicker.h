#pragma once
class ObjectPicker
{
public:
	ObjectPicker();
	~ObjectPicker();

	//랜더타겟용
	void SetObject();
	// 랜더타겟 출력용
	void Render();

private:
	Shader* shader;
	PerFrame* perframe;
	UINT pass = 0;
	UINT tech = 0;

	Vector4 perspertive;

	RenderTarget * targetColorID;
	RenderTarget * targetInst;

	DepthStencil* depthStencil;
	Viewport* viewport;

	Render2D* screen;

private:
	struct CS_OutputDesc
	{
		Color color;
		UINT inst;
	};

private:
	Shader* computeShader;
	StructuredBuffer* computeBuffer = NULL;

	CS_OutputDesc* csOutput = NULL;
	CS_OutputDesc* csResult = NULL;

	ID3DX11EffectConstantBuffer* sComputeCB;

	ID3DX11EffectUnorderedAccessViewVariable* sUav;
};

