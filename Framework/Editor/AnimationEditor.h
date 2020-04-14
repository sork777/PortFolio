#pragma once
#include "IEditor.h"

/*
	모델 에셋 목록에서 모델을 선택하자
	걔에 대한 본 정보,
	어태치 모델,
	마테리얼 정보,
	트랜스폼 정보,
	애니메이션 정보,
	등만을 가짐

	따로 임포트나 파일시스템 관련은 X
	TODO: 저장 정보는?
*/

class AnimationEditor :public IEditor
{
public:
	AnimationEditor();
	~AnimationEditor();

	// IEditor을(를) 통해 상속됨
	virtual void Initialize() override;
	virtual void Destroy() override;

	virtual void Update() override;
	virtual void PreRender() override;
	virtual void Render() override;
	virtual void PostRender() override;

private:
	void Pass(UINT mesh, UINT render, UINT anim);

	void Animate();
private:
	void ModelController();
	void AnimationController();

private:
	void SelectedBoneViewer();
	void BoneHierarchy();

	void BoneHierarchy_Popup();
	void AddSocket();

private:
	void ModelsViewer();
	void ModelAttach();

	void AttachesViewer();
//TODO: 밖에서 불러온 액터를 대상으로 작업하도록 변경 필요.
private:
	void SaveAnimationClip(wstring path = L"");
	void AddAnimationClip(wstring path=L"");

	void LoadModel(wstring path = L"");

private:
	enum class AnimationState : UINT
	{
		Stop = 0,
		Play,
		Pause
	} state = AnimationState::Stop;

	struct ModelAnimData
	{
		wstring ModelName;
		ModelAnimator* animator;
		vector<ModelBone*> boneNames;
		vector<Transform*> boneTrans;
		ModelAnimData()
		{
			ModelName = L"";
			animator = NULL;
		}
		~ModelAnimData()
		{
			SafeDelete(animator);
			boneNames.clear();
			boneNames.shrink_to_fit();
			boneTrans.clear();
			boneTrans.shrink_to_fit();
		}
	};
private:
	enum class AttachType
	{
		None,
		Render,
		Animator
	};
	enum class GizmoType
	{
		None,
		Bone,
		Model,
		Attach
	};
	struct Attach
	{
		int			bone		= -1;
		OBBCollider* collider	= NULL;
		AttachType	type		= AttachType::None;
		union AttachModel
		{
			ModelAnimator*	animator;
			ModelRender*	render;
		}attach;
	};

	vector<Attach*>		attaches;
	int selectedAttach	= 0;
	GizmoType gizmoType = GizmoType::None;
private:
	int selectedFrame	= 0;
	int selectedModel	= 0;
	int selectedBone	= 0;

	UINT	clip		= 0;
	float	takeTime	= 1.0f;
	bool	bPlay		= false;
private:
	Shader*				shader;	
	
	int 				curIndex;
	ModelAnimator*		curAnimator;
	Model*				curModel;
	vector<ModelBone*>	curBoneNames;
	vector<Transform*>	curBoneTrans;
	
	//실질적으로 만드는것은 액터오브젝트에서 추가할것.
	//vector<Model*>		prevModel;
	vector<ModelAnimData*>	prevAnims;
	vector<ModelRender*>	prevRenders;

private:
	class Sky*	sky;
	Material*	floor;
	MeshRender* grid;

	wstring textureDir	= L"../../_Textures/Model/";
	wstring modelDir	= L"../../_Models/";

private:
	Model*			newModel	= NULL;
	ModelRender*	newRender	= NULL;
	ModelAnimator*	newAnimator	= NULL;
};
