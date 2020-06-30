#include "Framework.h"
#include "AnimationEditor.h"
#include "Environment/Sky.h"


AnimationEditor::AnimationEditor()
{
	Initialize();
}


AnimationEditor::~AnimationEditor()
{
	Destroy();
}

void AnimationEditor::Initialize()
{
	shader = SETSHADER(L"027_Animation.fx");
	sky = new Sky(L"Environment/GrassCube1024.dds");
	//Create Material
	{
		floor = new Material(shader);
		floor->LoadDiffuseMap("Floor.png");
	}

	//Create Mesh
	{
		Transform* transform = NULL;

		grid = new MeshRender(shader, new MeshGrid(5, 5));
		grid->AddInstance();
		transform = grid->GetTransform(0);
		transform->Position(0, 0, 0);
		transform->Scale(12, 1, 12);
	}
	grid->UpdateTransforms();
}

void AnimationEditor::Destroy()
{
	SafeDelete(sky);
	SafeDelete(grid);
	SafeDelete(floor);

	//meshes.clear();
	//meshes.shrink_to_fit();
	prevAnims.clear();
	prevAnims.shrink_to_fit();
	prevRenders.clear();
	prevRenders.shrink_to_fit();

	
}

void AnimationEditor::Update()
{
	sky->Update();
	grid->Update();

	

	if (curAnimator != NULL)
	{
		AnimationController();
		ModelController();
		Matrix world = curModel->GetTransform(0)->World();
		for (Attach* att : attaches)
		{
			UINT bone = att->bone;
			Matrix attach = curAnimator->GetboneWorld(0, bone);
			attach *= world;
			if (att->collider != NULL)
			{
				att->collider->GetTransform()->Parent(attach);
				att->collider->Update();
			}			
			switch (att->type)
			{
			case AttachType::Render:
				att->attach.render->GetModel()->GetTransform(0)->Parent(attach);
				att->attach.render->Update();
				break;
			case AttachType::Animator:
				att->attach.animator->GetModel()->GetTransform(0)->Parent(attach);
				att->attach.animator->Update();
				break;
			}
		}
	}
}

void AnimationEditor::PreRender()
{
}

void AnimationEditor::Render()
{
	sky->Render();

	Pass(0, 1, 2);

	floor->Render();
	grid->Render();

	//for (MeshRender* temp : meshes)
	//	temp->Render();

	ModelsViewer();

	if (curAnimator != NULL)
	{
		SelectedBoneViewer();
		AttachesViewer();		
		BoneHierarchy();
		Animate();
		curAnimator->Render();
	}
	for (Attach* temp : attaches)
	{
		switch (temp->type)
		{
		case AttachType::Render:
			temp->attach.render->Render();
			break;
		case AttachType::Animator:
			temp->attach.animator->Render();
			break;
		}
	}
}

void AnimationEditor::PostRender()
{
}

void AnimationEditor::Pass(UINT mesh, UINT render, UINT anim)
{
	
	grid->Pass(mesh);
	for (Attach* temp : attaches)
	{
		switch (temp->type)
		{
		case AttachType::Render:
			temp->attach.render->Pass(render);
			break;
		case AttachType::Animator:
			temp->attach.animator->GetModel()->Pass(anim);
			break;
		default:
			break;
		}		
	}

	if (curAnimator != NULL)
	{
		curAnimator->GetModel()->Pass(anim);
	}
}

void AnimationEditor::Animate()
{
	if (curAnimator->ClipCount() < 1)
		return;
	switch (state)
	{
	case AnimationState::Stop:
		bPlay = false;
		break;
	case AnimationState::Play:
		selectedFrame = curAnimator->GetCurrFrame(0);
		bPlay = true;
		break;
	case AnimationState::Pause:
		bPlay = false;
		break;
	default:
		break;
	}	
	if(bPlay)
		curAnimator->PlayAnim();
}

void AnimationEditor::ModelController()
{
	bool bDocking = true;
	ImGui::Begin("Animation", &bDocking);
	{
		bool bChange = false;
		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
		if (ImGui::CollapsingHeader("Materials", ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (Material* mat : curModel->Materials())
			{
				mat->Property();
				ImGui::Separator();
			}
		}

		if (ImGui::CollapsingHeader("Selected_Parts", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (selectedBone >= 0)
			{
				Transform* transform = new Transform();
				Transform* boneTransform = curBoneNames[selectedBone]->GetBoneTransform();
				//Matrix mat = curModel->BoneByIndex(selectedBone + 1)->BoneWorld();
				Matrix mat = curAnimator->GetboneWorld(0, selectedBone + 1);
				//boneTransform->Local(mat);
				//boneTransform->Update();
				transform->World(mat);
				Vector3 S, R, T;
				/* 파츠의 기본 좌표 출력 */
				{
					transform->Scale(&S);
					transform->Rotation(&R);
					transform->Position(&T);
					ImGui::Text("Scale    : <%.2f, %.2f, %.2f> ", S.x, S.y, S.z);
					ImGui::Text("Rotation : <%.2f, %.2f, %.2f> ", R.x, R.y, R.z);
					ImGui::Text("Position : <%.2f, %.2f, %.2f> ", T.x, T.y, T.z);

					ImGui::Separator();
				}
				/* 변형 */
				if (state != AnimationState::Play)
				{
					curBoneTrans[selectedBone]->Local(boneTransform->ParentTransform()->Local());
					curBoneTrans[selectedBone]->Update();

					if (gizmoType == GizmoType::Bone)
						Gui::Get()->SetGizmo(boneTransform, curModel->GetTransform(0), true);

					bChange |= curBoneTrans[selectedBone]->Property();
					curBoneTrans[selectedBone]->Position(&T);
					T.y = T.y > 0 ? T.y : 0;
					curBoneTrans[selectedBone]->Position(T);

					if (bChange)
					{
						UINT clip = curAnimator->GetCurrClip(0);
						boneTransform->ParentTransform()->Local(curBoneTrans[selectedBone]->Local());
						boneTransform->Update();
						curAnimator->GetModel()->UpdateBoneTransform(selectedBone + 1, clip, curBoneTrans[selectedBone]);
					}
				}
				/* 재생중일때 정보 표기 */
				else
				{
					ImGui::Text("BoneEditTransform");

					transform = (curBoneTrans[selectedBone]);
					transform->Scale(&S);
					transform->Rotation(&R);
					transform->Position(&T);

					ImGui::Text("Scale    : <%.2f, %.2f, %.2f> ", S.x, S.y, S.z);
					ImGui::Text("Rotation : <%.2f, %.2f, %.2f> ", R.x, R.y, R.z);
					ImGui::Text("Position : <%.2f, %.2f, %.2f> ", T.x, T.y, T.z);
				}
			}
			ImGui::Separator();
		}
		ImGui::PopStyleVar();

		

		if (ImGui::Button("Reset"))
		{
			Matrix iden;
			D3DXMatrixIdentity(&iden);
			for (UINT j = 0; j < curBoneNames.size(); j++)
			{
				curBoneNames[j]->GetBoneTransform()->ParentTransform()->Local(iden);
				curBoneNames[j]->GetBoneTransform()->Update();
				//curBoneTrans[j]->Parent(iden);
				curBoneTrans[j]->Local(iden);
				curBoneTrans[j]->Update();
				curAnimator->GetModel()->UpdateBoneTransform(j + 1, clip, curBoneTrans[j]);
			}
		}

	}
	ImGui::End();
}

void AnimationEditor::AnimationController()
{	
	bool bDocking = true;
	ImGui::Begin("Animator", &bDocking);
	{
		if (ImGui::Button("SaveClip"))
		{
			SaveAnimationClip();
		}
		ImGui::SameLine();

		if (ImGui::Button("AddAnimationClip"))
		{
			AddAnimationClip();
		}
		ImGui::Separator();

		if (curAnimator->ClipCount() > 0)
		{
			bool bChange = false;
			bChange |= ImGui::InputInt("AnimClip", (int*)&clip);
			bChange |= ImGui::InputFloat("TakeTime", &takeTime, 0.1f);
			takeTime = Math::Clamp(takeTime, 0.1f, 3.0f);
			clip %= curAnimator->ClipCount();

			if (bChange)
				curAnimator->PlayClip(0, clip, takeTime);

			curAnimator->Update();

			int frameCount = curAnimator->GetFrameCount(0) - 1;
			ImGui::SliderInt("Frames", &selectedFrame, 0, frameCount);
			selectedFrame = Math::Clamp(selectedFrame, 0, frameCount);
		}
		/* Play */
		bool playbutton = (state == AnimationState::Play);

		if (!playbutton)
		{
			curAnimator->SetFrame(0, selectedFrame);
			if (ImGui::Button("Play"))
			{
				state = AnimationState::Play;
			}
		}
		else if (ImGui::Button("Pause"))
		{
			state = AnimationState::Pause;
		}

		ImGui::SameLine();
		if (ImGui::Button("Stop"))
		{
			curAnimator->SetFrame(0, 0);
			selectedFrame = 0;
			state = AnimationState::Stop;
		}
		ImGui::SameLine();
	}
	ImGui::End();
}

void AnimationEditor::SelectedBoneViewer()
{
	if (selectedBone < 0)
		return;
	Matrix W, V, P;
	D3DXMatrixIdentity(&W);
	V = Context::Get()->View();
	P = Context::Get()->Projection();
	Vector3 textPos, pos;

	Matrix mat = curAnimator->GetboneWorld(0, selectedBone + 1);
	Matrix world = curModel->GetTransform(0)->World();
	Transform sTransform;
	mat*=world;
	sTransform.World(mat);
	sTransform.Position(&pos);
	Context::Get()->GetViewport()->Project(&textPos, pos, W, V, P);
	string str;
	str += "Part_Name :" + String::ToString(curBoneNames[selectedBone]->Name());

	Gui::Get()->RenderText(textPos.x - 50, textPos.y, str.c_str());
}

void AnimationEditor::BoneHierarchy()
{
	int click = -1;

	selectedBone = curModel->BoneHierarchy(&click) - 1;

	if (0 == click)
		gizmoType = GizmoType::Bone;
	if (1 == click)
		ImGui::OpenPopup("Bones_Popup");

	BoneHierarchy_Popup();
}

void AnimationEditor::BoneHierarchy_Popup()
{
	
	bool boo = false;
	int itemIdx = -1;
	if (ImGui::BeginPopup("Bones_Popup"))
	{
		//TODO: MenuItem은 openpopup이 안먹힘
		if (ImGui::BeginMenu("AddSocket"))
		{
			wstring str = curBoneNames[selectedBone]->Name() + L"_AddSocket";
			boo			= ImGui::MenuItem(String::ToString(str).c_str());
			itemIdx		= 0;
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("PrevAttach"))
		{
			//boo = ImGui::MenuItem("PrevAttachModel");
			ModelAttach();
			itemIdx = 1;
			ImGui::EndMenu();
		}
		ImGui::EndPopup();
	}

	// 왜인지 OpenPopup을 밖으로 빼줘야함...
	if (boo)
	{
		switch (itemIdx)
		{
		case 0:
			ImGui::OpenPopup("Add_Socket?");
			break;
		case 1:
			ImGui::OpenPopup("Select_PrevAttachModel");
			break;
		//case 2:
		//case 3:
		//	break;
		}
	}
	AddSocket();
}

void AnimationEditor::AddSocket()
{
	if (ImGui::BeginPopupModal("Add_Socket?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		static char name[64] = "NewSocket";

		ImGui::Text("SocketName:");
		ImGui::InputText("##edit", name, IM_ARRAYSIZE(name));
		if (ImGui::Button("OK", ImVec2(120, 0))) {

			curAnimator->AddSocket(selectedBone + 1, String::ToWString(name));

			UINT boneIdx = curModel->BoneCount() - 1;
			ModelBone* bone = curModel->BoneByIndex(boneIdx);
			curBoneNames.emplace_back(bone);

			curBoneTrans.emplace_back(new Transform());

			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void AnimationEditor::SaveAnimationClip(wstring path)
{
	if (path.length() < 1)
	{
		function<void(wstring)> f = bind(&AnimationEditor::SaveAnimationClip, this, placeholders::_1);
		Path::OpenFileDialog(L"", L"CLIP\0*.clip", L"../../_Models/", f);
	}
	else
	{
		wstring dir = Path::GetDirectoryName(path);
		path = Path::GetFileNameWithoutExtension(path);
		curAnimator->SaveChangedClip(clip, path, dir, false);
	}
}

void AnimationEditor::AddAnimationClip(wstring path)
{
	if (path.length() < 1)
	{
		function<void(wstring)> f = bind(&AnimationEditor::AddAnimationClip, this, placeholders::_1);
		Path::OpenFileDialog(L"", L"CLIP\0*.clip", L"../../_Models/", f);
	}
	else
	{
		wstring dir = Path::GetDirectoryName(path);
		path = Path::GetFileNameWithoutExtension(path);
		curAnimator->AddClip(path, dir);
	}
}

void AnimationEditor::LoadModel(wstring path)
{
	if(path.length()<1)
	{ 
		function<void(wstring)> f = bind(&AnimationEditor::LoadModel, this, placeholders::_1);
		//추출할때 material-mesh는 한쌍 필수니까 마테리얼만 불러보자.
		Path::OpenFileDialog(L"", Path::ConvModelDataFilter, L"", f);
	}
	else
	{
		wstring modelFilePath	= Path::GetFileNameWithoutExtension(path);
		wstring modelDirPath	= Path::GetDirectDirectoryName(path); 
		modelFilePath = modelDirPath + L"/" + modelFilePath;
		
		//파일을 읽고 모델 인스턴스 생성
		newModel = new Model(shader);
		//modelDir = Path::GetDirectoryName(Path::GetDirectoryName(path));
		// 일단 설정된 디렉토리에서 가져옴
		newModel->ReadMaterial(modelFilePath, textureDir);
		newModel->ReadMesh(modelFilePath, modelDir);
		newModel->AddInstance();
		//모델이 애니메이션인지 아닌지에 따라 애니메이션이나 랜더 선택
		if (newModel->IsAnimationModel())
		{
			newAnimator = new ModelAnimator(newModel);
			newAnimator->ReadClip(modelFilePath, modelDir);
			newAnimator->Update();
			newAnimator->Render();
			ModelAnimData* data		= new ModelAnimData();
			{
				vector<ModelBone*> boneNames;
				vector<Transform*> boneTrans;
				for (ModelBone* bone : newModel->Bones())
				{
					if (bone->Index() < 0)continue;
					boneNames.emplace_back(bone);
				}

				for (UINT j = 0; j < boneNames.size(); j++)
				{
					boneTrans.emplace_back(new Transform());
				}
				data->animator	= newAnimator;
				data->ModelName = newModel->Name();
				data->boneTrans = boneTrans;
				data->boneNames = boneNames;
			}
			//로드한 애니메이션 모델이 없으면 로드 즉시 cur객체로
			if (prevAnims.size() < 1)
			{
				curIndex		= 0;
				curModel		= newModel;
				curAnimator		= data->animator;
				curBoneNames	= data->boneNames;
				curBoneTrans	= data->boneTrans;
			}
			prevAnims.emplace_back(data);
		}
		else
		{
			ModelRender* newRender = new ModelRender(newModel);
			newRender->Render();
			prevRenders.emplace_back(newRender);
		}
	}
}

void AnimationEditor::ModelsViewer()
{
	bool bDocking = true;
	ImGui::Begin("Model_Index", &bDocking);
	{
		if (ImGui::Button("LoadModel"))
		{
			LoadModel();
		}
		ImGui::Text("Animations");
		
		for (UINT i = 0; i < prevAnims.size(); i++)
		{
			ImVec2 pos		= ImGui::GetItemRectMin() - ImGui::GetWindowPos();
			ImVec2 wSize	= ImGui::GetWindowSize();
			if (wSize.y < pos.y) continue;
			if (wSize.x < pos.x) continue;

			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_DefaultOpen;

			if (i == selectedModel)
				flags |= ImGuiTreeNodeFlags_Selected;
			ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 10.0f);

			if (ImGui::TreeNodeEx(String::ToString(prevAnims[i]->ModelName).c_str(), flags))
			{
				if (ImGui::IsItemClicked())
				{
					selectedModel =i;
				}
				if (ImGui::IsItemClicked(1))
				{
					selectedModel = i;
				}
				
				ImGui::TreePop();
			}
			ImGui::PopStyleVar();
		}

		if (ImGui::Button("ChangeModel"))
		{
			if (curIndex != selectedModel)
			{
				curIndex		= selectedModel;
				curAnimator		= prevAnims[selectedModel]->animator;
				curModel		= prevAnims[selectedModel]->animator->GetModel();
				curBoneNames	= prevAnims[selectedModel]->boneNames;
				curBoneTrans	= prevAnims[selectedModel]->boneTrans;
			}
		}
		ImGui::Separator();
		//랜더들은 목록에서선택 안할거다.
		ImGui::Text("Renders");
		for (UINT i = 0; i < prevRenders.size(); i++)
		{
			ImVec2 pos		= ImGui::GetItemRectMin() - ImGui::GetWindowPos();
			ImVec2 wSize	= ImGui::GetWindowSize();
			if (wSize.y < pos.y) continue;
			if (wSize.x < pos.x) continue;

			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_DefaultOpen;
						
			ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 10.0f);

			if (ImGui::TreeNodeEx(String::ToString(prevRenders[i]->GetModel()->Name()).c_str(), flags))
			{				
				ImGui::TreePop();
			}
			ImGui::PopStyleVar();
		}

		ImGui::Separator();
		if (curModel != NULL)
		{
			curModel->GetTransform(0)->Property();
		}
	}
	ImGui::End();
}

void AnimationEditor::ModelAttach()
{
	{
		ImGui::Text("prevAttach");
		int selAttachAnim = -1;
		int selAttachRender = -1;
		
		for (UINT i = 0; i < prevAnims.size(); i++)
		{
			//현재 모델하고 같으면 패스
			if (i == curIndex)				continue;

			ImVec2 pos = ImGui::GetItemRectMin() - ImGui::GetWindowPos();
			ImVec2 wSize = ImGui::GetWindowSize();
			if (wSize.y < pos.y) continue;
			if (wSize.x < pos.x) continue;

			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_DefaultOpen;

			ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 10.0f);

			if (ImGui::TreeNodeEx(String::ToString(prevAnims[i]->ModelName).c_str(), flags))
			{
				if (ImGui::IsItemClicked())
				{
					selAttachAnim = i;
				}
				ImGui::TreePop();
			}
			ImGui::PopStyleVar();
		}
		ImGui::Separator();		
		ImGui::Text("Renders");
		for (UINT i = 0; i < prevRenders.size(); i++)
		{
			ImVec2 pos = ImGui::GetItemRectMin() - ImGui::GetWindowPos();
			ImVec2 wSize = ImGui::GetWindowSize();
			if (wSize.y < pos.y) continue;
			if (wSize.x < pos.x) continue;

			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_DefaultOpen;

			ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 10.0f);

			if (ImGui::TreeNodeEx(String::ToString(prevRenders[i]->GetModel()->Name()).c_str(), flags))
			{
				if (ImGui::IsItemClicked())
				{
					selAttachRender = i;
				}
				ImGui::TreePop();
			}
			ImGui::PopStyleVar();
		}

		Attach* newattach	= new Attach();
		//루트본을 제외한 인덱스라 +1
		newattach->bone		= selectedBone + 1;
		if (selAttachAnim > -1)
		{
			newattach->type				= AttachType::Animator;
			newattach->attach.animator	= prevAnims[selAttachAnim]->animator;
			attaches.emplace_back(newattach);
		}
		else if (selAttachRender > -1)
		{
			newattach->type				= AttachType::Render;
			newattach->attach.render	= prevRenders[selAttachRender];
			attaches.emplace_back(newattach);
		}
		//선택 없으면 객체 해제
		else
			SafeDelete(newattach);		
		//ImGui::EndPopup();
	}
}

void AnimationEditor::AttachesViewer()
{
	bool bDocking = true;
	ImGui::Begin("Attach_Index", &bDocking);
	{
		if (attaches.size() > 0)
		{
			Transform* attachTrans=NULL;
			
			switch (attaches[selectedAttach]->type)
			{
			case AttachType::Render:
				attachTrans = attaches[selectedAttach]->attach.render->GetModel()->GetTransform(0);
				break;
			case AttachType::Animator:
				attachTrans = attaches[selectedAttach]->attach.animator->GetModel()->GetTransform(0);
				break;
			}
			if (attachTrans != NULL)
			{
				attachTrans->Property();
				if (gizmoType == GizmoType::Attach)
				{
					Gui::Get()->SetGizmo(attachTrans);
				}
			}
		}
		ImGui::Separator();
		ImGui::Text("Attachs");
		for (UINT i = 0; i < attaches.size(); i++)
		{
			ImVec2 pos = ImGui::GetItemRectMin() - ImGui::GetWindowPos();
			ImVec2 wSize = ImGui::GetWindowSize();
			if (wSize.y < pos.y) continue;
			if (wSize.x < pos.x) continue;

			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_DefaultOpen;

			if (i == selectedAttach)
				flags |= ImGuiTreeNodeFlags_Selected;
			ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 10.0f);

			string str = "";
			string typeInfo = "";
			switch (attaches[i]->type)
			{
				case AttachType::Render:
					str = String::ToString(attaches[i]->attach.render->GetModel()->Name());
					typeInfo = "Render";
					break;
				case AttachType::Animator:
					str = String::ToString(attaches[i]->attach.animator->GetModel()->Name());
					typeInfo = "Animator";
					break;
			}			
			if (ImGui::TreeNodeEx(str.c_str(), flags))
			{
				if (ImGui::IsItemClicked())
				{
					selectedAttach = i;
					gizmoType = GizmoType::Attach;
				}
				ImGui::TreePop();
			}
			ImGui::SameLine(120);
			ImGui::LabelText("##AttachType",typeInfo.c_str());
			ImGui::PopStyleVar();
		}
	}
	ImGui::End();
}
