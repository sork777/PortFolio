#pragma once

#include "Systems\IExecute.h"

class CSDemo :public IExecute
{
private:


	// IExecute을(를) 통해 상속됨
	virtual void Initialize() override;
	virtual void Destroy() override {};
	virtual void Update() override {};
	virtual void Render() override {};
	virtual void Ready() override {};
	virtual void PreRender() override {};
	virtual void PostRender() override {};
	virtual void ResizeScreen() override {};

private:
	void CS_Raw();
	void CS_Structured();
	void CS_Intersection();
};
