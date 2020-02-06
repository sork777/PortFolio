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
class ModelAnimator //: public Model
{
public:
	ModelAnimator(Model* model);
	~ModelAnimator();

	void Update();
	void Render();

	void Pass(UINT pass) { model->Pass(pass); }
	void Tech(UINT tech) { model->Tech(tech); }

	void ChangeModel(Model* newmodel) { model = newmodel; }
public:
	ID3D11ShaderResourceView* GetClipTransformSrv() { return clipSrv; }
	Model* GetModel() { return model; }
	
public:	
	Matrix GetboneWorld(UINT instance, UINT boneIndex) ;

public:	
	void AddClip(wstring file, wstring directoryPath = L"../../_Models/");
	void ReadClip(wstring file, wstring directoryPath = L"../../_Models/");
	void SaveChangedClip(UINT clip, wstring file, wstring directoryPath = L"../../_Models/", bool bOverwrite=false);

	void AddSocket(int parentBoneIndex, wstring bonename = L"");

public:
	void PlayAnim(UINT instance = 0);
	void PlayClip(UINT instance, UINT clip, float speed = 1.0f, float takeTime = 1.0f);
	UINT GetCurrClip(UINT instance);
	UINT GetCurrFrame(UINT instance)		{ return tweenDesc[instance].Curr.CurrFrame; }
	TweenDesc GetCurrTween(UINT instance)	{ return tweenDesc[instance]; }
	void SetFrame(UINT instance, int frameNum);
	UINT GetFrameCount(UINT instance);

public:
	UINT ClipCount()					{ return clips.size(); }
	vector<ModelClip *>& Clips()		{ return clips; }
	ModelClip* ClipByIndex(UINT index)	{ return clips[index]; }
	ModelClip* ClipByName(wstring name);

private:
	void CreateTexture();
	void CreateClipTransform(UINT index);
	void CreateNoClipTransform(UINT index);
	void CreateComputeDesc();

private:
	struct ClipTransform
	{
		Matrix** Transform;

		ClipTransform()
		{
			Transform = new Matrix*[MAX_MODEL_KEYFRAMES];

			for (UINT i = 0; i < MAX_MODEL_KEYFRAMES; i++)
				Transform[i] = new Matrix[MAX_MODEL_TRANSFORMS];
		}

		~ClipTransform()
		{
			for (UINT i = 0; i < MAX_MODEL_KEYFRAMES; i++)
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

	ID3D11Texture2D* clipTexture = NULL;
	//vector<ID3D11Texture2D*> clipTextures;
	ID3D11ShaderResourceView* clipSrv;
	//vector<ID3D11ShaderResourceView*> clipSrvs;
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
	void UpdateBoneTransform(UINT part, UINT clipID, Transform* transform);
	void UpdateChildBones(UINT parentID, UINT childID, UINT clipID);
	ID3D11ShaderResourceView* GetEditSrv() { return editSrv; }

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