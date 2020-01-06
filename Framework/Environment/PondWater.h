#pragma once

class PondWater : public Renderer
{
public:
	PondWater(Shader* shader, float height, float radius);
	~PondWater();

	void Update() override;
	void Render() override;
	
	void SetReflection();
	void SetRefraction();
private:
	struct Desc
	{
		Matrix ReflectionMatrix;
		Color RefractionColor = Color(0.2f, 0.3f, 1.0f, 1.0f);
		Vector2 NormalMapTile=Vector2(0.1f,0.2f);
		float WaveTranslation = 0.0f;
		float WaveScale=0.1f;
		float WaterShininess=200.0f;

		float Padding[3];
	}desc;
private:
	float height, radius;

	ConstantBuffer* buffer;
	Texture* waveMap;

	float waveSpeed;
	Vector2 mapScale;

	Fixity* reflectionCamera;
	RenderTarget* reflection;
	RenderTarget* refraction;

	DepthStencil* depthStencil;
	Viewport* viewport;

	//ID3DX11EffectShaderResourceVariable* sReflection;
	ID3DX11EffectVectorVariable* sClipPlane;

};

