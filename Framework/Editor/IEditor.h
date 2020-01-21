#pragma once

class IEditor
{
public:
	virtual void Initialize()=0;
	virtual void Destroy() = 0;

	virtual void Update() = 0;
	virtual void PreRender() = 0;
	virtual void Render() = 0;
	virtual void PostRender() = 0;
/*
protected:
	virtual void Inspector() = 0;*/
};
