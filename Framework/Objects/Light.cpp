#include "Framework.h"
#include "Light.h"



Lights::Light::Light(Shader * shader)
	:shader(shader)
{
	//perframe = new PerFrame(shader);
	//transform = new Transform(shader);
	//mat = new Material(shader);
}

Lights::Light::~Light()
{
	SafeDelete(cbuffer);
	SafeRelease(sCbuffer);
	SafeRelease(sShadowBuffer);
}

void Lights::Light::SetSize(UINT width, UINT height)
{
	this->width = (width < 1) ? 1024 : width;
	this->height = (height < 1) ? 1024 : height;

}
//
//void Lights::Light::Update()
//{
//	perframe->Update();
//	transform->Update();
//}
//
//void Lights::Light::Render()
//{
//	/*perframe->Render();
//	transform->Render();
//	mat->Render();*/
//}

////////////////////////////////////////////////////////////////////////////////
// Point Light //
////////////////////////////////////////////////////////////////////////////////


Lights::PointLight::PointLight(Shader* shader)
	:Light(shader)
{
	cbuffer = new ConstantBuffer(&pointlightDesc, sizeof(LightDesc));
	sCbuffer = shader->AsConstantBuffer("CB_PointLights");
	sShadowBuffer = shader->AsMatrix("PL_CubeViewProj");
	SetSize();
}

Lights::PointLight::~PointLight()
{
}

void Lights::PointLight::Render()
{
	//Super::Render();
	for (UINT i = 0; i < lights.size(); i++)
	{
		pointlightDesc.Lights = lights[i];

		Matrix S, T, V, P;
		float r = 1.0f / pointlightDesc.Lights.RangeReverse;

		float x =  pointlightDesc.Lights.Position.x;
		float y =  pointlightDesc.Lights.Position.y;
		float z =  pointlightDesc.Lights.Position.z;

		D3DXMatrixScaling(&S, r, r, r);
		D3DXMatrixTranslation(&T, x,y,z);
		/* view */
		V = Context::Get()->View();
		/* Projection */
		P = Context::Get()->Projection();

		pointlightDesc.Lights.LightProjection = S * T*V*P;

		shader->AsScalar("bDrawPointShadow")->SetInt(shadows[i].bShadow ? 1 : 0);
		if (shadows[i].bShadow == true)
		{
			shader->AsSRV("PointShadowMapTexture")->SetResource(shadows[i].srv);
		}

		cbuffer->Apply();
		sCbuffer->SetConstantBuffer(cbuffer->Buffer());


		D3D::GetDC()->IASetInputLayout(NULL);
		D3D::GetDC()->IASetVertexBuffers(0, 0, NULL, NULL, NULL);
		D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);
		shader->Draw(tech, pass, 2);
	}
}

void Lights::PointLight::LightController()
{
	float range = 1/lights[lightIndex].RangeReverse;

	ImGui::Separator();
	ImGui::Text("Point Light");
	ImGui::InputInt("PL_Index", (int*)&lightIndex);
	lightIndex %= lights.size();

	ImGui::ColorEdit4("PL_Color", (float*)&lights[lightIndex].color);
	ImGui::SliderFloat("PL_Specular", &lights[lightIndex].Specular, 1, 100);
	ImGui::SliderFloat3("PL_Position", (float*)&lights[lightIndex].Position, -200, 200);
	ImGui::SliderFloat("PL_Range", &range, 10, 100);

	lights[lightIndex].RangeReverse = 1 / range;

	/* 그림자 맵의 상수값 변경 */
	Vector3 LightPos = lights[lightIndex].Position;
	{
		float x = LightPos.x;
		float y = LightPos.y;
		float z = LightPos.z;

		Vector3 direction[6] =
		{
			Vector3(x + 1,y,z),Vector3(x - 1,y,z),
			Vector3(x,y + 1,z),Vector3(x,y - 1,z),
			Vector3(x,y,z + 1),Vector3(x,y,z - 1)
		};

		//lookat용
		Vector3 up[6] =
		{
			Vector3(0,1,0),Vector3(0,1,0),
			Vector3(0,0,-1),Vector3(0,0,1),
			Vector3(0,1,0),Vector3(0,1,0)
		};
		Matrix matPointProj;
		D3DXMatrixPerspectiveFovLH(&matPointProj, D3DX_PI * 0.5f, 1.0, 0.1f, 1 / lights[lightIndex].RangeReverse);

		for (UINT i = 0; i < 6; i++)
		{
			D3DXMatrixLookAtLH(&shadows[lightIndex].ViewProjection[i], &LightPos, &direction[i], &up[i]);
			shadows[lightIndex].ViewProjection[i] *= matPointProj;
		}
		lights[lightIndex].LightPerspectiveValues = Vector2(matPointProj.m[2][2], matPointProj.m[3][2]);

	}
}

void Lights::PointLight::SetIsDrawing(Vector3 & objPos, float drawRad)
{
	for (UINT i = 0; i < lights.size(); i++)
	{
		float len = D3DXVec3Length(&(lights[i].Position - objPos));

		shadows[i].bShadow = (len < drawRad) ? true : false;
	}
}

bool Lights::PointLight::LightPreSet()
{
	D3D11_VIEWPORT vp[6] = {
		{ 0, 0, width, height, 0.0f, 1.0f },
	{ 0, 0, width, height, 0.0f, 1.0f },
	{ 0, 0, width, height, 0.0f, 1.0f },
	{ 0, 0, width, height, 0.0f, 1.0f },
	{ 0, 0, width, height, 0.0f, 1.0f },
	{ 0, 0, width, height, 0.0f, 1.0f } };

	ID3D11RenderTargetView* nullRT = NULL;

	D3D::GetDC()->RSSetViewports(1, vp);
	D3D::GetDC()->OMSetRenderTargets(0, &nullRT, shadows[lightCount].dsv);
	D3D::GetDC()->ClearDepthStencilView(shadows[lightCount].dsv, D3D11_CLEAR_DEPTH, 1, 0);

	sShadowBuffer->SetMatrixArray((float*)shadows[lightCount].ViewProjection, 0, 6);

	lightCount++;
	if (lightCount >= shadows.size())
	{
		lightCount %= shadows.size();
		return false;
	}
	return true;
}

void Lights::PointLight::AddLight(PointLightInfo & light)
{
	LightInfo plight;
	plight.color = light.color;
	plight.Position = light.Position;
	plight.RangeReverse = 1.0f / light.Range;
	plight.Specular = light.Specular;

	/* 그림자 큐브맵 설정 */
	ShadowInfo shadow;
	{
		float x = light.Position.x;
		float y = light.Position.y;
		float z = light.Position.z;

		Vector3 direction[6] =
		{
			Vector3(x + 1,y,z),Vector3(x - 1,y,z),
			Vector3(x,y + 1,z),Vector3(x,y - 1,z),
			Vector3(x,y,z + 1),Vector3(x,y,z - 1)
		};

		//lookat용
		Vector3 up[6] =
		{
			Vector3(0,1,0),Vector3(0,1,0),
			Vector3(0,0,-1),Vector3(0,0,1),
			Vector3(0,1,0),Vector3(0,1,0)
		};
		Matrix matPointProj;
		D3DXMatrixPerspectiveFovLH(&matPointProj, D3DX_PI * 0.5f, 1.0, 0.1f, light.Range);

		for (UINT i = 0; i < 6; i++)
		{
			D3DXMatrixLookAtLH(&shadow.ViewProjection[i], &light.Position, &direction[i], &up[i]);
			shadow.ViewProjection[i] *= matPointProj;
		}
		plight.LightPerspectiveValues = Vector2(matPointProj.m[2][2], matPointProj.m[3][2]);

	}
	//Create Texture
	{
		ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
		textureDesc.Width = width;
		textureDesc.Height = height;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 6;
		textureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
		textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

		HRESULT hr = D3D::GetDevice()->CreateTexture2D(&textureDesc, NULL, &shadow.backBuffer);
		Check(hr);
	}

	//Create DSV
	{
		ZeroMemory(&dsvDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		dsvDesc.Texture2DArray.FirstArraySlice = 0;
		dsvDesc.Texture2DArray.ArraySize = 6;

		HRESULT hr = D3D::GetDevice()->CreateDepthStencilView(shadow.backBuffer, &dsvDesc, &shadow.dsv);
		Check(hr);
	}
	//Create SRV
	{
		ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MipLevels = 1;
		srvDesc.TextureCube.MostDetailedMip = 0;
		HRESULT hr = D3D::GetDevice()->CreateShaderResourceView(shadow.backBuffer, &srvDesc, &shadow.srv);
		Check(hr);
	}

	lights.emplace_back(plight);
	//pl shadow
	shadows.emplace_back(shadow);
}



////////////////////////////////////////////////////////////////////////////////
// Spot Light //
////////////////////////////////////////////////////////////////////////////////


Lights::SpotLight::SpotLight(Shader * shader)
	:Light(shader)
{
	cbuffer = new ConstantBuffer(&spotlightDesc, sizeof(LightDesc));
	sCbuffer = shader->AsConstantBuffer("CB_SpotLights");
	sShadowBuffer = shader->AsMatrix("ToShadowMap");
	SetSize();
}

Lights::SpotLight::~SpotLight()
{
}

void Lights::SpotLight::Render()
{
	for (UINT i = 0; i < lights.size(); i++)
	{
		spotlightDesc.Lights = lights[i];
		Vector3 vDir = spotlightDesc.Lights.Direction;
		Vector3 vPos = spotlightDesc.Lights.Position;
		Vector3 vUp = (vDir.y > 0.9 || vDir.y < -0.9) ? Vector3(0.0f, 0.0f, vDir.y) : Vector3(0.0f, 1.0f, 0.0f);
		Vector3 vRight;
		D3DXVec3Cross(&vRight, &vUp, &vDir);
		D3DXVec3Normalize(&vRight, &vRight);
		D3DXVec3Cross(&vUp, &vDir, &vRight);
		D3DXVec3Normalize(&vUp, &vUp);
		Matrix LightWorldTransRotate;
		D3DXMatrixIdentity(&LightWorldTransRotate);
		for (int i = 0; i < 3; i++)
		{
			LightWorldTransRotate.m[0][i] = (&vRight.x)[i];
			LightWorldTransRotate.m[1][i] = (&vUp.x)[i];
			LightWorldTransRotate.m[2][i] = (&vDir.x)[i];
			LightWorldTransRotate.m[3][i] = (&vPos.x)[i];
		}

		Matrix S, V, P;
		float r = 1.0f / spotlightDesc.Lights.RangeReverse;
		D3DXMatrixScaling(&S, r, r, r);
		/* view */
		V = Context::Get()->View();
		/* Projection */
		P = Context::Get()->Projection();

		spotlightDesc.Lights.LightProjection = S * LightWorldTransRotate*V*P;
		spotlightDesc.Lights.Direction = -vDir;
		/* 그림자 평면의 변환 행렬*/
		sShadowBuffer->SetMatrix((float*)shadows[i].ToShadowMap);
		/* 그림자 여부 */
		shader->AsScalar("bDrawSpotShadow")->SetInt(shadows[i].bShadow ? 1 : 0);
		/* 그림자 맵 세팅 */
		if (shadows[i].bShadow == true)
		{
			shader->AsSRV("SpotShadowMapTexture")->SetResource(shadows[i].srv);
		}
		/* 버퍼 갱신 */
		cbuffer->Apply();
		sCbuffer->SetConstantBuffer(cbuffer->Buffer());

		D3D::GetDC()->IASetInputLayout(NULL);
		D3D::GetDC()->IASetVertexBuffers(0, 0, NULL, NULL, NULL);
		D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);
		shader->Draw(tech, pass, 1);
	}
}

void Lights::SpotLight::LightController()
{
	float range = 1 / lights[lightIndex].RangeReverse;

	ImGui::Separator();
	ImGui::Text("Spot Light");
	ImGui::InputInt("SL_Index", (int*)&lightIndex);
	lightIndex %= lights.size();

	ImGui::ColorEdit4("SL_Color", (float*)&lights[lightIndex].color);
	ImGui::SliderFloat("SL_Specular", &lights[lightIndex].Specular, 1, 100);

	ImGui::SliderFloat3("SL_Position", (float*)&lights[lightIndex].Position, -200, 200);
	ImGui::SliderFloat3("SL_Direction", (float*)&lights[lightIndex].Direction, -1, 1);
	D3DXVec3Normalize(&lights[lightIndex].Direction, &lights[lightIndex].Direction);
	
	float inner = lights[lightIndex].InnerAngle;
	float outer = lights[lightIndex].OuterAngle;

	ImGui::SliderFloat("InnerAngle", &inner, 10, 80);
	inner = Math::Clamp(inner, 10, outer);
	ImGui::SliderFloat("OuterAngle", &outer, 10, 80);
	outer = Math::Clamp(outer, inner, 80);
	
	ImGui::SliderFloat("SL_Range", &range, 10, 100);
	lights[lightIndex].RangeReverse = 1 / range;


	lights[lightIndex].InnerAngle = inner;
	lights[lightIndex].OuterAngle = outer; 
	outer = Math::ToRadian(outer);
	inner = Math::ToRadian(inner);

	lights[lightIndex].SinAngle = sinf(outer);
	lights[lightIndex].CosAngle = cosf(outer);
	lights[lightIndex].SpotCosOuterCone = cosf(outer);
	lights[lightIndex].SpotCosConeAttRange = cosf(inner) - cosf(outer);

}

void Lights::SpotLight::SetIsDrawing(Vector3 & objPos, float drawRad)
{
	for (UINT i = 0; i < lights.size(); i++)
	{
		float len = D3DXVec3Length(&(lights[i].Position - objPos));

		shadows[i].bShadow = (len < drawRad) ? true : false;
	}
}

bool Lights::SpotLight::LightPreSet()
{
	D3D11_VIEWPORT vp[1] = { { 0, 0, width, height, 0.0f, 1.0f } };

	ID3D11RenderTargetView* nullRT = NULL;

	D3D::GetDC()->RSSetViewports(1, vp);
	D3D::GetDC()->OMSetRenderTargets(0, &nullRT, shadows[lightCount].dsv);
	D3D::GetDC()->ClearDepthStencilView(shadows[lightCount].dsv, D3D11_CLEAR_DEPTH, 1, 0);

	Matrix matSpotView;
	Vector3 vLookAt = lights[lightCount].Position + lights[lightCount].Direction * 1/lights[lightCount].RangeReverse;
	Vector3 vUp = (lights[lightCount].Direction.y > 0.9 || lights[lightCount].Direction.y < -0.9) ? D3DXVECTOR3(0.0f, 0.0f, lights[lightCount].Direction.y) : D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	Vector3 vRight;
	D3DXVec3Cross(&vRight, &vUp, &lights[lightCount].Direction);
	D3DXVec3Normalize(&vRight, &vRight);
	D3DXVec3Cross(&vUp, &lights[lightCount].Direction, &vRight);
	D3DXVec3Normalize(&vUp, &vUp);
	D3DXMatrixLookAtLH(&matSpotView, &lights[lightCount].Position, &vLookAt, &vUp);
	
	Matrix matSpotProj;
	float outer = Math::ToRadian(lights[lightCount].OuterAngle);
	D3DXMatrixPerspectiveFovLH(&matSpotProj, 2.0f * outer, 1.0, 5, 1/lights[lightCount].RangeReverse);

	shadows[lightCount].ToShadowMap= matSpotView * matSpotProj;

	sShadowBuffer->SetMatrix((float*)shadows[lightCount].ToShadowMap);

	lightCount++;
	if (lightCount >= shadows.size())
	{
		lightCount %= shadows.size();
		return false;
	}
	return true;
}

void Lights::SpotLight::AddLight(SpotLightInfo & light)
{
	LightInfo slight;
	slight.OuterAngle = light.OuterAngle;
	slight.InnerAngle = light.InnerAngle;
	light.OuterAngle = Math::ToRadian(light.OuterAngle);
	light.InnerAngle = Math::ToRadian(light.InnerAngle);
	slight.color = light.color;
	slight.Position = light.Position;
	slight.RangeReverse = 1.0f / light.Range;
	D3DXVec3Normalize(&slight.Direction, &light.Direction);
	slight.SpotCosOuterCone = cosf(light.OuterAngle);
	slight.color = light.color;
	slight.SpotCosConeAttRange = cosf(light.InnerAngle) - cosf(light.OuterAngle);
	slight.SinAngle = sinf(light.OuterAngle);
	slight.CosAngle = cosf(light.OuterAngle);
	slight.Specular = light.Specular;

	lights.emplace_back(slight);


	ShadowInfo shadow;

	//Create Texture
	{
		ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
		textureDesc.Width = width;
		textureDesc.Height = height;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;

		HRESULT hr = D3D::GetDevice()->CreateTexture2D(&textureDesc, NULL, &shadow.backBuffer);
		Check(hr);
	}

	//Create DSV
	{
		ZeroMemory(&dsvDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

		HRESULT hr = D3D::GetDevice()->CreateDepthStencilView(shadow.backBuffer, &dsvDesc, &shadow.dsv);
		Check(hr);
	}
	//Create SRV
	{
		ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		HRESULT hr = D3D::GetDevice()->CreateShaderResourceView(shadow.backBuffer, &srvDesc, &shadow.srv);
		Check(hr);
	}
	shadows.emplace_back(shadow);
}


////////////////////////////////////////////////////////////////////////////////
// Capsule Light //
////////////////////////////////////////////////////////////////////////////////

Lights::CapsuleLight::CapsuleLight(Shader * shader)
	:Light(shader)
{
	cbuffer = new ConstantBuffer(&capsulelightDesc, sizeof(LightDesc));
	sCbuffer = shader->AsConstantBuffer("CB_CapsuleLights");	
}

Lights::CapsuleLight::~CapsuleLight()
{
}

void Lights::CapsuleLight::Render()
{
	for (UINT i = 0; i < lights.size(); i++)
	{
		capsulelightDesc.Lights = lights[i];
		Vector3 vDir = capsulelightDesc.Lights.Direction;
		Vector3 vPos = capsulelightDesc.Lights.Position;
		float fLen = capsulelightDesc.Lights.Length;
		Vector3 vUp = (vDir.y > 0.9 || vDir.y < -0.9) ? Vector3(0.0f, 0.0f, vDir.y) : Vector3(0.0f, 1.0f, 0.0f);
		Vector3 vRight;
		D3DXVec3Cross(&vRight, &vUp, &vDir);
		D3DXVec3Normalize(&vRight, &vRight);
		D3DXVec3Cross(&vUp, &vDir, &vRight);
		D3DXVec3Normalize(&vUp, &vUp);
		Vector3 vCenterPos = vPos + 0.5f * vDir * fLen;

		Matrix LightWorldTransRotate;
		D3DXMatrixIdentity(&LightWorldTransRotate);
		for (int i = 0; i < 3; i++)
		{
			LightWorldTransRotate.m[0][i] = (&vRight.x)[i];
			LightWorldTransRotate.m[1][i] = (&vUp.x)[i];
			LightWorldTransRotate.m[2][i] = (&vDir.x)[i];
			LightWorldTransRotate.m[3][i] = (&vCenterPos.x)[i];
		}

		Matrix V, P;
		/* view */
		V = Context::Get()->View();
		/* Projection */
		P = Context::Get()->Projection();

		capsulelightDesc.Lights.LightProjection = LightWorldTransRotate*V*P;

		cbuffer->Apply();
		sCbuffer->SetConstantBuffer(cbuffer->Buffer());

		D3D::GetDC()->IASetInputLayout(NULL);
		D3D::GetDC()->IASetVertexBuffers(0, 0, NULL, NULL, NULL);
		D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);
		shader->Draw(tech, pass, 2);
	}
}

void Lights::CapsuleLight::LightController()
{
	float range = lights[lightIndex].Range;
	float length = lights[lightIndex].Length;

	ImGui::Separator();
	ImGui::Text("Capsule Light");
	ImGui::InputInt("CL_Index", (int*)&lightIndex);
	lightIndex %= lights.size();

	ImGui::ColorEdit4("CL_Color", (float*)&lights[lightIndex].color);
	ImGui::SliderFloat("CL_Specular", &lights[lightIndex].Specular, 1, 100);

	ImGui::SliderFloat3("CL_Position", (float*)&lights[lightIndex].Position, -200, 200);
	ImGui::SliderFloat3("CL_Direction", (float*)&lights[lightIndex].Direction, -1, 1);
	D3DXVec3Normalize(&lights[lightIndex].Direction, &lights[lightIndex].Direction);

	ImGui::SliderFloat("CL_Range", &range, 10, 100);
	lights[lightIndex].Range = range;
	lights[lightIndex].RangeRcp = 1 / range;

	ImGui::SliderFloat("Length", &length, 10, 100);
	lights[lightIndex].Length = length;
	lights[lightIndex].HalfLen = length*0.5f;
}

void Lights::CapsuleLight::AddLight(CapsuleLightInfo & light)
{
	LightInfo clight;
	clight.color = light.color;
	clight.Direction = light.Direction;
	clight.Position = light.Position;
	clight.Range = light.Range;
	clight.RangeRcp = 1/light.Range;
	clight.Length = light.Length;
	clight.HalfLen = light.Length*0.5f;
	clight.Specular = light.Specular;

	lights.emplace_back(clight);
}
