#pragma once
#include "Objects/BaseObject.h"
/*
	얘를 상속받아 Actor 오브젝트를 따로 만들어서 쓸것.
	애니메이션 설정들을 전부 직접 작성.

	TODO: 0902 rootMesh 기반으로 메시 통일 필요함
	component 업데이트시 상속 위치 찾는데 너무 자원이 많이 듬.
	TODO: 0910 컨트롤러 관련 함수 or 클래스 필요
	현재 레벨 관련 정보를 받아 이동할때 쓸 함수 필요
*/

class Actor :public BaseObject
{
public:
	Actor();
	Actor(Model& model);
	Actor(const Actor& actor);
	virtual ~Actor();

	// BaseObject을(를) 통해 상속됨
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
	//밖에서 터레인 좌표같은걸 받아서 오기.

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