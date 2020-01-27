#include "Framework.h"
#include "ModelIllusion.h"



ModelIllusion::ModelIllusion(Model* model, UINT meshIdx, UINT illusionCount)
	:illusionCount(illusionCount)
	, boneSrv(NULL), transformSrv(NULL), animEditSrv(NULL)
	, illusionColor(1,1,1,1)
{
	Initialize();

	if (model == NULL)
	{
		type = IllusionType::None;		
	}
	if (dynamic_cast<ModelRender*>(model) != NULL)
	{
		type = IllusionType::Render;		
	}
	else if (dynamic_cast<ModelAnimator*>(model) != NULL)
	{
		type = IllusionType::Animation;		
		animEditSrv = dynamic_cast<ModelAnimator*>(model)->GetEditSrv();
		shader->AsSRV("AnimEditTransformMap")->SetResource(animEditSrv);

		frameBuffer = new ConstantBuffer(&tweenDesc, sizeof(TweenDesc) * MAX_ILLUSION_COUNT);
		sFrameBuffer = shader->AsConstantBuffer("CB_AnimationFrame");
	}
	
	if (type != IllusionType::None)
	{
		pass = (UINT)type-1;
		if (meshIdx > model->MeshCount())
			meshIdx = 0;
		boneSrv = model->GetBoneSrv();
		transformSrv = model->GetTransformSrv();
		vertexBuffer = model->MeshByIndex(meshIdx)->GetVertexBuffer();
		indexBuffer = model->MeshByIndex(meshIdx)->GetIndexBuffer();
		indexCount = model->MeshByIndex(meshIdx)->GetIndexCount();
		
		Material* srcMaterial = model->MaterialByIndex(0);
		material->Name(L"ILLUSION_MAT");
		material->Ambient(srcMaterial->Ambient());
		material->Diffuse(srcMaterial->Diffuse());
		material->Specular(srcMaterial->Specular());
		material->Emissive(srcMaterial->Emissive());


		if (srcMaterial->DiffuseMap() != NULL)
			material->LoadDiffuseMapW(
				srcMaterial->DiffuseMap()->GetFile(),
				srcMaterial->DiffuseMap()->GetDir()
			);

		if (srcMaterial->SpecularMap() != NULL)
			material->LoadSpecularMapW(
				srcMaterial->SpecularMap()->GetFile(),
				srcMaterial->SpecularMap()->GetDir()
			);

		if (srcMaterial->NormalMap() != NULL)
			material->LoadNormalMapW(
				srcMaterial->NormalMap()->GetFile(),
				srcMaterial->NormalMap()->GetDir()
			);


		shader->AsSRV("TransformsMap")->SetResource(transformSrv);
		shader->AsSRV("BoneTransformsMap")->SetResource(boneSrv);
	}
	
}


ModelIllusion::~ModelIllusion()
{
	SafeDelete(perframe);
	SafeDelete(transform);
	//SafeDelete(model);
		;
	SafeDelete(vertexBuffer);
	SafeDelete(boneSrv);
	SafeDelete(transformSrv);
	SafeDelete(animEditSrv);

	SafeDelete(frameBuffer);
	SafeDelete(shader);
	tweens.clear();
	tweens.shrink_to_fit();
}

void ModelIllusion::Initialize()
{
	shader = new Shader(L"Effect/Illusion.fx");
	perframe = new PerFrame(shader);
	transform = new Transform(shader);
	material = new Material(shader);
}

void ModelIllusion::Update(TweenDesc tween, float gap)
{
	gap = Math::Clamp(gap,0.0f,1.0f);


	perframe->Update();
	material->Update();

	deltaStoreTime += Time::Delta();

	if (deltaStoreTime >= 0.01f)
	{
		deltaStoreTime -= 0.01f;

		SetAndShiftBuffer(tween, gap);
	}
	for (UINT i = 0; i < tweens.size(); i++)
		tweenDesc[i] = tweens[i];
}

void ModelIllusion::Render()
{
	perframe->Render();
	transform->Render();
	material->Render();

	frameBuffer->Apply();
	sFrameBuffer->SetConstantBuffer(frameBuffer->Buffer());
	shader->AsVector("ILL_Color")->SetFloatVector(illusionColor);

	vertexBuffer->Render();
	indexBuffer->Render();
	
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	shader->DrawIndexedInstanced(tech, pass, indexCount, illusionCount);
}


bool ModelIllusion::Property()
{
	bool bChange=false;
	{
		ImGui::Text("ILLUSION Property");
		bChange|=ImGui::ColorEdit4("ILL_COLOR", (float*)illusionColor);
		bChange | material->Property();
		bChange | transform->Property();
	}
	return bChange;
}

void ModelIllusion::SetAndShiftBuffer(TweenDesc & insertFirstTween, float& gap)
{
	float calTakegap = insertFirstTween.TakeTime - gap;
	float calTweengap = 0;
	
	calTakegap = Math::Clamp(calTakegap, 0.0f, 1.0f);

	if (insertFirstTween.Next.Clip > -1)
	{
		calTweengap = insertFirstTween.TweenTime - gap;
		calTweengap = Math::Clamp(calTweengap, 0.0f, 1.0f);
	}

	tweens.insert(tweens.begin(), insertFirstTween);
	tweens[0].TakeTime = calTakegap;
	tweens[0].TweenTime = calTweengap;
	if (tweens.size() > illusionCount)
		tweens.pop_back();
}
