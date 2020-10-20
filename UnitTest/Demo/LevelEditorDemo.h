#pragma once

#include "Systems\IExecute.h"
/*
	구성
	1. 터레인/터레인에디터
	2. 하늘
	3. deffered용 gbuffer 와 csm
	4. frustum용 데이터
	5. 액터 매니저
	6. 빌보드 매니저

	플레이/에딧 모드 구분
	레벨 자체의 Save/Load는 후순위로
	액터 구성 Save/Load도 좀 뒤로.

*/

// 1015 얘가 어디서 꼬였는지 CS모델랜더가 꼬임
// 1016 디퍼드랑 뭔상관이길래 디퍼드 쓰면 꼬이지;
class Actor;

class LevelEditorDemo :public IExecute
{
private:
	// IExecute을(를) 통해 상속됨
	virtual void Initialize() override;
	virtual void Destroy() override;
	virtual void Update() override;
	virtual void Ready() override {};
	virtual void PreRender() override;
	virtual void Render() override;
	virtual void PostRender() override {};
	virtual void ResizeScreen() override {};

private:
	//void SetGBuffer();
	void CreateBaseActor();

private:
	void SetObjectTech(const UINT& mesh, const UINT& model, const UINT& anim);
	void SetObjectPass(const UINT& mesh, const UINT& model, const UINT& anim);
	void SceneRender();

private:

	void PlayModeUpdate();
	void EditModeUpdate();
	void PlayModePreRender();
	void EditModePreRender();
	void PlayModeRender();
	void EditModeRender();

private:	
	Shader* skyShader;
	Shader* terrainShader;
	Shader* shader;
	class TerrainLod* terrain;
	class Atmosphere* sky;
	class SSAO* ssao;
	class CSM* shadow;
	GBuffer* gBuffer;

	Camera* camera;
	Perspective* perspective;
	Frustum* frustum;
	
	Material* terrainMat;

	TextureCube* cubeTex;

private:
	class ActorManager* actorMgr;
	class TerrainEditor* terrainEditor;

private:
	bool bEditMode = true;
	bool bOpenActorEditor = false;
	bool bActorSpwan = false;
};
