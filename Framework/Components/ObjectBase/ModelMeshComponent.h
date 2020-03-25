#pragma once


/*
	Model���� �����ð���.
	���� �ִϸ��̼� ������ ���� ���� ���� ���� ����.

	�ϴ� ��
	1. �޽� ���
	2. �� �Ӽ� Ȯ��(���׸���, Ʈ������, �ִϸ��̼� Ŭ��)
	3. ������ ������.	>> Collision�� ������Ʈ �ٰ���.
	
	����ġ�� ���� �Լ� �ʿ�.
	-> �𵨸޽ð� ���� ������Ʈ�� �����ϸ鼭 ���Ͽ� �°� �˾Ƽ� �����Ұ���.
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

