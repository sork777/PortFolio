#pragma once
#include "Framework.h"
//#include "IComponent.h"

enum class BaseCompType
{
	Renderer,
	Transform,
	Material,
	PerFrame,
};

class BaseComponent
{
	virtual void Update() = 0;
	virtual void Render() = 0; 
	virtual bool Property() = 0;
	virtual void Redo() = 0;
	virtual void Undo() = 0;

	virtual void SetShader(Shader* shader) = 0;
	//virtual Shader* GetShader() = 0;

protected:
	Shader* shader;
	BaseCompType type;
};