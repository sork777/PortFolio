#pragma once



class MaterialPBR :public BaseComponent
{
public:
	MaterialPBR();
	MaterialPBR(Shader* shader);
	~MaterialPBR();

	Shader* GetShader() { return shader; }

	virtual void Update() override ;
	virtual void Render() override;
	virtual bool Property() override;

	virtual void SetShader(Shader* shader) override;
private:
	void Initialize();

public:
	void Name(wstring val) { name = val; }
	wstring Name() { return name; }

	// Desc

	Color& Albedo() { return colorDesc.Albedo; }
	void Albedo(Color& color);
	void Albedo(float r, float g, float b, float a = 1.0f);
	
	Vector3& F0() { return colorDesc.F0; }
	void F0(Vector3& fo);
	void F0(float r, float g, float b);

	Color& Emissive() { return colorDesc.Emissive; }
	void Emissive(Color& color);
	void Emissive(float r, float g, float b, float a=1.0f);

	float& Lough() { return colorDesc.Loughness; }
	void Lough(float lough = 1.0f);	

	// Texture

	Texture* AlbedoMap() { return albedoMap; }
	void LoadAlbedoMap(string file, string dir = "../../_Textures/MaterialPBR/");
	void LoadAlbedoMapW(wstring file, wstring dir = L"../../_Textures/MaterialPBR/");

	Texture* HeightMap() { return heightMap; }
	void LoadHeightMap(string file, string dir = "../../_Textures/");
	void LoadHeightMapW(wstring file, wstring dir = L"../../_Textures/");

	Texture* NormalMap() { return normalMap; }
	void LoadNormalMap(string file, string dir = "../../_Textures/MaterialPBR/");
	void LoadNormalMapW(wstring file, wstring dir = L"../../_Textures/MaterialPBR/");

	Texture* LoughnessMap() { return loughnessMap; }
	void LoadLoughnessMap(string file, string dir = "../../_Textures/MaterialPBR/");
	void LoadLoughnessMapW(wstring file, wstring dir = L"../../_Textures/");

	Texture* MetalicMap() { return metalicMap; }
	void LoadMetalicMap(string file, string dir = "../../_Textures/MaterialPBR/");
	void LoadMetalicMapW(wstring file, wstring dir = L"../../_Textures/MaterialPBR/");



private:
	struct ColorDesc
	{
		Color Albedo = Color(1, 1, 1, 1);
		Color Emissive = Color(0, 0, 0, 1);
		Vector3 F0 = Vector3(0.04f, 0.04f, 0.04f);
		float Loughness = 1.0f;
	} colorDesc;

private:

	wstring name;

	Texture* albedoMap;
	Texture* normalMap;
	Texture* heightMap;
	Texture* loughnessMap;
	Texture* metalicMap;

	ConstantBuffer* buffer;

	ID3DX11EffectConstantBuffer* sBuffer;
	ID3DX11EffectShaderResourceVariable* sAlbedoMap;
	ID3DX11EffectShaderResourceVariable* sNormalMap;
	ID3DX11EffectShaderResourceVariable* sHeightMap;
	ID3DX11EffectShaderResourceVariable* sLoughnessMap;
	ID3DX11EffectShaderResourceVariable* sMetalicMap;
};