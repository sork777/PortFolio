#pragma once

#include "Systems\IExecute.h"

class ThreadDemo :public IExecute
{
private:


	// IExecute을(를) 통해 상속됨
	virtual void Initialize() override;
	virtual void Destroy() override {};
	virtual void Update() override;
	virtual void Render() override;
	virtual void Ready() override {};
	virtual void PreRender() override {};
	virtual void PostRender() override {};
	virtual void ResizeScreen() override {};

private:
	void Func();
	void Func2();
	void WhileFunc();

private:
	void Basic();
	void MultiThread();
	void MultiThreadLamda();
	void Detach();

	void Mutex();
	void Mutex_RW();

	void Time();
private:
	Timer timer;
	Timer timer2;
	Timer timer3;
	Timer timer4;
	
	bool bLoaded = false;
	ID3D11ShaderResourceView* srv;

	Render2D * render2D;

	mutex m;
	thread* t;
};
