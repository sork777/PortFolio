#include "Framework.h"
#include "Mesh.h"

Mesh::Mesh()
{
	topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}

Mesh::~Mesh()
{
	SafeDelete(perFrame);

	SafeDeleteArray(vertices);
	SafeDeleteArray(indices);
}

void Mesh::Update()
{
	assert(perFrame != NULL);

	perFrame->Update();
}
void Mesh::Render(const UINT& drawCount)
{
	if (vertexBuffer == NULL || indexBuffer == NULL)
	{
		Create();
		
		vertexBuffer = new VertexBuffer(vertices, vertexCount, sizeof(MeshVertex));
		indexBuffer = new IndexBuffer(indices, indexCount);
	}
	if(indexTessBuffer == NULL && bTess)
		indexTessBuffer = new IndexBuffer(TessIndices, tessICount);

	
	perFrame->Render();
	vertexBuffer->Render();
	shader->AsScalar("DisplacementFactor")->SetFloat(displacement);
	D3D::GetDC()->IASetPrimitiveTopology(topology);
	if (bTess)
	{
		indexTessBuffer->Render();
		shader->DrawIndexedInstanced(tech, pass, tessICount, drawCount);
	}
	else
	{
		indexBuffer->Render();
		shader->DrawIndexedInstanced(tech, pass, indexCount, drawCount);
	}
}

void Mesh::SetShader(Shader * shader)
{
	this->shader = shader;

	SafeDelete(perFrame);
	perFrame = new PerFrame(shader);
}
