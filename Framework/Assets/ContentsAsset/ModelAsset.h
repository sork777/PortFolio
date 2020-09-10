#pragma once
#include "../ContentsAsset.h"

class ModelMeshComponent;
/*
	모델 관련 에셋
	0903 모델메시 컴포넌트를 넘길때
		 애니메이션 클립들도 같이 넘기도록 수정
	0904 버튼 이미지 관련 정리
		 모델 자동 스케일링

	TODO: 에셋 저장/불러오기
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
	
	// 버튼 이미지를 위한 것.
	Vector3 mouseVal = Vector3(Math::ToRadian(180.0f),Math::ToRadian(45.0f), 0);
	class Orbit* orbitCam;
	RenderTarget* renderTarget;
	DepthStencil* depthStencil;
};