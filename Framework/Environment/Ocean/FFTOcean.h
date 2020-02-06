#pragma once

#include "FFTOceanSimulator.h"
#include "Utilities/QuadTree.h"

#define FRESNEL_TEX_SIZE			256
#define PERLIN_TEX_SIZE				64

struct Const_Shading
{
	// Water-reflected sky color
	Color	SkyColor;
	// The color of bottomless water body
	Color	WaterbodyColor;
	
	Color		SunColor;

	Vector3		SunDir;
	float Shineness;

	// The parameter is used for fixing an artifact
	Vector3	BendParam;	
	// Perlin noise for distant wave crest
	float PerlinSize;

	// Constants for calculating texcoord from position
	Vector3	PerlinAmplitude;
	float TexelLength_x2;
	Vector3	PerlinOctave;
	float UVScale;
	Vector3	PerlinGradient;
	float UVOffset;
};

class FFTOcean :public Renderer
{
public:
	/* 초기화 정보를 여기서 몰아서 만들거임 */
	struct InitializeInfo;
public:
	FFTOcean(InitializeInfo& info);
	~FFTOcean();

	void Update(float time);
	void Render(float time);

	void RenderFFT();
	void Property();

	Texture* HeightMap() { return heightMap; }

public:
	struct InitializeInfo
	{
		Shader* shader;

		wstring heightMap;
		float CellSpacing;
		UINT CellsPerPatch;
	};
private:
	void InitializeOcean();
	bool InBounds(UINT row, UINT col);

	void CreateVertexData();
	void CreateIndexData();

	void CreateOceanSimAndRender();
	void InitRenderResource(const OceanParameter& ocean_param);
	// create color/fresnel lookup table.
	void CreateFresnelMap();
	// create perlin noise texture for far-sight rendering
	void LoadTextures();

private:
	/*QuadTree* quadTree;
	QuadTreeNode* CreateQuadTreeData(QuadTreeNode* parent, Vector2& TopLeft, Vector2& BottomRight);*/
private:
	struct VertexOcean
	{
		Vector3 Position;
		Vector2 Uv;
	};

	struct BufferDesc
	{
		float MinDistance = 1.0f;
		float MaxDistance = 500.0f;
		float MinTessellation = 2.0f;
		float MaxTessellation = 64.0f;

		Vector2 HalfOceanSize;
		Vector2 PerlinMovement;
	} callDesc;
private:
	UINT faceCount;
	UINT patchVertexRows;
	UINT patchVertexCols;

private:
	InitializeInfo info;

	ConstantBuffer* PerCallCB;
	ID3DX11EffectConstantBuffer* sPerCallCB;

	Frustum* frustum;
	Camera* camera;
	Perspective* perspective;

	Texture* heightMap;
	ID3DX11EffectShaderResourceVariable* sHeightTexture;

	UINT width, height;

	VertexOcean* vertices;
	UINT* indices;

	vector<Vector2> bounds;


private:
	FFTOceanSimulator* oceanSimulator;

	Const_Shading shadingDesc;
		
	ConstantBuffer* ShadingCB;

	Render2D* renderSrvs[5];
private:
	
	///////////////////////////////////////////////////////////////////////////////
	// Mesh properties:
	float PatchLength;
	// Dimension of displacement map
	int DisplaceMapDim;
	///////////////////////////////////////////////////////////////////////////////
	// Shading properties:
	// Two colors for waterbody and sky color
	Color SkyColor = Color(0.38f, 0.45f, 0.56f,1.0f);
	Color WaterbodyColor = Color(0.07f, 0.15f, 0.2f, 1.0f);
	//Color SunColor = Color(0.74f, 0.59f, 0.247f, 1);
	Color SunColor = Color(1.0f, 1.0f, 0.6f, 1);
	// Blending term for sky cubemap
	float SkyBlending = 16.0f;

	// Perlin wave parameters
	float PerlinSize = 1.0f;
	float PerlinSpeed = 0.06f;
	Vector3 PerlinAmplitude = Vector3(35, 42, 57);
	Vector3 PerlinGradient = Vector3(1.4f, 1.6f, 2.2f);
	Vector3 PerlinOctave = Vector3(1.12f, 0.59f, 0.23f);
	Vector2 WindDir;

	Vector3 BendParam = Vector3(0.1f, -0.4f, 0.3f);

	// Sunspot parameters
	float Shineness = 400.0f;

private:
	// Color look up 1D texture
	ID3D11Texture1D* FresnelMap = NULL;
	ID3D11ShaderResourceView* SRV_Fresnel = NULL;
	// Distant perlin wave
	ID3D11ShaderResourceView* SRV_Perlin = NULL;
	Texture* Perlin;
	ID3D11ShaderResourceView* SRV_ReflectCube=NULL;
	Texture* ReflectionCube;

	ID3D11ShaderResourceView* SRV_Displacemnet=NULL;
	ID3D11ShaderResourceView* SRV_Gradient = NULL;

};

