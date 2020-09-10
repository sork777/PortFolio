#include "Framework.h"
#include "ActorManager.h"
#include "Viewer/Orbit.h"



ActorManager::ActorManager()
{
	Initialize();
}


ActorManager::~ActorManager()
{
	Destroy();
}

void ActorManager::Initialize()
{
	selectedActor = NULL;
	bSpawningObject = false;
	bOpenActorEditor = false;
	spawnInstance = -1;
	spawnPos = Vector3(-1, -1, -1);

	float width = D3D::Width();
	float height = D3D::Height();

	renderTarget = new RenderTarget((UINT)width, (UINT)height);
	depthStencil = new DepthStencil(width, height);

	Vector3 mouseVal = Vector3(Math::ToRadian(150.0f), Math::ToRadian(45.0f), 0);
	orbitCam = new Orbit(30.0f, 30.0f, 30.0f);
	
	orbitCam->CameraMove(mouseVal);
	orbitCam->Update();

	actorEditor = new ActorEditor();

}

void ActorManager::Destroy()
{
	unordered_map<UINT, Actor*>::iterator iter = actorMap.begin();
	for (; iter != actorMap.end(); iter++)
	{
		UINT objID = iter->first;
		Actor* actor = actorMap[objID];
		SafeRelease(actorSrvMap[actor]);
		SafeDelete(actor);
	}
	actorSrvMap.clear();
	actorMap.clear();

	actorList.clear();
	actorList.shrink_to_fit();
}

void ActorManager::Update()
{
	for (Actor* actor : actorList)
	{
		actor->Update();
		//에디터에서 변경하면 Icon도 바꿔줘야 해서...
		if (true == actor->IsIconReMake())
			readytoCreateList.emplace_back(actor);
	}

	if (NULL != terrain)
	{
		if (true == terrain->IsChangedHeightMap())
		{
			for (Actor* actor : actorList)
				actor->ObjectArrangementAtTerrain(terrain);
		}
	}
	ObjectSpawn();
	OpenActorEditor();
}

void ActorManager::PreRender()
{
	for (Actor* actor : actorList)
		actor->PreRender();
}

void ActorManager::Render()
{
	for (Actor* actor : actorList)
		actor->Render();

	//액터가 등록 되는대로 여기서 한번씩 찍고 나갈거라 생각됨
	if (readytoCreateList.size() > 0)
	{
		for (Actor* actor : readytoCreateList)
		{
			CreatActorButtonImage(actor);
			actor->ToReMakeIcon(false);
		}
		//어차피 오면 다 빼냄
		readytoCreateList.clear();
	}
}

void ActorManager::Tech(const UINT & mesh, const UINT & model, const UINT & anim)
{
	for (Actor* actor : actorList)
		actor->Tech(mesh, model, anim);
}

void ActorManager::Pass(const UINT & mesh, const UINT & model, const UINT & anim)
{
	for (Actor* actor : actorList)
		actor->Pass(mesh, model, anim);
}

void ActorManager::OpenActorEditor()
{
	if (true == bSpawningObject) return;
	if (false == bOpenActorEditor) return;
	if (NULL == selectedActor) return;
	
	bOpenActorEditor = actorEditor->IsOpenedEditor();
	actorEditor->PreRender();
	actorEditor->Update();
	actorEditor->Render();
}

void ActorManager::ObjectSpawn()
{
	if (true == bOpenActorEditor) return;
	if (false == bSpawningObject) return;
	if (NULL == selectedActor) return;

	//스폰한 인스턴스를 갱신하고 있는 스폰 좌표로 이동중
	selectedActor->GetTransform(spawnInstance)->Position(spawnPos);
	//놓으면 끝 그리고 초기화
	if (Mouse::Get()->Down(0))
	{
		selectedActor = NULL;
		bSpawningObject = false;
		spawnInstance = -1;
		spawnPos = Vector3(-1, -1, -1);
	}

}

Actor * ActorManager::RegistActor(Actor * actor)
{
	UINT objID = actor->GetID();
	//없으면 추가
	if (actorMap.count(objID) < 1)
	{
		actorMap[objID] = actor;
		actorList.emplace_back(actor);
		//readytoCreateList.emplace_back(actor);
		//버튼용 srv
		actorSrvMap[actor] = NULL;
		return actor;
	}
	return actorMap[objID];
}

void ActorManager::SetSpawnPosition(const Vector3 & position)
{
	if (true == bSpawningObject)
		spawnPos = position;
}

const bool& ActorManager::AddInstanceData()
{
	return selectedActor->AddInstanceData();
}

const bool& ActorManager::DelInstanceData(const UINT & instance)
{
	
	return selectedActor->DelInstanceData(instance);
}


//TODO : 0910 마지막 액터의 RTV가 덮어씌우는 상황 배제해야함
void ActorManager::CreatActorButtonImage(Actor* actor)
{
	actor->ActorSyncBaseTransform();
	if (actorSrvMap.count(actor) < 1)
		RegistActor(actor);
	Actor* tempActor = new Actor(*actor);
	tempActor->SetShader(SETSHADER(L"027_Animation.fx"));
	tempActor->GetTransform()->Position(Vector3(0, 0, 0));
	tempActor->GetTransform()->Rotation(Vector3(0, 0, 0));
	// 액터별 위치 조정을 위한 부분
	{
		Model* model = tempActor->GetRootMeshData()->GetMesh();
		model->Update();
		Vector3 autoCalVolume = model->GetMax() - model->GetMin();
		Vector3 scale;
		model->GetTransform()->Scale(&scale);
		Vector3 TargetPos = Vector3(0, autoCalVolume.y*scale.y*0.5f, 0);
		orbitCam->SetObjPos(TargetPos);
		orbitCam->Update();
	}
	//서브에 오빗 등록
	Context::Get()->SetSubCamera(orbitCam);
	renderTarget->Set(depthStencil->DSV(), Color(0.5f, 0.6f, 0.5f, 1.0f));
	{
		//위치 조정하고 한번 더해줘야 제대로 랜더됨...
		tempActor->Update();
		tempActor->Update();
		tempActor->Tech(1, 1, 1);
		tempActor->Pass(0, 1, 2);
		tempActor->Render();
	}
	//TODO: 0910 데이터만 복사하게 바꿔야함.
	actorSrvMap[actor] = renderTarget->SRV();

	// 컨텍스트의 서브카메라 삭제
	Context::Get()->SetMainCamera();
}

const bool & ActorManager::ObjectIcon()
{
	/*
		액터별 구성
		1. 액터 이미지
		2. 액터 스폰 설정창
		3. 액터 에디터 설정창
		하는일
		1. 액터 선택
		2. 선택시 인스턴싱 추가 가능여부에 따라 spawn 결정
		3. spawn 가능시 선택 액터에 따른 스폰 인스턴스번호 및 액터 설정		
	*/
	bool bSelected = false;
	for (Actor* actor: actorList)
	{
		ImGui::Image(actorSrvMap[actor], ImVec2(80, 80));
		//ImGui의 그룹 짝 맞추려면 끝나고 꺼야함
		bool bBreak = false;
		ImGui::SameLine();
		ImGui::BeginGroup();
		{
			string btnLabel = String::ToString(actor->GetName()) + "_Add";
			bSelected = ImGui::Button(btnLabel.c_str());
			if (bSelected == true)
			{
				if (true == actor->AddInstanceData())
				{
					spawnInstance = actor->GetInstSize() - 1;
					selectedActor = actor;
					bSpawningObject = true;
					bBreak = true;
				}
				else
					bSelected = false;
			}
			btnLabel = String::ToString(actor->GetName()) + "_Show_Editor";
			bSelected = ImGui::Button(btnLabel.c_str());
			if (bSelected == true)
			{
				actorEditor->SetActor(actor);
				selectedActor = actor;
				bOpenActorEditor = true;
				bBreak = true;
			}
			ImGui::EndGroup();
		}
		if (true == bBreak)
			break;
	}
	return bSelected;
}
