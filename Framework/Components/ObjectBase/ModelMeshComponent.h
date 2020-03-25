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

class ModelMeshComponent :public ObjectBaseComponent
{
public:
	ModelMeshComponent(Model* model);
	~ModelMeshComponent();

	virtual void Update() override;
	virtual void Render() override;
	virtual bool Property() override;
	virtual Transform* GetTransform(const UINT& instance = 0) override;

	virtual void Tech(const UINT& mesh, const UINT& model, const UINT& anim) override;
	virtual void Pass(const UINT& mesh, const UINT& model, const UINT& anim) override;

	virtual void SetShader(Shader* shader) override;

	virtual void AddInstanceData() override;
	virtual void DelInstanceData(const UINT& instance) override;

public:
	//inline virtual const UINT& GetInstSize() override { return skeletonMesh->GetInstSize(); }
	inline ModelAnimator* GetAnimation() { return animation; }
	inline ModelRender* GetRender() { return meshRender; }
	inline Model* GetMesh() { return skeletonMesh; }
	
	inline void PlayAnim(const UINT& instance = 0);
	inline void PlayClip(const UINT& instance, const UINT& clip, const float& speed = 1.0f, const float& takeTime = 1.0f);


private:
	Model* skeletonMesh;
	ModelRender* meshRender;
	ModelAnimator* animation;
	//AnimationComponent* animation;

};

