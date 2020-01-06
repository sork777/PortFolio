#include "stdafx.h"
#include "AnimationEditorDemo.h"
#include "Viewer/Freedom.h"
#include "Environment/Sky.h"
#include "Model/ModelMesh.h"
#include "Viewer/Orbit.h"
#include "Utilities/Xml.h"

void AnimationEditorDemo::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(21, 0, 0);
	Context::Get()->GetCamera()->Position(0, 30, -50);
	Context::Get()->GetFreeCam()->Movable(false);
	Context::Get()->GetOrbitCam()->Speed(1.0f);
	Context::Get()->GetOrbitCam()->SetRad(30.0f,25.0f,150.0f);

	shader = new Shader(L"029_NormalMap.fx");
	instMeshShader = new Shader(L"024_Instance.fx");
	
	sky = new Sky(L"Environment/GrassCube1024.dds");

	//CreateMesh
	{
		floor = new Material(shader);
		floor->DiffuseMap("Floor.png"); 
		floor->SpecularMap("Floor_Specular.png");

		
		grid = new MeshGrid(shader, 3, 3);
		grid->GetTransform()->Position(0, 0, 0);
		grid->GetTransform()->Scale(20, 1, 20);

	}

	//Load Model
	{
		sword = new Model();
		sword->ReadMaterial(L"swords/sword_1");
		sword->ReadMesh(L"swords/sword_1");
		swordRender = new ModelRender(shader, sword);
		swordRender->AddTransform();
		swordRender->UpdateTransform();
		trans_sword = new Transform();
		trans_swordF = new Transform();
		Matrix mat = swordRender->GetboneTrans(0, 2);
		trans_swordF->World(mat);
		trans_swordF->Scale(1,1,1);
		col_Sword = new Collider(trans_swordF, swordRender->GetCubeTransform(0));

		shader = new Shader(L"035_Model.fx");
		model = new Model();
		model->ReadMaterial(L"Kachujin/Kachujin");
		model->ReadMesh(L"Kachujin/Kachujin");
		model->ReadClip(L"Kachujin/Ninja_Idle");
		model->ReadClip(L"Kachujin/Ninja_Walk");
		model->ReadClip(L"Kachujin/Ninja_Run");
		modelRender = new ModelAnimator(shader, model);
		modelRender->AddTransform();
		modelRender->GetTransform(0)->Scale(0.05f,0.05f,0.05f);
		//modelRender->GetTransform(0)->Position(0,3.5f,0);
		//modelRender->AddTransform();
		modelRender->UpdateTransform();
		col_Model = new Collider(modelRender->GetTransform(0), modelRender->GetCubeTransform(0));

		

		sphere = new MeshSphere(instMeshShader, 20.0f,true);
		
		for (ModelBone* bone : model->Bones())
		{
			if (bone->BoneIndex() < 0)continue;
			UINT index = sphere->Push();
			Transform* trans = sphere->GetInstTransform(index);
			trans->World(bone->Transform());
			boneNames.emplace_back(bone);
		}
		
		for (UINT i = 0; i < MAX_FRAMES+1; i++)
		{
			Keyframe frame;

			for (UINT j = 0; j < boneNames.size(); j++)
			{
				frame.partsTrans.emplace_back(new Transform());
				frame.partsTransIndep.emplace_back(new Transform());
			}
			keyframes.emplace_back(frame);
		}

	}
	((Orbit *)Context::Get()->GetCamera())->GetObjPos(Vector3(0,0,0));	
	sphere->Render();

}

void AnimationEditorDemo::Destroy()
{
	SafeDelete(modelRender);
	SafeDelete(model);
	
	boneNames.clear();
	boneNames.shrink_to_fit();
	keyframes.clear();
	keyframes.shrink_to_fit();

	SafeDelete(floor);
	SafeDelete(grid);
	SafeDelete(sky);
	SafeDelete(shader);
}



void AnimationEditorDemo::Update()
{
	sky->Update();
	grid->Update();

	sphere->Update();
	modelRender->Update();
	modelRender->UpdateTransform();
	col_Model->ChangeInit(modelRender->GetCubeTransform(0));
	col_Model->ChangeTrans(modelRender->GetTransform(0));
	col_Model->Update();

	SelectedPartsView();
	
	Context::Get()->GetOrbitCam()->OrbitStaus();

	if(bAttach)
	{
		AttachModel();
		AttachController();
		swordRender->Update();		
	}
	ImGUIController();
	if (bPlay)
	{
		AnimationController();
		col_Model->Update();
		col_Sword->Update();
	}
	ShowParts();
}

void AnimationEditorDemo::Render()
{
	sky->Render();



	floor->Render();
	grid->Pass(0);
	grid->Render();


	//sphere->Pass(0);
	//sphere->Render();
	if (bAttach)
	{
		swordRender->Pass(1);
		swordRender->Render();
	}
	if (bPlay)
	{
		col_Model->Render(Color(0, 1, 0, 1));
		
		if (bAttach)
			col_Sword->Render(Color(0, 1, 0, 1));
	}

	modelRender->Pass(2);
	modelRender->Render();
}

void AnimationEditorDemo::ImGUIController()
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
		if (ImGui::CollapsingHeader("Selected_Parts",ImGuiTreeNodeFlags_DefaultOpen))
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
			if(bPlay != true)
			{
				if(bIndep == true)
					transform = (keyframes[selectedFrame].partsTransIndep[selected]);
				else
					transform = (keyframes[selectedFrame].partsTrans[selected]);
				transform->Scale(&S);
				transform->Rotation(&R);
				transform->Position(&T);
				Vector3 S2, R2, T2;
				S2 = S;
				R2 = R;
				T2 = T;
				ImGui::SliderFloat3("Scale", (float*)&S, 0.01f, 100.0f);
				ImGui::SliderFloat3("Rotation", (float*)&R, -Math::PI*0.9f, Math::PI*0.9f);
				ImGui::SliderFloat3("Position", (float*)&T, -100.0f, 100.0f);
				if (T2 == T ||
					S2 == S ||
					R2 == R)
					bChange = true;

				T.y = T.y > 0 ? T.y : 0;

				transform->Scale(S);
				transform->Rotation(R);
				transform->Position(T);
				//if(bChange)
					modelRender->UpdateInstTransform(0, selected + 1, selectedFrame, transform->World());
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
				ImGui::SliderInt("Frames", &selectedFrame, 0, MAX_FRAMES);
				selectedFrame = selectedFrame >= modelRender->GetFrameCount(0) ?
					modelRender->GetFrameCount(0) - 1 : selectedFrame;
				modelRender->SetFrame(0, selectedFrame);
				ImGui::Checkbox("Parts_Indepency", &bIndep);
			}
			else
			{
				selectedFrame = modelRender->GetCurrFrame(0);
				ImGui::Text("Frame Number : %d", selectedFrame);
			}
			
			/* 프레임에 따라 모든 파츠가 변동되게 하기 위함 */
			//if (bChange)
			{
				for (UINT i = 0; i < boneNames.size(); i++)
				{
					Transform* transform = keyframes[selectedFrame].partsTrans[i];
					modelRender->UpdateInstTransform(0, i + 1, selectedFrame, transform->World());
					transform = keyframes[selectedFrame].partsTransIndep[i];
					modelRender->UpdateInstTransform(0, i + 1, selectedFrame, transform->World());

					Matrix mat = modelRender->GetboneTrans(0, i+1);
					Matrix world = modelRender->GetTransform(0)->World();
					mat *= world;
					sphere->GetInstTransform(i)->World(mat);
				}
			}
		}
		ImGui::PopStyleVar();

		ImGui::Checkbox("Attach", &bAttach);
		/* Play */
		if (ImGui::Button("Play"))
		{
			if (bPlay != true)
			{
				bPlay = true;
				selectedFrame = 0;
			}
		}

		ImGui::Checkbox("Repeat", &brepeat);
		if (bPlay == true)
		{
			modelRender->PlayAnim();
			if(brepeat != true)
			if (selectedFrame >= modelRender->GetFrameCount(0)-1)
			{
				bPlay = false;
				selectedFrame = 0;
			}
		}

		/* Save */
		if (ImGui::Button("Save"))
			SaveAnimation();
		ImGui::SameLine();
		/* Load */
		if (ImGui::Button("Load"))
			LoadAnimation();
		ImGui::SameLine();
		/* Frame 정보 리셋 */
		if (ImGui::Button("Reset"))
		{
			keyframes.clear();
			keyframes.shrink_to_fit();
			for (UINT i = 0; i < MAX_FRAMES + 1; i++)
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

void AnimationEditorDemo::AnimationController()
{
	bool bDocking = true;
	//ImGui::ShowDemoWindow(&bDocking);
	ImGui::Begin("Animator", &bDocking);
	{
		

		static int clip = 0;
		//ImGui::InputInt("Clip", &clip);
		clip %= 3;

		static float time = 0.15f;
		ImGui::InputFloat("Time", &time, 0.1f);

		static float speed = 0.0f;
		ImGui::InputFloat("Speed", &speed, 0.1f);

		float tclip = clip;
		bool change = false;
		if (speed < 0.1f)
			clip = 0;
		else if (speed >= 0.1f && speed < 2.0f)
			clip = 1;
		else
			clip = 2;
		if (clip != tclip)
			change = true;
		if (change)
			modelRender->PlayNextClip(0, clip, time);
	}
	ImGui::End();
}

void AnimationEditorDemo::AttachController()
{
	bool bDocking = true;
	ImGui::Begin("Attachment", &bDocking);
	{
		{
			Transform* transform = new Transform();
			Matrix world = swordRender->GetboneTrans(0, 2);
			transform->World(world);
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
			if (bAttach == true)
			{
				trans_sword->Scale(&S);
				trans_sword->Rotation(&R);
				trans_sword->Position(&T);
				ImGui::SliderFloat3("aScale", (float*)&S, 0.01f, 100.0f);
				ImGui::SliderFloat3("aRotation", (float*)&R, -Math::PI*0.9f, Math::PI*0.9f);
				ImGui::SliderFloat3("aPosition", (float*)&T, -10.0f, 10.0f);

				trans_sword->Scale(S);
				trans_sword->Rotation(R);
				trans_sword->Position(T);
				//swordRender->UpdateInstTransform(0, 2, trans_sword->World());

			}

		}
		ImGui::End();
	}
}

void AnimationEditorDemo::ShowParts()
{
	bool bDocking = true;
	/* 파츠 선택 */
	ImGui::Begin("Parts", &bDocking);
	{
		for (UINT i = 0; i < boneNames.size(); i++)
		{
			auto root = boneNames[i];
			if (boneNames[i]->ParentIndex() < 0)
				ShowChild(root);
		}
	}
	ImGui::End();
}

void AnimationEditorDemo::SaveAnimation(wstring savePath)
{
	if (savePath.length() < 1)
	{
		Path::SaveFileDialog(L"", Path::XmlFilter, L"", bind(&AnimationEditorDemo::SaveAnimation, this, placeholders::_1));
	}
	else
	{
		string folder = String::ToString(Path::GetDirectoryName(savePath));
		string file = String::ToString(Path::GetFileName(savePath));

		Path::CreateFolders(folder);

		Xml::XMLDocument* document = new Xml::XMLDocument();

		Xml::XMLDeclaration* decl = document->NewDeclaration();
		document->LinkEndChild(decl);

		Xml::XMLElement* root = document->NewElement("AnimationEditorDemo");
		document->LinkEndChild(root);


		Xml::XMLElement* node = document->NewElement("Animation");
		node->SetAttribute("MAX_FRAMES", MAX_FRAMES);
		node->SetAttribute("Size", boneNames.size());
		root->LinkEndChild(node);

		for (UINT i = 0; i < MAX_FRAMES + 1; i++)
		{
			Xml::XMLElement* frameNode = document->NewElement("Frame");
			frameNode->SetAttribute("FrameNumber", i);
			node->LinkEndChild(frameNode);

			for (UINT j = 0; j < boneNames.size(); j++)
			{

				Xml::XMLElement* transformNode = document->NewElement(String::ToString(boneNames[j]->Name()).c_str());
				frameNode->LinkEndChild(transformNode);

				Xml::XMLElement* element = NULL;

				//Transform 데이터 저장
				Vector3	pos;
				Vector3 scale;
				Vector3 rot;

				keyframes[i].partsTrans[j]->Scale(&scale);
				keyframes[i].partsTrans[j]->Position(&pos);
				keyframes[i].partsTrans[j]->Rotation(&rot);

				element = document->NewElement("Scale");
				element->SetAttribute("ScaleX", scale.x);
				element->SetAttribute("ScaleY", scale.y);
				element->SetAttribute("ScaleZ", scale.z);
				transformNode->LinkEndChild(element);

				element = document->NewElement("Rotation");
				element->SetAttribute("RotationX", rot.x);
				element->SetAttribute("RotationY", rot.y);
				element->SetAttribute("RotationZ", rot.z);
				transformNode->LinkEndChild(element);

				element = document->NewElement("Position");
				element->SetAttribute("PositionX", pos.x);
				element->SetAttribute("PositionY", pos.y);
				element->SetAttribute("PositionZ", pos.z);
				transformNode->LinkEndChild(element);
			}
		}

		document->SaveFile((folder + file+".xml").c_str());
	}
}

void AnimationEditorDemo::LoadAnimation(wstring file)
{
	if (file.length() < 1)
	{
		Path::OpenFileDialog(L"", Path::XmlFilter, L"", bind(&AnimationEditorDemo::LoadAnimation, this, placeholders::_1));
	}
	else
	{
		keyframes.clear();
		keyframes.shrink_to_fit();

		Xml::XMLDocument* document = new Xml::XMLDocument();
		Xml::XMLError error = document->LoadFile(String::ToString(file).c_str());
		assert(error == Xml::XML_SUCCESS);

		Xml::XMLElement * root = document->FirstChildElement();
		Xml::XMLElement * node = root->FirstChildElement();
		int frames = node->IntAttribute("MAX_FRAMES")+1;
		int size = node->IntAttribute("Size");


		Xml::XMLElement * frameNode = node->FirstChildElement();
		for (UINT i = 0; i < frames; i++)
		{
			Keyframe frame;
			Xml::XMLElement * boneNode = frameNode->FirstChildElement();

			for (UINT j = 0; j < size; j++)
			{
				Transform* trans = new Transform();
				Vector3	pos;
				Vector3 scale;
				Vector3 rot;
				
				Xml::XMLElement* node = NULL;
				node = boneNode->FirstChildElement();
				
				scale.x = node->FloatAttribute("ScaleX");
				scale.y = node->FloatAttribute("ScaleY");
				scale.z = node->FloatAttribute("ScaleZ");
				node = node->NextSiblingElement();

				rot.x = node->FloatAttribute("RotationX");
				rot.y = node->FloatAttribute("RotationY");
				rot.z = node->FloatAttribute("RotationZ");
				node = node->NextSiblingElement();
				
				pos.x = node->FloatAttribute("PositionX");
				pos.y = node->FloatAttribute("PositionY");
				pos.z = node->FloatAttribute("PositionZ");

				trans->Scale(scale);
				trans->Rotation(rot);
				trans->Position(pos);
				/* 프레임 트랜스폼에 넣기 */
				frame.partsTrans.emplace_back(trans);

				boneNode = boneNode->NextSiblingElement();
			}
			/* 불러온 프레임 정보 저장 */
			keyframes.emplace_back(frame);
			frameNode = frameNode->NextSiblingElement();
		}
	}
}

void AnimationEditorDemo::BoneCircle()
{
	UINT stackCount = 60;
	float thetaStep = 2.0f * Math::PI / stackCount;
	Vector3 pos;
	Matrix mat = modelRender->GetboneTrans(0, selected+1);
	Matrix world = modelRender->GetTransform(0)->World();
	Transform trans;
	Vector3 scale;
	mat *= world;
	trans.World(mat);
	trans.Position(&pos);
	trans.Scale(&scale);

	float radius =2.0f;

	vector<Vector3> v, v2, v3;
	for (UINT i = 0; i <= stackCount; i++)
	{
		float theta = i * thetaStep;

		Vector3 p = Vector3
		(
			(radius * cosf(theta)),
			0,
			(radius * sinf(theta))
		);
		Vector3 p2 = Vector3
		(
			(radius * cosf(theta)),
			(radius * sinf(theta)),
			0
		);
		Vector3 p3 = Vector3
		(
			0,
			(radius * cosf(theta)),
			(radius * sinf(theta))
		);
		p += pos;
		p2 += pos;
		p3 += pos;
		v.emplace_back(p);
		v2.emplace_back(p2);
		v3.emplace_back(p3);
	}
	for (UINT i = 0; i < stackCount; i++)
	{
		DebugLine::Get()->RenderLine(v[i], v[i + 1]);
		DebugLine::Get()->RenderLine(v2[i], v2[i + 1]);
		DebugLine::Get()->RenderLine(v3[i], v3[i + 1]);
	}
}

void AnimationEditorDemo::AttachModel()
{
	/* 모델 연결 */
	Matrix mat = modelRender->GetboneTrans(0, selected);
	Matrix world = modelRender->GetTransform(0)->World();
	mat*=world;	
	Transform trans;
	trans.World(mat);
	trans.Scale(1, 1, 1);
	swordRender->UpdateInstTransform(0, 2, trans_sword->World()*trans.World());
	mat = swordRender->GetboneTrans(0, 2);
	trans_swordF->World(mat);
	trans_swordF->Scale(1, 1, 1);
}

void AnimationEditorDemo::SelectedPartsView()
{
	/* 파츠 선택 */
	if (Mouse::Get()->Down(0))
		for (int i = 0; i < boneNames.size(); i++)
		{
			if (sphere->GetPicked(i) == true)
			{
				selected = i; break;
			}
		}
	/* 선택 파츠 위치 표기 */
	Matrix W, V, P;
	D3DXMatrixIdentity(&W);
	V = Context::Get()->View();
	P = Context::Get()->Projection();
	Vector3 textPos, pos;
	sphere->GetInstTransform(selected)->Position(&pos);
	Context::Get()->GetViewport()->Project(&textPos, pos, W, V, P);
	string str;
	str += "Part_Name :" + String::ToString(boneNames[selected]->Name());

	Gui::Get()->RenderText(textPos.x - 50, textPos.y, str.c_str());

	/* 선택 파츠 구형 표기 */
	BoneCircle();
}

void AnimationEditorDemo::ShowChild(ModelBone * bone)
{
	auto childs = bone->Childs();
	ImGuiTreeNodeFlags flags = childs.empty() ? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnDoubleClick;
	
	if(bone->BoneIndex()-1 == selected)
		flags |= ImGuiTreeNodeFlags_Selected;

	if (ImGui::TreeNodeEx(String::ToString(bone->Name()).c_str(), flags))
	{
		if (ImGui::IsItemClicked())
		{
			selected = bone->BoneIndex() - 1;
		}
		for (auto& child : childs)
		{
			ShowChild(child);
		}
		ImGui::TreePop();
	}
}
