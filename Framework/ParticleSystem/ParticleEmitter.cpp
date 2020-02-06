#include "Framework.h"
#include "ParticleEmitter.h"



ParticleEmitter::ParticleEmitter()
{
	Initialize();
}


ParticleEmitter::~ParticleEmitter()
{
	SafeDelete(instanceBuffer);
	
}

void ParticleEmitter::Initialize()
{
	pass	= 0;
	tech	= 0;
	
	shader			= new Shader(L"Effect/Particle.fx");
	computeShader	= new Shader(L"Effect/ParticleSimulation.fx");

	perFrame	= new PerFrame(shader);
	transform	= new Transform(shader);
	
	//인스턴싱이라 얘 하나 있으면 됨
	point.Position	= Vector3(0, 0, 0);
	point.Scale		= Vector2(1, 1);
	vertexBuffer	= new VertexBuffer(&point, 1, sizeof(ParticleVertex));

	for (UINT i = 0; i < MAX_INSTANCE; i++)
	{
		D3DXMatrixIdentity(&instDesc[i].world);
	}

	instanceBuffer	= new ConstantBuffer(&instDesc, MAX_INSTANCE, sizeof(ParticleDesc));
	sInstanceBuffer = shader->AsConstantBuffer("CB_Particle");
	sParticleTex	= shader->AsSRV("ParticleTex");
}

void ParticleEmitter::Update()
{
	perFrame->Update();
	transform->Update();

	PlayParticle();

	//빌보드와 달리 실행시 매 타임 바꿔줘야함
	for (UINT i = 0; i < particles.size(); i++)
	{
		Matrix S, R, T;
		//D3DXMatrixScaling(&S, csOutput[i].Scale.x, csOutput[i].Scale.y, csOutput[i].Scale.z);
		D3DXMatrixRotationYawPitchRoll(&R, csOutput[i].Rotation.x, csOutput[i].Rotation.y, csOutput[i].Rotation.z);
		D3DXMatrixTranslation(&T, csOutput[i].Postion.x, csOutput[i].Postion.y, csOutput[i].Postion.z);
		instDesc[i].world = R * T;
		memcpy(&instDesc[i].color, &csOutput[i].ResultColor, sizeof(Color));
	}

	D3D11_MAPPED_SUBRESOURCE subResource;
	D3D::GetDC()->Map(instanceBuffer->Buffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
	{
		memcpy(subResource.pData, &instDesc, sizeof(ParticleDesc) * MAX_INSTANCE);
	}
	D3D::GetDC()->Unmap(instanceBuffer->Buffer(), 0);
}

void ParticleEmitter::Render()
{
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	perFrame->Render();
	transform->Render();

	instanceBuffer->Apply();
	sInstanceBuffer->SetConstantBuffer(instanceBuffer->Buffer());
	if(particleTex !=NULL)
		sParticleTex->SetResource(particleTex->SRV());
	shader->DrawInstanced(tech, pass, 1, particles.size());
}

void ParticleEmitter::PlayParticle()
{
	csInfoDesc.DeltaTime= Time::Get()->Delta();

	csConstBuffer->Apply();
	computeShader->AsConstantBuffer("CB_CS")->SetConstantBuffer(csConstBuffer->Buffer());
	computeShader->AsSRV("InputDatas")->SetResource(computeBuffer->SRV());
	computeShader->AsUAV("OutputDatas")->SetUnorderedAccessView(computeBuffer->UAV());

	computeShader->Dispatch(csTech, csPass, particleCount / 1024.0f, 1, 1);
	computeBuffer->Copy(csOutput, sizeof(CS_OutputDesc) * MAX_INSTANCE);
}

void ParticleEmitter::CreateComputeShader()
{
	csConstBuffer = new ConstantBuffer(&csInfoDesc, sizeof(ParticleInfoDesc));

	UINT outSize = MAX_INSTANCE;
	csInput = new CS_InputDesc[outSize];
	csOutput = new CS_OutputDesc[outSize];

	computeBuffer = new StructuredBuffer
	(
		csInput,
		sizeof(CS_InputDesc), outSize,
		true,
		sizeof(CS_OutputDesc), outSize
	);

}
