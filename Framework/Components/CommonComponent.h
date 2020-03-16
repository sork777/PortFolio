#pragma once
#include "IComponent.h"

/*
	Object에 기본적으로 있어야 할 컴포넌트들
*/
enum class CommonComponentType
{
	Renderer,
	Transform,
	Material,
	MaterialPBR,
	PerFrame,
};

class CommonComponent : public IComponent
{
public:
	//CommonComponent() = default;
	//virtual ~CommonComponent() = default;
	// IComponent을(를) 통해 상속됨
	virtual void Update() override {}
	virtual void Render() override {}
	virtual bool Property() abstract;

	virtual void SetShader(Shader* shader) abstract;

protected:
	Shader* shader;
	CommonComponentType type;
};