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

	Mesh();
	ModelLoad();
	for (ModelBone* bone : kachujin->Bones())
	{
		if (bone->Index() < 0)continue;
		boneNames.emplace_back(bone);
	}

	for (UINT i = 0; i < MAX_MODEL_KEYFRAMES; i++)
	{
		Keyframe frame;

		for (UINT j = 0; j < boneNames.size(); j++)
		{
			frame.partsTrans.emplace_back(new Transform());
		}
		keyframes.emplace_back(frame);
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
	
	AnimationController();
	ImGUIController();
	NotifyController();
	for (int i = 0; i < 3; i++)
	{
		Matrix attach = kachujin->GetboneTransform(i,11);
		colliders[i].Collider->GetTransform()->Parent(attach);
		colliders[i].Collider->Update();
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
	/*if (ModelList[selectedModel])
		ModelList[selectedModel]->animator->Render();*/
/*
	for (int i = 0; i < 3; i++)
		colliders[i].Collider->Render(Color(0, 1, 0, 1));*/
}

void InstanceColliderDemo::Pass(UINT mesh, UINT model, UINT anim)
{
	for (MeshRender* temp : meshes)
		temp->Pass(mesh);

	for (ModelRender* temp : models)
		temp->Pass(model);

	for (ModelAnimator* temp : animators)
		temp->Pass(anim);
	/*if(ModelList[selectedModel])
		ModelList[selectedModel]->animator->Pass(anim);*/
}

#pragma region LoadRegion

void InstanceColliderDemo::Mesh()
{
	//Create Material
	{
		floor = new Material(shader);
		floor->DiffuseMap("Floor.png");
		//floor->SpecularMap("Floor_Specular.png");
		//floor->NormalMap("Floor_Normal.png");
		//floor->Specular(1, 1, 1, 20);
	}

	//Create Mesh
	{
		Transform* transform = NULL;

		grid = new MeshRender(shader, new MeshGrid(5, 5));
		transform = grid->AddTransform();
		transform->Position(0, 0, 0);
		transform->Scale(12, 1, 12);

	}

	grid->UpdateTransforms();

	meshes.push_back(grid);
}

void InstanceColliderDemo::ModelLoad()
{
	sword = new ModelRender(shader);
	sword->ReadMaterial(L"Weapon/Sword");
	sword->ReadMesh(L"Weapon/Sword");
	

	kachujin = new ModelAnimator(shader);
	kachujin->ReadMaterial(L"Megan/Mesh");
	kachujin->ReadMesh(L"Megan/Mesh");
	//kachujin->ReadClip(L"Megan/Mesh");
	kachujin->ReadClip(L"Megan/Taunt");
	kachujin->ReadClip(L"Megan/Dancing");
	//kachujin->ReadClip(L"Kachujin/Running");
	//kachujin->ReadClip(L"Kachujin/Jump");
	//kachujin->ReadClip(L"Kachujin/Hip_Hop_Dancing");

	kachujin->Attach((Model*)sword, 11, sword->GetInstSize());
	Transform* attachTransform = sword->AddTransform();
	attachTransform->Position(10, -5, -15);
	attachTransform->Scale(1.0f, 1.0f, 1.0f);
	attachTransform->RotationDegree(180.0f, 0.0f, 0.0f);



	Transform* transform = NULL;

	transform = kachujin->AddTransform();
	transform->Position(-25, 0, 0);
	transform->Scale(0.075f, 0.075f, 0.075f);
	kachujin->PlayClip(0, 0, 1.0f);

	transform = kachujin->AddTransform();
	transform->Position(0, 0, 0);
	transform->Scale(0.075f, 0.075f, 0.075f);
	kachujin->PlayClip(1, 1, 1.0f);

	transform = kachujin->AddTransform();
	transform->Position(25, 0, 0);
	transform->Scale(0.075f, 0.075f, 0.075f);
	kachujin->PlayClip(2, 2, 0.75f);
		
	kachujin->UpdateTransforms();

	animators.push_back(kachujin);
	kachujin->Render();
	
	

	for (UINT i = 0; i < 3; i++)
	{
		colliders[i].Init = new Transform();
		colliders[i].Init->Scale(10, 10, 120);
		colliders[i].Init->Position(10, -5, -65);
		
		colliders[i].Transform = new Transform();
		colliders[i].Collider = new Collider(colliders[i].Transform, colliders[i].Init);
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
			for(Material* mat : kachujin->Materials())
			{
				mat->Property();
				ImGui::Separator();
			}
		}
		
		
		if (ImGui::CollapsingHeader("Selected_Parts", ImGuiTreeNodeFlags_DefaultOpen))
		{
			Transform* transform = new Transform();
			Matrix transMat = boneNames[selected]->Transform();
			transform->World(transMat);
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
								
				transform = (keyframes[selectedFrame].partsTrans[selected]);
				Gui::Get()->SetGizmo(transform);

				transform->Scale(&S);
				transform->Rotation(&R);
				transform->Position(&T);

				
				bChange |= ImGui::SliderFloat3("Scale", (float*)&S, 0.01f, 100.0f);
				bChange |= ImGui::SliderFloat3("Rotation", (float*)&R, -Math::PI*0.9f, Math::PI*0.9f);
				bChange |= ImGui::SliderFloat3("Position", (float*)&T, -100.0f, 100.0f);
				T.y = T.y > 0 ? T.y : 0;

				transform->Scale(S);
				transform->Rotation(R);
				transform->Position(T);
				if(bChange)
					kachujin->UpdateInstTransform(instance, selected + 1,transform->World());
			}
			/* 재생중일때 정보 표기 */
			else
			{
				transform = (keyframes[selectedFrame].partsTrans[selected]);
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
			keyframes.clear();
			keyframes.shrink_to_fit();
			for (UINT i = 0; i < MAX_MODEL_KEYFRAMES; i++)
			{
				Keyframe frame;

				for (UINT j = 0; j < boneNames.size(); j++)
					frame.partsTrans.emplace_back(new Transform());
				keyframes.emplace_back(frame);
			}
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
		instance %= kachujin->GetInstSize();

		if (bChange)
			kachujin->PlayClip(instance, clip, takeTime);

		kachujin->Update(instance);
		if (ImGui::Button("addTest"))
		{
			kachujin->AddClip(L"Kachujin/Jump");
			kachujin->AddClip(L"Kachujin/Hip_Hop_Dancing");
		}
	}
	ImGui::End();
}

void InstanceColliderDemo::NotifyController()
{
	/*bool bDocking = true;
	ImGui::Begin("AnimationNotify", &bDocking);
	{
		for (int notiID = 0; notiID < notifies.size(); notiID++)
		{
			ImGui::Text(notifies[notiID].NotifyName.c_str());

			ImGui::PushItemWidth(60);
			{
				string str = "##" + to_string(notiID) + notifies[notiID].NotifyName + "_Start";
				ImGui::Text("StartFrame");
				ImGui::SameLine();
				ImGui::SliderInt(str.c_str(), &notifies[notiID].StartFrame, 0, notifies[notiID].EndFrame);
				ImGui::SameLine();
				ImGui::Text("EndFrame");
				ImGui::SameLine();
				str = "##" + to_string(notiID) + notifies[notiID].NotifyName + "_End";
				ImGui::SameLine();
				ImGui::SliderInt(str.c_str(), &notifies[notiID].EndFrame, notifies[notiID].StartFrame, MAX_MODEL_KEYFRAMES);
			}
			ImGui::PopItemWidth();
			if (notifies[notiID].StartFrame < notifies[notiID].EndFrame
				&& notifies[notiID].StartFrame > -1)
				notifies[notiID].bSet = true;

		}

		if (ImGui::Button("AddNotify"))
		{
			ImGui::OpenPopup("AddNotify?");
		}
		if (ImGui::BeginPopupModal("AddNotify?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			static char name[64] = "New Notify Name";

			ImGui::Text("Edit name:");
			ImGui::InputText("##edit", name, IM_ARRAYSIZE(name));
			if (ImGui::Button("OK", ImVec2(120, 0))) {

				Notify noti;
				noti.NotifyName = name;
				notifies.push_back(noti);
				strcpy_s(name, "New Notify Name");
				ImGui::CloseCurrentPopup();
			}
			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
			ImGui::EndPopup();
		}
	}
	ImGui::End();*/
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
	
	Matrix mat = kachujin->GetboneTransform(instance, selected + 1);
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

		for (UINT i = 0; i < ModelList[selectedModel]->boneNames.size(); i++)
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
	auto childs = bone->Childs();
	ImGuiTreeNodeFlags flags = childs.empty() ? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnDoubleClick;

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
	if (ImGui::BeginPopup("Popup"))
	{
		//TODO: MenuItem은 openpopup이 안먹힘
		if (ImGui::BeginMenu("AddSocket"))
		{
			//TODO:Button 안하면 종료 안하고 계속 오픈 상태가 됨
			wstring str = boneNames[selected]->Name() + L"_AddSocket";
			if(ImGui::Button(String::ToString( str).c_str()))
				ImGui::OpenPopup("Add_Socket?");
			AddSocket();
			ImGui::EndMenu();
		}
		ImGui::EndPopup();
	}
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

			UINT boneIdx = kachujin->BoneCount()-1;
			ModelBone* bone = kachujin->BoneByIndex(boneIdx);
			boneNames.emplace_back(bone);

			for (UINT i = 0; i < MAX_MODEL_KEYFRAMES; i++)
			{
				keyframes[i].partsTrans.emplace_back(new Transform());
			}
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
