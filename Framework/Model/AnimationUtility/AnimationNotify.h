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
Notify : Ʈ���ŷ� ���.
NotifyState : ����, ����, ��.

�ش� Ŭ������ ��ӹ޴� ���� Ŭ�������� �ִϸ����� ������ ���� �޾� �ش� ���ǿ��� Ư�� �ൿ ����

���� ���� ����
1. �ݶ��̴��� on/off
2. ��ƼŬ �ý��� ������� ��ƼŬ ���
3. �ִϸ��̼� Ŭ�� ��ȯ.

3������ �� �� �ְ� �����

��� ����
1. ���� frame.
2. �� frame.

�Լ� -notify
1. notify(trigger)

-notifystate
1. start
2. tick
3. end

*/