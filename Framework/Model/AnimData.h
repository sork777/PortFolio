#pragma once
#include "Framework.h"

enum class AnimationState : UINT
{
	Stop = 0,
	Play,
	Pause
};

struct KeyframeDesc
{
	int Clip = 0;

	UINT CurrFrame = 0;
	UINT NextFrame = 0;

	float Time = 0.0f;
	float RunningTime = 0.0f;

	float Speed = 1.0f;

	Vector2 Padding;
}; //keyframeDesc;
struct TweenDesc
{
	float TakeTime = 1.0f;
	float TweenTime = 0.0f;
	float RunningTime = 0.0f;
	//float Padding;
	AnimationState state = AnimationState::Play;

	KeyframeDesc Curr;
	KeyframeDesc Next;

	TweenDesc()
	{
		Curr.Clip = 0;
		Next.Clip = -1;
	}
};