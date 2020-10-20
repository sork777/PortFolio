#pragma once
#include "Systems\IExecute.h"

// CS랜더 방식 실험작, 
// 1019 해결, 애니메이터에서 쉐이더 제대로 안잡아줬었음.

class ModelPreCsRenderDemo :public IExecute
{
public:
	// IExecute을(를) 통해 상속됨
	virtual void Initialize() override;
	virtual void Ready() override;
	virtual void Destroy() override;
	virtual void Update() override;
	virtual void PreRender() override;
	virtual void Render() override;
	virtual void PostRender() override;
	virtual void ResizeScreen() override;

private:
	void CreateBaseActor();

private:
	void SetObjectTech(const UINT& mesh, const UINT& model, const UINT& anim);
	void SetObjectPass(const UINT& mesh, const UINT& model, const UINT& anim);
	void SceneRender();

private:
	Shader* skyShader;
	Shader* terrainShader;
	Shader* shader;
	class TerrainLod* terrain;
	class Atmosphere* sky;
	GBuffer* gBuffer;
	class SSAO* ssao;
	//class CSM* shadow;
	class TerrainEditor* terrainEditor;

	Material* terrainMat;

	TextureCube* cubeTex;
	bool bEditMode = false;
	bool bActorSpwan = false;
	class ActorManager* actorMgr;

	RenderTarget* renderTarget;
	DepthStencil* depthStencil;
	Render2D* render2D;

};

