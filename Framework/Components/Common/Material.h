#pragma once



class Material :public CommonComponent
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
	void Name(const wstring& val) { name = val; }
	const wstring& Name() { return name; }

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
	void LoadDiffuseMap(const string& file, const string& dir = "../../_Textures/Material/");
	void LoadDiffuseMapW(const wstring& file, const wstring& dir = L"../../_Textures/Material/");

	Texture* SpecularMap() { return specularMap; }
	void LoadSpecularMap(const string& file, const string& dir = "../../_Textures/Material/");
	void LoadSpecularMapW(const wstring& file, const wstring& dir = L"../../_Textures/Material/");

	Texture* NormalMap() { return normalMap; }
	void LoadNormalMap(const string& file, const string& dir = "../../_Textures/Material/");
	void LoadNormalMapW(const wstring& file, const wstring& dir = L"../../_Textures/Material/");

	Texture* HeightMap() { return heightMap; }
	void LoadHeightMap(const string& file, const string& dir = "../../_Textures/Material/");
	void LoadHeightMapW(const wstring& file, const wstring& dir = L"../../_Textures/Material/");

private:
	void LoadTexture(function<void(string, string)> func, const wstring& filePath = L"");

private:
	struct ColorDesc
	{
		Color Ambient = Color(0, 0, 0, 1);
		Color Emissive = Color(0, 0, 0, 1);
		Color Diffuse = Color(1, 1, 1, 1);
		Color Specular = Color(1, 1, 1, 1);
	} colorDesc;

private:
	wstring name;

	Texture* diffuseMap;
	Texture* specularMap;
	Texture* normalMap;
	Texture* heightMap;

	ConstantBuffer* buffer;

	ID3DX11EffectConstantBuffer* sBuffer;
	ID3DX11EffectShaderResourceVariable* sDiffuseMap;
	ID3DX11EffectShaderResourceVariable* sSpecularMap;
	ID3DX11EffectShaderResourceVariable* sNormalMap;
	ID3DX11EffectShaderResourceVariable* sHeightMap;
};