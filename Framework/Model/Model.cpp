#include "Framework.h"
#include "Model.h"
#include "ModelMesh.h"
#include "Utilities/Xml.h"
#include "Utilities/BinaryFile.h"

Model::Model(Shader* shader)
	:shader(shader)
{
	for (UINT i = 0; i < MAX_MODEL_INSTANCE; i++)
		D3DXMatrixIdentity(&worlds[i]);

	instanceBuffer = new VertexBuffer(worlds, MAX_MODEL_INSTANCE, sizeof(Matrix), 1, true);
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
	SafeDelete(instanceBuffer);

}
#pragma region Render와 Animator 공용

void Model::Update()
{
	for (ModelMesh* mesh : meshes)
		mesh->Update();
	UpdateTransforms();
	
	for (AttachModelData* data : attaches)
	{
		UINT attachbone = data->AttachedBoneIndex;
		for (int i = 0; i < GetInstSize(); i++)
		{
			Matrix mat = GetboneTransform(i, attachbone);
			//인스턴스 갯수 안맞는 경우.
			if (data->AttachInstances.size() == i)
			{
				int size = data->model->GetInstSize();
				data->AttachInstances.emplace_back(size);
				data->model->AddInstance();
				data->model->GetTransform(size)->World(
					data->model->GetTransform(data->AttachInstances[0])->World()
				);
			}
			int attachInstance = data->AttachInstances[i];

			data->model->GetTransform(i)->Parent(mat);
		}
		data->model->Update();
	}
}

void Model::Render()
{
	if (texture == NULL)
		CreateTexture();

	instanceBuffer->Render();

	for (ModelMesh* mesh : meshes)
		mesh->Render(transforms.size());

	//Attach
	for (AttachModelData* data : attaches)
		data->model->Render();
}

void Model::Pass(UINT pass)
{
	for (ModelMesh* mesh : meshes)
		mesh->Pass(pass);
}

void Model::Tech(UINT tech)
{
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

void Model::DelInstance(UINT instance)
{
	if (instance >= transforms.size())
		return;
	transforms.erase(transforms.begin() + instance);
	for (AttachModelData* data : attaches)
	{
		UINT attachinst = data->AttachInstances[instance];
		data->AttachInstances.erase(data->AttachInstances.begin() + instance);
		data->model->DelInstance(attachinst);
	}
}

void Model::AddTransform()
{
	Transform* transform = new Transform();
	transforms.push_back(transform);

	for (AttachModelData* data : attaches)
	{
		if (data->AttachInstances.size() < GetInstSize())
		{
			int size = data->model->GetInstSize();
			data->AttachInstances.emplace_back(size);
			data->model->AddInstance();
			data->model->GetTransform(size)->World(
				data->model->GetTransform(data->AttachInstances[0])->World()
			);
		}
	}
}

#pragma endregion


#pragma region 이름으로 데이터 읽기

Material * Model::MaterialByName(wstring name)
{
	for (Material* material : materials)
	{
		if (material->Name() == name)
			return material;
	}
	
	return NULL;
}

ModelBone * Model::BoneByName(wstring name)
{
	for (ModelBone* bone : bones)
	{
		if (bone->Name() == name)
			return bone;
	}

	return NULL;
}

ModelMesh * Model::MeshByName(wstring name)
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

void Model::ReadMaterial(wstring file, wstring directoryPath)
{
	file = directoryPath + file + L".material";

	Xml::XMLDocument* document = new Xml::XMLDocument();
	Xml::XMLError error = document->LoadFile(String::ToString(file).c_str());
	assert(error == Xml::XML_SUCCESS);

	Xml::XMLElement* root = document->FirstChildElement();
	Xml::XMLElement* materialNode = root->FirstChildElement();


	do
	{
		Material* material = new Material();


		Xml::XMLElement* node = NULL;

		node = materialNode->FirstChildElement();
		material->Name(String::ToWString(node->GetText()));


		wstring directory = Path::GetDirectoryName(file);
		String::Replace(&directory, L"../../_Textures", L"");


		wstring texture = L"";

		node = node->NextSiblingElement();
		texture = String::ToWString(node->GetText());
		if (texture.length() > 0)
			material->DiffuseMap(directory + texture);

		node = node->NextSiblingElement();
		texture = String::ToWString(node->GetText());
		if (texture.length() > 0)
			material->SpecularMap(directory + texture);

		node = node->NextSiblingElement();
		texture = String::ToWString(node->GetText());
		if (texture.length() > 0)
			material->NormalMap(directory + texture);


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

void Model::ReadMesh(wstring file, wstring directoryPath)
{
	//wstring clipfile = directoryPath + file + L".clip";
	//bool bReadClip = Path::ExistFile(clipfile);
	//clipfile = file;
	file = directoryPath + file + L".mesh";


	BinaryReader* r = new BinaryReader();
	r->Open(file);

	UINT count = 0;

	count = r->UInt();
	for (UINT i = 0; i < count; i++)
	{
		ModelBone* bone = new ModelBone();

		bone->index = r->Int();
		bone->name = String::ToWString(r->String());		
		int index = bone->name.find(L":", 0);
		if (index >= 0)
			bone->name = bone->name.substr(index + 1, bone->name.length());

		bone->parentIndex = r->Int();
		bone->transform = r->Matrix();
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
	/*if(bReadClip)
		ReadClip(clipfile);*/

}

void Model::BindBone()
{
	rootBone = bones[0];
	for (ModelBone* bone : bones)
	{
		if (bone->parentIndex > -1)
		{
			bone->parent = bones[bone->parentIndex];
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

void Model::Attach(Model * model, int parentBoneIndex, UINT instanceIndex, Transform* transform)
{
	class ModelRender* render = dynamic_cast<class ModelRender*>(model);
	class ModelAnimator* animator = dynamic_cast<class ModelAnimator*>(model);

	AttachModelData* data = new AttachModelData();
	data->AttachedBoneIndex = parentBoneIndex;
	if (transform != NULL)
		model->GetTransform(instanceIndex)->Local(transform->World());
	data->AttachInstances.emplace_back(instanceIndex);
	data->model = model;
	if (render)
	{
		data->type = ModelType::Model_Render;
	}
	else if (animator)
	{
		data->type = ModelType::Model_Animator;
	}

	while (GetInstSize() > data->AttachInstances.size())
	{
		int size = data->model->GetInstSize();
		data->AttachInstances.emplace_back(size);
		data->model->AddInstance();
		data->model->GetTransform(size)->World(
			data->model->GetTransform(data->AttachInstances[0])->World()
		);
	}
	attaches.emplace_back(data);

}

void Model::AddSocket(int parentBoneIndex, wstring bonename)
{
	ModelBone* parentBone = BoneByIndex(parentBoneIndex);
	ModelBone* newBone = new ModelBone();
	newBone->name = bonename;
	newBone->transform = parentBone->transform;
	newBone->parentIndex = parentBoneIndex;
	newBone->parent = parentBone;
	newBone->parent->childs.push_back(newBone);

	newBone->index = bones.size();

	bones.push_back(newBone);
}

///////////////////////////////////////////////////////////////////////////////
