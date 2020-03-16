#pragma once

/*
	ModelAnimator에서 가져올거임.
	만들어진 animator를 받아서 액터의 컴포넌트로 쓸것.

	하는 일
	1. 애니메이션 재생
	2. 애니메이션 프레임에 따른 이벤트
	끗.

	필요 한것
	1. 클립 재생
	2. 프레임 틱 확인	>> 노티파이?	
*/

class AnimationComponent :public ObjectBaseComponent
{
public:
	AnimationComponent(ModelAnimator* animator);
	~AnimationComponent();

	// ObjectBaseComponent을(를) 통해 상속됨
	virtual void Update() override;
	virtual void Render() override;
	virtual bool Property(const UINT& instance) override;

	inline void PlayAnim(const UINT& instance = 0);
	inline void PlayClip(const UINT& instance, const UINT& clip, const float& speed = 1.0f, const float& takeTime = 1.0f);

private:
	ModelMeshComponent* mesh;
	ModelAnimator* animator;
};

