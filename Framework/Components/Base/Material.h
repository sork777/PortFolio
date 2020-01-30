#pragma once



class Material :public BaseComponent
{
public:
	Material();
	Material(Shader* shader);
	~Material();

	Shader* GetShader() { return shader; }

	virtual void Update() override {};
	virtual void Render() override;
	virtual bool Property() override;

	virtual void SetShader(Shader* shader) override;
	void CloneMaterial(Material* material);
private:
	void Initialize();

public:
	void Name(wstring val) { name = val; }
	wstring Name() { return name; }

	// Desc

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


	// Texture

	Texture* DiffuseMap() { return diffuseMap; }
	void LoadDiffuseMap(string file, string dir = "../../_Textures/");
	void LoadDiffuseMapW(wstring file, wstring dir = L"../../_Textures/");

	Texture* SpecularMap() { return specularMap; }
	void LoadSpecularMap(string file, string dir = "../../_Textures/");
	void LoadSpecularMapW(wstring file, wstring dir = L"../../_Textures/");

	Texture* NormalMap() { return normalMap; }
	void LoadNormalMap(string file, string dir = "../../_Textures/");
	void LoadNormalMapW(wstring file, wstring dir = L"../../_Textures/");
private:
	struct ColorDesc
	{
		Color Ambient = Color(0, 0, 0, 1);
		Color Emissive = Color(0, 0, 0, 1);
		Color Diffuse = Color(1, 1, 1, 1);
		Color Specular = Color(0, 0, 0, 1);
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