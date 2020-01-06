#pragma once

#include "Systems\IExecute.h"
#define MAX_KEYFRAMES 500

class AnimationEditor :public IExecute
{
private:
	// IExecute��(��) ���� ��ӵ�
	virtual void Initialize() override;
	virtual void Destroy() override;
	virtual void Update() override;
	virtual void Render() override;
	virtual void Ready() override {};
	virtual void PreRender() override;
	virtual void PostRender() override;
	virtual void ResizeScreen() override {};

private:
	void ImGUIController();
	void AnimationController();
	
private:
	void SelectedPartsViewer();
	void ModelsViewer();
	void PartsViewer();
	void ChildViewer(ModelBone * bone);

private:
	void Popup();
	void AddSocket();
	void LoadModel();
	void AddAnimation();

	void ImportModel(wstring path=L"");
private:

	struct Keyframe
	{
		vector<Transform*> partsTrans;
	};

	struct ModelAnimData
	{
		wstring ModelName;
		ModelAnimator* animator;
		vector<ModelBone*> boneNames;
		vector<Keyframe> keyframes;
		ModelAnimData()
		{
			ModelName = L"";
			animator = NULL;
		}
	};
private:
	UINT meshPass = 0;
	UINT modelPass = 0;

	Shader * shader;
	class Sky* sky;
	Material* floor;

	vector<ModelAnimData*> ModelList;
	//ModelRender* SelectedModel;
	//ModelAnimator* SelectedModel;
	vector<string> ClipNames;
private:
	int selectedFrame = 0;
	int selectedModel = 0;
	int selected = 0;
	bool bPlay = false;

	Transform* selectedTransform = NULL;
	UINT clip = 0;
	UINT instance = 0;
	float takeTime = 1.0f;

private:
	vector<class IWidget*> widgets;
	vector<Collider*> Colliders;

};

