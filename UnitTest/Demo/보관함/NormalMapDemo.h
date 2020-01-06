#pragma once

#include "Systems\IExecute.h"

class NormalMapDemo :public IExecute
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
	void PointLightArea(int index);
	void SpotLightArea(int index);
	void CapsuleLightArea(int index);

	void PointLightController(int index);
	void SpotLightController(int index);
	void CapsuleLightController(int index);
private:
	UINT meshPass = 0;
	UINT modelPass = 0;

	Shader * shader;

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

	Texture* PLIcon;
	Texture* SLIcon;
	Texture* CLIcon;
};
