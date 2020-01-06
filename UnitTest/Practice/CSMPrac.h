#pragma once

#include "Systems\IExecute.h"

class CSMPrac :public IExecute
{
private:
	// IExecute을(를) 통해 상속됨
	virtual void Initialize() override;
	virtual void Destroy() override {};
	virtual void Update() override;
	virtual void Render() override;
	virtual void Ready() override {};
	virtual void PreRender() override ;
	virtual void PostRender() override ;
	virtual void ResizeScreen() override {};

private:
	void RenderObj();
private:
	UINT meshPass = 0;
	UINT modelPass = 1;
	UINT animPass = 2;

	Shader * shader;

	class CSM* shadow;

	Render2D* depth2D;

	Material* floor;
	Material* stone;
	Material* brick;
	Material* wall;

	MeshSphere* sphere[10];
	MeshCylinder* cylinder[10];
	MeshCube* cube;
	MeshGrid* grid;

	Model* model;
	ModelRender* modelRender;
};
