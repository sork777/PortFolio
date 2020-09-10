#include "Framework.h"
#include "ModelRender.h"
#include "ModelMesh.h"

ModelRender::ModelRender(Model* model)
	: model(model)
{
	texture = NULL;
	srv = NULL;
	SetShader(model->GetShader());
}

ModelRender::~ModelRender()
{
	SafeRelease(sTransformsSRV);
	//SafeRelease(srv);
	SafeRelease(texture);
}

void ModelRender::Update()
{
	if (texture == NULL)
		CreateTexture();
	//model->Update();
}

void ModelRender::Render()
{

	if (srv != NULL)
		sTransformsSRV->SetResource(srv);
	//model->Render();
}

void ModelRender::SetShader(Shader * shader)
{
	model->SetShader(shader);
	sTransformsSRV = shader->AsSRV("TransformsMap");
}

void ModelRender::UpdateTransform(UINT instanceId, UINT boneIndex, Transform & transform)
{
	Matrix destMatrix = transform.World();

	ModelBone* bone = model->BoneByIndex(boneIndex);
	boneTransforms[instanceId][boneIndex] = destMatrix * boneTransforms[instanceId][boneIndex];

	int tempBoneIndex = boneIndex;
	for (ModelBone* child : bone->Childs())
	{
		Matrix parent = boneTransforms[instanceId][boneIndex];

		Matrix invParent;
		D3DXMatrixInverse(&invParent, NULL, &parent);
		tempBoneIndex++;

		Matrix temp = boneTransforms[instanceId][tempBoneIndex] * invParent;
		boneTransforms[instanceId][tempBoneIndex] = temp * destMatrix * parent;
	}

	D3D11_MAPPED_SUBRESOURCE subResource;
	D3D::GetDC()->Map(texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
	{
		memcpy(subResource.pData, boneTransforms, MAX_MODEL_INSTANCE * MAX_BONE_TRANSFORMS * sizeof(Matrix));
	}
	D3D::GetDC()->Unmap(texture, 0);
}

Matrix ModelRender::GetboneWorld(UINT instance, UINT boneIndex)
{
	Matrix transform = boneTransforms[instance][boneIndex];
	Matrix world = model->GetTransform(instance)->World();

	return transform * world;
}

void ModelRender::CreateTexture()
{
	//CreateTexture
	{
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
		desc.Width = MAX_BONE_TRANSFORMS * 4;
		desc.Height = MAX_MODEL_INSTANCE;
		desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.SampleDesc.Count = 1;

		for (UINT i = 0; i < MAX_MODEL_INSTANCE; i++)
		{
			for (UINT b = 0; b < model->BoneCount(); b++)
			{
				ModelBone* bone = model->BoneByIndex(b);

				Matrix parent;
				int parentIndex = bone->ParentIndex();

				if (parentIndex < 0)
					D3DXMatrixIdentity(&parent);
				else
					parent = bones[parentIndex];

				Matrix matrix = bone->BoneWorld();
				bones[b] = parent;
				boneTransforms[i][b] = matrix * bones[b];
			}//for(b)
		}//for(i)

		
		D3D11_SUBRESOURCE_DATA subResource;
		subResource.pSysMem = boneTransforms;
		subResource.SysMemPitch = MAX_BONE_TRANSFORMS * sizeof(Matrix);
		subResource.SysMemSlicePitch = MAX_BONE_TRANSFORMS * sizeof(Matrix) * MAX_MODEL_INSTANCE;

		Check(D3D::GetDevice()->CreateTexture2D(&desc, &subResource, &texture));
	}


	//Create SRV
	{
		D3D11_TEXTURE2D_DESC desc;
		texture->GetDesc(&desc);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Format = desc.Format;

		Check(D3D::GetDevice()->CreateShaderResourceView(texture, &srvDesc, &srv));
	}

	//for (ModelMesh* mesh : model->Meshes())
	//	mesh->TransformsSRV(srv);
}

