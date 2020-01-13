#include "Framework.h"
#include "PerFrame.h"



PerFrame::PerFrame(Shader* shader)
	:shader(shader)
{
	buffer = new ConstantBuffer(&bufferDesc, sizeof(BufferDesc));
	sBuffer = shader->AsConstantBuffer("CB_PerFrame");
	
	lightBuffer = new ConstantBuffer(&lightDesc, sizeof(LightDesc));
	sLightBuffer = shader->AsConstantBuffer("CB_Light");
	
}


PerFrame::~PerFrame()
{
	SafeDelete(buffer);
	SafeDelete(lightBuffer);
	
}

void PerFrame::Update()
{
	bufferDesc.Time = Time::Get()->Running();

	lightDesc.Ambient = Context::Get()->LightAmbient();
	lightDesc.Specular = Context::Get()->LightSpecular();
	lightDesc.Position = Context::Get()->LightPosition();
	lightDesc.Direction = Context::Get()->LightDirection(); 
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
}
