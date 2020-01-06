#pragma once

class ModelAnimator : public Model
{
public:
	ModelAnimator(Shader* shader);
	~ModelAnimator();

	void Update(UINT instance=-1,bool bPlay=true);
	virtual void Render() override;

public:
	void AddClip(wstring file);
	void AddSocket(int parentBoneIndex, wstring bonename = L"");
public:
	void PlayAnim(UINT instance=-1);
	void PlayClip(UINT instance, UINT clip, float speed = 1.0f, float takeTime = 1.0f);
	UINT GetCurrClip(UINT instance) { return tweenDesc[instance].Curr.Clip; }
	UINT GetCurrFrame(UINT instance) { return tweenDesc[instance].Curr.CurrFrame; }
	void SetFrame(UINT instance, int frameNum);
	UINT GetFrameCount(UINT instance);
	
public:	
	/* 해당 인스턴스의 본의 트랜스폼 */
	virtual Matrix GetboneTransform(UINT instance, UINT boneIndex) override;
	void UpdateInstTransform(UINT instance, UINT part, Matrix trans);
	void UpdateChildTransform(UINT parentID,UINT childID,UINT clipID,UINT frameID, Matrix trans);

private:
	virtual void CreateTexture() override;
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
	ClipTransform* boneAinTransforms = NULL;
	
private:
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
	} tweenDesc[MAX_MODEL_INSTANCE];

	ConstantBuffer* frameBuffer;
	ID3DX11EffectConstantBuffer* sFrameBuffer;

private:
	struct CS_InputDesc
	{
		Matrix Bone;
	};

	struct CS_OutputDesc
	{
		Matrix Result;
	};

private:
	Shader* computeShader;
	StructuredBuffer* computeBuffer = NULL;

	CS_InputDesc* csInput = NULL;
	CS_OutputDesc* csOutput = NULL;
	
	ID3DX11EffectConstantBuffer* sComputeFrameBuffer;

	ID3DX11EffectShaderResourceVariable* sSrv;
	ID3DX11EffectUnorderedAccessViewVariable* sUav;
};