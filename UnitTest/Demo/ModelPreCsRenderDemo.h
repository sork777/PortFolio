#pragma once
#include "Systems\IExecute.h"

// CS랜더 방식 실험작, 
// Actor 문제 없음
// ActorEditro 문제 없음
// 결론 시부레 ActorManager가 문제다.
class ModelPreCsRenderDemo :public IExecute
{
public:
	ModelPreCsRenderDemo();
	~ModelPreCsRenderDemo();

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
	//GBuffer* gBuffer;
	//class SSAO* ssao;
	//class CSM* shadow;
	class TerrainEditor* terrainEditor;

	Material* terrainMat;

	TextureCube* cubeTex;
	bool bEditMode = true;

private:
	vector<class Actor*> actors;
	unordered_map< class Actor*, UINT> actorInstMap;
	ActorEditor* actorEditor;

};

