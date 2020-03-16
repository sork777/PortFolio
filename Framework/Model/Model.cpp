#include "Framework.h"
#include "Model.h"
#include "ModelMesh.h"
#include "Utilities/Xml.h"
#include "Utilities/BinaryFile.h"

Model::Model(Shader* shader)
	:shader(shader)
	,name(L"")
	,materialFilePath(L""), materialDirPath(L"")
	,meshFilePath(L""), meshDirPath(L"")
{
	for (UINT i = 0; i < MAX_MODEL_INSTANCE; i++)
		D3DXMatrixIdentity(&worlds[i]);
	instanceBuffer = new VertexBuffer(worlds, MAX_MODEL_INSTANCE, sizeof(Matrix), 1, true);

	sBoneTransformsSRV = shader->AsSRV("BoneTransformsMap");
}

Model::Model(Model* model)
	: name(L"")
	, materialFilePath(L""), materialDirPath(L"")
	, meshFilePath(L""), meshDirPath(L"")
{
	for (UINT i = 0; i < MAX_MODEL_INSTANCE; i++)
		D3DXMatrixIdentity(&worlds[i]);
	instanceBuffer = new VertexBuffer(worlds, MAX_MODEL_INSTANCE, sizeof(Matrix), 1, true);


	shader				= model->GetShader();
	sBoneTransformsSRV	= shader->AsSRV("BoneTransformsMap");

	materialFilePath	= model->MaterialPath();
	materialDirPath		= model->MaterialDir();
	meshFilePath		= model->MeshPath();
	meshDirPath			= model->MeshDir();	
	ReadMaterial(materialFilePath, materialDirPath);
	ReadMesh(meshFilePath, meshDirPath);
}

Model::~Model()
{
	for (Material* material : materials)
		SafeDelete(material);

	for (ModelBone* bone : bones)
		SafeDelete(bone);

	for (ModelMesh* mesh : meshes)
		SafeDelete(mesh);

	for (Transform* transform : transforms)
		SafeDelete(transform);

	SafeDelete(shader);
	SafeRelease(bonebuffer);
	SafeDelete(instanceBuffer);

}
#pragma region Render와 Animator 공용

void Model::Update()
{
	for (ModelMesh* mesh : meshes)
		mesh->Update();
	UpdateTransforms();
}

void Model::Render()
{
	if (bonebuffer == NULL)
		CreateBoneBuffer();


	instanceBuffer->Render();
	
	if (boneSrv != NULL)
		sBoneTransformsSRV->SetResource(boneSrv);

	for (ModelMesh* mesh : meshes)
		mesh->Render(transforms.size());	
}

void Model::Pass(const UINT& pass)
{
	for (ModelMesh* mesh : meshes)
		mesh->Pass(pass);
}

void Model::Tech(const UINT& tech)
{
	for (ModelMesh* mesh : meshes)
		mesh->Tech(tech);
}

void Model::UpdateTransforms()
{
	bool bChange = false;

	for (UINT i = 0; i < transforms.size(); i++)
	{
		if (bChange)
		{
			memcpy(worlds[i], transforms[i]->World(), sizeof(Matrix));
		}
		else if (worlds[i] != transforms[i]->World())
		{
			memcpy(worlds[i], transforms[i]->World(), sizeof(Matrix));
			bChange = true;
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
	AddTransform();	
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

int Model::BoneIndexByName(const wstring& name)
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
	materialDirPath= directoryPath;
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
	name = Path::GetDirectDirectoryName(readfile);

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
	
	CreateBoneBuffer();
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
}

void Model::CreateBoneBuffer()
{
	// 값 할당
	{
		for (UINT b = 0; b < BoneCount(); b++)
		{
			ModelBone* bone = BoneByIndex(b);
			Matrix InvGlobal = bone->BoneWorld();
			D3DXMatrixInverse(&InvGlobal, NULL, &InvGlobal);
			// 모델 본 그자체의 데이터
			boneTrans[b] = InvGlobal;			
		}//for(b)
	}

	//CreateTexture
	{
		D3D11_TEXTURE1D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE1D_DESC));
		desc.Width = MAX_MODEL_TRANSFORMS * 4;
		desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.MipLevels = 1;
		desc.ArraySize = 1;

		D3D11_SUBRESOURCE_DATA subResource;
		subResource.pSysMem = boneTrans;
		subResource.SysMemPitch = MAX_MODEL_TRANSFORMS * sizeof(Matrix);
		subResource.SysMemSlicePitch = MAX_MODEL_TRANSFORMS * sizeof(Matrix);

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
	
	//for (ModelMesh* mesh : Meshes())
	//	mesh->BoneTransformsSRV(boneSrv);
}
