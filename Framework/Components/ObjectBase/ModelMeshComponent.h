#pragma once


/*
	Model에서 가져올거임.
	모델의 애니메이션 유무에 따라 랜더 여부 정할 거임.

	하는 일
	1. 메시 출력
	2. 모델 속성 확인(마테리얼, 트랜스폼, 애니메이션 클립)
	3. 있으면 물리도.	>> Collision은 컴포넌트 줄거임.
	
	어태치를 위한 함수 필요.
	-> 모델메시가 상위 컴포넌트면 업뎃하면서 소켓에 맞게 알아서 조절할거임.
*/
#include "Model/AnimData.h"


class ModelMeshComponent :public ObjectBaseComponent
{
	void ClonningComp(const ModelMeshComponent& oComp);
public:
	ModelMeshComponent(Model* model);
	//복사생성자 - 깊은 복사
	ModelMeshComponent(const ModelMeshComponent& modelComp);
	~ModelMeshComponent();

	void CompileComponent(const ModelMeshComponent& OBComp);

	virtual void Update() override;
	virtual void Render() override;
	virtual bool Property(const int& instance = -1) override;

	void Tech(const UINT& mesh, const UINT& model, const UINT& anim) override;
	void Pass(const UINT& mesh, const UINT& model, const UINT& anim) override;

	void SetShader(Shader* shader) override;

public:
	void AddInstanceData() override;
	void DelInstanceData(const UINT& instance) override;
	
	virtual const UINT& GetInstSize() override;
	virtual Transform* GetTransform(const UINT& instance = 0) override;

	void SetAnimState(const AnimationState& state = AnimationState::Stop, const UINT& instance = 0);
	void PlayAllAnim();
	void StopAllAnim();

public:
	Model* GetMesh()				{ return skeletonMesh; }
	ModelAnimator* GetAnimation()	{ return animation; }
	ModelRender* GetRender()		{ return meshRender; }
	
private:
	Model* skeletonMesh;
	ModelRender* meshRender;
	ModelAnimator* animation;

	float time = 0.0f;
	UINT count = 0;
};

