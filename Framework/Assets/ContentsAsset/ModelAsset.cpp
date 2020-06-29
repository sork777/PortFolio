#include "Framework.h"
#include "ModelAsset.h"
#include "Viewer/Orbit.h"
#include "Environment/Sky.h"


ModelAsset::ModelAsset(Model* model)
{
	assetType = ContentsAssetType::Model;
	assetName = model->Name();
	//sky = new Sky(L"Environment/GrassCube1024.dds");
	//��ư �̹����� ���� �غ�
	this->model = model;
	//TODO: �� �⺻ ũ�⿡ ���缭 �ڵ� �����ϸ� �ʿ�
	model->AddInstance();
	model->UpdateTransforms();
	
	modelRender = new ModelRender(this->model);

	if (model->IsAnimationModel() == true)
	{
		modelAnimation = new ModelAnimator(this->model);
		modelAnimation->ReadClip(model->MeshPath(), model->MeshDir());
	}
	
	float width = D3D::Width();
	float height = D3D::Height();
	renderTarget = new RenderTarget((UINT)width, (UINT)height);
	depthStencil = new DepthStencil(width, height);
	
}

ModelAsset::~ModelAsset()
{
}

void ModelAsset::CreateButtonImage()
{
	// ��ư �̹��� ����

	// �� Ÿ�� ������ �Ѵ�... �ƴϸ� ����Ÿ���� ��ҵ�..
	//if (true == bCreateButton)
	//	return;
	Vector3 campos, camrot;
	Context::Get()->GetCamera()->RotationDegree(&camrot);
	Context::Get()->GetCamera()->Position(&campos);

	// ��ư �̹����� ���� ī�޶� ����
	Context::Get()->GetCamera()->RotationDegree(0,0,0);
	Context::Get()->GetCamera()->Position(0,45,-100);
	
	renderTarget->Set(depthStencil->DSV());

	//sky->Update();
	//sky->Render();
	if (modelAnimation != NULL)
	{
		modelAnimation->Update();
		modelAnimation->PlayAnim(0);
		model->Pass(2);
		modelAnimation->Render();
	}
	else if (modelRender != NULL)
	{
		modelRender->Update();
		model->Pass(1);
		modelRender->Render();
	}
	buttonSrv = renderTarget->SRV();

	Context::Get()->GetCamera()->RotationDegree(camrot);
	Context::Get()->GetCamera()->Position(campos);

	bCreateButton = true;
}

void ModelAsset::SaveAssetContents()
{
	//�� �޽� ���� ���� ���� ����.
	model->MeshPath();
	model->MeshDir();
	for (Material* mat : model->Materials())
	{
		wstring name = mat->Name();
		Color ambient = mat->Ambient();
		Color specular = mat->Specular();
		Color diffuse = mat->Diffuse();
		Color emissive = mat->Emissive();

		wstring diffuseMapFile = mat->DiffuseMap()->GetFile();
		wstring specularMaFile = mat->SpecularMap()->GetFile();
		wstring normalMapFile = mat->NormalMap()->GetFile();
		wstring heightMapFile = mat->HeightMap()->GetFile();

		wstring diffuseMapDir = mat->DiffuseMap()->GetDir();
		wstring specularMaDir = mat->SpecularMap()->GetDir();
		wstring normalMapDir = mat->NormalMap()->GetDir();
		wstring heightMapDir = mat->HeightMap()->GetDir();
		
	}
}

void ModelAsset::LoadAssetContents()
{
}

ModelMeshComponent * ModelAsset::GetModelMeshCompFromModelAsset()
{
	// ������ �и��� ���� ���� ����
	Model* newModel = new Model(*model);
	ModelMeshComponent * newModelMesh = new ModelMeshComponent(newModel);
	return newModelMesh;
}
