#include "stdafx.h"
#include "BillBoardInstancingDemo.h"
#include "Environment/Terrain.h"
#include "Environment/BillBoard.h"
#include "Environment/Sky.h"
#include "Viewer/Projector.h"
#include "Utilities/Xml.h"

void BillBoardInstancingDemo::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(45, 0, 0);
	Context::Get()->GetCamera()->Position(15, 150, -150);

	terrainShader = new Shader(L"036_Projector.fx");
	projector = new Projector(terrainShader, L"Rect_Brush.png", true);
	projector->AddTexture(L"Circle_Brush.png");

	terrain = new Terrain(terrainShader, L"HeightMap/Gray256.png");
	terrain->GetTransform()->Position(-120, 0.01f, -120);
	terrain->BaseMap(L"Terrain/Dirt.png");

	shader = new Shader(L"025_Geometry_BillBoard_Instance.fx");

	sky = new Sky(L"Environment/GrassCube1024.dds");
	vector<wstring> names = { L"Terrain/Tree.png",L"Terrain/Tree2.png", L"Terrain/Tree3.png", L"Terrain/Tree4.png", L"Terrain/Tree5.png" };
	bill = new BillBoard(shader, names);
	textureImages.push_back(new Texture(L"Terrain/Tree.png"));
	textureImages.push_back(new Texture(L"Terrain/Tree2.png"));
	textureImages.push_back(new Texture(L"Terrain/Tree3.png"));
	textureImages.push_back(new Texture(L"Terrain/Tree4.png"));
	textureImages.push_back(new Texture(L"Terrain/Tree5.png"));


	//CreateMesh
	{
		floor = new Material(shader);
		floor->DiffuseMap("Floor.png");
		grid = new MeshGrid(shader, 3, 3);
		grid->GetTransform()->Position(0, 0, 0);
		grid->GetTransform()->Scale(20, 1, 20);
	}

	for (int i = 0; i < 4; i++)
		filePaths[i] = L"";

}

void BillBoardInstancingDemo::Destroy()
{

	for (UINT i = 0; i < textureImages.size(); i++)
		SafeDelete(textureImages[i]);
	textureImages.clear();
	textureImages.shrink_to_fit();

	for (UINT i = 0; i < bills.size(); i++)
		SafeDelete(bills[i]);
	bills.clear();
	bills.shrink_to_fit();
	SafeDelete(floor);
	SafeDelete(grid);
	
	SafeDelete(terrain);
	SafeDelete(terrainShader);
	SafeDelete(sky);
	SafeDelete(shader);
}


void BillBoardInstancingDemo::Update()
{
	sky->Update();
	terrain->Update();

	int type0 = terrain->GetBrushType() - 1;

	/* 값이 변경 될때만 함수 호출 */
	if (type != type0)
	{
		type = type0;
		projector->SelectTex(type);
	}

	/* 마우스의 터레인 피킹 좌표 */
	Vector3 picked = terrain->GetPickedPosition();
	Vector3 tPos;
	terrain->GetTransform()->Position(&tPos);
	picked += tPos;
	/* 반지름 기준이라 2배 + 여유분? */
	float size = terrain->GetBrushRange() * 2 + 4;
	if (Mouse::Get()->Press(1) == false)
		projector->GetCamera()->Position(picked);
	projector->SetWH(size, size);
	projector->Update();
	UpdateSplatting();

	UINT bType = terrain->GetBrushType();
	
	/* 빌보드 브러시 상태 라디오 박스 */
	//ImGui::Separator();
	ImGui::Text("Bill_Brush_state");
	static int billState = 0;
	ImGui::RadioButton("None", &billState, 0); ImGui::SameLine();
	ImGui::RadioButton("Create", &billState, 1); ImGui::SameLine();
	ImGui::RadioButton("Delete", &billState, 2);

	billState = bType == 2 ? billState : billState == 2 ? 2 : 0;
	if (Mouse::Get()->Down(0) && Keyboard::Get()->Press(VK_SHIFT) == false)
	{
		if (billState == 1)
			CreateBills();
		if (billState == 2)
			DeleteBills();
	}
	
	/*텍스쳐 선택 콤보박스*/

	const char* items[] = { "Tree", "Tree2", "Tree3", "Tree4", "Tree5" };
	static const char* current_item = "Tree";

	if (ImGui::BeginCombo("##TreeType", current_item))
	{
		for (int n = 0; n < IM_ARRAYSIZE(items); n++)
		{
			bool is_selected = (current_item == items[n]);
			ImGui::Image(textureImages[n]->SRV(), ImVec2(32.0f, 32.0f));
			ImGui::SameLine();
			if (ImGui::Selectable(items[n], is_selected))
			{
				current_item = items[n];
				treeIndex = n;
				//meshType = MeshType(n);
			}
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	ImGui::SameLine();
	ImGui::Image(textureImages[treeIndex]->SRV(), ImVec2(48.0f, 48.0f));

	/* 높이 변동에 따른 재정렬 */
	if (ImGui::Button("Arrangement"))
		ReArrangeBills();
	ImGui::Separator();

	if (ImGui::Button("SaveToXml"))
		SaveToXml(L"TestSave.xml");
	if (ImGui::Button("LoadFromxml"))
		LoadFromXml();

	grid->Update();

	for (UINT i = 0; i < bills.size(); i++)
		bills[i]->Update();
}

void BillBoardInstancingDemo::Render()
{
	sky->Render();

	terrain->Pass(3);
	terrain->Render();

	//floor->Render();
	//grid->Pass(0);
	//grid->Render();

	for (UINT i = 0; i < bills.size(); i++)
	{
		bills[i]->Pass(2);
		bills[i]->Render();
	}
}

void BillBoardInstancingDemo::OpenFileDialog(UINT type, const wstring & filePath)
{
	if (filePath.length() < 1)
	{
		Path::OpenFileDialog(L"", Path::ImageFilter, L"", bind(&BillBoardInstancingDemo::OpenFileDialog, this, type, placeholders::_1));
	}
	else
	{
		switch (type)
		{
		case 0:
			terrain->LayerMaps(filePath, 0);
			filePaths[0] = filePath;
			break;
		case 1:
			terrain->LayerMaps(filePath, 1);
			filePaths[1] = filePath;
			break;
		case 2:
			terrain->LayerMaps(filePath, 2);
			filePaths[2] = filePath;
			break;
		case 3:
			terrain->BaseMap(filePath);
			filePaths[3] = filePath;
			break;
		default:
			break;
		}
	}
}

void BillBoardInstancingDemo::OpenTxtDialog(const wstring & filePath)
{
	if (filePath.length() < 1)
	{
		Path::OpenFileDialog(L"", Path::TextFilter, L"", bind(&BillBoardInstancingDemo::OpenTxtDialog, this, placeholders::_1));
	}
	else
	{
		//	FILE* file = fopen(String::ToString(filePath).c_str(), "r");
		char str[255];
		FILE* file;
		if (fopen_s(&file, String::ToString(filePath).c_str(), "r") != NULL)
		{
			assert(false);
		}
		fgets(str, 255, file);
		str[strlen(str) - 1] = '\0';
		terrain->AlphaMap(String::ToWString(str));

		fgets(str, 255, file);
		str[strlen(str) - 1] = '\0';
		terrain->BaseMap(String::ToWString(str));
		filePaths[3] = String::ToWString(str);

		fgets(str, 255, file);
		str[strlen(str) - 1] = '\0';
		terrain->LayerMaps(String::ToWString(str), 0);
		filePaths[0] = String::ToWString(str);

		fgets(str, 255, file);
		str[strlen(str) - 1] = '\0';
		terrain->LayerMaps(String::ToWString(str), 1);
		filePaths[1] = String::ToWString(str);

		fgets(str, 255, file);
		str[strlen(str) - 1] = '\0';
		terrain->LayerMaps(String::ToWString(str), 2);
		filePaths[2] = String::ToWString(str);

		//fprintf(file, "%s\n% s\n% s\n% s\n", String::ToString(filePaths[3]).c_str(), String::ToString(filePaths[0]).c_str(), String::ToString(filePaths[1]).c_str(), String::ToString(filePaths[2]).c_str());
		fclose(file);
	}
}

void BillBoardInstancingDemo::UpdateSplatting()
{

	static int layerIndex = 0;
	ImGui::RadioButton("LayerMap_1", &layerIndex, 0); ImGui::SameLine();
	ImGui::RadioButton("LayerMap_2", &layerIndex, 1); ImGui::SameLine();
	ImGui::RadioButton("LayerMap_3", &layerIndex, 2);
	ImGui::Separator();

	if (ImGui::Button("BaseMap Load"))
	{
		OpenFileDialog(3);
	}
	ImGui::SameLine();
	ImGui::Text(String::ToString(filePaths[3]).c_str());

	if (ImGui::Button("LayerMap_1 Load"))
	{
		OpenFileDialog(0);
		layerIndex = 0;
	}
	ImGui::SameLine();
	ImGui::Text(String::ToString(filePaths[0]).c_str());



	if (ImGui::Button("LayerMap_2 Load"))
	{
		OpenFileDialog(1);
		layerIndex = 1;
	}
	ImGui::SameLine();
	ImGui::Text(String::ToString(filePaths[1]).c_str());


	if (ImGui::Button("LayerMap_3 Load"))
	{
		OpenFileDialog(2);
		layerIndex = 2;
	}
	ImGui::SameLine();
	ImGui::Text(String::ToString(filePaths[2]).c_str());


	terrain->SetLayer(layerIndex);

	if (ImGui::Button("Save"))
	{
		wstring wstr = terrain->SaveTerrain();
		FILE* file;
		fopen_s(&file, "SplattingTest.txt", "w");
		fprintf(file, "%s\n", String::ToString(wstr).c_str());
		fprintf(file, "%s\n% s\n% s\n% s\n", String::ToString(filePaths[3]).c_str(), String::ToString(filePaths[0]).c_str(), String::ToString(filePaths[1]).c_str(), String::ToString(filePaths[2]).c_str());
		fclose(file);
	}

	if (ImGui::Button("Open"))
	{
		OpenTxtDialog();
	}

}

void BillBoardInstancingDemo::CreateBills()
{
	Vector3 picked = terrain->GetPickedPosition();
	//범위 밖이면 리턴
	if (picked.y < 0) return;

	Vector3 center;
	terrain->GetTransform()->Position(&center);
	float range = terrain->GetBrushRange();
	int minCount = range * range*0.5f;
	int maxCount = minCount * 2.0f;
	int randomV = maxCount;// Math::Random(minCount, maxCount);

	vector<Vector3> temps;

	for (float z = -range; z < range; z += 0.3f)
	{
		for (float x = -range; x < range; x += 0.3f)
		{
			float dist = sqrt(x * x + z * z);

			if (dist < range)
			{
				/* 선택한 위치를 기준으로 원형 영역 좌표 추출 */
				Vector3 fPos = Vector3(x, 0.0f, z) + picked;
				/* 높이에 따른 위치 조정 */
				float height = terrain->GetPickedHeight(fPos);
				if (height < 0)
					continue;
				fPos.y = height;
				/* 터레인 이동 값 보정 */
				fPos += center;
				temps.emplace_back(fPos);
			}
		}
	}

	center += picked;
	//범위내 
	for (UINT i = 0; i < bills.size(); i++)
	{
		if (1 > randomV) break;
		UINT count = bills[i]->GetCount();
		for (UINT j = 0; j < count; j++)
		{
			Vector3 pos;
			bills[i]->GetInstTransform(j)->Position(&pos);
			float dx = center.x - pos.x;
			float dz = center.z - pos.z;
			float dist = sqrt(dx * dx + dz * dz);

			if (dist < range)
			{
				UINT texNum = bills[i]->GetInstTexNum(j);
				/*
				만약 범위내에 현재 선택한 나무와 다른게 있다면 변경
				*/
				if (texNum != treeIndex)
				{
					bills[i]->SetInstTex(j, treeIndex);
					randomV--;
				}
			}
		}
		bills[i]->ResizeBuffer();
	}

	
	random_shuffle(temps.begin(), temps.end());
	

	/* randomV가 인스턴스 최대값을 넘길 수 있어서 */
	/* 나눗셈은 버림이라 +1 */
	UINT instCount = randomV / MAX_INSTANCE + 1;

	for (UINT j = 0; j < instCount; j++)
	{
		// 같이 안 넣으면 경로가 계속 붙음...
		vector<wstring> names = { L"Terrain/Tree.png",L"Terrain/Tree2.png", L"Terrain/Tree3.png", L"Terrain/Tree4.png", L"Terrain/Tree5.png" };
		BillBoard* instance = new BillBoard(shader, names);

		for (UINT i = 0; i < temps.size(); i++)
		{
			//인스턴스 최대값 이상이면 그만
			if (i >= MAX_INSTANCE)
			{
				randomV -= MAX_INSTANCE;
				break;
			}
			//랜덤값 넘으면 그만
			if (i >= randomV) break;

			UINT index = instance->Push();
			Transform* transform = instance->GetInstTransform(index);
			Vector3 position = temps[i];
			Vector3 scale = Math::RandomVec3(5, 10);
			position.y += scale.y *0.5f;
			scale.z = 0;
			transform->Position(position);
			transform->Scale(scale);

			instance->SetInstTex(index, treeIndex);
		}
		instance->ResizeBuffer();
		bills.emplace_back(instance);
	}
}

void BillBoardInstancingDemo::DeleteBills()
{
	Vector3 picked = terrain->GetPickedPosition();
	//범위 밖이면 리턴
	if (picked.y < 0) return;

	Vector3 center;
	terrain->GetTransform()->Position(&center);
	center += picked;
	float range = terrain->GetBrushRange();

	for (UINT i = 0; i < bills.size(); i++)
	{
		//UINT count = bills[i]->GetCount();
		for (UINT j = 0; j < bills[i]->GetCount();)
		{
			Vector3 pos;
			bills[i]->GetInstTransform(j)->Position(&pos);
			float dx = center.x - pos.x;
			float dz = center.z - pos.z;
			float dist = sqrt(dx * dx + dz * dz);

			//범위 내에 있으면 지우기
			if (dist < range)
			{
				bills[i]->Pop(j);
			}
			else
				j++;
		}
		bills[i]->ResizeBuffer();
	}

	for (UINT i = 0; i < bills.size(); )
	{
		//크기 0이면 삭제
		if (bills[i]->GetCount() < 1)
			bills.erase(bills.begin() + i);
		//안 지웠을때만 다음 주소로
		else
			i++;
	}
}

void BillBoardInstancingDemo::ReArrangeBills()
{
	Vector3 terrainCenter;
	terrain->GetTransform()->Position(&terrainCenter);

	for (UINT i = 0; i < bills.size(); i++)
	{
		UINT count = bills[i]->GetCount();
		for (UINT j = 0; j < count; j++)
		{
			Vector3 pos;
			bills[i]->GetInstTransform(j)->Position(&pos);
			Vector3 scale;
			bills[i]->GetInstTransform(j)->Scale(&scale);
			/* 터레인의 포지션을 0,0,0 기준으로 돌려서 높이 계산 */
			pos -= terrainCenter;
			float height = terrain->GetPickedHeight(pos);
			/* 터레인 위치 보정 */
			pos += terrainCenter;
			pos.y = height + scale.y *0.5f;
			/* 포지션 변경 */
			bills[i]->GetInstTransform(j)->Position(pos);
		}
		bills[i]->ResizeBuffer();
	}

}


void BillBoardInstancingDemo::SaveToXml(wstring savePath)
{
	string folder = String::ToString(Path::GetDirectoryName(savePath));
	string file = String::ToString(Path::GetFileName(savePath));

	Path::CreateFolders(folder);

	Xml::XMLDocument* document = new Xml::XMLDocument();

	Xml::XMLDeclaration* decl = document->NewDeclaration();
	document->LinkEndChild(decl);

	Xml::XMLElement* root = document->NewElement("GeometryDemo");
	document->LinkEndChild(root);

	Xml::XMLElement* node = document->NewElement("BillBoards");
	node->SetAttribute("Size", bills.size());
	root->LinkEndChild(node);

	for (UINT i = 0; i < bills.size(); i++)
	{
		UINT count = bills[i]->GetCount();
		Xml::XMLElement* billNode = document->NewElement("Bills");
		billNode->SetAttribute("DrawCount", count);
		node->LinkEndChild(billNode);


		for (UINT j = 0; j < count; j++)
		{
			Xml::XMLElement* transformNode = document->NewElement("Transforms");
			billNode->LinkEndChild(transformNode);

			Xml::XMLElement* element = NULL;

			//Transform 데이터 저장
			Vector3	pos;
			bills[i]->GetInstTransform(j)->Position(&pos);
			Vector3 scale;
			bills[i]->GetInstTransform(j)->Scale(&scale);

			element = document->NewElement("Scale");
			element->SetAttribute("ScaleX", scale.x);
			element->SetAttribute("ScaleY", scale.y);
			element->SetAttribute("ScaleZ", scale.z);
			transformNode->LinkEndChild(element);

			element = document->NewElement("Position");
			element->SetAttribute("PositionX", pos.x);
			element->SetAttribute("PositionY", pos.y);
			element->SetAttribute("PositionZ", pos.z);
			transformNode->LinkEndChild(element);

			element = document->NewElement("TextureNumber");
			element->SetAttribute("Number", bills[i]->GetInstTexNum(j));
			transformNode->LinkEndChild(element);
			//
		}
	}

	document->SaveFile((folder + file).c_str());

	terrain->SaveTerrain();
}

void BillBoardInstancingDemo::LoadFromXml(wstring file)
{
	if (file.length() < 1)
	{
		Path::OpenFileDialog(L"", Path::XmlFilter, L"", bind(&BillBoardInstancingDemo::LoadFromXml, this, placeholders::_1));
	}
	else
	{
		//불렀으면 기존것 삭제
		bill->Clear();


		Xml::XMLDocument* document = new Xml::XMLDocument();
		Xml::XMLError error = document->LoadFile(String::ToString(file).c_str());
		assert(error == Xml::XML_SUCCESS);

		Xml::XMLElement * root = document->FirstChildElement();
		Xml::XMLElement * node = root->FirstChildElement();
		int size = node->IntAttribute("Size");


		Xml::XMLElement * billNode = node->FirstChildElement();
		for (UINT i = 0; i < size; i++)
		{
			int count = billNode->IntAttribute("DrawCount");
			Xml::XMLElement * transformNode = billNode->FirstChildElement();

			/* */
			vector<wstring> names = { L"Terrain/Tree.png",L"Terrain/Tree2.png", L"Terrain/Tree3.png", L"Terrain/Tree4.png", L"Terrain/Tree5.png" };
			BillBoard* instance = new BillBoard(shader, names);
			
			for (UINT j = 0; j < count; j++)
			{
				Xml::XMLElement* node = NULL;
				node = transformNode->FirstChildElement();

				Vector3 pos;
				Vector3 scale;
				UINT texNum;
				scale.x = node->FloatAttribute("ScaleX");
				scale.y = node->FloatAttribute("ScaleY");
				scale.z = node->FloatAttribute("ScaleZ");
				node = node->NextSiblingElement();
				pos.x = node->FloatAttribute("PositionX");
				pos.y = node->FloatAttribute("PositionY");
				pos.z = node->FloatAttribute("PositionZ");

				node = node->NextSiblingElement();
				texNum = (UINT)node->IntAttribute("Number");

				UINT index = instance->Push();
				Transform* transform = instance->GetInstTransform(index);
				transform->Position(pos);
				transform->Scale(scale);
				instance->SetInstTex(index, texNum);

				transformNode = transformNode->NextSiblingElement();
			}
			instance->ResizeBuffer();
			bills.emplace_back(instance);
			billNode = billNode->NextSiblingElement();
		}
	}
	terrain->AlphaMap(L"TestAlphaMap.png");
	ReArrangeBills();
}
