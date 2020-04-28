#pragma once
class ObjectPicker
{
public:
	ObjectPicker();
	~ObjectPicker();

	//����Ÿ�ٿ�
	void SetObject();
	// ����Ÿ�� ��¿�
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

