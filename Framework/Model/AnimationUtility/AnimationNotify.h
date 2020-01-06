#pragma once

enum NotifyType :UINT
{
	Notify_None,
	Notify_Trigger,
	Notify_State
};

class AnimationNotify
{
public:
	AnimationNotify();
	~AnimationNotify();

	void Notify_Trigger();
	void Notify_Tick();
	void Notify_Start();
	void Notify_End();

private:
	NotifyType Type;
	UINT StartFrame = -1;
	UINT EndFrame = -1;
};

/*
Notify : 트리거로 사용.
NotifyState : 시작, 진행, 끝.

해당 클래스를 상속받는 전용 클래스에서 애니메이터 따위를 전달 받아 해당 조건에서 특정 행동 실행

깊이 가지 말고
1. 콜라이더의 on/off
2. 파티클 시스템 만들고나면 파티클 출력
3. 애니메이션 클립 전환.

3가지만 할 수 있게 만들기

멤버 변수
1. 시작 frame.
2. 끝 frame.

함수 -notify
1. notify(trigger)

-notifystate
1. start
2. tick
3. end

*/