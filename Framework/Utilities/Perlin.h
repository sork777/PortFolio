#pragma once


class Perlin
{
public:
	Perlin();
	~Perlin();
	
	void PerlinController();
	void GeneratorNoise2D();
	int GetSize() { return (int)perlinDesc.Res.x; }
	bool& CanUsing() { return bPerlinUse; }
	ID3D11ShaderResourceView* GetPerlinSrv() { return NoiseCs->SRV(); }
	vector<Color>& GetPixels() { return PerlinPixel; }

	void Resize(UINT size=0);
private:
	void ReSeed();
private:
	struct PerlinDesc
	{
		Color TopColor;
		Color BottomColor;

		Vector2 Res;
		int Octave;
		float Persistence;
		float GridSize;
		int Seed;
		int SmoothedN;


		float padding;
	}perlinDesc;

private:
	Shader* shader;


	vector<int> p;
	ConstantBuffer* cbuffer;
	CsTexture* NoiseCs;
	vector<Color> PerlinPixel;
	bool bPerlinUse=false;
};

