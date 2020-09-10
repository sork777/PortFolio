#pragma once
#include "IObjectManager.h"

/*
	역할
	1. 여러종의 액터와 연결
	2. 특정 레벨의 터레인 데이터와 연결
	2-1. 어차피 레벨마다 매니저 1개라 괜찮을거임.
	3. 액터의 이동 관리 함수 필요.
*/
class Actor;
class TerrainLod;

class ActorManager:public IObjectManager
{
public:
	ActorManager();
	~ActorManager();

	// IObjectManager을(를) 통해 상속됨
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
	
	//있으면 있는녀석 받고 없으면 등록

private:
	bool	bSpawningObject;
	bool	bOpenActorEditor;
	int		spawnInstance;
	Vector3 spawnPos;

private:
	virtual const bool& AddInstanceData() override;
	virtual const bool& DelInstanceData(const UINT & instance) override;

private:
	//찾기 쉽게(n(0)) 맵으로
	unordered_map<UINT, Actor*> actorMap;
	// 순회 쉽게 하기 위함, 버튼쪽에도 쓸때 필요
	vector< Actor*> actorList;
	vector< Actor*> readytoCreateList;
	Actor* selectedActor;
//////////////////////////////////////////////////////////////////////
// 버튼 생성
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
// 터레인 연결
//////////////////////////////////////////////////////////////////////
	TerrainLod* terrain = NULL;
public:
	void SetTerrain(TerrainLod* CurrentTerrain) { terrain = CurrentTerrain; }

};

