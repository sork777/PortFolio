#pragma once

#define MAX_TRAILBUFFER_COUNT 500

class TrailRenderer
{
public:
	TrailRenderer(UINT splitCount = 128);
	~TrailRenderer();

	void Initialize();
	void Update(Matrix parent);
	void Render();

	void ClearTrail(Matrix mat);
	Transform* GetTransform() { return transform; }


	Texture* TrailTexture() { return trailTexture; }
	void TrailTexture(Texture* texture) { trailTexture = texture; }
	void TrailTexture(string path, string dir = "../../_Textures/");
	void TrailTexture(wstring path, wstring dir = L"../../_Textures/");

	Texture* MaskTexture() { return maskTexture; }
	void MaskTexture(Texture* texture) { maskTexture = texture; }
	void MaskTexture(string path, string dir = "../../_Textures/");
	void MaskTexture(wstring path, wstring dir = L"../../_Textures/");

	bool Property();
private:
	void SetAndShiftTrailMatBuffer(Matrix& insertFirstMat);

private:
	struct TrailDesc
	{
		D3DXMATRIX buffer[MAX_TRAILBUFFER_COUNT];

		int TrailCount;
		float Padding[3];

		TrailDesc()
		{
			for (UINT i = 0; i < MAX_TRAILBUFFER_COUNT; i++)
				D3DXMatrixIdentity(&buffer[i]);
		}
	} trailDesc;

	vector<Matrix> mats;

private:
	Shader* shader;
	Transform* transform;
	PerFrame* perframe;

private:
	VertexBuffer* vertexBuffer = NULL;
	IndexBuffer* indexBuffer = NULL;

	VertexTexture* vertices;
	UINT* indices;

	UINT vertexCount, indexCount;

private:
	ConstantBuffer* trailBuffer;
	ID3DX11EffectConstantBuffer* sTrailBuffer;
	Texture* trailTexture;
	ID3DX11EffectShaderResourceVariable* sTrailSrv;
	Texture* maskTexture;
	ID3DX11EffectShaderResourceVariable* sMaskSrv;

private:
	bool bViewTrail;

	int boneIdx;
	UINT splitCount;
	float deltaStoreTime;

};

