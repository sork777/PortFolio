#pragma once
#include "Systems\IExecute.h"

#define MAX_FRAMES 300

class AnimationEditorDemo :public IExecute
{
private:


	// IExecute��(��) ���� ��ӵ�
	virtual void Initialize() override;
	virtual void Destroy() override;
	virtual void Update() override;
	virtual void Render() override;
	virtual void Ready() override {};
	virtual void PreRender() override {};
	virtual void PostRender() override {};
	virtual void ResizeScreen() override {};

private:
	void ImGUIController();
	void AnimationController();
	void AttachController();
	void ShowParts();
	void ShowChild(ModelBone * bone);

	void SaveAnimation(wstring savePath=L"");
	void LoadAnimation(wstring file = L"");

private:
	void BoneCircle();
	void AttachModel();
	void SelectedPartsView();
private:
	
	struct Keyframe
	{
		vector<Transform*> partsTrans;
		vector<Transform*> partsTransIndep;
	};
private:
	UINT meshPass = 0;
	UINT modelPass = 0;

	class Sky* sky;

	Shader * shader;
	Material* floor;
	
	Shader * instMeshShader;
	/* �޽� ��ŷ���� �����ϱ� ���� ���Ǿ� */
	MeshSphere* sphere;
	MeshGrid* grid;

	Model* sword;
	ModelRender* swordRender;
	Transform* trans_sword;
	Transform* trans_swordF;
	Collider* col_Sword;

	Model* model;
	ModelAnimator* modelRender;
	Collider* col_Model;

private:
	vector<ModelBone*> boneNames;
	vector<Keyframe> keyframes;
	int selectedFrame = 0;
	int selected = 0;
	bool bAttach = false;
	bool bPlay = false;

};
