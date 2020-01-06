#pragma once

class PerFrame
{
public:
	PerFrame(Shader* shader);
	~PerFrame();

public:
	void Update();
	void Render();

private:
	//C������ �е� �������
	struct BufferDesc
	{
		Matrix View;
		Matrix ViewInverse;
		Matrix Projection;
		//�ѹ��� ��ȯ�Ѱ� ������ ���� ������ �ϱ�����
		Matrix VP;	
		//�ǹ������� �� ����, W,V,P, VP, WVP��

		float Time;
		float Padding[3];
	}bufferDesc;

	struct LightDesc
	{
		Color Ambient;
		Color Specular;

		Vector3 Direction;
		float Padding;

		Vector3 Position;
		float Padding2;
	}lightDesc;

	struct PointLightDesc
	{
		UINT Count = 0;
		float Padding[3];

		PointLight Lights[MAX_POINT_LIGHT];
	} pointLightDesc;

	struct SpotLightDesc
	{
		UINT Count = 0;
		float Padding[3];

		SpotLight Lights[MAX_SPOT_LIGHT];
	} spotLightDesc;

	struct CapsuleLightDesc
	{
		UINT Count = 0;
		float Padding[3];

		CapsuleLight Lights[MAX_CAPSULE_LIGHT];
	} capsuleLightDesc;
private:
	Shader * shader;

	ConstantBuffer* buffer;
	ID3DX11EffectConstantBuffer* sBuffer;

	ConstantBuffer* lightBuffer;
	ID3DX11EffectConstantBuffer* sLightBuffer;

	ConstantBuffer* pointLightBuffer;
	ID3DX11EffectConstantBuffer* sPointLightBuffer;

	ConstantBuffer* spotLightBuffer;
	ID3DX11EffectConstantBuffer* sSpotLightBuffer;

	ConstantBuffer* capsuleLightBuffer;
	ID3DX11EffectConstantBuffer* sCapsuleLightBuffer;
};

