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
#include "Model/AnimData.h"


class ModelMeshComponent :public ObjectBaseComponent
{
	void ClonningComp(const ModelMeshComponent& oComp);
public:
	ModelMeshComponent(Model* model);
	//��������� - ���� ����
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

