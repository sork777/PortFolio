#pragma once

/*
	ModelAnimator���� �����ð���.
	������� animator�� �޾Ƽ� ������ ������Ʈ�� ����.

	�ϴ� ��
	1. �ִϸ��̼� ���
	2. �ִϸ��̼� �����ӿ� ���� �̺�Ʈ
	��.

	�ʿ� �Ѱ�
	1. Ŭ�� ���
	2. ������ ƽ Ȯ��	>> ��Ƽ����?	
*/

class AnimationComponent :public ObjectBaseComponent
{
public:
	AnimationComponent(ModelAnimator* animator);
	~AnimationComponent();

	// ObjectBaseComponent��(��) ���� ��ӵ�
	virtual void Update() override;
	virtual void Render() override;
	virtual bool Property(const UINT& instance) override;

	inline void PlayAnim(const UINT& instance = 0);
	inline void PlayClip(const UINT& instance, const UINT& clip, const float& speed = 1.0f, const float& takeTime = 1.0f);

private:
	ModelMeshComponent* mesh;
	ModelAnimator* animator;
};

