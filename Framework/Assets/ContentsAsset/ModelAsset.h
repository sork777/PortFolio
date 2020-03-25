#pragma once
#include "../ContentsAsset.h"

class ModelMeshComponent;
/*
	�� �޽� ����
	�ִϸ��̼��� �ٸ� ���¿�.
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

	ModelMeshComponent* GetModelMeshCompFromModelAsset();
private:
	//class Sky* sky;
	Model* model;
	ModelRender* modelRender;
	ModelAnimator* modelAnimation;
	// ��ư �̹����� ���� ��.
	RenderTarget* renderTarget;
	DepthStencil* depthStencil;
};