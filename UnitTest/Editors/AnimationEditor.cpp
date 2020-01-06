#include "stdafx.h"
#include "AnimationEditor.h"
#include "Viewer/Freedom.h"
#include "Environment/Sky.h"

void AnimationEditor::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(23, 0, 0);
	Context::Get()->GetCamera()->Position(0, 32, -67);
	((Freedom *)Context::Get()->GetCamera())->Speed(20, 2);
	sky = new Sky(L"Environment/GrassCube1024.dds");
	shader = new Shader(L"027_Animation.fx");

}

void AnimationEditor::Destroy()
{
	SafeDelete(shader);
	SafeDelete(sky);
}

void AnimationEditor::Update()
{
	sky->Update();
	
	AnimationController();
	ImGUIController();



}

void AnimationEditor::Render()
{
	sky->Render();

	ModelsViewer();
	PartsViewer();
	SelectedPartsViewer();
}

void AnimationEditor::PreRender()
{
}

void AnimationEditor::PostRender()
{
}

void AnimationEditor::ImGUIController()
{
	bool bDocking = true;
	//ImGui::ShowDemoWindow(&bDocking);
	ImGui::Begin("Animation", &bDocking);
	{
		//리스트가 없으면 실행 안함
		if (ModelList.size() < 1)
			return;
		/* 파츠 움직임의 독립성 여부 */
		static bool bIndep = false;
		static bool brepeat = false;
		bool bChange = false;
		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
		if (ImGui::CollapsingHeader("Selected_Parts", ImGuiTreeNodeFlags_DefaultOpen))
		{
			Transform* transform = new Transform();
			Matrix transMat = ModelList[selectedModel]->boneNames[selected]->Transform();
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
				transform = (ModelList[selectedModel]->keyframes[selectedFrame].partsTrans[selected]);

				//Gui::Get()->SetGizmo(transform);
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
				if (bChange)
					ModelList[selectedModel]->animator->UpdateInstTransform(instance, selected + 1, transform->World());
			}
			/* 재생중일때 정보 표기 */
			else
			{
				transform = (ModelList[selectedModel]->keyframes[selectedFrame].partsTrans[selected]);
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
				selectedFrame = selectedFrame >= ModelList[selectedModel]->animator->GetFrameCount(instance) ?
					ModelList[selectedModel]->animator->GetFrameCount(instance) - 1 : selectedFrame;
				ModelList[selectedModel]->animator->SetFrame(instance, selectedFrame);

			}
			else
			{
				selectedFrame = ModelList[selectedModel]->animator->GetCurrFrame(instance);
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
			ModelList[selectedModel]->animator->PlayAnim();
			if (brepeat != true)
				if (selectedFrame >= ModelList[selectedModel]->animator->GetFrameCount(instance) - 1)
				{
					bPlay = false;
					selectedFrame = 0;
				}
		}

		if (ImGui::Button("Reset"))
		{
			ModelList[selectedModel]->keyframes.clear();
			ModelList[selectedModel]->keyframes.shrink_to_fit();
			for (UINT i = 0; i < MAX_MODEL_KEYFRAMES; i++)
			{
				Keyframe frame;

				for (UINT j = 0; j < ModelList[selectedModel]->boneNames.size(); j++)
					frame.partsTrans.emplace_back(new Transform());
				ModelList[selectedModel]->keyframes.emplace_back(frame);
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
		//리스트가 없으면 실행 안함
		if (ModelList.size() < 1)
			return;
		bool bChange = false;
		bChange |= ImGui::InputInt("AnimClip", (int*)&clip);
		ImGui::InputInt("AnimInstance", (int*)&instance);
		bChange |= ImGui::InputFloat("TakeTime", &takeTime, 0.1f);
		takeTime = Math::Clamp(takeTime, 0.1f, 3.0f);
		clip %= ModelList[selectedModel]->animator->ClipCount();
		instance %= ModelList[selectedModel]->animator->GetInstSize();

		if (bChange)
			ModelList[selectedModel]->animator->PlayClip(instance, clip, takeTime);

		ModelList[selectedModel]->animator->Update(instance);		
	}
	ImGui::End();
}


#pragma region ImGuiViewer

void AnimationEditor::SelectedPartsViewer()
{
	if (ModelList.size() < 1)
		return;
	
	if (selectedTransform == NULL)
		selectedTransform = new Transform();

	/* 선택 파츠 위치 표기 */
	Matrix W, V, P;
	D3DXMatrixIdentity(&W);
	V = Context::Get()->View();
	P = Context::Get()->Projection();
	Vector3 textPos, pos;

	Matrix mat = ModelList[selectedModel]->animator->GetboneTransform(instance, selected + 1);
	selectedTransform->World(mat);
	selectedTransform->Position(&pos);
	Context::Get()->GetViewport()->Project(&textPos, pos, W, V, P);
	string str;
	str += "Part_Name :" + String::ToString(ModelList[selectedModel]->boneNames[selected]->Name());
	
	Gui::Get()->RenderText(textPos.x - 50, textPos.y, str.c_str());

}

void AnimationEditor::ModelsViewer()
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

void AnimationEditor::PartsViewer()
{
	bool bDocking = true;
	/* 파츠 선택 */
	ImGui::Begin("PartsViewer", &bDocking);
	{
		for (UINT i = 0; i < ModelList[selectedModel]->boneNames.size(); i++)
		{
			auto root = ModelList[selectedModel]->boneNames[i];
			if (ModelList[selectedModel]->boneNames[i]->ParentIndex() < 0)
				ChildViewer(root);
		}
	}
	ImGui::End();
}

void AnimationEditor::ChildViewer(ModelBone * bone)
{
	auto childs = bone->Childs();
	ImGuiTreeNodeFlags flags = childs.empty() ? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnDoubleClick;

	if (bone->Index() - 1 == selected)
		flags |= ImGuiTreeNodeFlags_Selected;

	if (ImGui::TreeNodeEx(String::ToString(bone->Name()).c_str(), flags))
	{
		if (ImGui::IsItemClicked())
		{
			selected = bone->Index() - 1;
		}
		for (auto& child : childs)
		{
			ChildViewer(child);
		}
		ImGui::TreePop();
	}
}

#pragma endregion


void AnimationEditor::Popup()
{
	//리스트가 없으면 실행 안함
	if (ModelList.size() < 1)
		return;
	if (ImGui::BeginPopup("Popup"))
	{
		//TODO: MenuItem은 openpopup이 안먹힘
		if (ImGui::BeginMenu("AddSocket"))
		{
			//TODO:Button 안하면 종료 안하고 계속 오픈 상태가 됨
			wstring str = ModelList[selectedModel]->boneNames[selected]->Name() + L"_AddSocket";
			if (ImGui::Button(String::ToString(str).c_str()))
				ImGui::OpenPopup("Add_Socket?");
			AddSocket();
			ImGui::EndMenu();
		}
		ImGui::EndPopup();
	}
}

void AnimationEditor::AddSocket()
{
	if (ImGui::BeginPopupModal("Add_Socket?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		static char name[64] = "NewSocket";

		ImGui::Text("SocketName:");
		ImGui::InputText("##edit", name, IM_ARRAYSIZE(name));
		if (ImGui::Button("OK", ImVec2(120, 0))) {
			UINT boneIdx = ModelList[selectedModel]->animator->BoneCount();
			ModelBone* bone = ModelList[selectedModel]->animator->BoneByIndex(boneIdx);
			ModelList[selectedModel]->boneNames.emplace_back(bone);

			for (UINT i = 0; i < MAX_MODEL_KEYFRAMES; i++)
			{
				ModelList[selectedModel]->keyframes[i].partsTrans.emplace_back(new Transform());
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

void AnimationEditor::LoadModel()
{
}

void AnimationEditor::AddAnimation()
{
}

void AnimationEditor::ImportModel(wstring path)
{
	if(path.length()<1)
	{ 
		Path::OpenFileDialog(L"", Path::FbxModelFilter, L"", bind(&AnimationEditor::ImportModel, this, placeholders::_1));
	}
	else
	{
		//TODO: 모델 확장자 확인
		if (Path::IsRawModelFile(path))
		{
			wstring modelFilePath = Path::GetFileName(path);
			wstring modelDir = Path::GetDirectoryName(path);

			Converter* conv = new Converter();
			//modelFilePath = modelDir + L"/" + modelFilePath;
			conv->ReadFile(modelFilePath, modelDir);
			modelDir = Path::GetDirectDirectoryName(path);
			modelFilePath = modelDir + L"/" + Path::GetFileNameWithoutExtension(modelFilePath);
			conv->ExportMaterial(modelFilePath, L"../Contents/", false);
			conv->ExportMesh(modelFilePath, L"../Contents/");
		}

	}
}
