#pragma once
#include "../ContentsAsset.h"

class ModelMeshComponent;
/*
	�� ���� ����
	0903 �𵨸޽� ������Ʈ�� �ѱ涧
		 �ִϸ��̼� Ŭ���鵵 ���� �ѱ⵵�� ����
	0904 ��ư �̹��� ���� ����
		 �� �ڵ� �����ϸ�

	TODO: ���� ����/�ҷ�����
*/
class ModelAsset : public ContentsAsset
{
public:
	ModelAsset(Model* model);
	~ModelAsset();

	// ContentsAsset��(��) ���� ��ӵ�
	virtual void CreateButtonImage() override;
	virtual void SaveAssetContents() override;
	virtual void LoadAssetContents() override;

	const bool& HasAnim() { return bHasAnim; }
	void SetClip(const wstring& file, const wstring& directoryPath = L"../../_Models/");

	ModelMeshComponent* GetModelMeshCompFromModelAsset();

private:
	bool bHasAnim = false;

private:
	Shader* shader;
	Model* model;
	ModelRender* modelRender;
	ModelAnimator* modelAnimation;
	
	// ��ư �̹����� ���� ��.
	Vector3 mouseVal = Vector3(Math::ToRadian(180.0f),Math::ToRadian(45.0f), 0);
	class Orbit* orbitCam;
	RenderTarget* renderTarget;
	DepthStencil* depthStencil;
};