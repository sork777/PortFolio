#include "Framework.h"
#include "Actor.h"

Actor::Actor()	
	:root(NULL)
{
	Initailize();
}

Actor::Actor(Model & model)
{
	root = new ModelMeshComponent(&model);
	Initailize();
	Object_Name += root->GetMesh()->Name();
}

Actor::Actor(const Actor& actor)
{
	root = new ModelMeshComponent(*(actor.root));
	root->AddInstanceData();

	Initailize();
	Object_Name += root->GetMesh()->Name();

}


Actor::~Actor()
{
	Destroy();
}

void Actor::Initailize()
{
	Object_Name = L"Actor_";
}

void Actor::Destroy()
{
	SafeDelete(root);
}

void Actor::Update()
{
	if (NULL == root) return;

	root->Update();

	
}

void Actor::PreRender()
{
}

void Actor::Render()
{
	if (NULL == root) return;
	root->Render();
}

const bool& Actor::ObjectProperty()
{
	return false;
}

void Actor::ObjectArrangementAtTerrain(TerrainLod * CurrentTerrain)
{
	int loop = root->GetInstSize();
	for (int inst = 0; inst < loop; inst++)
	{
		Vector3 pos;
		root->GetTransform(inst)->Position(&pos);
		pos.y = CurrentTerrain->GetPickedHeight(pos);
		root->GetTransform(inst)->Position(pos);
	}	
}

void Actor::Tech(const UINT & mesh, const UINT & model, const UINT & anim)
{
	if (NULL == root) return;
	root->Tech(mesh, model, anim);
}

void Actor::Pass(const UINT & mesh, const UINT & model, const UINT & anim)
{
	if (NULL == root) return;
	root->Pass(mesh, model, anim);
}

void Actor::ShowCompHeirarchy(OUT ObjectBaseComponent** selectedComp)
{
	if (NULL == root) return;
	/* 파츠 선택 */
	root->ComponentHeirarchy(selectedComp);
}


Transform * Actor::GetTransform(const UINT & instance)
{
	if (true == bEditMode) return root->GetTransform();

	return root ? root->GetTransform(instance) : NULL;
}

const bool& Actor::AddInstanceData()
{
	//에딧일때 안씀
	if (true == bEditMode)  return false;
	if (NULL == root)  return false;

	//TODO: 나중에 모델쪽으로 빼서 Add/Del관련 전부 bool 리턴으로
	if (MAX_MODEL_INSTANCE <= root->GetInstSize() + 1)
		return false;
	root->AddInstanceData();
	return true;
}

const bool& Actor::DelInstanceData(const UINT & instance)
{
	if (true == bEditMode) return false;
	if (NULL == root)  return false;
	if (instance >= root->GetInstSize()) return false;

	root->DelInstanceData(instance);
	return true;
}

void Actor::SetMainClip(const UINT & clip)
{
	int size = root->GetInstSize();
	for(int i=0;i<size;i++)
		root->GetAnimation()->PlayClip(i, clip);
}

void Actor::ActorCompile(const Actor& editActor)
{
	root->CompileComponent(*editActor.root);
}

void Actor::SetRootComponent(ModelMeshComponent * actorRoot)
{
	//에딧 모드인 액터는 이미 루트가 정해져있는애임.
	if (true == bEditMode) return;
	root = actorRoot;

	Object_Name += root->GetMesh()->Name();
}