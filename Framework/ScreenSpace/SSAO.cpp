#include "Framework.h"
#include "SSAO.h"


SSAO::SSAO()
{
	shader = SETSHADER(L"HW00_SSAO.fx");
	Initialize();
}
SSAO::SSAO(Shader* shader)
	:shader(shader)
{
	Initialize();
}


SSAO::~SSAO()
{
	SafeDelete(cBuffer);
	
	
	for (UINT i = 0; i < 2; i++)
		SafeDelete(renderSrvs[i]);
	
	SafeRelease(SSAO_RT);
	SafeRelease(SSAO_UAV);
	SafeRelease(SSAO_SRV);

	SafeRelease(miniDepthBuffer);
	SafeRelease(miniDepthUAV);
	SafeRelease(miniDepthSRV);

}

void SSAO::Initialize()
{
	for (UINT i = 0; i < 2; i++)
	{
		renderSrvs[i] = new Render2D();
		renderSrvs[i]->GetTransform()->Position(60+100*i, 160, 0);
		renderSrvs[i]->GetTransform()->Scale(100, 100, 1);
	}
	width = D3D::Width() / 2;
	height = D3D::Height() / 2;

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// SSAO
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	
	D3D11_TEXTURE2D_DESC t2dDesc = {
		width,				//UINT Width;
		height,				//UINT Height;
		1,					//UINT MipLevels;
		1,					//UINT ArraySize;
		DXGI_FORMAT_R32_TYPELESS,	//DXGI_FORMAT_R8_TYPELESS, 
		1,					//DXGI_FORMAT Format;
		0,					//DXGI_SAMPLE_DESC SampleDesc;
		D3D11_USAGE_DEFAULT,		//D3D11_USAGE Usage;
		D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE,	//UINT BindFlags;
		0,					//UINT CPUAccessFlags;
		0					//UINT MiscFlags;    
	};
	D3D::GetDevice()->CreateTexture2D(&t2dDesc, NULL, &SSAO_RT);

	D3D11_UNORDERED_ACCESS_VIEW_DESC UAVDesc;
	ZeroMemory(&UAVDesc, sizeof(UAVDesc));
	UAVDesc.Format = DXGI_FORMAT_R32_FLOAT;
	UAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	D3D::GetDevice()->CreateUnorderedAccessView(SSAO_RT, &UAVDesc, &SSAO_UAV);

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	ZeroMemory(&SRVDesc, sizeof(SRVDesc));
	SRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = 1;
	D3D::GetDevice()->CreateShaderResourceView(SSAO_RT, &SRVDesc, &SSAO_SRV);

	
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// Down Scale
	//////////////////////////////////////////////////////////////////////////////////////////////////////

	//D3D11_BUFFER_DESC bufferDesc;
	//ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	//bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	//bufferDesc.StructureByteStride = 4 * sizeof(float);
	//bufferDesc.ByteWidth = width * height * bufferDesc.StructureByteStride;
	//bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	//D3D::GetDevice()->CreateBuffer(&bufferDesc, NULL, &miniDepthBuffer);

	//ZeroMemory(&UAVDesc, sizeof(UAVDesc));
	//UAVDesc.Format = DXGI_FORMAT_UNKNOWN;
	//UAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	//UAVDesc.Buffer.FirstElement = 0;
	//UAVDesc.Buffer.NumElements = width * height;
	//D3D::GetDevice()->CreateUnorderedAccessView(miniDepthBuffer, &UAVDesc, &miniDepthUAV);

	//ZeroMemory(&SRVDesc, sizeof(SRVDesc));
	//SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
	////SRVDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	//SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	//SRVDesc.Buffer.FirstElement = 0;
	//SRVDesc.Buffer.NumElements = width * height;
	//D3D::GetDevice()->CreateShaderResourceView(miniDepthBuffer, &SRVDesc, &miniDepthSRV);
	//
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// Constant Buffer
	//////////////////////////////////////////////////////////////////////////////////////////////////////

	cBuffer = new ConstantBuffer(&desc, sizeof(TDownscaleDesc));
	CSminiDepthbuffer = new StructuredBuffer(NULL, 4 * sizeof(float), width * height);
}

void SSAO::RenderSSAO()
{
	renderSrvs[0]->SRV(SSAO_SRV);
	renderSrvs[0]->Render();

	renderSrvs[1]->SRV(miniDepthSRV);
	renderSrvs[1]->Render();
}
void SSAO::Compute(ID3D11ShaderResourceView * DepthSRV, ID3D11ShaderResourceView * NormalsSRV)
{
	DownscaleDepth(DepthSRV, NormalsSRV);
	ComputeSSAO();
}

void SSAO::DownscaleDepth(ID3D11ShaderResourceView * DepthSRV, ID3D11ShaderResourceView * NormalsSRV)
{

	desc.Width = width;
	desc.Height = height;
	desc.HorResRcp = 1.0f / width;
	desc.VerResRcp = 1.0f / height;
	Matrix proj = Context::Get()->Projection();
	float zf = Context::Get()->GetPerspective()->GetFarPlane();
	float zn = Context::Get()->GetPerspective()->GetNearPlane();
	float Q = zf / (zf - zn);
	desc.ProjParams = { 1 / proj.m[0][0],1 / proj.m[1][1],-zn * Q,-Q };
	desc.ViewMatrix = Context::Get()->View();

	desc.OffsetRadius = SSAOSampRadius;
	desc.Radius = Radius;
	desc.MaxDepth = zf;
	cBuffer->Apply();
	shader->AsConstantBuffer("CB_DownScale")->SetConstantBuffer(cBuffer->Buffer());
	shader->AsSRV("DepthTexture")->SetResource(DepthSRV);
	shader->AsSRV("NormalTexture")->SetResource(NormalsSRV);
	shader->AsUAV("MiniDepthRW")->SetUnorderedAccessView(CSminiDepthbuffer->UAV());
	//shader->AsUAV("MiniDepthRW")->SetUnorderedAccessView(miniDepthUAV);

	
	shader->Dispatch(0, passMini, (UINT)ceil((float)(width * height) / 1024.0f), 1, 1);

}

void SSAO::ComputeSSAO()
{

	cBuffer->Apply();
	shader->AsConstantBuffer("CB_DownScale")->SetConstantBuffer(cBuffer->Buffer()); 
	//shader->AsSRV("MiniDepth")->SetResource(miniDepthSRV);
	shader->AsSRV("MiniDepth")->SetResource(CSminiDepthbuffer->SRV());
	shader->AsUAV("AO")->SetUnorderedAccessView(SSAO_UAV);

	shader->Dispatch(0, passAO, (UINT)ceil((float)(width * height) / 1024.0f), 1, 1);
}
