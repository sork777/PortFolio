#pragma once

class Fog : public Render2D
{
public:
	Fog();
	~Fog();

	void Update() override;
	void Render() override;

	void Property();

	void SetDepthSrv(ID3D11ShaderResourceView* depth);
private:
	struct FogDesc
	{
		Color FogColor;
		Vector2 FogDistance;	//y°ªÀÌ heightfalloff?
		float FogDensity;
		UINT FogType;
	}fogDesc;

private:
	Vector4 perspertive;
	ConstantBuffer* fogBuffer;
	ID3D11ShaderResourceView* depthsrv =NULL;
	ID3DX11EffectConstantBuffer* sFogBuffer;
};
