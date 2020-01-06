#pragma once

#include "Systems\IExecute.h"

class TerrainLodDemo :public IExecute
{
private:


	// IExecute을(를) 통해 상속됨
	virtual void Initialize() override;
	virtual void Destroy() override;
	virtual void Update() override;
	virtual void Render() override;
	virtual void Ready() override {};
	virtual void PreRender() override;
	virtual void PostRender() override {};
	virtual void ResizeScreen() override {};

private:
	void SetGBuffer();
private:
	Shader* skyShader;
	Shader* terrainShader;
	class TerrainLod* terrain;
	class CSM* shadow;
	class Atmosphere* sky;
	class SSAO* ssao;
	GBuffer* gBuffer;

	Camera* camera;
	Perspective* perspective;
	Frustum* frustum;

	Shader* shader;
	

	Material* terrainMat;
};
