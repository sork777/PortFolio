#pragma once


class Terrain : public Renderer
{
public:
	typedef VertexTextureColorNormalTangent TerrainVertex;

	Terrain(Shader* shader, wstring heightMap);
	virtual ~Terrain();


	void Update();
	void Render();

	void BaseMap(wstring file);

	void LayerMap(wstring layer, wstring alpha);
	void LayerMaps(wstring layer, UINT layerIndex);
	void SetLayer(UINT layerIndex) { this->layerIndex = layerIndex; }

	void AlphaMap(wstring alpha);
	void LoadAlphaMap();
	wstring SaveTerrain();
public:
	float GetHeight(Vector3& position, Vector3& pUp);
	float GetPickedHeight(Vector3& position);
	Vector3 GetPickedPosition();
	Vector2 GetSize() { return Vector2(width, height); }

	UINT GetBrushType() { return brushDesc.Type; }
	float GetBrushRange() { return brushDesc.Range; }

	void RaiseHeightQuad(Vector3& position, UINT type, UINT range);
	void RaiseHeightCircle(Vector3& position, UINT type, UINT range, float rfactor);
	
	void Splatting(Vector3& position, UINT range, float gfactor);	
private:
	void CreateVertexData();
	void CreateIndexData();
	void CreateNormalData();

	//CB는 이런식으로 만들거임
private:
	struct BrushDesc
	{
		Color Color = D3DXCOLOR(0, 1, 0, 1);	//이름이 겹쳐서 바꿔줌
		Vector3 Location;
		UINT Type = 0;	//0이면 브러쉬 안그림
		UINT Range = 1;	//브러쉬의 거리

		float Padding[3];
	}brushDesc;

	struct LineColorDesc
	{
		Color Color = D3DXCOLOR(0, 1, 0, 1);	//이름이 겹쳐서 바꿔줌
		UINT Visible = 1;
		float Thickness = 0.1f;
		float Size = 1;

		float Padding;
	}lineColorDesc;

private:
	Texture* heightMap;

	UINT width, height;
	//읽어들인 이미지의 최소 높이로 빼서 알파값이 전부 1이어도 받게
	float minHeight;
	Vector2 spacing;		//텍스쳐의 간격

	TerrainVertex* vertices;
	UINT* indices;
	Texture* baseMap;
	ID3DX11EffectShaderResourceVariable* sBaseMap;

	UINT layerIndex;
	//Shader* layerShaders[3];
	Texture* layerMaps[3];
	Texture* layerMap;
	ID3D11ShaderResourceView* layerViews[3] = { 0 };
	ID3DX11EffectShaderResourceVariable* sLayerMap;
	ID3DX11EffectShaderResourceVariable* sLayerMaps;

	Texture* alphaMap;
	ID3DX11EffectShaderResourceVariable* sAlphaMap;

	ConstantBuffer* brushBuffer;
	ID3DX11EffectConstantBuffer* sBrushBuffer;
	ConstantBuffer* lineColorBuffer;
	ID3DX11EffectConstantBuffer* sLineColorBuffer;


	/* CS용 */
private:
	Shader* CS_shader;

	struct InputDesc
	{
		UINT Index;
		Vector3 V0;
		Vector3 V1;
		Vector3 V2;

	};

	struct OutputDesc
	{
		UINT Picked;
		float U;
		float V;
		float Distance;
	};

	InputDesc* input;
	OutputDesc* output;
	StructuredBuffer* buffer;
	UINT size;
};

