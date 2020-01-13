#pragma once

class Material
{
public:
	Material();
	Material(Shader* shader);
	~Material();

	Shader* GetShader() { return shader; }
	void SetShader(Shader* shader);


public:
	void Name(wstring val) { name = val; }
	wstring Name() { return name; }

	Color& Ambient() { return colorDesc.Ambient; }
	void Ambient(Color& color);
	void Ambient(float r, float g, float b, float a = 1.0f);

	Color& Diffuse() { return colorDesc.Diffuse; }
	void Diffuse(Color& color);
	void Diffuse(float r, float g, float b, float a = 1.0f);

	Color& Specular() { return colorDesc.Specular; }
	void Specular(Color& color);
	void Specular(float r, float g, float b, float a = 1.0f);

	Color& Emissive() { return colorDesc.Emissive; }
	void Emissive(Color& color);
	void Emissive(float r, float g, float b, float a = 1.0f);


	Texture* DiffuseMap() { return diffuseMap; }
	void DiffuseMap(string file, string dir = "../../_Textures/");
	void DiffuseMap(wstring file, wstring dir = L"../../_Textures/");

	Texture* SpecularMap() { return specularMap; }
	void SpecularMap(string file, string dir = "../../_Textures/");
	void SpecularMap(wstring file, wstring dir = L"../../_Textures/");

	Texture* NormalMap() { return normalMap; }
	void NormalMap(string file, string dir = "../../_Textures/");
	void NormalMap(wstring file, wstring dir = L"../../_Textures/");

public:
	void Render();
	void Property();
private:
	void Initialize();

private:
	struct ColorDesc
	{
		Color Ambient = Color(0, 0, 0, 1);
		Color Diffuse = Color(1, 1, 1, 1);
		Color Specular = Color(0, 0, 0, 1);
		Color Emissive = Color(0, 0, 0, 1);
	} colorDesc;

private:
	Shader * shader;

	wstring name;

	Texture* diffuseMap;
	Texture* specularMap;
	Texture* normalMap;

	ConstantBuffer* buffer;

	ID3DX11EffectConstantBuffer* sBuffer;
	ID3DX11EffectShaderResourceVariable* sDiffuseMap;
	ID3DX11EffectShaderResourceVariable* sSpecularMap;
	ID3DX11EffectShaderResourceVariable* sNormalMap;
};