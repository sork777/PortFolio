#pragma once
#include "Systems\IExecute.h"

class PBRDemo :public IExecute
{
public:
	// IExecute을(를) 통해 상속됨
	virtual void Initialize() override;
	virtual void Ready() override {};
	virtual void Destroy() override;
	virtual void Update() override;
	virtual void PreRender() override {};
	virtual void Render() override;
	virtual void PostRender() override {};
	virtual void ResizeScreen() override {};

private:
	Shader * skyShader;
	class Sky* sky;

private:
	Shader * shader;
	UINT meshPass = 0;

	Texture* brdfLut;
	MaterialPBR* materials[9];
	MeshRender* sphere;
	MeshRender* spheres[9];

	Material*	floor;
	MeshRender* grid;
};

