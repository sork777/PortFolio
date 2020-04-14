#include "Framework.h"
#include "CSM.h"

CSM::CSM(Shader* shader, UINT width, UINT height)
	:shader(shader), width(width), height(height),pass(0)
{
	position = Vector3(0, 0, 0);
	//shader에 값 설정
	buffer = new ConstantBuffer(&casDesc, sizeof(CascadeDesc));
	shadowbuffer = new ConstantBuffer(&shadowDesc, sizeof(DirShadowDesc));
	sBuffer = shader->AsConstantBuffer("CB_CSM");
	sShadowBuffer = shader->AsConstantBuffer("CB_DirShadow");

	CascadeSize = 3;

	this->width = (width < 1) ? 1024 : width;
	this->height = (height < 1) ? 1024 : height;


	//DS 생성
	CreateDepthStencil();
	CreateSampler();

	//값 초기화 

	CSMradius[0] = Context::Get()->GetPerspective()->GetNearPlane();
	CSMradius[1] = 30.0f;
	CSMradius[2] = 70.0f;
	CSMradius[3] = 300.0f;
	for (int i = 0; i < 3; i++)
	{
		cascadeBoundCenter[i] = Vector3(0, 0, 0);
		cascadeBoundRadius[i] = 0.0f;
	}
}

CSM::~CSM()
{
	SafeRelease(dsv);
	SafeRelease(backBuffer);
	SafeRelease(srv);
	
	SafeDelete(buffer);
	SafeDelete(samplerState);
}

void CSM::CreateDepthStencil()
{
	casDesc.MapSize = Vector2((float)width, (float)height);
	D3D11_TEXTURE2D_DESC textureDesc;
	{
		ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
		textureDesc.Width = this->width;
		textureDesc.Height = this->height;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = CascadeSize;
		textureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;

		HRESULT hr = D3D::GetDevice()->CreateTexture2D(&textureDesc, NULL, &backBuffer);
		Check(hr);
	}

	//Create DSV
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
		desc.Format = DXGI_FORMAT_D32_FLOAT;
		desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.FirstArraySlice = 0;
		desc.Texture2DArray.ArraySize = CascadeSize;

		HRESULT hr = D3D::GetDevice()->CreateDepthStencilView(backBuffer, &desc, &dsv);
		Check(hr);
	}
	//Create SRV
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		desc.Format = DXGI_FORMAT_R32_FLOAT;
		desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.FirstArraySlice = 0;
		desc.Texture2DArray.MipLevels = 1;
		desc.Texture2DArray.ArraySize = CascadeSize;

		HRESULT hr = D3D::GetDevice()->CreateShaderResourceView(backBuffer, &desc, &srv);
		Check(hr);
	}
	

}

void CSM::CreateSampler()
{
	D3D11_SAMPLER_DESC samDesc;
	ZeroMemory(&samDesc, sizeof(samDesc));
	samDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	samDesc.AddressU = samDesc.AddressV = samDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samDesc.MaxAnisotropy = 1;
	samDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	samDesc.MinLOD = 0;
	samDesc.MaxLOD = D3D11_FLOAT32_MAX;
	
	D3D::GetDevice()->CreateSamplerState(&samDesc, &samplerState);

	shader->AsSampler("PCFSampler")->SetSampler(0, samplerState);

}

void CSM::Update()
{
}

void CSM::Set()
{
	D3D11_VIEWPORT vp[3] = { 
		{ 0, 0, width, height, 0.0f, 1.0f },
	{ 0, 0, width, height, 0.0f, 1.0f }, 
	{ 0, 0, width, height, 0.0f, 1.0f }};

	ID3D11RenderTargetView* nullRT = NULL;

	D3D::GetDC()->RSSetViewports(3, vp);
	D3D::GetDC()->OMSetRenderTargets(0, &nullRT, dsv);
	D3D::GetDC()->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH , 1, 0);

	UpdateVolume();
	
	buffer->Apply();
	sBuffer->SetConstantBuffer(buffer->Buffer());
	shadowbuffer->Apply();
	sShadowBuffer->SetConstantBuffer(shadowbuffer->Buffer());
	shader->AsSRV("CSMTexture")->SetResource(srv);
	shader->AsScalar("UseCSM")->SetInt(1);
}

void CSM::UpdateVolume()
{
	Context::Get()->GetCamera()->Position(&c_pos);
	Vector3 vUp;
	Vector3 vFor = Context::Get()->GetCamera()->Forward();

	Vector3 direction = Context::Get()->LightDirection();
	
	float zf = Context::Get()->GetPerspective()->GetFarPlane();

	Vector3 vWorldCenter = c_pos + vFor * CSMradius[3]*0.5f;
	Vector3 vPos = vWorldCenter;
	Vector3 vLookAt = vWorldCenter + direction * zf;
	Vector3 vRight(1, 0, 0);
	
	D3DXVec3Cross(&vUp, &direction, &vRight);
	D3DXVec3Normalize(&vUp, &vUp);
	
	
	/* 쉐도우맵의 전체 영역*/
	Matrix ShadowView;
	D3DXMatrixLookAtLH(&ShadowView, &vPos, &(vLookAt), &vUp);
	float radius;
	Vector3 tCenter;
	BoundSphere(CSMradius[0], CSMradius[3], tCenter, radius);
	shadowBoundRadius = max(shadowBoundRadius, radius);

	Matrix ShadowProj;
	D3DXMatrixOrthoLH(&ShadowProj, shadowBoundRadius, shadowBoundRadius, -shadowBoundRadius, shadowBoundRadius);

	shadowDesc.ToShadowSpace = ShadowView * ShadowProj;


	Matrix ShadowViewInv;
	D3DXMatrixTranspose(&ShadowViewInv, &ShadowView);

	for (UINT i = 0; i < 3; i++)
	{
		Matrix cascadeTrans;
		Matrix cascadeScale;

		Vector3 newCenter;
		BoundSphere(CSMradius[i], CSMradius[i+1], newCenter, radius);
		cascadeBoundRadius[i] = max(cascadeBoundRadius[i], radius);

		Vector3 offset;
		if (NeedUpdate(ShadowView,i, newCenter,offset))
		{
			Vector3 offsetOut;
			D3DXVec3TransformNormal(&offsetOut, &offset, &ShadowViewInv);
			cascadeBoundCenter[i] += offsetOut;
		}
		Vector3 CascadeShadowCenterSpace;
		D3DXVec3TransformCoord(&CascadeShadowCenterSpace, &cascadeBoundCenter[i], &shadowDesc.ToShadowSpace);

		// Update the translation from shadow to cascade space
		shadowDesc.ToCascadeOffsetX[i] = -CascadeShadowCenterSpace.x;
		shadowDesc.ToCascadeOffsetY[i] = -CascadeShadowCenterSpace.y;
		D3DXMatrixTranslation(&cascadeTrans, shadowDesc.ToCascadeOffsetX[i], shadowDesc.ToCascadeOffsetY[i], 0.0f);

		// Update the scale from shadow to cascade space
		shadowDesc.ToCascadeScale[i] = shadowBoundRadius / cascadeBoundRadius[i];
		D3DXMatrixScaling(&cascadeScale, shadowDesc.ToCascadeScale[i], shadowDesc.ToCascadeScale[i], 1.0f);
		
		
		casDesc.ViewProjection[i] = shadowDesc.ToShadowSpace*cascadeTrans*cascadeScale;

	}

	shadowDesc.ToCascadeOffsetX[3] = 500.0f;
	shadowDesc.ToCascadeOffsetY[3] = 500.0f;
	shadowDesc.ToCascadeScale[3] = 0.1f;
}


void CSM::BoundSphere(float nearplane, float farplane, Vector3& boundCenter, float & radius)
{
	Vector3 cFor = Context::Get()->GetCamera()->Forward();
	Vector3 cRight = Context::Get()->GetCamera()->Right();
	Vector3 cUp = Context::Get()->GetCamera()->Up();
	float fov = Context::Get()->GetPerspective()->GetFOV();
	float aspect = width / height;

	float tanx = tanf(aspect*fov);
	float tany = tanf(aspect);

	boundCenter = c_pos + cFor * (nearplane + 0.5f*(nearplane + farplane));
	Vector3 boundSpan = c_pos + (-cRight * tanx + cUp * tany + cFor)*farplane - boundCenter;
	radius = D3DXVec3Length(&boundSpan);
}

bool CSM::NeedUpdate(const Matrix & shadowV, int idx, const Vector3 & newCenter, Vector3 & offset)
{
	Vector3 vOldCenterInCascade;
	D3DXVec3TransformCoord(&vOldCenterInCascade, &cascadeBoundCenter[idx], &shadowV);
	Vector3 vNewCenterInCascade;
	D3DXVec3TransformCoord(&vNewCenterInCascade, &newCenter, &shadowV);
	Vector3 vCenterDiff = vNewCenterInCascade - vOldCenterInCascade;

	// Find the pixel size based on the diameters and map pixel size
	float factor = 1 / (2.0f * cascadeBoundRadius[idx]);
	float fPixelSizeX =  width*factor;
	float fPixelSizeY = height * factor;

	float fPixelOffX = vCenterDiff.x * fPixelSizeX;
	float fPixelOffY = vCenterDiff.y * fPixelSizeY;

	// Check if the center moved at least half a pixel unit
	bool bNeedUpdate = abs(fPixelOffX) > 0.5f || abs(fPixelOffY) > 0.5f;
	if (bNeedUpdate)
	{
		// Round to the 
		offset.x = floorf(0.5f + fPixelOffX) / fPixelSizeX;
		offset.y = floorf(0.5f + fPixelOffY) / fPixelSizeY;
		offset.z = vCenterDiff.z;
	}

	return bNeedUpdate;
}
