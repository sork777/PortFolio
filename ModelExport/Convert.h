#pragma once
#include "Systems/IExecute.h"

class Convert : public IExecute
{
	virtual void Initialize() override;
	virtual void Ready() {}
	virtual void Destroy() override {};
	virtual void Update() override {};
	virtual void PreRender() {};
	virtual void Render() override {};
	virtual void PostRender() {};
	virtual void ResizeScreen() {};

private:
	void Airplane();
	void Tower();
	void Tank();
	void Kachujin();
	void Megan();
	void Mutant();
	void Weapon();
	void Temp();
};