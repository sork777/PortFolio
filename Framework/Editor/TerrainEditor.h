#pragma once
#include "IEditor.h"
/*
	0729
	1. TerrainLodŬ�������� CS������ ���� �������� �б�
	2. Brush ���� ���Ѽ� ���⼭ �����ϱ�.

	0821 - 1��
	1. ���CS���� Editor��
	2. �귯�� �Ӽ�->Brush�� �и�
	3. Terrain�� �ؽ��ĳ� ����������+����Ʈ��, uv��ŷ�� ���� RTV ����
*/

class TerrainLod;
class TerrainBrush;

class TerrainEditor : public IEditor
{
public:	
	TerrainEditor(TerrainLod* terrainLod);
	~TerrainEditor();

	// IEditor��(��) ���� ��ӵ�
	virtual void Initialize() override;
	virtual void Destroy() override;
	virtual void Update() override;
	virtual void PreRender() override;
	virtual void Render() override;
	virtual void PostRender() override;

public:
	void TerrainController();

private:
	/* ���� ��� */
	void RaiseHeightQuad();
	/* ���� ��� */
	void RaiseHeightCircle();
	void RaiseHeightSlope();
	void HeightNoise();
	void HeightSmoothing();


private:
	Shader* rShader;
	Shader* csShader;

//////////////////////////////////////////////////////////////////////////////
// Perlin
//////////////////////////////////////////////////////////////////////////////
private:
	void LoadPerlinMap();

private:
	Texture* PerlinTex;
	vector<D3DXCOLOR> PerlinPixel;
	class Perlin* perlinGen;

//////////////////////////////////////////////////////////////////////////////
// Line
//////////////////////////////////////////////////////////////////////////////
private:
	struct LineColorDesc
	{
		Color Color = D3DXCOLOR(0, 1, 0, 1);	//�̸��� ���ļ� �ٲ���
		UINT Visible = 0;
		float Thickness = 0.1f;
		float Size = 1.0f;

		float Padding;
	}lineColorDesc;

	ConstantBuffer* lineColorBuffer;
	ID3DX11EffectConstantBuffer* sLineColorBuffer;

//////////////////////////////////////////////////////////////////////////////
// Raise
//////////////////////////////////////////////////////////////////////////////
private:	
	struct RaiseDesc* raiseDesc;

	ConstantBuffer* raiseBuffer;
	ID3DX11EffectConstantBuffer* sRaiseBuffer;
	CsTexture* raiseCT[2];
	ID3D11ShaderResourceView* HMapSrv;
	Shader* raiseCS;

	Vector4 BrushedArea;

private:
	bool bSplat = false;
	bool bSlope = false;
	bool bQuadFrame = false;

	Vector4 slopBox = Vector4(+FLT_MAX, -FLT_MAX, +FLT_MAX, -FLT_MAX);

private:
	bool bEditMode = true;
	bool bLod = false;
	bool bWire = false;

private:
	void UpdateAlphaMap(UINT pass, UINT tech = 0);

private:
	TerrainLod* curTerrainLod;
	TerrainBrush* brush;
};

