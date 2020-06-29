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

void MeshRender::Render(const int& drawCount)
{
	if (material != NULL)
		material->Render();

	instanceBuffer->Render();

	int draw = drawCount;
	if (drawCount <= 0)
		draw = transforms.size();

	mesh->Render(draw);
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

void MeshRender::MeshChanger(Mesh * mesh)
{
	Shader* shader = this->mesh->GetShader();
	this->mesh = mesh;
	mesh->SetShader(shader);
}

void MeshRender::SetMaterial(Material * material)
{
	this->material = material;
	this->material->SetShader(mesh->GetShader());
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
