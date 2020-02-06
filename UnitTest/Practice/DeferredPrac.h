#pragma once

#include "Systems\IExecute.h"

class DeferredPrac :public IExecute
{
private:


	// IExecute을(를) 통해 상속됨
	virtual void Initialize() override;
	virtual void Destroy() override {};
	virtual void Update() override;
	virtual void Render() override;
	virtual void Ready() override {};
	virtual void PreRender() override;
	virtual void PostRender() override;
	virtual void ResizeScreen() override {};

private:
	void SetGBuffer();

	void SetShadow(UINT tech);
private:
	Shader * skyShader;
	Shader* terrainShader;
	class TerrainLod* terrain;
	class Atmosphere* sky;
	class FFTOcean* ocean;
	UINT meshPass = 0;
	UINT modelPass = 1;
	UINT animPass = 2;
	UINT terrainPass = 3;

	class CSM* shadow;
	class GBuffer* gBuffer;
	class SSAO* ssao;
	class SSLR* sslr;
	class HDR_Tone* hdr;

	Shader * shader;

	Lights::PointLight* pointLights;
	Lights::SpotLight* spotLights;
	Lights::CapsuleLight* capLights;

	Material* floor;
	Material* stone;
	Material* brick;
	Material* wall;

	MeshRender* sphere;
	MeshRender* cylinder;
	MeshRender* cube;
	MeshRender* grid;
	MeshRender* quad;

	Model* model;
	ModelRender* modelRender;
	ModelAnimator* modelAnim;


	float app_time = 0;
};
