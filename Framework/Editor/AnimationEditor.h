#pragma once
#include "IEditor.h"

/*
	�� ���� ��Ͽ��� ���� ��������
	�¿� ���� �� ����,
	����ġ ��,
	���׸��� ����,
	Ʈ������ ����,
	�ִϸ��̼� ����,
	��� ����

	���� ����Ʈ�� ���Ͻý��� ������ X
	TODO: ���� ������?
*/

class AnimationEditor :public IEditor
{
public:
	AnimationEditor();
	~AnimationEditor();

	// IEditor��(��) ���� ��ӵ�
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
//TODO: �ۿ��� �ҷ��� ���͸� ������� �۾��ϵ��� ���� �ʿ�.
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
	
	//���������� ����°��� ���Ϳ�����Ʈ���� �߰��Ұ�.
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
