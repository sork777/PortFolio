#pragma once

class Moon
{
public:
	Moon(Shader* shader);
	~Moon();

	void Update();
	void Render(bool bGlow = false);

	float MoonAlpha(float theta);

	Matrix GetTransform(float theta);
	Matrix GetGlowTransform(float theta);

private:
	Shader * shader;

	VertexBuffer * vertexBuffer;
	float distance, glowDistance;

	Texture* moon;
	Texture* moonGlow;

	ID3DX11EffectShaderResourceVariable* sMoon;
};