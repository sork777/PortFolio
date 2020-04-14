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


	void Tech(const UINT& mesh, const UINT& model, const UINT& anim);
	void Pass(const UINT& mesh, const UINT& model, const UINT& anim);

public:
	Transform* GetTransform(const UINT& instance = 0);
	void AddInstanceData();
	void DelInstanceData(const UINT& instance);

public:
	void SetEditMode(const bool& bEdit) { root->SetEditMode(bEdit); }

///////////////////////////////////////////////////////////////////////////////
public:
	inline ModelMeshComponent* GetRootMeshData() { return root; }

public:
	void ShowCompHeirarchy(OUT ObjectBaseComponent** selectedComp);
	//�ۿ��� �ͷ��� ��ǥ������ �޾Ƽ� ����.
	void SetSpawnPosition(const Vector3& position);

private:
	ModelMeshComponent* root;

	bool	bSpawningObject;
	int		spawnInstance;
	Vector3 spawnPos;
};

