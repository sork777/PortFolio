#pragma once

#include "AnimData.h"

class ModelAnimator //: public Model
{
public:
	ModelAnimator(const ModelAnimator& animator);
	ModelAnimator(Model* model);
	~ModelAnimator();

private:
	void Initialize();

public:
	void Update();
	void Render();

	void Pass(const UINT& pass) { model->Pass(pass); }
	void Tech(const UINT& tech) { model->Tech(tech); }

	void SetShader(Shader* shader);

	Model* GetModel() { return model; }

	void CloneClips(const vector<ModelClip *>& oClips);
public:
	ID3D11ShaderResourceView* GetClipTransformSrv() { return clipSrv; }
	
public:	
	Matrix GetboneWorld(const UINT& instance, const UINT& boneIndex) ;

public:	
	void AddClip(const wstring& file, const wstring& directoryPath = L"../../_Models/");
	void ReadClip(const wstring& file, const wstring& directoryPath = L"../../_Models/");
	void SaveChangedClip(const UINT& clip, const wstring& file, const wstring& directoryPath = L"../../_Models/", bool bOverwrite=false);

	void AddSocket(const int& parentBoneIndex, const wstring& bonename = L"");

public:
	void SetAnimState(const AnimationState& state = AnimationState::Stop,const UINT& instance = 0);
	void PlayAnim(const UINT& instance = 0);
	void PlayClip(const UINT& instance, const UINT& clip, float speed = 1.0f, float takeTime = 1.0f);
	UINT GetCurrClip(const UINT& instance);
	void SetFrame(const UINT& instance, int frameNum);
	UINT GetFrameCount(const UINT& instance);
	const UINT& GetCurrFrame(const UINT& instance)		{ return tweenDesc[instance].Curr.CurrFrame; }
	const TweenDesc& GetCurrTween(const UINT& instance)	{ return tweenDesc[instance]; }

public:
	const UINT& ClipCount()			{ return clips.size(); }
	vector<ModelClip *>& Clips()	{ return clips; }
	ModelClip* ClipByName(const wstring& name);
	ModelClip* ClipByIndex(const UINT& index)	{ return clips[index]; }

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
			Transform = new Matrix*[MAX_ANIM_KEYFRAMES];

			for(UINT i = 0; i < MAX_ANIM_KEYFRAMES; i++)
				Transform[i] = new Matrix[MAX_BONE_TRANSFORMS];
		}

		~ClipTransform()
		{
			for(UINT i = 0; i < MAX_ANIM_KEYFRAMES; i++)
				SafeDeleteArray(Transform[i]);

			SafeDeleteArray(Transform);
		}
	};
	ClipTransform* clipTransforms = NULL;

	vector<ModelClip *> clips;
private:
	TweenDesc tweenDesc[MAX_MODEL_INSTANCE];
	ConstantBuffer* frameBuffer = NULL;
	ID3DX11EffectConstantBuffer* sFrameBuffer;

private:
	Shader* shader;
	Model* model;

	ID3D11Texture2D* clipTextureArray = NULL;
	ID3D11ShaderResourceView* clipSrv = NULL;
	ID3DX11EffectShaderResourceVariable* sTransformsSRV;

	float time = 0.0f;
///////////////////////////////////////////////////////////////////////////////
// CS ¿µ¿ª
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