#pragma once
#include "IObjectManager.h"

/*
	����
	1. �������� ���Ϳ� ����
	2. Ư�� ������ �ͷ��� �����Ϳ� ����
	2-1. ������ �������� �Ŵ��� 1���� ����������.
	3. ������ �̵� ���� �Լ� �ʿ�.
*/
class Actor;
class TerrainLod;

class ActorManager:public IObjectManager
{
public:
	ActorManager();
	~ActorManager();

	// IObjectManager��(��) ���� ��ӵ�
	virtual void Initialize() override;
	virtual void Destroy() override;

	virtual void Update() override;
	virtual void PreRender() override;
	virtual void Render() override;


	void Tech(const UINT& mesh, const UINT& model, const UINT& anim);
	void Pass(const UINT& mesh, const UINT& model, const UINT& anim);

	Actor* RegistActor(Actor* actor);
	void SetSpawnPosition(const Vector3& position);


public:
	void OpenActorEditor();
	virtual void ObjectSpawn() override;
	virtual const bool & ObjectIcon() override;
	
	//������ �ִ³༮ �ް� ������ ���

private:
	bool	bSpawningObject;
	bool	bOpenActorEditor;
	int		spawnInstance;
	Vector3 spawnPos;

private:
	virtual const bool& AddInstanceData() override;
	virtual const bool& DelInstanceData(const UINT & instance) override;

private:
	//ã�� ����(n(0)) ������
	unordered_map<UINT, Actor*> actorMap;
	// ��ȸ ���� �ϱ� ����, ��ư�ʿ��� ���� �ʿ�
	vector< Actor*> actorList;
	vector< Actor*> readytoCreateList;
	Actor* selectedActor;
//////////////////////////////////////////////////////////////////////
// ��ư ����
//////////////////////////////////////////////////////////////////////
public:
	void CreatActorButtonImage(Actor* actor);

private:
	class Orbit* orbitCam;
	RenderTarget* renderTarget;
	DepthStencil* depthStencil;
	unordered_map<Actor*, ID3D11ShaderResourceView*> actorSrvMap;
	ActorEditor* actorEditor;

//////////////////////////////////////////////////////////////////////
// �ͷ��� ����
//////////////////////////////////////////////////////////////////////
	TerrainLod* terrain = NULL;
public:
	void SetTerrain(TerrainLod* CurrentTerrain) { terrain = CurrentTerrain; }

};

