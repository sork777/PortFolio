#include "Framework.h"
#include "SSLR.h"



SSLR::SSLR()
{
	shader = SETSHADER(L"HW06_SSLR.fx");
	Init();
}

SSLR::SSLR(Shader* shader)
	:shader(shader)
{
	Init();
}


SSLR::~SSLR()
{
}

void SSLR::Init()
{
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// ConstantBuffer
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	occlussionBuffer = new ConstantBuffer(&OccDesc, sizeof(OcculusionDesc));
	rayTraceBuffer = new ConstantBuffer(&RTDesc, sizeof(RayTraceDesc));

	sOcclussionBuffer = shader->AsConstantBuffer("CB_Occlusion");
	sRayTraceBuffer = shader->AsConstantBuffer("CB_RayTrace");

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	Width = D3D::Width() / 2;
	Height = D3D::Height() / 2;

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// Allocate the occlusion resources
	D3D11_TEXTURE2D_DESC t2dDesc = {
		Width,					//UINT Width;
		Height,					//UINT Height;
		1,						//UINT MipLevels;
		1,						//UINT ArraySize;
		DXGI_FORMAT_R8_TYPELESS, //DXGI_FORMAT Format;
		1,						//DXGI_SAMPLE_DESC SampleDesc;
		0,
		D3D11_USAGE_DEFAULT,	//D3D11_USAGE Usage;
		D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE,//UINT BindFlags;
		0,						//UINT CPUAccessFlags;
		0						//UINT MiscFlags;    
	};
	D3D::GetDevice()->CreateTexture2D(&t2dDesc, NULL, &OcclusionTex);
	
	D3D11_UNORDERED_ACCESS_VIEW_DESC UAVDesc;
	ZeroMemory(&UAVDesc, sizeof(UAVDesc));
	UAVDesc.Format = DXGI_FORMAT_R8_UNORM;
	UAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	D3D::GetDevice()->CreateUnorderedAccessView(OcclusionTex, &UAVDesc, &OcclusionUAV);

	D3D11_SHADER_RESOURCE_VIEW_DESC dsrvd =
	{
		DXGI_FORMAT_R8_UNORM,
		D3D11_SRV_DIMENSION_TEXTURE2D,
		0,
		0
	};
	dsrvd.Texture2D.MipLevels = 1;
	D3D::GetDevice()->CreateShaderResourceView(OcclusionTex, &dsrvd, &OcclusionSRV);

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// Allocate the light rays resources
	t2dDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	D3D::GetDevice()->CreateTexture2D(&t2dDesc, NULL, &LightRaysTex);

	D3D11_RENDER_TARGET_VIEW_DESC rtsvd =
	{
		DXGI_FORMAT_R8_UNORM,
		D3D11_RTV_DIMENSION_TEXTURE2D
	};
	D3D::GetDevice()->CreateRenderTargetView(LightRaysTex, &rtsvd, &LightRaysRTV);
	D3D::GetDevice()->CreateShaderResourceView(LightRaysTex, &dsrvd, &LightRaysSRV);

}

void SSLR::Render(ID3D11ShaderResourceView * pMiniDepthSRV)
{

	ID3D11RenderTargetView* oldRTV;
	D3D::GetDC()->OMGetRenderTargets(1, &oldRTV,NULL);
	
	Vector3 sunDir = Context::Get()->LightDirection();
	Vector3 camFor = Context::Get()->GetCamera()->Forward();
	const float dotCamSun = -D3DXVec3Dot(&camFor, &sunDir);

	if (dotCamSun <= 0.0f)
	{
		return;
	}

	//날이 저물면 꺼야함
	Vector3 dayVector(0, 1, 0);
	const float dayTime = -D3DXVec3Dot(&dayVector, &sunDir);
	if (dayTime <= 0.0f)
	{
		return;
	}

	//해의 위치 설정
	Vector3 vSunPos = -500.0f * sunDir;
	Vector3 vEyePos;
	//카메라 시점과 조합
	Context::Get()->GetCamera()->Position(&vEyePos);
	vSunPos.x += vEyePos.x;
	vSunPos.z += vEyePos.z;
	Matrix View = Context::Get()->View();
	Matrix Proj = Context::Get()->Projection();
	Matrix ViewProjection = View * Proj;
	//화면공간으로 전환
	Vector3 vSunPosSS;
	D3DXVec3TransformCoord(&vSunPosSS, &vSunPos, &ViewProjection);

	//// If the sun is too far out of view we just want to turn off the effect
	static const float fMaxSunDist = 3.3f;
	if (abs(vSunPosSS.x) >= fMaxSunDist || abs(vSunPosSS.y) >= fMaxSunDist)
	{
		return;
	}

	// Attenuate the sun color based on how far the sun is from the view
	intensity = Context::Get()->LightSpecular().a;
	//데이타임으로 감쇠 강화
	Vector3 vSunColorAtt = Context::Get()->LightSpecular()*intensity*dayTime;
	float fMaxDist = max(abs(vSunPosSS.x), abs(vSunPosSS.y));
	if (fMaxDist >= 1.0f)
	{
		vSunColorAtt *= (fMaxSunDist - fMaxDist);
	}

	PrepareOcclusion( pMiniDepthSRV);
	RayTrace( D3DXVECTOR2(vSunPosSS.x, vSunPosSS.y), vSunColorAtt);
	Combine(oldRTV);
}

void SSLR::SSLRController()
{
}

void SSLR::PrepareOcclusion(ID3D11ShaderResourceView * pMiniDepthSRV)
{
	OccDesc.Height = Height;
	OccDesc.Width = Width;

	occlussionBuffer->Apply();
	sOcclussionBuffer->SetConstantBuffer(occlussionBuffer->Buffer());
	
	shader->AsUAV("OcclusionRW")->SetUnorderedAccessView(OcclusionUAV);
	shader->AsSRV("DepthTex")->SetResource(pMiniDepthSRV);

	shader->Dispatch(0, occPass, (UINT)ceil((float)(Width *Height) / 1024.0f), 1, 1);
}

void SSLR::RayTrace(const D3DXVECTOR2 & vSunPosSS, const Vector3 & vSunColor)
{	
	D3D::GetDC()->ClearRenderTargetView(LightRaysRTV, Color(0, 0, 0, 0));

	D3D11_VIEWPORT oldvp;
	UINT num = 1;
	D3D::GetDC()->RSGetViewports(&num, &oldvp);
	
	{
		D3D11_VIEWPORT vp[1] = { { 0, 0, (float)Width, (float)Height, 0.0f, 1.0f } };
		D3D::GetDC()->RSSetViewports(1, vp);
	
		D3D::GetDC()->OMSetRenderTargets(1, &LightRaysRTV, NULL);
	}
	
	//clip공간 좌표화?
	RTDesc.SunPos = Vector2(0.5f * vSunPosSS.x + 0.5f, -0.5f * vSunPosSS.y + 0.5f);
	RTDesc.InitDecay = InitDecay;
	RTDesc.DistDecay = DistDecay;
	RTDesc.RayColor = vSunColor;// Vector3(0.7f, 0.0f, 0.0f);
	RTDesc.MaxDeltaLen = MaxDeltaLen;

	rayTraceBuffer->Apply();
	sRayTraceBuffer->SetConstantBuffer(rayTraceBuffer->Buffer());
	shader->AsSRV("OcclusionTex")->SetResource(OcclusionSRV);

	D3D::GetDC()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	shader->Draw(0, rtPass, 4);

	D3D::GetDC()->RSSetViewports(1, &oldvp);
}

void SSLR::Combine(ID3D11RenderTargetView * pLightAccumRTV)
{

	// Restore the light accumulation view
	D3D::GetDC()->OMSetRenderTargets(1, &pLightAccumRTV, NULL);

	rayTraceBuffer->Apply();
	sRayTraceBuffer->SetConstantBuffer(rayTraceBuffer->Buffer());
	shader->AsSRV("LightRaysTex")->SetResource(LightRaysSRV);

	// Primitive settings
	D3D::GetDC()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// Set the shaders
	shader->Draw(0, combinePass, 4);
}
