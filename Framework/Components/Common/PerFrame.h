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
	//C에서는 패딩 맞춰줘야
	struct BufferDesc
	{
		Matrix View;
		Matrix ViewInverse;
		Matrix Projection;
		//한번에 변환한걸 보내서 연산 빠르게 하기위해
		Matrix VP;	
		//실무에서는 다 보냄, W,V,P, VP, WVP등

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

