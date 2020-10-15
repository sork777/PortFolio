#pragma once

#include "AnimData.h"
/*
	1. 기본적으로 Animation을 가진 모델은 
	Vertex에 있는 indice,weights 정보를 가지고
	Texture로 기록한 BoneTransform을 계산해서 얻는다.
	
	2. Model에 직접 Attach하지 않고 계산 줄여 상속할 방법은?
	직접 Attach해도 Mesh가 분할되어있어 랜더 자체는 달리한다.
	BoneTexture에 기록되는 형태만 달라질뿐.
	
	3. 필요한것
	1) 부모의 framebuffer용 함수가 필요.
	2) 부모Bone과 AnimClip을 가지고 만든 Texture가 필요함.
	2-2) RenderingPipeLine을 통해 계산할 최종정보를 CS에서 계산하면?
	2-3) MainMesh의 계산된 Inst별 BoneTransform을 저장할 Texture2D 데이터면 될것.
	3) Vertex정보는 결국 본의 최종정보를 가지고 가중치 계산한것임, 본은 선계산 해도 될듯

	4. 구현(수정) 해볼것
	1) CS에서 나오는 RWTexture2D 데이터 구성은
	1-1) Width : Bone , Height : Instance 
	1-2) 각 Inst별 Frame과 Clip에 따른 본의 최종 계산값
	2) Rendering시 계산요소
	2-1) Cs에서 계산한 Srv값에 각 버텍스의 가중치 계산


	
*/
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
	//CsCopy 최소화를 위한 분리
	void ReadyforGetBoneworld(const UINT& boneIndex);
	Matrix GetboneWorld(const UINT& instance) ;

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
	UINT maxAnimFrame = 0;
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

	CsTexture* testCSTex = NULL;
};