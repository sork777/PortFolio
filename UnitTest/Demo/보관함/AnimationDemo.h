#pragma once
#include "Systems\IExecute.h"

#define MAX_FRAMES 300

class AnimationDemo :public IExecute
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
	void ShowParts();
	void ShowChild(ModelBone * bone);
	/* �Ǻ� ���� ���� �Լ� */
	void Normalizer();
	void Normalizer(UINT start, UINT end);

	void SaveAnimation(wstring savePath=L"");
	void LoadAnimation(wstring file = L"");

private:
	void BoneCircle();

private:
	
	struct Keyframe
	{
		vector<Transform*> partsTrans;
		vector<Transform*> partsTransIndep;
		bool pivot = false;
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
	Gizmo* gizmo;
	MeshGrid* grid;

	Model* model;
	ModelRender* modelRender;
private:
	vector<ModelBone*> boneNames;
	vector<Keyframe> keyframes;
	int selectedFrame = 0;
	int selected = 0;

};
