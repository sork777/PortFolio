#pragma once

#include "../BaseObject.h"

/*
	�긦 ��ӹ޾� Actor ������Ʈ�� ���� ���� ����.
	�ִϸ��̼� �������� ���� ���� �ۼ�.
*/
class Actor :public BaseObject
{
	float time = 0.0f;
	UINT count = 0;
public:
	Actor();
	Actor(Model& model);
	Actor(const Actor& actor);
	virtual ~Actor();

	// BaseObject��(��) ���� ��ӵ�
	virtual void Initailize() override;
	virtual void Destroy() override;
	virtual void Update() override;
	virtual void PreRender() override;
	virtual void Render() override;
	virtual void PostRender() override;

	void SetShader(Shader* shader) { root->SetShader(shader); }
	void Tech(const UINT& mesh, const UINT& model, const UINT& anim);
	void Pass(const UINT& mesh, const UINT& model, const UINT& anim);
	
public:
	Transform* GetTransform(const UINT& instance = 0);
	void AddInstanceData();
	void DelInstanceData(const UINT& instance);

public:
	void ActorCompile(const Actor& editActor);
	
///////////////////////////////////////////////////////////////////////////////
public:
	void SetRootComponent(ModelMeshComponent* actorRoot);
	inline ModelMeshComponent* GetRootMeshData() { return root; }

public:
	void ShowCompHeirarchy(OUT ObjectBaseComponent** selectedComp);
	//�ۿ��� �ͷ��� ��ǥ������ �޾Ƽ� ����.
	void SetSpawnPosition(const Vector3& position);

private:
	ModelMeshComponent* root;

	bool	bEditMode = false;
	bool	bSpawningObject;
	int		spawnInstance;
	Vector3 spawnPos;
};

