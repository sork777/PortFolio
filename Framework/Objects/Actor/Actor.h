#pragma once
#include "Objects/BaseObject.h"
/*
	�긦 ��ӹ޾� Actor ������Ʈ�� ���� ���� ����.
	�ִϸ��̼� �������� ���� ���� �ۼ�.

	TODO: 0902 rootMesh ������� �޽� ���� �ʿ���
	component ������Ʈ�� ��� ��ġ ã�µ� �ʹ� �ڿ��� ���� ��.
	TODO: 0910 ��Ʈ�ѷ� ���� �Լ� or Ŭ���� �ʿ�
	���� ���� ���� ������ �޾� �̵��Ҷ� �� �Լ� �ʿ�
*/

class Actor :public BaseObject
{
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
	virtual const bool& ObjectProperty() override;

	virtual void ObjectArrangementAtTerrain(TerrainLod* CurrentTerrain) override;

	void SetShader(Shader* shader) { root->SetShader(shader); }
	Shader* GetShader() { return root->GetMesh()->GetShader(); }
	void Tech(const UINT& mesh, const UINT& model, const UINT& anim);
	void Pass(const UINT& mesh, const UINT& model, const UINT& anim);
	
public:
	const UINT& GetInstSize() { return root->GetInstSize(); }
	Transform* GetTransform(const UINT& instance = 0);
	const bool& AddInstanceData();
	const bool& DelInstanceData(const UINT& instance);

public:
	void ActorCompile(const Actor& editActor);
	void ActorSyncBaseTransform() { root->SyncBaseTransform(); }
///////////////////////////////////////////////////////////////////////////////
public:
	void SetRootComponent(ModelMeshComponent* actorRoot);
	ModelMeshComponent* GetRootMeshData() { return root; }

public:
	void ShowCompHeirarchy(OUT ObjectBaseComponent** selectedComp);
	//�ۿ��� �ͷ��� ��ǥ������ �޾Ƽ� ����.

private:
	ModelMeshComponent* root;

public:
	void EditModeOn() { bEditMode = true; }
	const bool& IsEditMode() { return bEditMode; }

	void ToReMakeIcon(const bool& bReMake) { bReMakeIcon = bReMake; }
	const bool& IsIconReMake() { return bReMakeIcon; }
private:
	bool bReMakeIcon	= true;
	bool bEditMode	= false;

};