#pragma once

#include "../BaseObject.h"

/*
	일단 얘 관련은 여기에 때려박고 나중에 수정하자..

	액터 배치까지 하고 나면 애니메이션 에디터를 컴포넌트 연계로 수정할것.

*/
class Actor :public BaseObject
{
public:
	Actor(ModelMeshComponent* actorRoot);
	~Actor();

	// BaseObject을(를) 통해 상속됨
	virtual void Initailize() override;
	virtual void Destroy() override;
	virtual void Update() override;
	virtual void PreRender() override;
	virtual void Render() override;
	virtual void PostRender() override;

public:
	void ShowCompHeirarchy(OUT ObjectBaseComponent** selectedComp);
	//밖에서 터레인 좌표같은걸 받아서 오기.
	void SetSpawnPosition(const Vector3& position);

public:
	Transform* GetTransform(const UINT& instance = 0);
	void AddInstanceData();
	void DelInstanceData(const UINT& instance);

	inline ModelMeshComponent* GetRootMeshData() { return root; }
private:
	ModelMeshComponent* root;

	bool bSpawningObject = false;
	int spawnInstance = -1;
	Vector3 spawnPos = Vector3(-1, -1, -1);
};

