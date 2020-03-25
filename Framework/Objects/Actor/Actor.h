#pragma once

#include "../BaseObject.h"

/*
	�ϴ� �� ������ ���⿡ �����ڰ� ���߿� ��������..

	���� ��ġ���� �ϰ� ���� �ִϸ��̼� �����͸� ������Ʈ ����� �����Ұ�.

*/
class Actor :public BaseObject
{
public:
	Actor(ModelMeshComponent* actorRoot);
	~Actor();

	// BaseObject��(��) ���� ��ӵ�
	virtual void Initailize() override;
	virtual void Destroy() override;
	virtual void Update() override;
	virtual void PreRender() override;
	virtual void Render() override;
	virtual void PostRender() override;

public:
	void ShowCompHeirarchy(OUT ObjectBaseComponent** selectedComp);
	//�ۿ��� �ͷ��� ��ǥ������ �޾Ƽ� ����.
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

