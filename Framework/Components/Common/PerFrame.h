#pragma once

class PerFrame :public CommonComponent
{
public:
	PerFrame(Shader* shader);
	~PerFrame();

public:
	virtual void SetShader(Shader* shader) override;

	virtual void Update() override;
	virtual void Render() override;
	virtual bool Property() override;

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

private:
	ConstantBuffer* buffer;
	ID3DX11EffectConstantBuffer* sBuffer;

	ConstantBuffer* lightBuffer;
	ID3DX11EffectConstantBuffer* sLightBuffer;
};

