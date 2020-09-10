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
/*
	0821 브러시, 에디터 분리작업 성공
	기울기 기반 추가는 나중에...
*/
class TerrainLod :public Renderer
{
	//알파맵같은거 사용하게 하기
	friend class TerrainBrush;
	friend class TerrainEditor;

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
	
	void Initialize();
	void PreRender();
	void Update();
	void Render();

	void CreateInitHeightMap();
	void SetDebugMode(const bool& bDebug)
	{
		quadTree->GetCollider()->SetDebugMode(bDebug);
	}
	void SetDefferedMode(const bool& bDeffered)
	{
		quadTree->GetCollider()->SetDefferedMode(bDeffered);
	}
private:
	bool InBounds(UINT row, UINT col);
	void CalcBoundY();
	void CalcPatchBounds(UINT row, UINT col);

	void CreateVertexData();
	void CreateIndexData();

public:
	void QuadTreeRender();

	void SetHMapCond(const bool& bChanged) { bChangedHMap = bChanged; }
	const bool& IsChangedHeightMap() { return bChangedHMap; }
	void SetEditMode(const bool& bEdit) { bEditMode = bEdit; }
	const bool& GetEditMode() { return bEditMode; }
	void SetLod(const bool& bLod) { bufferDesc.UseLOD = bLod; }
public:
	Vector3 GetPickedPosition();
	float GetPickedHeight(const Vector3& position);

	void TerrainLayerProperty(bool* bChangeAlpha, bool* bUseAlpha);

private:
	bool bChangedHMap = false;	//높이가 변할때만 오브젝트 재정리하기위함
	bool bEditMode = false;
	bool bChangeAlpha = false;
	bool bUseAlpha = false;
////////////////////////////////////////////////////////////////////////////////
// Textures
public:
	void BaseTexture(wstring file);
	void AlphaTexture(wstring file, bool bUseAlpha=false);
	void LayerTextures(wstring layer, UINT layerIndex);
	void NDTexture(wstring normal);
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
	ID3D11ShaderResourceView* HMapSrv;

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
		int UseLOD = 0;
		Vector2 Padding;
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
	Vector2 GetMinMaxY(Vector2& TopLeft, Vector2& BottomRight);
	QuadTreeNode* CreateQuadTreeData(QuadTreeNode* parent, Vector2& TopLeft, Vector2& BottomRight);

	void UpdateQuadHeight(const Vector4& BrushedArea);

	void CheckQuadCollider(QuadTreeNode* node, vector< QuadTreeNode*>& updateNode);
private:
	QuadTree* quadTree;

private:
	OBBCollider* AreaCol;

/////////////////////////////////////////////////////////////////////////////
//UV Picking
private:
	Shader*		uvPickShader;
	RenderTarget* renderTarget;
	DepthStencil* depthStencil;
	ID3D11ShaderResourceView* preTerrainSrv;
	//Render2D* render2D;

	//uv랜더타겟에서 집어올 uv 색상
	Color PickColor;
	StructuredBuffer* computeBuffer = NULL;
};
