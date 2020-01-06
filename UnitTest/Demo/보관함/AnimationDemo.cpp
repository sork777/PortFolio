#include "stdafx.h"
#include "AnimationDemo.h"
#include "Viewer/Freedom.h"
#include "Environment/Sky.h"
#include "Model/ModelMesh.h"
#include "Viewer/Orbit.h"
#include "Utilities/Xml.h"

void AnimationDemo::Initialize()
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
		model = new Model();
		model->ReadMaterial(L"tank/tank");
		model->ReadMesh(L"tank/tank");
		//model->ReadMaterial(L"Kachujin/Kachujin");
		//model->ReadMesh(L"Kachujin/Kachujin");
		modelRender = new ModelRender(shader, model);
		modelRender->AddTransform();
		modelRender->GetTransform(0)->Scale(1.05f,1.05f,1.05f);
		//modelRender->GetTransform(0)->Position(0,3.5f,0);
		//modelRender->AddTransform();
		modelRender->UpdateTransform();

		

		sphere = new MeshSphere(instMeshShader, 20.0f);
		
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
			frame.pivot = i==0?true:false;
			keyframes.emplace_back(frame);
		}

		gizmo = new Gizmo(sphere->GetInstTransform(0));
	}
	((Orbit *)Context::Get()->GetCamera())->GetObjPos(Vector3(0,0,0));	
	sphere->Render();

}

void AnimationDemo::Destroy()
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



void AnimationDemo::Update()
{
	sky->Update();
	grid->Update();

	sphere->Update();

	modelRender->Update();
	modelRender->UpdateTransform();

	//TODO: 함수화
	/* 모델 연결 */
	/*Matrix mat = modelRender->GetboneTrans(0, 11);
	Matrix world = modelRender->GetTransform(0)->World();
	mat*=world;
	modelRender->GetTransform(1)->World(mat);
	Transform trans;
	trans.World(mat);
	Vector3 pos;
	modelRender->GetTransform(1)->Position(&pos);
	pos -= trans.Forward()*5.0f;
	modelRender->GetTransform(1)->Position(pos);
	modelRender->GetTransform(1)->Scale(0.3f, 0.3f, 0.3f);*/
	
	//TODO: 함수화
	/* 파츠 선택 */
	if(Mouse::Get()->Down(0))
		for (int i = 0; i < boneNames.size(); i++)
		{
			if (sphere->GetPicked(i) == true)
			{
				selected = i; break;
			}
		}
	//TODO: 함수화
	/* 선택 파츠 위치 표기 */
	Matrix W, V, P;
	D3DXMatrixIdentity(&W);
	V = Context::Get()->View();
	P = Context::Get()->Projection();
	Vector3 textPos,pos;
	sphere->GetInstTransform(selected)->Position(&pos);
	Context::Get()->GetViewport()->Project(&textPos, pos, W, V, P);
	string str;
	str += "Part_Name :" + String::ToString(boneNames[selected]->Name());
	
	Gui::Get()->RenderText(textPos.x-50,textPos.y,str.c_str());

	/* 선택 파츠 구형 표기 */
	BoneCircle();
	
	gizmo->ChangeTrans(sphere->GetInstTransform(selected));
	gizmo->Update();
	ImGUIController();
	ShowParts();
	Normalizer();
}

void AnimationDemo::Render()
{
	sky->Render();



	floor->Render();
	grid->Pass(0);
	grid->Render();


	//sphere->Pass(0);
	//sphere->Render();

	modelRender->Pass(1);
	modelRender->Render();
	/* 뎁스를 껐음...*/
	gizmo->Render();
}

void AnimationDemo::ImGUIController()
{
	bool bDocking = true;
	//ImGui::ShowDemoWindow(&bDocking);
	ImGui::Begin("Animation", &bDocking);
	{
		static bool bPlay = false;
		/* 파츠 움직임의 독립성 여부 */
		static bool bIndep = false;
		static int timer = 0;

		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
		if (ImGui::CollapsingHeader("Selected_Parts",ImGuiTreeNodeFlags_DefaultOpen))
		{
			/* 그룹으로 묶어서 파트 선택 옆에 놓기 */
			ImGui::BeginGroup();
			{
				
				Transform* transform = new Transform();
				Matrix transMat = boneNames[selected]->Transform();
				transform->World(transMat);
				Vector3 S, R, T;
				Vector3 bS, bR, bT;
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
					bS = S;
					bR = R;
					bT = T;
					ImGui::SliderFloat3("Scale", (float*)&S, 0.01f, 100.0f);
					ImGui::SliderFloat3("Rotation", (float*)&R, -Math::PI*0.9f, Math::PI*0.9f);
					if (bIndep == true)
						ImGui::SliderFloat3("Position", (float*)&T, -100.0f, 100.0f);
					else
						ImGui::SliderFloat3("Position", (float*)&T, -10.0f, 10.0f);

					T.y = T.y > 0 ? T.y : 0;

					/* 변형시 피봇 자동 적용 */
					if (bS != S)
						keyframes[selectedFrame].pivot = true;
					else if (bR != R)
						keyframes[selectedFrame].pivot = true;
					else if (bT != T)
						keyframes[selectedFrame].pivot = true;

					transform->Scale(S);
					transform->Rotation(R);
					transform->Position(T);
				
					modelRender->UpdateInstTransform(0, selected + 1, transform->World(), bIndep);
				}
				/* 재생중일때 정보 표기 */
				else
					{
						transform = (keyframes[timer].partsTrans[selected]);
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
					ImGui::Checkbox("Pivot?", &keyframes[selectedFrame].pivot);
					ImGui::Checkbox("Parts_Indepency", &bIndep);
				}
				else
					ImGui::Text("Frame Number : %d", timer);

				ImGui::SameLine();
				
				/* 프레임에 따라 모든 파츠가 변동되게 하기 위함 */
				{
					for (UINT i = 0; i < boneNames.size(); i++)
					{
						Transform* transform = keyframes[selectedFrame].partsTrans[i];
						modelRender->UpdateInstTransform(0, i + 1, transform->World());
						transform = keyframes[selectedFrame].partsTransIndep[i];
						modelRender->UpdateInstTransform(0, i + 1, transform->World(),true);

						Matrix mat = modelRender->GetboneTrans(0, i+1);
						Matrix world = modelRender->GetTransform(0)->World();
						mat *= world;
						sphere->GetInstTransform(i)->World(mat);
					}
				}
				

			}
			ImGui::EndGroup();

		}
		ImGui::PopStyleVar();

		/* Play */
		if (ImGui::Button("Play"))
		{
			if (bPlay != true)
				bPlay = true;
		}
		if (bPlay == true)
		{
			for (UINT b = 0; b < boneNames.size(); b++)
			{
				Transform* transform = keyframes[timer].partsTrans[b];
				modelRender->UpdateInstTransform(0, b + 1, transform->World());
			}
			timer++;
			if (timer > MAX_FRAMES)
			{
				bPlay = false;
				timer = 0;
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
				frame.pivot = i == 0 ? true : false;
				keyframes.emplace_back(frame);
			}
		}

	}
	ImGui::End();
}

void AnimationDemo::ShowParts()
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

void AnimationDemo::Normalizer()
{
	UINT startPivot = 0;
	UINT endPivot = 0;
	bool start = false;
	bool end = false;
	for (UINT i = 0; i < MAX_FRAMES+1; i++)
	{
		/* start가 선택 x*/
		if (start != true)
		{
			start = keyframes[i].pivot;
			startPivot = start ? i : 0;
		}
		/* end가 선택 x*/
		else if (end != true)
		{
			end = keyframes[i].pivot;
			endPivot = end ? i : 0;
		}

		/* start end 둘다 선택*/
		if (start && end)
		{
			Normalizer(startPivot, endPivot);
			end = false;
			startPivot = endPivot;
		}
		else if (i == MAX_FRAMES)
		{
			/* 시작 피봇만 있는채로 프레임 끝나면 시작 피봇 부터 끝까지 같은 형태로 */
			for (UINT b = 0; b < boneNames.size(); b++)
			{
				Transform* selectedTrans = keyframes[startPivot].partsTrans[b];
				Vector3 S, R, T;
				selectedTrans->Scale(&S);
				selectedTrans->Rotation(&R);
				selectedTrans->Position(&T);
				for (UINT j = startPivot ; j < MAX_FRAMES+1; j++)
				{
					Transform* trans = keyframes[j].partsTrans[b];
					trans->Scale(S);
					trans->Rotation(R);
					trans->Position(T);
				}
			}
		}
	}
}

void AnimationDemo::Normalizer(UINT start, UINT end)
{
	/* 갭 */
	float gap = end - start;

	for (UINT i = 0; i < boneNames.size(); i++)
	{
		/* 보간 시작 점 */
		Transform* startTrans = keyframes[start].partsTrans[i];
		/* 보간 끝 점 */
		Transform* endTrans = keyframes[end].partsTrans[i];
		/* 보간 시작 요소*/
		Vector3 sS, sR, sT;
		/* 보간 끝 요소*/
		Vector3 eS, eR, eT;
		/* 보간 요소별 차이 */
		Vector3 gS, gR, gT;
		startTrans->Scale(&sS);
		startTrans->Rotation(&sR);
		startTrans->Position(&sT);

		endTrans->Scale(&eS);
		endTrans->Rotation(&eR);
		endTrans->Position(&eT);
		
		/* 요소별 차이 계산 */
		gT = (eT - sT) / gap;
		gR = (eR - sR) / gap;
		gS = Vector3(
			sS.x == 0.0f ? 1.0f : pow(eS.x / sS.x, 1/gap),
			sS.y == 0.0f ? 1.0f : pow(eS.y / sS.y, 1/gap),
			sS.z == 0.0f ? 1.0f : pow(eS.z / sS.z, 1/gap));
		
		/* 보간 시작 */
		for (UINT j = start + 1; j < end; j++)
		{
			UINT times = j - start;
			Transform* trans = keyframes[j].partsTrans[i];
			Vector3 S, R, T;
			T = sT + gT * times;
			R = sR + gR * times;
			S = Vector3(
				sS.x*pow(gS.x, times),
				sS.y*pow(gS.y, times),
				sS.z*pow(gS.z, times));
			trans->Scale(S);
			trans->Rotation(R);
			trans->Position(T);
		}
	}
}

void AnimationDemo::SaveAnimation(wstring savePath)
{
	if (savePath.length() < 1)
	{
		Path::SaveFileDialog(L"", Path::XmlFilter, L"", bind(&AnimationDemo::SaveAnimation, this, placeholders::_1));
	}
	else
	{
		string folder = String::ToString(Path::GetDirectoryName(savePath));
		string file = String::ToString(Path::GetFileName(savePath));

		Path::CreateFolders(folder);

		Xml::XMLDocument* document = new Xml::XMLDocument();

		Xml::XMLDeclaration* decl = document->NewDeclaration();
		document->LinkEndChild(decl);

		Xml::XMLElement* root = document->NewElement("AnimationDemo");
		document->LinkEndChild(root);


		Xml::XMLElement* node = document->NewElement("Animation");
		node->SetAttribute("MAX_FRAMES", MAX_FRAMES);
		node->SetAttribute("Size", boneNames.size());
		root->LinkEndChild(node);

		for (UINT i = 0; i < MAX_FRAMES + 1; i++)
		{
			Xml::XMLElement* frameNode = document->NewElement("Frame");
			frameNode->SetAttribute("FrameNumber", i);
			frameNode->SetAttribute("Pivot", keyframes[i].pivot);
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

void AnimationDemo::LoadAnimation(wstring file)
{
	if (file.length() < 1)
	{
		Path::OpenFileDialog(L"", Path::XmlFilter, L"", bind(&AnimationDemo::LoadAnimation, this, placeholders::_1));
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
			frame.pivot= frameNode->BoolAttribute("Pivot");
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

void AnimationDemo::BoneCircle()
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

void AnimationDemo::ShowChild(ModelBone * bone)
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
