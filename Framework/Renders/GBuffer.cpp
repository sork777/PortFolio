#include "Framework.h"
#include "GBuffer.h"


GBuffer::GBuffer(Shader* shader)
	:shader(shader)
{
	float width = D3D::Width();
	float height = D3D::Height();

	
	for (UINT i = 0; i < 4; i++)
	{
		DepNorDifSpe[i] = new Render2D();
		DepNorDifSpe[i]->GetTransform()->Position(60 + 100 * i, 60, 0);
		DepNorDifSpe[i]->GetTransform()->Scale(100, 100, 1);
	}

	targetColor = new RenderTarget((UINT)width, (UINT)height);
	targetNormal = new RenderTarget((UINT)width, (UINT)height, DXGI_FORMAT_R11G11B10_FLOAT);
	targetSpec = new RenderTarget((UINT)width, (UINT)height);
	targetDepth = new RenderTarget((UINT)width, (UINT)height);

	depthStencil = new DepthStencil((UINT)width, (UINT)height, true);

	viewport = new Viewport((UINT)width, (UINT)height);

	///////////////////////////////////////////////////////
	/* 텍스쳐 보내기*/
	shader->AsSRV("ColorSpecIntTexture")->SetResource(targetColor->SRV());
	shader->AsSRV("NormalTexture")->SetResource(targetNormal->SRV());
	shader->AsSRV("SpecPowTexture")->SetResource(targetSpec->SRV());
	shader->AsSRV("DepthTexture")->SetResource(depthStencil->SRV());
}


GBuffer::~GBuffer()
{
	SafeDelete(viewport);
	SafeDelete(depthStencil);
	SafeDelete(targetColor);
	SafeDelete(targetNormal);
	SafeDelete(targetSpec);
	
	for (UINT i = 0; i < 4; i++)
		SafeDelete(DepNorDifSpe[i]);
}

void GBuffer::Update()
{
	Vector3 camPos;
	Context::Get()->GetCamera()->Position(&camPos);

	Matrix proj = Context::Get()->Projection();
	perspertive.x = 1.0f / proj.m[0][0];
	perspertive.y = 1.0f / proj.m[1][1];
	perspertive.z = proj.m[3][2];
	perspertive.w = -proj.m[2][2];

	shader->AsVector("PerspectiveValues")->SetFloatVector(perspertive);
}

void GBuffer::Render()
{
	D3D::GetDC()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	shader->Draw(tech, pass, 4);
}

void GBuffer::SetRTVs()
{
	vector<RenderTarget*> rtvs;
	rtvs.push_back(targetColor);		//Color
	rtvs.push_back(targetNormal);		//Normal
	rtvs.push_back(targetSpec);			//Specular
	rtvs.push_back(targetDepth);		//Depth
	RenderTarget::Sets(rtvs, depthStencil->DSV());
	viewport->RSSetViewport();
}

void GBuffer::RenderGBuffers()
{
	DepNorDifSpe[0]->SRV(targetColor->SRV());
	DepNorDifSpe[0]->Render();

	DepNorDifSpe[1]->SRV(targetNormal->SRV());
	DepNorDifSpe[1]->Render();

	DepNorDifSpe[2]->SRV(targetSpec->SRV());
	DepNorDifSpe[2]->Render();

	DepNorDifSpe[3]->SRV(targetDepth->SRV());
	DepNorDifSpe[3]->Render();
}
