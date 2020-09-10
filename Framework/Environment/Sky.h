#pragma once
class Sky
{
public:
	Sky();
	Sky(wstring cubeMapFile);
	~Sky();

	void Initialize();

	void Update();
	void Render();

	ID3D11ShaderResourceView* CubeSRV() { return cubeSRV; }

private:
	struct Desc
	{
		Color Center = Color(0xFF0080FF);
		Color Apex = Color(0xFF9BCDFF);
		float Height = 4.0f;
		float Padding[3];
	}desc;
private:
	Shader * shader;
	//얘를 통해 전역광원 업뎃해도 왜 sky에 영향이 없지..?
	//PerFrame* perframe;
	MeshRender* sphere;

	ConstantBuffer* buffer;
	ID3DX11EffectConstantBuffer* sBuffer;

	ID3D11ShaderResourceView* cubeSRV;
	ID3DX11EffectShaderResourceVariable* sCubeSRV;
};

