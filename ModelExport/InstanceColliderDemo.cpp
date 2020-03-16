#include "stdafx.h"
#include "InstanceColliderDemo.h"
#include "Viewer/Freedom.h"
#include "Environment/Sky.h"


void InstanceColliderDemo::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(23, 0, 0);
	Context::Get()->GetCamera()->Position(0, 32, -67);
	((Freedom *)Context::Get()->GetCamera())->Speed(20, 2);


	shader = new Shader(L"027_Animation.fx");
	sky = new Sky(L"Environment/GrassCube1024.dds");
	trail = new TrailRenderer(128);
	trail->GetTransform()->Scale(10, 120, 30);
	trail->GetTransform()->RotationDegree(0, 90, 90);
	trail->GetTransform()->Position(0, 0, -60);
	Mesh();
	ModelLoad();
	for (ModelBone* bone : model->Bones())
	{
		if (bone->Index() < 0)continue;
		boneNames.emplace_back(bone);
	}
	
	for (UINT j = 0; j < boneNames.size(); j++)
	{
		boneTrans.emplace_back(new Transform());
	}
	
	Widget_Contents* wContents = new Widget_Contents();
	wContents->SetImport(bind(&InstanceColliderDemo::ImportModel,this,placeholders::_1));
	widgets.emplace_back(wContents);
	
}

void InstanceColliderDemo::Destroy()
{
	
}

void InstanceColliderDemo::Update()
{
	sky->Update();
	grid->Update();


	for (ModelRender* temp : models)
		temp->Update();
	
	//for (ModelAnimator* temp : animators)
	//	temp->Update();

	if(megan !=NULL)
		megan->Update();
	AnimationController();
	ImGUIController();
	
	for (int i = 0; i < model->GetInstSize(); i++)
	{
		int arms = kachujin->GetModel()->BoneIndexByName(L"RightHand");

		Matrix attach = kachujin->GetboneWorld(i, arms);
		Matrix world = kachujin->GetModel()->GetTransform(i)->World();
		attach *= world;
		sword->GetModel()->GetTransform(i)->Parent(attach);
		colliders[i].Collider->GetTransform()->Parent(attach);
		colliders[i].Collider->Update();
	}	
	if (illusion != NULL)
	{
		illusion->Update(kachujin->GetCurrTween(0),0.3f);
	}
}

void InstanceColliderDemo::Render()
{
	PartsViewer();
	SelectedPartsViewer();

	for (auto widget : widgets)
	{
		if (widget->IsVisible())
		{
			widget->Begin();
			widget->Render();
			widget->End();
		}
	}

	sky->Render();

	Pass(0, 1, 2);
	
	floor->Render();
	grid->Render();
	
	for (ModelRender* temp : models)
		temp->Render();

	for (ModelAnimator* temp : animators)
		temp->Render();
	if (illusion != NULL)
	{
		illusion->Render();
		ImGui::Begin("Illusion");
		{
			illusion->Property();
			ImGui::End();
		}
	}
	trail->Render();
	ImGui::Begin("Trail");
	{
		static int SwordInstance = 0;
		ImGui::InputInt("SwordInstance", (int*)&SwordInstance);
		SwordInstance %= sword->GetModel()->GetInstSize();
		trail->Update(sword->GetModel()->GetTransform(SwordInstance)->World());
		trail->Property();
		ImGui::End();
	}
	for (int i = 0; i < model->GetInstSize(); i++)
	{
		colliders[i].Collider->Render(Color(1,1,0,1));
	}
}

void InstanceColliderDemo::Pass(UINT mesh, UINT model, UINT anim)
{
	for (MeshRender* temp : meshes)
		temp->Pass(mesh);

	for (ModelRender* temp : models)
		temp->Pass(model);

	for (ModelAnimator* temp : animators)
		temp->GetModel()->Pass(anim);
	/*if(ModelList[selectedModel])
		ModelList[selectedModel]->animator->Pass(anim);*/
}

#pragma region LoadRegion

void InstanceColliderDemo::Mesh()
{
	//Create Material
	{
		floor = new Material(shader);
		floor->LoadDiffuseMap("Floor.png");
		//floor->SpecularMap("Floor_Specular.png");
		//floor->NormalMap("Floor_Normal.png");
		//floor->Specular(1, 1, 1, 20);
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

	meshes.push_back(grid);
}

void InstanceColliderDemo::ModelLoad()
{
	Model* temp = new Model(shader);
	temp->ReadMaterial(L"Weapon/Sword",  L"../../_Textures/Model/");
	temp->ReadMesh(L"Weapon/Sword", L"../../_Models/");
	temp->AddInstance();
	temp->AddInstance();
	temp->AddInstance();
	temp->GetTransform(0)->RotationDegree(0, 0, 90);
	temp->GetTransform(0)->Position(-10, -5, -15); ;
	temp->GetTransform(1)->RotationDegree(0, 0, 90);
	temp->GetTransform(1)->Position(-10, -5, -15); ;
	temp->GetTransform(2)->RotationDegree(0, 0, 90);
	temp->GetTransform(2)->Position(-10, -5, -15); ;
	sword = new ModelRender(temp);
	models.emplace_back(sword);
	testModels.emplace_back(temp);
	
	model = new Model(shader);
	model->ReadMaterial(L"Kachujin/Mesh", L"../../_Textures/Model/");
	model->ReadMesh(L"Kachujin/Mesh", L"../../_Models/");
	testModels.emplace_back(model);

	kachujin = new ModelAnimator(model);
	kachujin->ReadClip(L"Kachujin/Mesh", L"../../_Models/");
	//kachujin->ReadClip(L"Megan/Taunt", L"../../_Models/");
	//kachujin->ReadClip(L"Megan/Dancing");
	kachujin->ReadClip(L"Kachujin/Idle", L"../../_Models/");
	kachujin->ReadClip(L"Kachujin/S_M_H_Attack", L"../../_Models/");
	kachujin->ReadClip(L"Kachujin/Running", L"../../_Models/");
	kachujin->ReadClip(L"Kachujin/Jump", L"../../_Models/");

	/*if(arms<0)
		kachujin->Attach((Model*)sword, 11, sword->GetInstSize());
	else
		kachujin->Attach((Model*)sword, arms, sword->GetInstSize());
*/
	

	Transform* transform = NULL;
	model->AddInstance();
	transform = model->GetTransform(0);
	transform->Position(-25, 0, 0);
	transform->Scale(0.075f, 0.075f, 0.075f);
	kachujin->PlayClip(0, 0, 1.0f);

	model->AddInstance();
	transform = model->GetTransform(1);
	transform->Position(0, 0, 0);
	transform->Scale(0.075f, 0.075f, 0.075f);
	kachujin->PlayClip(1, 1, 1.0f);

	model->AddInstance();
	transform = model->GetTransform(2);
	transform->Position(25, 0, 0);
	transform->Scale(0.075f, 0.075f, 0.075f);
	kachujin->PlayClip(2, 2, 0.75f);
		
	model->UpdateTransforms();

	animators.push_back(kachujin);
	kachujin->Render();
	

	




	for (UINT i = 0; i < 3; i++)
	{
		colliders[i].Init = new Transform();
		colliders[i].Init->Scale(10, 10, 120);
		colliders[i].Init->Position(10, -5, -65);
		
		colliders[i].Transform = new Transform();
		colliders[i].Collider = new OBBCollider(colliders[i].Transform, colliders[i].Init);
	}
}

void InstanceColliderDemo::AddAnimation()
{
}

#pragma endregion


#pragma region Controllers

void InstanceColliderDemo::ImGUIController()
{
	bool bDocking = true;
	//ImGui::ShowDemoWindow(&bDocking);
	ImGui::Begin("Animation", &bDocking);
	{
		/* 파츠 움직임의 독립성 여부 */
		static bool bIndep = false;
		static bool brepeat = false;
		bool bChange = false;
		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
		if (ImGui::CollapsingHeader("Materials", ImGuiTreeNodeFlags_DefaultOpen))
		{
			for(Material* mat : model->Materials())
			{
				mat->Property();
				ImGui::Separator();
			}
		}
		
		
		if (ImGui::CollapsingHeader("Selected_Parts", ImGuiTreeNodeFlags_DefaultOpen))
		{
			Transform* transform = new Transform();
			Transform* boneTransform = boneNames[selected]->GetBoneTransform();
			Matrix mat = model->BoneByIndex(selected + 1)->BoneWorld();
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
			if (bPlay != true)
			{
				boneTrans[selected]->Local(boneTransform->ParentTransform()->Local());
				boneTrans[selected]->Update();
				Gui::Get()->SetGizmo(boneTransform, model->GetTransform(instance),true);
				bChange |= boneTrans[selected]->Property();
				boneTrans[selected]->Position(&T);
				T.y = T.y > 0 ? T.y : 0;
				boneTrans[selected]->Position(T);
				
				if (bChange)
				{
					UINT clip = kachujin->GetCurrClip(instance);
					boneTransform->ParentTransform()->Local(boneTrans[selected]->Local());
					boneTransform->Update();

					//kachujin->UpdateInstTransform(instance, selected + 1, boneTrans[selected]->World());
					kachujin->UpdateBoneTransform(selected + 1,clip, boneTrans[selected]);
				}
			}
			/* 재생중일때 정보 표기 */
			else
			{
				transform = (boneTrans[selected]);
				transform->Scale(&S);
				transform->Rotation(&R);
				transform->Position(&T);

				ImGui::Text("Scale    : <%.2f, %.2f, %.2f> ", S.x, S.y, S.z);
				ImGui::Text("Rotation : <%.2f, %.2f, %.2f> ", R.x, R.y, R.z);
				ImGui::Text("Position : <%.2f, %.2f, %.2f> ", T.x, T.y, T.z);
			}

			ImGui::Separator();

			/* Frame 위치 및 피봇 여부 */
			if (bPlay != true)
			{
				ImGui::SliderInt("Frames", &selectedFrame, 0, MAX_MODEL_KEYFRAMES);
				selectedFrame = selectedFrame >= kachujin->GetFrameCount(instance) ?
					kachujin->GetFrameCount(instance) - 1 : selectedFrame;
				kachujin->SetFrame(instance, selectedFrame);
				
			}
			else
			{
				selectedFrame = kachujin->GetCurrFrame(instance);
				ImGui::Text("Frame Number : %d", selectedFrame);
			}
			
			

		}
		ImGui::PopStyleVar();

		ImGui::Checkbox("Repeat", &brepeat);
		ImGui::SameLine();
		/* Play */
		if (ImGui::Button("Play"))
		{
			if (bPlay != true)
			{
				bPlay = true;
				selectedFrame = 0;
			}
		}

		ImGui::SameLine();
		if (bPlay == true)
		{
			kachujin->PlayAnim();
			if (brepeat != true)
				if (selectedFrame >= kachujin->GetFrameCount(instance) - 1)
				{
					bPlay = false;
					selectedFrame = 0;
				}
		}

		if (ImGui::Button("Reset"))
		{
			boneTrans.clear();
			boneTrans.shrink_to_fit();

			for (UINT j = 0; j < boneNames.size(); j++)
				boneTrans.emplace_back(new Transform());
		}

	}
	ImGui::End();
}

void InstanceColliderDemo::AnimationController()
{
	bool bDocking = true;
	ImGui::Begin("Animator", &bDocking);
	{
		bool bChange = false;
		bChange |= ImGui::InputInt("AnimClip", (int*)&clip);
		ImGui::InputInt("AnimInstance", (int*)&instance);
		bChange |= ImGui::InputFloat("TakeTime", &takeTime, 0.1f);
		takeTime = Math::Clamp(takeTime, 0.1f, 3.0f);
		clip %= kachujin->ClipCount();
		instance %= model->GetInstSize();

		if (bChange)
			kachujin->PlayClip(instance, clip, takeTime);
		kachujin->Update();
		for (UINT i = 0; i < model->GetInstSize(); i++)
		{
			if (i == instance && bPlay == false) continue;
			kachujin->PlayAnim(i);
		}

		if (ImGui::Button("IllusionTest"))
		{
			illusion = new ModelIllusion(kachujin);
			illusion->GetTransform()->Parent(model->GetTransform(0));
		}
		if (ImGui::Button("SaveClipTest"))
		{
			wstring name = L"Test_" + to_wstring( clip);
			kachujin->SaveChangedClip(clip, name,L"../../_Models/Kachujin/",true);
			//SafeDelete(illusion);
			//kachujin->DelInstance(instance);
			//instance %= kachujin->GetInstSize();
		}
		if (ImGui::Button("AddClipTest"))
		{
			function<void(wstring)> f = bind(&InstanceColliderDemo::AddClip, this, placeholders::_1);
			Path::OpenFileDialog(L"", L"CLIP\0*.clip", L"../../_Models/", f);
			//kachujin.
		}
		
		
		ImGui::Separator();
		ImGui::Text("ModelClonningTest");
		static int selectmodel = 0;
		if (testModels.size() > 0)
		{
			ImGui::InputInt("SelectModel", (int*)&selectmodel);
			selectmodel %= testModels.size();
			ImGui::LabelText("##Selected", String::ToString(testModels[selectmodel]->Name()).c_str());

			if (ImGui::Button("Clonning"))
			{
				Model* newmodel = new Model(testModels[selectmodel]);
				newmodel->AddInstance();
				testModels.emplace_back(newmodel);
			}
			if (ImGui::Button("ChangeAnimModel"))
			{
				/*if (testModels[selectmodel]->IsAnimationModel())
				{
					model = testModels[selectmodel];
					kachujin->ChangeModel(model);
				}*/
			}
		}
		ImGui::Separator();
		for (Model* model : testModels)
		{
			ImGui::LabelText("##ModelName", String::ToString(model->Name()).c_str());
		}
	}
	ImGui::End();
}

void InstanceColliderDemo::AddClip(wstring name)
{
	wstring dir = Path::GetDirectoryName(name);
	name = Path::GetFileNameWithoutExtension(name);
	kachujin->AddClip(name,dir);
}

#pragma endregion


#pragma region ImGuiViewer

void InstanceColliderDemo::SelectedPartsViewer()
{
	if (selectedTransform == NULL)
		selectedTransform = new Transform();
	
	/* 선택 파츠 위치 표기 */
	Matrix W, V, P;
	D3DXMatrixIdentity(&W);
	V = Context::Get()->View();
	P = Context::Get()->Projection();
	Vector3 textPos, pos;
	
	Matrix mat = kachujin->GetboneWorld(instance, selected + 1);
	Matrix world = kachujin->GetModel()->GetTransform(instance)->World();
	mat*=world;

	//trail->Update(mat);

	selectedTransform->World(mat);
	selectedTransform->Position(&pos);
	Context::Get()->GetViewport()->Project(&textPos, pos, W, V, P);
	string str;
	str += "Part_Name :" + String::ToString(boneNames[selected]->Name());
		
	//Gui::Get()->SetGizmo(kachujin->GetTransform(instance));
	Gui::Get()->RenderText(textPos.x - 50, textPos.y, str.c_str());
	
}

void InstanceColliderDemo::ModelsViewer()
{
	bool bDocking = true;
	/* 모델 선택 */
	ImGui::Begin("ModelViewer", &bDocking);
	{
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf;

		for (UINT i = 0; i < ModelList.size(); i++)
		{
			if (i == selectedModel)
				flags |= ImGuiTreeNodeFlags_Selected;

			if (ImGui::TreeNodeEx(String::ToString(ModelList[i]->ModelName).c_str(), flags))
			{
				if (ImGui::IsItemClicked())
				{
					selectedModel = i;
				}
				ImGui::TreePop();
			}
		}
	}
	ImGui::End();
}

void InstanceColliderDemo::PartsViewer()
{
	bool bDocking = true;
	/* 파츠 선택 */
	ImGui::Begin("Parts", &bDocking);
	{
		for (UINT i = 0; i < boneNames.size(); i++)
		{
			auto root = boneNames[i];
			if (boneNames[i]->ParentIndex() < 0)
				ChildViewer(root);
			
		}		
	}
	ImGui::End();
}

void InstanceColliderDemo::ChildViewer(ModelBone * bone)
{
	//아이템 시작지점
	ImVec2 pos = ImGui::GetItemRectMin()- ImGui::GetWindowPos();
	//윈도우 사이즈
	ImVec2 wSize = ImGui::GetWindowSize();
	if (wSize.y < pos.y) return;
	if (wSize.x < pos.x) return;
	
	auto childs = bone->Childs();
	ImGuiTreeNodeFlags flags = childs.empty() ? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_DefaultOpen |ImGuiTreeNodeFlags_OpenOnDoubleClick;

	if (bone->Index() - 1 == selected)
		flags |= ImGuiTreeNodeFlags_Selected;
	ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 10.0f);

	if (ImGui::TreeNodeEx(String::ToString(bone->Name()).c_str(), flags))
	{
		if (ImGui::IsItemClicked())
		{
			selected = bone->Index() - 1;
		}
		if (ImGui::IsItemClicked(1))
		{
			selected = bone->Index() - 1;
			ImGui::OpenPopup("Popup");
		}
		Popup();

		for (auto& child : childs)
		{
			ChildViewer(child);
		}
		ImGui::TreePop();
	}	
	ImGui::PopStyleVar();
}

#pragma endregion

void InstanceColliderDemo::Popup()
{
	bool boo = false;
	if (ImGui::BeginPopup("Popup"))
	{
		//TODO: MenuItem은 openpopup이 안먹힘
		if (ImGui::BeginMenu("AddSocket"))
		{
			//TODO:Button 안하면 종료 안하고 계속 오픈 상태가 됨
			wstring str = boneNames[selected]->Name() + L"_AddSocket";
			boo = ImGui::MenuItem(String::ToString(str).c_str());
			
			ImGui::EndMenu();
		}
		ImGui::EndPopup();
	}

	//EndPopup 중첩되어서 밖으로 빼줘야함...
	if (boo)
		ImGui::OpenPopup("Add_Socket?");
	AddSocket();
}

void InstanceColliderDemo::AddSocket()
{
	if (ImGui::BeginPopupModal("Add_Socket?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		static char name[64] = "NewSocket";

		ImGui::Text("SocketName:");
		ImGui::InputText("##edit", name, IM_ARRAYSIZE(name));
		if (ImGui::Button("OK", ImVec2(120, 0))) {

			kachujin->AddSocket(selected + 1, String::ToWString(name));

			UINT boneIdx = model->BoneCount()-1;
			ModelBone* bone = model->BoneByIndex(boneIdx);
			boneNames.emplace_back(bone);

			boneTrans.emplace_back(new Transform());

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

void InstanceColliderDemo::ImportModel(wstring path)
{
	if (path.length() < 1)
	{
		Path::OpenFileDialog(L"", Path::FbxModelFilter, L"../../ ", bind(&InstanceColliderDemo::ImportModel, this, placeholders::_1));
	}
	else
	{
		//TODO: 모델 확장자 확인
		if (Path::IsRawModelFile(path))
		{
			wstring modelFilePath=Path::GetFileName(path);
			wstring modelDir=Path::GetDirectoryName(path);

			Converter* conv = new Converter();
			//modelFilePath = modelDir + L"/" + modelFilePath;
			conv->ReadFile(modelFilePath, modelDir);
			modelDir = Path::GetDirectDirectoryName(path);
			modelFilePath = modelDir + L"/" + Path::GetFileNameWithoutExtension( modelFilePath);
			conv->ExportMaterial(modelFilePath, L"../Contents/", false);
			conv->ExportMesh(modelFilePath, L"../Contents/");
		}

	}
}
