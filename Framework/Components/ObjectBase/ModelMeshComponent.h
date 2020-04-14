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
	virtual bool Property(const UINT& instance = 0) override;

	virtual void Tech(const UINT& mesh, const UINT& model, const UINT& anim) override;
	virtual void Pass(const UINT& mesh, const UINT& model, const UINT& anim) override;

	virtual void SetShader(Shader* shader) override;

	virtual void AddInstanceData() override;
	virtual void DelInstanceData(const UINT& instance) override;
	virtual const UINT& GetInstSize() override;

	virtual Transform* GetTransform(const UINT& instance = 0) override;

public:
	inline Model* GetMesh()				 { return skeletonMesh; }
	inline ModelAnimator* GetAnimation() { return animation; }
	inline ModelRender* GetRender()		 { return meshRender; }
	
private:
	Model* skeletonMesh;
	ModelRender* meshRender;
	ModelAnimator* animation;
	//AnimationComponent* animation;

};

