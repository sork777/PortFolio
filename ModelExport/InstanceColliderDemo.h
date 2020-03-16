#pragma once
#include "Systems/IExecute.h"

class InstanceColliderDemo : public IExecute 
{
public:
	virtual void Initialize() override;
	virtual void Ready() override {}
	virtual void Destroy() override;
	virtual void Update() override;
	virtual void PreRender() override {}
	virtual void Render() override;
	virtual void PostRender() override {}
	virtual void ResizeScreen() override {}

private:
	void Pass(UINT mesh, UINT model, UINT anim);
	
	void Mesh();
private:
	void ImGUIController();
	void AnimationController();

	void AddClip(wstring name);
private:
	void SelectedPartsViewer();
	void ModelsViewer();

	void PartsViewer();
	void ChildViewer(ModelBone * bone);
	
private:
	void Popup();
	void AddSocket();

	void ModelLoad();
	void AddAnimation();
	void ImportModel(wstring path=L"");
private:
	
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
	};
	vector<ModelBone*> boneNames;
	vector<Transform*> boneTrans;

	vector<ModelAnimData*> ModelList;
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
	Shader* shader;

	class Sky* sky;

	Material* floor;
	MeshRender* grid;

	ModelRender* sword = NULL;
	ModelAnimator* kachujin = NULL;
	ModelAnimator* megan = NULL;
	ModelAnimator* mutant = NULL;
	Model* model=NULL;
	Model* model2=NULL;

	vector<Model*> testModels;
private:
	struct ColliderDesc
	{
		Transform* Init;
		Transform* Transform;
		OBBCollider* Collider;
	} colliders[4];
	
private:
	vector<MeshRender *> meshes;
	vector<ModelRender *> models;
	vector<ModelAnimator *> animators;

	vector<ModelRender *> attachModels;
	vector<ModelAnimator *> attachAnimators;

	vector<OBBCollider*> Colliders;
	vector<class IWidget*> widgets;

	TrailRenderer* trail;
	ModelIllusion* illusion;
};