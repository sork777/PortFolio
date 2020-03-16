#include "Framework.h"
#include "MeshRender.h"

MeshRender::MeshRender(Shader * shader, Mesh * mesh)
	: mesh(mesh)
{
	Pass(0);

	mesh->SetShader(shader);

	for (UINT i = 0; i < MAX_MESH_INSTANCE; i++)
		D3DXMatrixIdentity(&worlds[i]);

	instanceBuffer = new VertexBuffer(worlds, MAX_MESH_INSTANCE, sizeof(Matrix), 1, true);
}

MeshRender::~MeshRender()
{
	for (Transform* transform : transforms)
		SafeDelete(transform);

	SafeDelete(instanceBuffer);
	SafeDelete(mesh);
}

void MeshRender::Update()
{
	mesh->Update();	
}

void MeshRender::Render()
{
	if (material != NULL)
		material->Render();
	else if (materialpbr != NULL)
		materialpbr->Render();

	instanceBuffer->Render();
	mesh->Render(transforms.size());
}

void MeshRender::AddInstance()
{
	AddTransform();
}

void MeshRender::DelInstance(const UINT & instance)
{
	if (instance >= transforms.size())
		return;
	transforms.erase(transforms.begin() + instance);
}

Material * MeshRender::GetMaterial()
{
	if (type == MaterialType::Material)
		return material;
	return nullptr;
}

void MeshRender::SetMaterial(Material * material)
{
	if (type != MaterialType::MaterialPBR)
	{
		type = MaterialType::Material;
		this->material = material;
	}

}

MaterialPBR * MeshRender::GetMaterialPBR()
{
	if (type == MaterialType::MaterialPBR)
		return materialpbr;
	return nullptr;
}

void MeshRender::SetMaterialPBR(MaterialPBR * material)
{
	if (type != MaterialType::Material)
	{
		type = MaterialType::MaterialPBR;
		this->materialpbr = material;
	}
}

void MeshRender::AddTransform()
{
	Transform* transform = new Transform();
	transforms.push_back(transform);
}

void MeshRender::UpdateTransforms()
{
	for (UINT i = 0; i < transforms.size(); i++)
		memcpy(worlds[i], transforms[i]->World(), sizeof(Matrix));

	D3D11_MAPPED_SUBRESOURCE subResource;
	D3D::GetDC()->Map(instanceBuffer->Buffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
	{
		memcpy(subResource.pData, worlds, sizeof(Matrix) * MAX_MESH_INSTANCE);
	}
	D3D::GetDC()->Unmap(instanceBuffer->Buffer(), 0);
}
