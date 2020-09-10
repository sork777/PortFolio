#pragma once
#include "IEditor.h"
/*
	0729
	1. TerrainLod클래스에서 CS관련은 전부 이쪽으로 밀기
	2. Brush 독립 시켜서 여기서 연동하기.

	0821 - 1차
	1. 상승CS관련 Editor로
	2. 브러시 속성->Brush로 분리
	3. Terrain은 텍스쳐나 랜더데이터+쿼드트리, uv피킹을 위한 RTV 보유
*/

class TerrainLod;
class TerrainBrush;

class TerrainEditor : public IEditor
{
public:	
	TerrainEditor(TerrainLod* terrainLod);
	~TerrainEditor();

	// IEditor을(를) 통해 상속됨
	virtual void Initialize() override;
	virtual void Destroy() override;
	virtual void Update() override;
	virtual void PreRender() override;
	virtual void Render() override;
	virtual void PostRender() override;

public:
	void TerrainController();

private:
	/* 쿼드 상승 */
	void RaiseHeightQuad();
	/* 원형 상승 */
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
		Color Color = D3DXCOLOR(0, 1, 0, 1);	//이름이 겹쳐서 바꿔줌
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

