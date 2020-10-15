#include "Framework.h"
#include "ModelAsset.h"
#include "Viewer/Orbit.h"
#include "Environment/Sky.h"


ModelAsset::ModelAsset(Model* model)
	:modelAnimation(NULL), modelRender(NULL)
{
	shader = SETSHADER(L"027_Animation.fx");

	assetType = ContentsAssetType::Model;
	assetName = model->Name();
	

	//버튼 이미지를 위한 준비
	this->model = new Model(*model);
	this->model->SetShader(shader);
	modelRender = new ModelRender(this->model);
	bHasAnim = this->model->IsAnimationModel();
	if (bHasAnim == true)
	{
		modelAnimation = new ModelAnimator(this->model);
		//T-pose를 자동 배정해주기 위함
		modelAnimation->ReadClip(model->MeshPath(), model->MeshDir());
	}
	//모델 기본 크기에 맞춰서 자동 스케일링
	this->model->Update();
	Vector3 autoCalVolume = this->model->GetMax() - this->model->GetMin();
	float max = autoCalVolume.x;
	bool bZRot=false;
	max = max > autoCalVolume.y ? max : autoCalVolume.y;
	if (max <= autoCalVolume.z)
	{
		max = autoCalVolume.z;
		bZRot = true;
		autoCalVolume.y *= 0.0f;
	}
	float scaleFactor = (bZRot?20.0f:8.0f) / max;
	autoCalVolume.x *= 0.0f;
	autoCalVolume.y *= scaleFactor;
	autoCalVolume.z *= 0.0f;

	this->model->AddInstance();
	this->model->GetTransform()->Scale(scaleFactor, scaleFactor, scaleFactor);
	this->model->GetTransform()->RotationDegree(Vector3(0, bZRot?90:0, 0));
	this->model->UpdateTransforms();
	
	
	
	//버튼이미지를 위한 랜더타겟 관련
	{
		float width = D3D::Width();
		float height = D3D::Height();
		renderTarget = new RenderTarget((UINT)width, (UINT)height);
		depthStencil = new DepthStencil(width, height);
		orbitCam = new Orbit(30.0f, 30.0f, 30.0f);
				
		orbitCam->SetObjPos(autoCalVolume);
		orbitCam->CameraMove(mouseVal);
		orbitCam->Update();
	}
}

ModelAsset::~ModelAsset()
{
	SafeDelete(orbitCam);
}

void ModelAsset::CreateButtonImage()
{
	// 프로젝트내의 버튼이미지 생성 위치에 따라 클리핑 되는 현상 있음
	// 버튼 이미지 생성
	orbitCam->Update();
	Context::Get()->SetSubCamera(orbitCam);

	renderTarget->Set(depthStencil->DSV(),Color(0.5f,0.6f,0.5f,1.0f));
	//model->Tech(1);
	if (modelAnimation != NULL)
	{
		modelAnimation->Update();
		model->Pass(2);
		modelAnimation->Render();
	}
	else if (modelRender != NULL)
	{
		modelRender->Update();
		model->Pass(1);
		modelRender->Render();
	}
	model->Update();
	model->Render();
	buttonSrv = renderTarget->SRV();
	Context::Get()->SetMainCamera();
	bCreateButton = true;
}

void ModelAsset::SaveAssetContents()
{
	//모델 메시 관련 에셋 저장 정보.
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
	// 데이터 분리를 위해 새로 생성
	Model* newModel = new Model(*model);
	ModelMeshComponent * newModelMesh = new ModelMeshComponent(newModel);
	if (bHasAnim == true)
		newModelMesh->GetAnimation()->CloneClips(modelAnimation->Clips());
	return newModelMesh;
}

void ModelAsset::SetClip(const wstring & file, const wstring & directoryPath)
{
	if (NULL != modelAnimation)
	{
		modelAnimation->ReadClip(file, directoryPath);
	}
}
