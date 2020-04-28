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
	//TODO: 나중에 모델 분리 하면 수정?
	void Render(const int& drawCount = -1);

	void Pass(const UINT& pass) { model->Pass(pass); }
	void Tech(const UINT& tech) { model->Tech(tech); }

	void SetShader(Shader* shader);

	void AnimatorClone(ModelAnimator* clone);
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
	void SetFrame(const UINT& instance, int frameNum);
	UINT GetFrameCount(const UINT& instance);
	inline const UINT& GetCurrFrame(const UINT& instance)		{ return tweenDesc[instance].Curr.CurrFrame; }
	inline const TweenDesc& GetCurrTween(const UINT& instance)	{ return tweenDesc[instance]; }

public:
	const UINT& ClipCount()			{ return clips.size(); }
	vector<ModelClip *>& Clips()	{ return clips; }
	ModelClip* ClipByName(const wstring& name);
	inline ModelClip* ClipByIndex(const UINT& index)	{ return clips[index]; }

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


///////////////////////////////////////////////////////////////////////////////
// CS 영역
private:
	struct CS_OutputDesc
	{
		Matrix Result;
	};

private:
	Shader* computeShader;
	StructuredBuffer* computeBuffer = NULL;

	CS_OutputDesc* csOutput = NULL;
	CS_OutputDesc* csResult = NULL;
	
	ID3DX11EffectConstantBuffer* sComputeFrameBuffer;

	ID3DX11EffectUnorderedAccessViewVariable* sUav;

};