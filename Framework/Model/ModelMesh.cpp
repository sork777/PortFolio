#include "Framework.h"
#include "ModelMesh.h"

ModelBone::ModelBone()
{
	editTransform = new Transform();
}

ModelBone::~ModelBone()
{
	SafeDelete(editTransform);
}

///////////////////////////////////////////////////////////////////////////////

ModelMesh::ModelMesh()
{
	boneBuffer = new ConstantBuffer(&boneDesc, sizeof(BoneDesc));
}

ModelMesh::~ModelMesh()
{
	SafeDelete(perFrame);

	SafeDeleteArray(vertices);
	SafeDeleteArray(indices);

	SafeDelete(vertexBuffer);
	SafeDelete(indexBuffer);

	SafeDelete(boneBuffer);
	//SafeDelete(material);

	SafeRelease(transformsSRV);
	SafeRelease(sTransformsSRV);
	SafeRelease(boneTransformsSRV);
	SafeRelease(sBoneTransformsSRV);
	SafeRelease(animEditSRV);
	SafeRelease(sAnimEditSRV);
}

void ModelMesh::Binding(Model * model)
{
	vertexBuffer = new VertexBuffer(vertices, vertexCount, sizeof(Model::ModelVertex));
	indexBuffer = new IndexBuffer(indices, indexCount);

	SafeDelete(material);
	material = model->MaterialByName(materialName);
}

void ModelMesh::SetShader(Shader * shader)
{
	this->shader = shader;

	SafeDelete(perFrame);
	perFrame = new PerFrame(shader);

	material->SetShader(shader);
	sBoneBuffer = shader->AsConstantBuffer("CB_Bone");

	sTransformsSRV = shader->AsSRV("TransformsMap");
	sBoneTransformsSRV = shader->AsSRV("BoneTransformsMap");
	sAnimEditSRV = shader->AsSRV("AnimEditTransformMap");
}

void ModelMesh::Update()
{
	boneDesc.Index = boneIndex;

	perFrame->Update();
}

void ModelMesh::Render(UINT drawCount)
{
	boneBuffer->Apply();
	sBoneBuffer->SetConstantBuffer(boneBuffer->Buffer());

	perFrame->Render();
	material->Render();

	if (transformsSRV != NULL)
		sTransformsSRV->SetResource(transformsSRV);

	if (boneTransformsSRV != NULL)
		sBoneTransformsSRV->SetResource(boneTransformsSRV);

	if (animEditSRV != NULL)
		sAnimEditSRV->SetResource(animEditSRV);


	vertexBuffer->Render();
	indexBuffer->Render();
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	shader->DrawIndexedInstanced(tech, pass, indexCount, drawCount);
}

void ModelMesh::TransformsSRV(ID3D11ShaderResourceView * srv)
{
	transformsSRV = srv;
}

void ModelMesh::BoneTransformsSRV(ID3D11ShaderResourceView* boneSrv)
{
	boneTransformsSRV = boneSrv;
}

void ModelMesh::AnimEditSrv(ID3D11ShaderResourceView * editSrv)
{
	animEditSRV = editSrv;

}
