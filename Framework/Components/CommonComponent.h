#pragma once
#include "IComponent.h"

/*
	Object�� �⺻������ �־�� �� ������Ʈ��
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
	// IComponent��(��) ���� ��ӵ�
	virtual void Update() override {}
	virtual void Render() override {}
	virtual bool Property() abstract;

	virtual void SetShader(Shader* shader) abstract;

protected:
	Shader* shader;
	CommonComponentType type;
};