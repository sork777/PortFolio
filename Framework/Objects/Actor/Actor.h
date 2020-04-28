#pragma once

#include "../BaseObject.h"

/*
	얘를 상속받아 Actor 오브젝트를 따로 만들어서 쓸것.
	애니메이션 설정들을 전부 직접 작성.
*/
class Actor :public BaseObject
{
public:
	Actor();
	virtual ~Actor();

	// BaseObject을(를) 통해 상속됨
	virtual void Initailize() override;
	virtual void Destroy() override;
	virtual void Update() override;
	virtual void PreRender() override;
	virtual void Render() override;
	virtual void PostRender() override;

	void Tech(const UINT& mesh, const UINT& model, const UINT& anim);
	void Pass(const UINT& mesh, const UINT& model, const UINT& anim);
	
public:
	Transform* GetTransform(const UINT& instance = 0);
	void AddInstanceData();
	void DelInstanceData(const UINT& instance);

public:
	void ActorCompile() { root->CompileComponent(); }
	void SetEditMode(const bool& bEdit) { root->SetEditMode(bEdit); }

///////////////////////////////////////////////////////////////////////////////
public:
	inline void SetRootComponent(ModelMeshComponent* actorRoot) { root = actorRoot; }
	inline ModelMeshComponent* GetRootMeshData() { return root; }

public:
	void ShowCompHeirarchy(OUT ObjectBaseComponent** selectedComp);
	//밖에서 터레인 좌표같은걸 받아서 오기.
	void SetSpawnPosition(const Vector3& position);

private:
	ModelMeshComponent* root;

	bool	bSpawningObject;
	int		spawnInstance;
	Vector3 spawnPos;
};

