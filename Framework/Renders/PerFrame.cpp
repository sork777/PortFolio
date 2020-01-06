#include "Framework.h"
#include "PerFrame.h"



PerFrame::PerFrame(Shader* shader)
	:shader(shader)
{
	buffer = new ConstantBuffer(&bufferDesc, sizeof(BufferDesc));
	sBuffer = shader->AsConstantBuffer("CB_PerFrame");
	
	lightBuffer = new ConstantBuffer(&lightDesc, sizeof(LightDesc));
	sLightBuffer = shader->AsConstantBuffer("CB_Light");
	/*
	pointLightBuffer = new ConstantBuffer(&pointLightDesc, sizeof(PointLightDesc));
	sPointLightBuffer = shader->AsConstantBuffer("CB_PointLights");
	
	spotLightBuffer = new ConstantBuffer(&spotLightDesc, sizeof(SpotLightDesc));
	sSpotLightBuffer = shader->AsConstantBuffer("CB_SpotLights");

	capsuleLightBuffer = new ConstantBuffer(&capsuleLightDesc, sizeof(CapsuleLightDesc));
	sCapsuleLightBuffer = shader->AsConstantBuffer("CB_CapsuleLights");*/
}


PerFrame::~PerFrame()
{
	SafeDelete(buffer);
	SafeDelete(lightBuffer);
	/*SafeDelete(pointLightBuffer);
	SafeDelete(spotLightBuffer);
	SafeDelete(capsuleLightBuffer);*/
}

void PerFrame::Update()
{
	bufferDesc.Time = Time::Get()->Running();

	lightDesc.Ambient = Context::Get()->LightAmbient();
	lightDesc.Specular = Context::Get()->LightSpecular();
	lightDesc.Position = Context::Get()->LightPosition();
	lightDesc.Direction = Context::Get()->LightDirection(); 
/*
	pointLightDesc.Count = Context::Get()->PointLights(pointLightDesc.Lights);

	spotLightDesc.Count = Context::Get()->SpotLights(spotLightDesc.Lights);
	capsuleLightDesc.Count = Context::Get()->CapsuleLights(capsuleLightDesc.Lights);*/
}

void PerFrame::Render()
{
	bufferDesc.View = Context::Get()->View();
	D3DXMatrixInverse(&bufferDesc.ViewInverse, NULL, &bufferDesc.View);
	
	bufferDesc.Projection = Context::Get()->Projection();
	bufferDesc.VP = bufferDesc.View*bufferDesc.Projection;

	buffer->Apply();
	sBuffer->SetConstantBuffer(buffer->Buffer());

	//if (sLightBuffer != NULL)
	{
		lightBuffer->Apply();
		sLightBuffer->SetConstantBuffer(lightBuffer->Buffer());
	}
/*
	pointLightBuffer->Apply();
	sPointLightBuffer->SetConstantBuffer(pointLightBuffer->Buffer());

	spotLightBuffer->Apply();
	sSpotLightBuffer->SetConstantBuffer(spotLightBuffer->Buffer());

	capsuleLightBuffer->Apply();
	sCapsuleLightBuffer->SetConstantBuffer(capsuleLightBuffer->Buffer());*/
}
