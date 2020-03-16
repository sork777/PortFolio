#pragma once

struct KeyframeDesc
{
	int Clip = 0;

	UINT CurrFrame = 0;
	UINT NextFrame = 0;

	float Time = 0.0f;
	float RunningTime = 0.0f;

	float Speed = 1.0f;

	Vector2 Padding;
}; //keyframeDesc;

struct TweenDesc
{
	float TakeTime = 1.0f;
	float TweenTime = 0.0f;
	float RunningTime = 0.0f;
	float Padding;

	KeyframeDesc Curr;
	KeyframeDesc Next;

	TweenDesc()
	{
		Curr.Clip = 0;
		Next.Clip = -1;
	}
};

class ModelAnimation //: public Model
{
public:
	ModelAnimation(Model* model);
	~ModelAnimation();

	void Update();
	void Render();

	inline Model* GetModel() { return model; }
public:
	inline ID3D11ShaderResourceView* GetClipTransformSrv() { return clipSrv; }
	
public:	
	Matrix GetboneWorld(const UINT& instance, const UINT& boneIndex) ;

public:	
	void AddClip(const wstring& file, const wstring& directoryPath = L"../../_Models/");
	void ReadClip(const wstring& file, const wstring& directoryPath = L"../../_Models/");
	void SaveChangedClip(const UINT& clip, const wstring& file, const wstring& directoryPath = L"../../_Models/", bool bOverwrite=false);

	void AddSocket(const int& parentBoneIndex, const wstring& bonename = L"");

public:
	void PlayAnim(const UINT& instance = 0);
	void PlayClip(const UINT& instance, const UINT& clip, float speed = 1.0f, float takeTime = 1.0f);
	UINT GetCurrClip(const UINT& instance);
	inline const UINT& GetCurrFrame(const UINT& instance)		{ return tweenDesc[instance].Curr.CurrFrame; }
	inline const TweenDesc& GetCurrTween(const UINT& instance)	{ return tweenDesc[instance]; }
	void SetFrame(const UINT& instance, int frameNum);
	UINT GetFrameCount(const UINT& instance);

public:
	inline const UINT& ClipCount()			{ return clips.size(); }
	inline vector<ModelClip *>& Clips()		{ return clips; }
	inline ModelClip* ClipByIndex(const UINT& index)	{ return clips[index]; }
	ModelClip* ClipByName(const wstring& name);

private:
	void UpdateTextureArray();
	void CreateClipTransform(const UINT& index);
	void CreateNoClipTransform(const UINT& index);
	void CreateComputeDesc();

private:
	struct ClipTransform
	{
		Matrix** Transform;

		ClipTransform()
		{
			Transform = new Matrix*[MAX_MODEL_KEYFRAMES];

			for(UINT i = 0; i < MAX_MODEL_KEYFRAMES; i++)
				Transform[i] = new Matrix[MAX_MODEL_TRANSFORMS];
		}

		~ClipTransform()
		{
			for(UINT i = 0; i < MAX_MODEL_KEYFRAMES; i++)
				SafeDeleteArray(Transform[i]);

			SafeDeleteArray(Transform);
		}
	};
	ClipTransform* clipTransforms = NULL;

	vector<ModelClip *> clips;
private:
	TweenDesc tweenDesc[MAX_MODEL_INSTANCE];
	ConstantBuffer* frameBuffer;
	ID3DX11EffectConstantBuffer* sFrameBuffer;

private:
	Shader* shader;
	Model* model;

	ID3D11Texture2D* clipTextureArray = NULL;
	ID3D11ShaderResourceView* clipSrv;
	ID3DX11EffectShaderResourceVariable* sTransformsSRV;
private:
	struct CS_OutputDesc
	{
		Matrix Result;
	};

private:
	Shader* computeShader;
	StructuredBuffer* computeBuffer = NULL;

	CS_OutputDesc* csOutput = NULL;
	
	ID3DX11EffectConstantBuffer* sComputeFrameBuffer;

	ID3DX11EffectUnorderedAccessViewVariable* sUav;
	bool bChangeCS=true;


#pragma region 애니메이션의 변화 조정 영역
public:
	void UpdateBoneTransform(const UINT& part, const UINT& clipID, Transform* transform);
	void UpdateChildBones(const UINT& parentID, const UINT& childID, const UINT& clipID);
	inline ID3D11ShaderResourceView* GetEditSrv() { return editSrv; }

private:
	void CreateAnimEditTexture();

private:
	// 애니메이션의 글로벌 변화
	// 최종적으로 클립 저장시 곱해서 변화된 애를 저장할 것.
	ID3D11Texture2D* editTexture = NULL;
	ID3D11ShaderResourceView* editSrv;
	ID3DX11EffectShaderResourceVariable* sAnimEditSRV;

	Matrix animEditTrans[MAX_ANIMATION_CLIPS][MAX_MODEL_TRANSFORMS];
#pragma endregion
};