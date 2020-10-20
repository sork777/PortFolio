#pragma once

#include "Systems\IExecute.h"
/*
	����
	1. �ͷ���/�ͷ��ο�����
	2. �ϴ�
	3. deffered�� gbuffer �� csm
	4. frustum�� ������
	5. ���� �Ŵ���
	6. ������ �Ŵ���

	�÷���/���� ��� ����
	���� ��ü�� Save/Load�� �ļ�����
	���� ���� Save/Load�� �� �ڷ�.

*/

// 1015 �갡 ��� �������� CS�𵨷����� ����
// 1016 ���۵�� ������̱淡 ���۵� ���� ������;
class Actor;

class LevelEditorDemo :public IExecute
{
private:
	// IExecute��(��) ���� ��ӵ�
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
