#pragma once
#include "Systems/IExecute.h"

class FogDemo :public IExecute
{
public:
	// IExecute을(를) 통해 상속됨
	virtual void Initialize() override;
	virtual void Ready() override {}
	virtual void Destroy() override;
	virtual void Update() override;
	virtual void PreRender() override;
	virtual void Render() override;
	virtual void PostRender() override {}
	virtual void ResizeScreen() override {}

private:
	void Mesh();
	void SceneRender();
	
private:
	Shader* shader;
	UINT tech = 1;
	UINT pass = 0;
	class Sky*	sky;

	Material* floor;
	Material* stone;
	Material* brick;
	Material* wall;

	MeshRender* sphere;
	MeshRender* cylinder;
	MeshRender* cube;
	MeshRender* grid;
	MeshRender* quad;

	Texture* brdfLut;
private:
	RenderTarget * target[2];

	DepthStencil* depthStencil;
	Viewport* viewport;

	class Fog* fog;
	Render2D* render2D;
	Render2D* finalRender2D;
};

