#pragma once
#include "../ContentsAsset.h"

class ModelMeshComponent;
/*
	모델 메시 관련
	애니메이션은 다른 에셋에.
*/
class ModelAsset : public ContentsAsset
{
public:
	ModelAsset(Model* model);
	~ModelAsset();

	// ContentsAsset을(를) 통해 상속됨
	virtual void CreateButtonImage() override;
	virtual void SaveAssetContents() override;
	virtual void LoadAssetContents() override;

	ModelMeshComponent* GetModelMeshCompFromModelAsset();
private:
	//class Sky* sky;
	Model* model;
	ModelRender* modelRender;
	ModelAnimator* modelAnimation;
	// 버튼 이미지를 위한 것.
	RenderTarget* renderTarget;
	DepthStencil* depthStencil;
};