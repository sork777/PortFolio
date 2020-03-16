#include "Framework.h"
#include "AnimationComponent.h"



AnimationComponent::AnimationComponent(ModelAnimator* animator)
	:animator(animator)
{
	type = ObjectBaseComponentType::Animation;

}


AnimationComponent::~AnimationComponent()
{
}

void AnimationComponent::Update()
{
	animator->Update();
}

void AnimationComponent::Render()
{
	animator->Render();
}

bool AnimationComponent::Property(const UINT& instance)
{
	return false;
}

inline void AnimationComponent::PlayAnim(const UINT & instance)
{
	if (animator != NULL)
		animator->PlayAnim(instance);
}

inline void AnimationComponent::PlayClip(const UINT & instance, const UINT & clip, const float & speed, const float & takeTime)
{
	if (animator != NULL)
		animator->PlayClip(instance, clip, speed, takeTime);
}
