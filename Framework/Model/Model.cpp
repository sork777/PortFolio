#include "Framework.h"
#include "Model.h"
#include "ModelMesh.h"
#include "Utilities/Xml.h"
#include "Utilities/BinaryFile.h"

Model::Model(Shader* shader)
{
	Initialize();
	SetShader(shader);
}

Model::Model(const Model& model)
{
	Initialize();
	//쉐이더는 같은애를 공유하는 거임. 깊은 복사때도 그냥 같은 주소
	SetShader(model.shader);
	materialFilePath = model.materialFilePath;
	materialDirPath = model.materialDirPath;
	meshFilePath = model.meshFilePath;
	meshDirPath = model.meshDirPath;
	animClipCount = model.animClipCount;

	ReadMaterial(materialFilePath, materialDirPath);
	ReadMesh(meshFilePath, meshDirPath);
}

Model::~Model()
{
	for (Material* material : materials)
		SafeDelete(material);
	materials.clear();
	materials.shrink_to_fit();

	for (ModelBone* bone : bones)
		SafeDelete(bone);
	bones.clear();
	bones.shrink_to_fit();

	for (ModelMesh* mesh : meshes)
		SafeDelete(mesh);
	meshes.clear();
	meshes.shrink_to_fit();

	for (Transform* transform : transforms)
		SafeDelete(transform);
	transforms.clear();
	transforms.shrink_to_fit();

	SafeRelease(editTexture);
	SafeRelease(editSrv);
	//
	SafeRelease(bonebuffer);
	SafeRelease(boneSrv);
	SafeDelete(instanceBuffer);

}

void Model::ModelMeshChanger(const Model& model)
{
	for (Material* material : materials)
		SafeDelete(material);
	materials.clear();
	materials.shrink_to_fit();

	for (ModelBone* bone : bones)
		SafeDelete(bone);
	bones.clear();
	bones.shrink_to_fit();

	for (ModelMesh* mesh : meshes)
		SafeDelete(mesh);
	meshes.clear();
	meshes.shrink_to_fit();

	SafeRelease(editTexture);
	SafeRelease(editSrv);
	SafeRelease(bonebuffer);
	SafeRelease(boneSrv);

	bAnimated = false;
	bCalcVolume = false;

	materialFilePath = model.materialFilePath;
	materialDirPath = model.materialDirPath;
	meshFilePath = model.meshFilePath;
	meshDirPath = model.meshDirPath;
	ReadMaterial(materialFilePath, materialDirPath);
	ReadMesh(meshFilePath, meshDirPath);
}

void Model::SetShader(Shader * shader)
{
	this->shader = shader;
	sBoneTransformsSRV = this->shader->AsSRV("BoneTransformsMap");
	sAnimEditSRV = this->shader->AsSRV("AnimEditTransformMap");

	for (ModelMesh* mesh : meshes)
		mesh->SetShader(shader);
}

void Model::Initialize()
{
	name = L"";
	materialFilePath = L"";
	materialDirPath = L"";
	meshFilePath = L"";
	meshDirPath = L"";

	for (UINT i = 0; i < MAX_MODEL_INSTANCE; i++)
		D3DXMatrixIdentity(&worlds[i]);
	instanceBuffer = new VertexBuffer(worlds, MAX_MODEL_INSTANCE, sizeof(Matrix), 1, true);

}

#pragma region 메시 출력 관련

void Model::Update()
{
	if (bonebuffer == NULL)
	{
		CreateBoneBuffer();
	}
	if (editTexture == NULL)
	{
		CreateAnimEditTexture();
	}

	if (false == bCalcVolume)
		CalcMeshVolume();
	for (ModelMesh* mesh : meshes)
		mesh->Update();
	UpdateTransforms();
}

void Model::Render()
{
	//일단 메시 연결하면 자동 생성되긴함.

	if (editSrv != NULL)
		sAnimEditSRV->SetResource(editSrv);

	instanceBuffer->Render();

	if (boneSrv != NULL)
		sBoneTransformsSRV->SetResource(boneSrv);

	int draw = transforms.size();
	for (ModelMesh* mesh : meshes)
		mesh->Render(draw);

	bAdded = false;
}

void Model::Pass(const UINT& pass)
{
	this->pass = pass;
	for (ModelMesh* mesh : meshes)
		mesh->Pass(pass);
}

void Model::Tech(const UINT& tech)
{
	this->tech = tech;
	for (ModelMesh* mesh : meshes)
		mesh->Tech(tech);
}


void Model::UpdateTransforms()
{
	bool bChange = false;

	for (UINT i = 0; i < transforms.size(); i++)
	{
		if (worlds[i] != transforms[i]->World())
		{
			bChange = true;
			memcpy(worlds[i], transforms[i]->World(), sizeof(Matrix));
		}
	}
	if (bChange)
	{
		D3D11_MAPPED_SUBRESOURCE subResource;
		D3D::GetDC()->Map(instanceBuffer->Buffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
		{
			memcpy(subResource.pData, worlds, sizeof(Matrix) * MAX_MODEL_INSTANCE);
		}
		D3D::GetDC()->Unmap(instanceBuffer->Buffer(), 0);
	}
}

void Model::AddInstance()
{
	if (MAX_MODEL_INSTANCE <= transforms.size()+1)
		return;

	AddTransform();
	bAdded = true;
}

void Model::DelInstance(const UINT& instance)
{
	if (instance >= transforms.size())
		return;
	transforms.erase(transforms.begin() + instance);

}

void Model::AddTransform()
{
	Transform* transform = new Transform();
	transforms.push_back(transform);
}

#pragma endregion


#pragma region 이름으로 데이터 읽기

Material * Model::MaterialByName(const wstring& name)
{
	for (Material* material : materials)
	{
		if (material->Name() == name)
			return material;
	}

	return NULL;
}

ModelBone * Model::BoneByName(const wstring& name)
{
	for (ModelBone* bone : bones)
	{
		if (bone->Name() == name)
			return bone;
	}

	return NULL;
}

const int& Model::BoneIndexByName(const wstring& name)
{
	int result = -1;
	for (ModelBone* bone : bones)
	{
		result++;
		if (bone->Name() == name)
			return result;
	}

	return -1;
}

ModelMesh * Model::MeshByName(const wstring& name)
{
	for (ModelMesh* mesh : meshes)
	{
		if (mesh->Name() == name)
			return mesh;
	}

	return NULL;
}


#pragma endregion


#pragma region ReadData

void Model::ReadMaterial(const wstring& file, const wstring& directoryPath)
{
	materialFilePath = file;
	materialDirPath = directoryPath;
	wstring readfile = directoryPath + file + L".material";

	Xml::XMLDocument* document = new Xml::XMLDocument();
	Xml::XMLError error = document->LoadFile(String::ToString(readfile).c_str());
	assert(error == Xml::XML_SUCCESS);

	Xml::XMLElement* root = document->FirstChildElement();
	bAnimated = root->BoolAttribute("IsAnimation");
	Xml::XMLElement* materialNode = root->FirstChildElement();


	do
	{
		Material* material = new Material();


		Xml::XMLElement* node = NULL;

		node = materialNode->FirstChildElement();
		material->Name(String::ToWString(node->GetText()));


		wstring directory = Path::GetDirectoryName(readfile);
		String::Replace(&directory, directoryPath, L"");


		wstring texture = L"";

		node = node->NextSiblingElement();
		texture = String::ToWString(node->GetText());
		if (texture.length() > 0)
			material->LoadDiffuseMapW(directory + texture, directoryPath);

		node = node->NextSiblingElement();
		texture = String::ToWString(node->GetText());
		if (texture.length() > 0)
			material->LoadSpecularMapW(directory + texture, directoryPath);

		node = node->NextSiblingElement();
		texture = String::ToWString(node->GetText());
		if (texture.length() > 0)
			material->LoadNormalMapW(directory + texture, directoryPath);


		D3DXCOLOR color;

		node = node->NextSiblingElement();
		color.r = node->FloatAttribute("R");
		color.g = node->FloatAttribute("G");
		color.b = node->FloatAttribute("B");
		color.a = node->FloatAttribute("A");
		material->Ambient(color);

		node = node->NextSiblingElement();
		color.r = node->FloatAttribute("R");
		color.g = node->FloatAttribute("G");
		color.b = node->FloatAttribute("B");
		color.a = node->FloatAttribute("A");
		material->Diffuse(color);

		node = node->NextSiblingElement();
		color.r = node->FloatAttribute("R");
		color.g = node->FloatAttribute("G");
		color.b = node->FloatAttribute("B");
		color.a = node->FloatAttribute("A");
		material->Specular(color);

		node = node->NextSiblingElement();
		color.r = node->FloatAttribute("R");
		color.g = node->FloatAttribute("G");
		color.b = node->FloatAttribute("B");
		color.a = node->FloatAttribute("A");
		material->Emissive(color);

		materials.push_back(material);


		materialNode = materialNode->NextSiblingElement();
	} while (materialNode != NULL);
}

void Model::ReadMesh(const wstring& file, const wstring& directoryPath)
{
	meshFilePath = file;
	meshDirPath = directoryPath;
	wstring readfile = directoryPath + file + L".mesh";
	name = Path::GetFileNameWithoutExtension(readfile);
	if (name.find(L"Mesh") != wstring::npos)
	{
		name = Path::GetDirectDirectoryName(readfile);
	}


	BinaryReader* r = new BinaryReader();
	r->Open(readfile);

	UINT count = 0;

	count = r->UInt();
	for (UINT i = 0; i < count; i++)
	{
		ModelBone* bone = new ModelBone();

		bone->index = r->Int();
		bone->name = String::ToWString(r->String());

		bone->parentIndex = r->Int();
		bone->world = r->Matrix();
		bone->boneTransform->Local(bone->world);
		//bone->attachID = 0;
		bones.push_back(bone);
	}


	count = r->UInt();
	for (UINT i = 0; i < count; i++)
	{
		ModelMesh* mesh = new ModelMesh();

		mesh->name = String::ToWString(r->String());
		mesh->boneIndex = r->Int();

		mesh->materialName = String::ToWString(r->String());

		//VertexData
		{
			UINT count = r->UInt();

			vector<Model::ModelVertex> vertices;
			vertices.assign(count, Model::ModelVertex());

			void* ptr = (void *)&(vertices[0]);
			r->Byte(&ptr, sizeof(Model::ModelVertex) * count);


			mesh->vertices = new Model::ModelVertex[count];
			mesh->vertexCount = count;
			copy
			(
				vertices.begin(), vertices.end(),
				stdext::checked_array_iterator<Model::ModelVertex *>(mesh->vertices, count)
			);
		}

		//IndexData
		{
			UINT count = r->UInt();

			vector<UINT> indices;
			indices.assign(count, UINT());

			void* ptr = (void *)&(indices[0]);
			r->Byte(&ptr, sizeof(UINT) * count);


			mesh->indices = new UINT[count];
			mesh->indexCount = count;
			copy
			(
				indices.begin(), indices.end(),
				stdext::checked_array_iterator<UINT *>(mesh->indices, count)
			);
		}

		meshes.push_back(mesh);
	}//for(i)

	r->Close();
	SafeDelete(r);


	BindBone();
	BindMesh();

	for (ModelMesh* mesh : meshes)
		mesh->SetShader(shader);
	// 값 할당
	{
		prevBoneCount = BoneCount();
		for (UINT b = 0; b < MAX_BONE_TRANSFORMS; b++)
		{
			if (b < prevBoneCount)
			{
				ModelBone* bone = BoneByIndex(b);
				Matrix InvGlobal = bone->BoneWorld();
				D3DXMatrixInverse(&InvGlobal, NULL, &InvGlobal);
				// 모델 본 그자체의 데이터
				boneTrans[b] = InvGlobal;
			}
			else
				D3DXMatrixIdentity(&boneTrans[b]);

			for (UINT c = 0; c < MAX_ANIMATION_CLIPS; c++)
			{
				D3DXMatrixIdentity(&animEditTrans[c][b]);
			}
		}//for(b)		
	}
	CreateBoneBuffer();
	CreateAnimEditTexture();
}

void Model::BindBone()
{
	rootBone = bones[0];
	for (ModelBone* bone : bones)
	{
		if (bone->parentIndex > -1)
		{
			bone->parent = bones[bone->parentIndex];
			//애니메이션에 맞춰 변동줄 녀석
			bone->GetEditTransform()->Parent(bone->parent->GetEditTransform());
			bone->parent->childs.push_back(bone);
		}
		else
			bone->parent = NULL;
	}
}

void Model::BindMesh()
{
	for (ModelMesh* mesh : meshes)
	{
		for (ModelBone* bone : bones)
		{
			if (mesh->boneIndex == bone->index)
			{
				mesh->bone = bone;

				break;
			}
		}

		mesh->Binding(this);
	}
}

#pragma endregion

///////////////////////////////////////////////////////////////////////////////

void Model::AddSocket(const int& parentBoneIndex, const wstring& socketName)
{
	//메시에는 변동 없음에 주의
	ModelBone* parentBone = BoneByIndex(parentBoneIndex);
	ModelBone* newBone = new ModelBone();
	newBone->name = socketName;
	newBone->world = parentBone->world;
	newBone->boneTransform->Local(newBone->world);
	newBone->editTransform->Local(parentBone->editTransform->Local());
	newBone->editTransform->Parent(parentBone->editTransform->ParentTransform());
	newBone->parentIndex = parentBoneIndex;
	newBone->parent = parentBone;
	newBone->parent->childs.push_back(newBone);

	newBone->index = bones.size();

	bones.push_back(newBone);

	Matrix InvGlobal = newBone->BoneWorld();
	D3DXMatrixInverse(&InvGlobal, NULL, &InvGlobal);
	boneTrans[prevBoneCount] = InvGlobal;

	CreateBoneBuffer();
	CreateAnimEditTexture();
}

void Model::CreateBoneBuffer()
{

	//CreateTexture
	{
		D3D11_TEXTURE1D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE1D_DESC));
		desc.Width = BoneCount() * 4;
		desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.MipLevels = 1;
		desc.ArraySize = 1;

		D3D11_SUBRESOURCE_DATA subResource;
		subResource.pSysMem = boneTrans;
		subResource.SysMemPitch = BoneCount() * sizeof(Matrix);
		subResource.SysMemSlicePitch = BoneCount() * sizeof(Matrix);

		Check(D3D::GetDevice()->CreateTexture1D(&desc, &subResource, &bonebuffer));
	}


	//Create SRV
	{
		D3D11_TEXTURE1D_DESC desc;
		bonebuffer->GetDesc(&desc);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
		srvDesc.Texture1D.MipLevels = 1;
		srvDesc.Format = desc.Format;

		Check(D3D::GetDevice()->CreateShaderResourceView(bonebuffer, &srvDesc, &boneSrv));
	}

}

#pragma region 애니메이션 클립 변화 텍스쳐 생성 및 변화 구현 영역

void Model::CreateAnimEditTexture()
{
	// 해당 버퍼가 애니메이션의 변화를 담당한다.
	// 기본 값은 identity
	//CreateTexture
	{
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
		desc.Width = BoneCount() * 4;
		desc.Height = MAX_ANIMATION_CLIPS;
		desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.SampleDesc.Count = 1;

		D3D11_SUBRESOURCE_DATA subResource;
		subResource.pSysMem = animEditTrans;
		subResource.SysMemPitch = BoneCount() * sizeof(Matrix);
		subResource.SysMemSlicePitch = BoneCount() * sizeof(Matrix)*MAX_ANIMATION_CLIPS;

		Check(D3D::GetDevice()->CreateTexture2D(&desc, &subResource, &editTexture));
	}

	//Create SRV
	{
		D3D11_TEXTURE2D_DESC desc;
		editTexture->GetDesc(&desc);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Format = desc.Format;

		Check(D3D::GetDevice()->CreateShaderResourceView(editTexture, &srvDesc, &editSrv));
	}

}

#pragma region BoneEdit영역

void Model::UpdateBoneTransform(const UINT& part, const UINT& clipID, Transform* transform)
{
	if (editTexture == NULL)
		return;
	ModelBone* bone = BoneByIndex(part);

	Matrix trans = transform->World();
	bone->GetEditTransform()->Local(trans);

	Matrix result = bone->GetEditTransform()->World();
	animEditTrans[clipID][part] = result;
	for (ModelBone* child : bone->Childs())
	{
		UpdateChildBones(part, child->Index(), clipID);
	}

	D3D11_BOX destRegion;
	/* 행 하나의 크기 */
	destRegion.left = 0;
	destRegion.right = 4 * BoneCount();
	/* 빼올 인스턴스 행의 위치 */
	destRegion.top = clipID;
	destRegion.bottom = clipID + 1;
	destRegion.front = 0;
	destRegion.back = 1;

	/* 업데이트 */
	D3D::GetDC()->UpdateSubresource
	(
		editTexture,
		0,
		&destRegion,
		animEditTrans[clipID],
		sizeof(Matrix)*BoneCount(),
		0
	);
}

void Model::UpdateChildBones(const UINT& parentID, const UINT& childID, const UINT& clipID)
{
	ModelBone* bone = BoneByIndex(childID);

	bone->GetEditTransform()->Update();
	Matrix result = bone->GetEditTransform()->World();
	animEditTrans[clipID][childID] = result;
	for (ModelBone* child : bone->Childs())
	{
		UpdateChildBones(childID, child->Index(), clipID);
	}
}

#pragma endregion

void Model::AddSocketEditData(const UINT& boneID, const UINT& clipCount)
{
	ModelBone* bone = BoneByIndex(boneID);
	for (UINT x = 0; x < clipCount; x++)
	{
		Matrix result = bone->GetEditTransform()->World();
		animEditTrans[x][boneID] = result;

		D3D11_BOX destRegion;
		/* 행 하나의 크기 */
		destRegion.left = 0;
		destRegion.right = 4 * BoneCount();
		/* 빼올 인스턴스 행의 위치 */
		destRegion.top = x;
		destRegion.bottom = x + 1;
		destRegion.front = 0;
		destRegion.back = 1;

		/* 업데이트 */
		D3D::GetDC()->UpdateSubresource
		(
			editTexture,
			0,
			&destRegion,
			animEditTrans[x],
			sizeof(Matrix)*BoneCount(),
			0
		);
	}
}

#pragma endregion

const int& Model::BoneHierarchy(int* click)
{
	/* 파츠 선택 */
	//ImGui::BeginChild("BoneHierarchy");
	{
		for (UINT i = 0; i < bones.size(); i++)
		{
			auto root = bones[i];
			if (bones[i]->ParentIndex() < 0)
				ChildBones(root, click);
		}
	}
	//ImGui::EndChild();
	return selectedBoneNum;
}

void Model::ChildBones(ModelBone * bone, int* click)
{
	ImVec2 pos = ImGui::GetItemRectMin() - ImGui::GetWindowPos();
	//윈도우 사이즈
	ImVec2 wSize = ImGui::GetWindowSize();
	if (wSize.y < pos.y) return;
	if (wSize.x < pos.x) return;

	auto childs = bone->Childs();
	ImGuiTreeNodeFlags flags = childs.empty() ? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnDoubleClick;

	if (bone->Index() == selectedBoneNum)
		flags |= ImGuiTreeNodeFlags_Selected;
	ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 10.0f);

	if (ImGui::TreeNodeEx(String::ToString(bone->Name()).c_str(), flags))
	{
		if (ImGui::IsItemClicked())
		{
			selectedBoneNum = bone->Index();
			*click = 0;
		}
		if (ImGui::IsItemClicked(1))
		{
			selectedBoneNum = bone->Index();
			*click = 1;
		}
		for (auto& child : childs)
		{
			ChildBones(child, click);
		}
		ImGui::TreePop();
	}
	ImGui::PopStyleVar();
}

void Model::CalcMeshVolume()
{
	bCalcVolume = true;
	bool bFirst = true;
	//ModelClip* clip = model->ClipByIndex(index);

	for (ModelMesh* mesh : meshes)
	{
		ModelBone* bone = mesh->Bone();

		UINT vCount = mesh->GetVertexCount();
		Model::ModelVertex* vertices = mesh->GetVertices();

		for (UINT i = 0; i < vCount; i++)
		{
			/* 전부 0인 행렬 */
			Vector3 vTemp = vertices[i].Position;
			if (true == bAnimated)
			{
				Matrix trans;
				D3DXMatrixIdentity(&trans);
				trans._11 = 0.0f;
				trans._22 = 0.0f;
				trans._33 = 0.0f;
				trans._44 = 0.0f;
				/* 각 버텍스가 가진 boneindex와 weight로
				해당 버텍스의 최종 변환 행렬 구하기 */
				for (UINT j = 0; j < 4; j++)
				{
					UINT index = vertices[i].BlendIndices[j];
					float weight = vertices[i].BlendWeights[j];
					Matrix matrix = boneTrans[index];
					trans += weight * matrix;
				}
				/* 해당 본의 Transform 행렬을 통해 정점의 위치를 이동 해야함 */
				D3DXVec3TransformCoord(&vTemp, &vTemp, &trans);
			}
			if (bFirst)
			{
				minV = vTemp;
				maxV = vTemp;
				bFirst = false;
			}
			else
			{
				minV.x = minV.x > vTemp.x ? vTemp.x : minV.x;
				minV.y = minV.y > vTemp.y ? vTemp.y : minV.y;
				minV.z = minV.z > vTemp.z ? vTemp.z : minV.z;

				maxV.x = maxV.x < vTemp.x ? vTemp.x : maxV.x;
				maxV.y = maxV.y < vTemp.y ? vTemp.y : maxV.y;
				maxV.z = maxV.z < vTemp.z ? vTemp.z : maxV.z;
			}
		}
	}


}