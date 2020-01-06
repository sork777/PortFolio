#pragma once


class Projector
{
public:
	Projector(Shader* shader, wstring textureFile, bool bOrtho = false, UINT width = 1, UINT height = 1);
	~Projector();

	void Update();

	Camera* GetCamera() { return (Camera *)camera; }

	void AddTexture(wstring textureFile);
	void SelectTex(int index);
	void SetWH(UINT width, UINT height) { this->width = width; this->height = height; }
private:
	struct Desc
	{
		Matrix View;
		Matrix Projection;

		Color Color = D3DXCOLOR(1, 1, 1, 1);

		int draw = 0;
		Vector3 padding;
	} desc;

private:
	Shader * shader;

	UINT width, height;

	class Fixity* camera;
	Projection* projection;
	bool bOrtho;


	vector<Texture*> textures;
	ID3DX11EffectShaderResourceVariable* sMap;

	ConstantBuffer* buffer;
	ID3DX11EffectConstantBuffer* sBuffer;
};