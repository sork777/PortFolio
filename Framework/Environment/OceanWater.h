#pragma once

class OceanWater : public Renderer
{
public:
	OceanWater(Shader* shader, UINT width, UINT height, float thick, Texture* heightMap);
	~OceanWater();

	void Update() override;
	void Render() override;

private:
	UINT width, height;
	float thick;

	bool bWireframe;

	Texture* heightMap;
	Material* material;

private:
	struct Desc
	{
		float WaveFrequency = 0.05f;
		float WaveAmplitude = 1.0f;
		Vector2 TextureScale = Vector2(14, 14);

		Vector2 BumpSpeed = Vector2(0.0f, 0.05f);
		float BumpHeight = 0.6f;
		float HeightRatio = 7.5f;

		Color ShallowColor = Color(1, 1, 1, 1.0f);
		Color DeepColor = Color(0.2f, 0.5f, 0.95f, 1.0f);

		float Bias = 0.8f;
		float Power = 0.5f;
		float Amount = 0.5f;
		float ShoreBlend = 35.0f;

		Vector2 OceanSize;
		float Padding[2];
	} desc;

	ConstantBuffer* buffer;
	ID3DX11EffectConstantBuffer* sBuffer;

	ID3DX11EffectShaderResourceVariable* sMap;
};