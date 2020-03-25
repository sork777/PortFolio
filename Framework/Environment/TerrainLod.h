#pragma once

#include "Utilities/QuadTree.h"

enum TextureLayerType
{
	Base,
	Alpha,
	Layer_1,
	Layer_2,
	Layer_3,
};

class TerrainLod :public Renderer
{
public:
	/* 초기화 정보를 여기서 몰아서 만들거임 */
	struct InitializeInfo
	{
		Shader* shader;

		float CellSpacing;
		UINT CellsPerPatch;
		float HeightRatio;		
	};
public:
	TerrainLod(InitializeInfo& info);
	~TerrainLod();
	
	void Update();
	void Render();
	void ColliderRender();
	void TerrainController();

	Texture* AlphaTexture() { return alphaTexture; }

private:
	bool InBounds(UINT row, UINT col);
	void CalcBoundY();
	void CalcPatchBounds(UINT row, UINT col);

	void CreateVertexData();
	void CreateIndexData();

////////////////////////////////////////////////////////////////////////////////
// Textures
public:
	void BaseTexture(wstring file);
	void AlphaTexture(wstring file, bool bUseAlpha=false);
	void LayerTextures(wstring layer, UINT layerIndex);
	void NDTexture(wstring normal);
	void SetLayer(UINT layerIndex);
	void OpenTextureLayer(TextureLayerType type, const wstring& filePath = L"");

	void SaveAlphaLayer(wstring* savePath,const wstring& filePath = L"");
	void SaveTerrainToXml(const wstring& filePath = L"");
	void LoadTerrainFromXml(const wstring& filePath = L"");

private:
	UINT layerIndex;
	Texture* baseTexture;
	ID3DX11EffectShaderResourceVariable* sBaseTexture;

	Texture* layerTexture[3];
	ID3D11ShaderResourceView* layerViews[3] = { 0 };
	ID3DX11EffectShaderResourceVariable* sLayerTexture;

	Texture* alphaTexture;
	ID3DX11EffectShaderResourceVariable* sAlphaTexture;
	
	Texture* normalTexture;
	ID3DX11EffectShaderResourceVariable* sNormalTexture;


private:
	void LoadPerlinMap();	
private:

	Texture* PerlinTex;
	vector<D3DXCOLOR> PerlinPixel;
	class Perlin* perlinGen;
////////////////////////////////////////////////////////////////////////////////
// Brush
public:
	Vector3 GetPickedPosition();
private:
	float GetPickedHeight();
	/* 브러시의 raiseDesc 업데이트 */
	void BrushUpdater(Vector3& position);
	/* 쿼드 상승 */
	void RaiseHeightQuad();
	/* 원형 상승 */
	void RaiseHeightCircle();
	void RaiseHeightSlope();
	void HeightNoise();
	void HeightSmoothing();
	/* CS로 변경 값 계산 실행 */

	void UpdateAlphaMap(UINT pass,UINT tech=0);
private:
	struct BrushDesc
	{
		Color Color = D3DXCOLOR(0, 1, 0, 1);	
		Vector3 Location;
		UINT Type = 0;
		UINT Range = 1;

		float Padding[3];
	}brushDesc;

	struct LineColorDesc
	{
		Color Color = D3DXCOLOR(0, 1, 0, 1);	//이름이 겹쳐서 바꿔줌
		UINT Visible = 1;
		float Thickness = 0.1f;
		float Size = 1.0f;

		float Padding;
	}lineColorDesc;

	ConstantBuffer* brushBuffer;
	ID3DX11EffectConstantBuffer* sBrushBuffer;
	ConstantBuffer* lineColorBuffer;
	ID3DX11EffectConstantBuffer* sLineColorBuffer;
private:
	struct RaiseDesc
	{
		Vector4 Box;
		
		Vector2 Res;
		Vector2 Position;

		float Radius;
		float Rate;
		float Factor;
		int RaiseType;		//0 상승 1 하강 2 플랫

		int SlopDir;	//1이면 수평 0이면 수직
		int SlopRev;
		int SplattingLayer;
		int BrushType;
	}raiseDesc;

	ConstantBuffer* raiseBuffer;
	ID3DX11EffectConstantBuffer* sRaiseBuffer;
	CsTexture* raiseCT[2];
	ID3D11ShaderResourceView* HMapSrv;
	Shader* raiseCS;

	Vector4 BrushedArea;
	OBBCollider* AreaCol;

	/* Controller */
private:
	bool bSplat = false;
	bool bSlope = false;
	bool bQuadFrame = false;

	float raiseRate = 50.0f;
	float rfactor = 1.0f;
	float gfactor = 1.0f;
	float sAngle = 0.0f;
	Vector3 brushPos = Vector3(-1, -1, -1);
	Vector4 slopBox = Vector4(+FLT_MAX, -FLT_MAX, +FLT_MAX, -FLT_MAX);


////////////////////////////////////////////////////////////////////////////////
// Terrain Lod Info
private:
	struct VertexTerrain
	{
		Vector3 Position;
		Vector2 Uv;
		Vector2 BoundsY;
	};

	struct BufferDesc
	{
		float MinDistance = 1.0f;
		float MaxDistance = 500.0f;
		float MinTessellation = 2.0f;
		float MaxTessellation = 64.0f;

		float TexelCellSpaceU;
		float TexelCellSpaceV;
		float WorldCellSpace = 1.0f;
		float HeightRatio = 10.0f;

		float TexScale = 1.0f;
		Vector3 Padding;
	} bufferDesc;
private:
	UINT faceCount;
	UINT patchVertexRows;
	UINT patchVertexCols;

private:
	InitializeInfo info;

	UINT width, height;
	ConstantBuffer* texelBuffer;
	ID3DX11EffectConstantBuffer* sTexelBuffer;

	vector<D3DXCOLOR> AlphaMapPixel;

	VertexTerrain* vertices;
	UINT* indices;

	vector<Vector2> bounds;

////////////////////////////////////////////////////////////////////////////////
// QuadTree
private:
	QuadTreeNode* CreateQuadTreeData(QuadTreeNode* parent, Vector2& TopLeft, Vector2& BottomRight);
	Vector2 GetMinMaxY(Vector2& TopLeft, Vector2& BottomRight);

	void UpdateQuadHeight();
	void CheckQuadCollider(QuadTreeNode* node, Collider* collider, vector< QuadTreeNode*>& updateNode);
private:
	QuadTree* quadTree;


private:
	OBBCollider* TestCol;
};
