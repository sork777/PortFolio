#include "Framework.h"
#include "ParticleEmitter.h"



ParticleEmitter::ParticleEmitter()
{
	Initialize();
}


ParticleEmitter::~ParticleEmitter()
{
	SafeDelete(particleTex);
	SafeDelete(vertexBuffer);
	SafeDelete(instanceBuffer);	
	SafeRelease(sInstanceBuffer);
	SafeRelease(sParticleTex);
	
	SafeRelease(sUav);
	SafeDelete(csConstBuffer);
	SafeDelete(computeBuffer);
	SafeDelete(computeShader);

	SafeDelete(perFrame);
	SafeDelete(transform);
	SafeDelete(shader);
}

void ParticleEmitter::Initialize()
{
	pass	= 0;
	tech	= 0;
	
	shader			= new Shader(L"Effect/ParticleEmitter.fx");
	computeShader	= new Shader(L"Effect/ParticleSimulation.fx");

	perFrame		= new PerFrame(shader);
	transform		= new Transform(shader);
	transform->Position(0, 10, 0);

	//�ν��Ͻ��̶� �� �ϳ� ������ ��
	point.Position	= Vector3(0, 0, 0);
	point.Scale		= Vector2(1, 1);
	vertexBuffer	= new VertexBuffer(&point, 1, sizeof(ParticleVertex));

	for (UINT i = 0; i < MAX_INSTANCE; i++)
	{
		D3DXMatrixIdentity(&instDesc[i].world);
	}

	instanceBuffer	= new ConstantBuffer(&instDesc, MAX_INSTANCE* sizeof(ParticleDesc));
	sInstanceBuffer = shader->AsConstantBuffer("CB_Particle");
	sParticleTex	= shader->AsSRV("ParticleTex");
			
	CreateComputeShader();
}

void ParticleEmitter::Update()
{
	perFrame->Update();
	transform->Update();

	PlayParticles();
	UpdateParticles();
}

void ParticleEmitter::Render()
{

	perFrame->Render();
	transform->Render();

	if (vertexBuffer != NULL)
		vertexBuffer->Render();
	instanceBuffer->Apply();
	sInstanceBuffer->SetConstantBuffer(instanceBuffer->Buffer());
	if(particleTex !=NULL)
		sParticleTex->SetResource(particleTex->SRV());
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	//�ν��Ͻ� ���� ������ ��ƼŬ ���� ����
	shader->DrawInstanced(tech, pass, 1, particleCount);
}

void ParticleEmitter::Property()
{
	ImGui::Begin("EmitterProperty", &bPropertyOpen);
	{
		ImGui::SliderInt("EmittPerSecond", &EmittPerSec, 1, MAX_INSTANCE);
		ImGui::LabelText("##P_Count", "ParticleCount : %d", particleCount);
		ImGui::Separator();
		//�޺��ڽ� �귯�� Ÿ��
		string emitterTypes[] = { "Point", "Sphere" , "Cube" };// , "Cone" };
		string emittTypes[] = { "Direct", "Round" };// , "Wave" };
		string particleTypes[] = { "Texture","Quad", "Diamond" };// , "Wave" };

		if (ImGui::BeginCombo("##EmitterType", current_emitter.c_str()))
		{
			int size = sizeof(emitterTypes) / sizeof(string);
			for (int n = 0; n < size; n++)
			{
				bool is_selected = (current_emitter == emitterTypes[n]);
				if (ImGui::Selectable(emitterTypes[n].c_str(), is_selected))
				{
					current_emitter = emitterTypes[n];
					emitter = EmitterType(n);
					particleCount = 0;
					roundCount	= 0;
					cubeCount	= 0;
				}
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		if (ImGui::BeginCombo("##EmittType", current_emit.c_str()))
		{
			int size = sizeof(emittTypes) / sizeof(string);
			for (int n = 0; n < size; n++)
			{
				bool is_selected = (current_emit == emittTypes[n]);
				if (ImGui::Selectable(emittTypes[n].c_str(), is_selected))
				{
					current_emit = emittTypes[n];
					emittype = EmittType(n);
					particleCount = 0;
				}
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		if (ImGui::BeginCombo("##ParticleType", current_particle.c_str()))
		{
			int size = sizeof(particleTypes) / sizeof(string);
			for (int n = 0; n < size; n++)
			{
				bool is_selected = (current_particle == particleTypes[n]);
				if (ImGui::Selectable(particleTypes[n].c_str(), is_selected))
				{
					current_particle = particleTypes[n];
					particle = ParticleType(n);
					pass = n;
				}
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		
		ImGui::Separator();
		ImGui::SliderFloat3("ParticleScale", (float*)&particleScale, 0.1f, 10.0f);
		
		particleRot.x = Math::ToDegree(particleRot.x);
		particleRot.y = Math::ToDegree(particleRot.y);
		particleRot.z = Math::ToDegree(particleRot.z);
		ImGui::SliderFloat3("ParticleRot", (float*)&particleRot, 0.0f, 170.0f);
		particleRot.x = Math::ToRadian(particleRot.x);
		particleRot.y = Math::ToRadian(particleRot.y);
		particleRot.z = Math::ToRadian(particleRot.z);

		particleRotDir.x = Math::ToDegree(particleRotDir.x);
		particleRotDir.y = Math::ToDegree(particleRotDir.y);
		particleRotDir.z = Math::ToDegree(particleRotDir.z);
		ImGui::SliderFloat3("ParticleRotDir", (float*)&particleRotDir, 0.0f, 170.0f);
		particleRotDir.x = Math::ToRadian(particleRotDir.x);
		particleRotDir.y = Math::ToRadian(particleRotDir.y);
		particleRotDir.z = Math::ToRadian(particleRotDir.z);

		ImGui::SliderFloat("RandLifeSpan", &randLifeSpan, 0.0f, 10.0f);
		ImGui::SliderFloat("RandPosSpan", &randPosSpan, 0.0f, 100.0f);

		randRotSpan = Math::ToDegree(randRotSpan);
		ImGui::SliderFloat("RandRotSpan", &randRotSpan, 0.0f, 15.0f);
		randRotSpan = Math::ToRadian(randRotSpan);

		ImGui::SliderFloat3("EmittDir", (float*)&ComDir, -1.0f, 1.0f);

		if (D3DXVec3Length(&ComDir) > 1)
			D3DXVec3Normalize(&ComDir, &ComDir);

		ImGui::Separator();
		ImGui::ColorEdit4("StartColor", csInfoDesc.StartColor);
		ImGui::ColorEdit4("EndColor", csInfoDesc.EndColor);
		ImGui::Separator();
		ImGui::SliderFloat("LifeTime", &csInfoDesc.Time, 0.1f, 100.0f);
		ImGui::SliderFloat("Velocity", &csInfoDesc.Velocity, 0.1f, 100.0f);
		ImGui::SliderFloat("Accelation", &csInfoDesc.Accelation, -10.0f, 10.0f);
		ImGui::SliderFloat("RotVel", &csInfoDesc.RotVel, 0.1f, 100.0f);
		ImGui::SliderFloat("RotAccel", &csInfoDesc.RotAccel, -10.0f, 10.0f);
		ImGui::SliderInt("StackCount", &StackCount, 10, 20);
		ImGui::SliderFloat("RadiusStep", &radiusStep, 0.01f, 1.0f);
		if (emitter == EmitterType::Cube)
		{
			ImGui::SliderFloat("CubeStep", &CubeStep, 0.1f, 10.0f);
			ImGui::SliderInt3("CubeSize", CubeSize, 1, 100);
		}
		ImGui::Separator();
		ID3D11ShaderResourceView* srv = NULL;
		if (particleTex != NULL)
			srv = particleTex->SRV();
		if (ImGui::ImageButton(srv, ImVec2(50, 50)))
		{
			ChangeTex();
		}
		ImGui::Separator();
		transform->Property();
		
	}
	ImGui::End();

}


void ParticleEmitter::PlayParticles()
{
	csInfoDesc.DeltaTime = Time::Get()->Delta();
	
	//����
	EmittParticles();

	//input������ ����
	computeBuffer->UpdateInput();
	//CS ���
	csConstBuffer->Apply();
	computeShader->AsConstantBuffer("CB_CS")->SetConstantBuffer(csConstBuffer->Buffer());
	computeShader->AsSRV("InputDatas")->SetResource(computeBuffer->SRV());
	computeShader->AsUAV("OutputDatas")->SetUnorderedAccessView(computeBuffer->UAV());

	computeShader->Dispatch(csTech, csPass, 1, 1, 1);
	//��� ��� ī��
	computeBuffer->Copy(csOutput, sizeof(CS_OutputDesc) * MAX_INSTANCE);
}

void ParticleEmitter::UpdateParticles()
{
	
	//��� �������� ������
	Matrix S;
	D3DXMatrixScaling(&S, particleScale.x, particleScale.y, particleScale.z);
	//������� �޸� ����� �� Ÿ�� �ٲ������

	int outidx = 0;
	//outidx�� ����� �����͵��� �ε���
	//i�� ���������ο� ������ ���� ������
	for (UINT i = 0; i < particleCount; )
	{
		// ���� �Ⱓ�� ���� ��ƼŬ ����
		//* �����δ� ��ƼŬ ī���͸� ���̰� ���� �ε����� �����ͷ� ��ü.
		if (csOutput[outidx].Time >= csOutput[outidx].LifeTime)
		{
			outidx++;
			particleCount--;
			continue;
		}
		outidx++;
		i++;
		csInput[i].Postion	= csOutput[outidx].Postion;
		csInput[i].Rotation	= csOutput[outidx].Rotation;
		csInput[i].LifeTime	= csOutput[outidx].LifeTime;
		csInput[i].Time		= csOutput[outidx].Time;
		csInput[i].RotDir	= csInput[outidx].RotDir;
		csInput[i].Direction = csInput[outidx].Direction;
		Matrix R, T;
		D3DXMatrixTranslation(&T, csOutput[outidx].Postion.x, csOutput[outidx].Postion.y, csOutput[outidx].Postion.z);
		D3DXMatrixRotationYawPitchRoll(&R, csOutput[outidx].Rotation.x, csOutput[outidx].Rotation.y, csOutput[outidx].Rotation.z);
		
		instDesc[i].world = S * R * T;
		instDesc[i].color = csOutput[outidx].ResultColor;
	}
}

void ParticleEmitter::EmittParticles()
{
	//��� �ð��� ��Ÿ ����
	elapsed += csInfoDesc.DeltaTime;

	if (particleCount >= MAX_INSTANCE)
		return;
	// �ð��� ���ⷮ�� ���ؼ� ������ ��ƼŬ�� ����
	int emitCount = elapsed * EmittPerSec;

	// ���ⷮ�� ���ϸ� ��ƼŬ�ִ�ġ�� �ʰ��ϴ� ���
	if (particleCount + emitCount >= MAX_INSTANCE)
		emitCount = MAX_INSTANCE - particleCount-1;

	// ���� �ݺ�
	while (emitCount > 0)
	{
		switch (emitter)
		{
		case EmitterType::Point:
			EmittPoint();
			break;
		case EmitterType::Sphere:
			EmittSphere();
			break;
		case EmitterType::Cube:
			EmittCube();
			break;
		default:
			break;
		}

		//��ƼŬ ����
		particleCount++;

		//���ⷮ ����
		emitCount--;
		//����� ����
		elapsed -= 1.0f/ EmittPerSec;
	}
	
}

//�� ������ ��ƼŬ �����
void ParticleEmitter::EmittPoint()
{
	csInput[particleCount].Postion	= Math::RandomVec3(-randPosSpan, randPosSpan);
	csInput[particleCount].Rotation = Math::RandomVec3(-randRotSpan, randRotSpan);
	csInput[particleCount].Rotation += particleRot;
	csInput[particleCount].RotDir	= particleRotDir;
	csInput[particleCount].LifeTime = Math::Random(-randLifeSpan, randLifeSpan);
	csInput[particleCount].LifeTime += csInfoDesc.Time;
	csInput[particleCount].Time		= 0.0f;

	float phiStep = Math::PI / StackCount;
	float thetaStep = 2.0f * Math::PI / StackCount;
	int j = roundCount % StackCount;
	int i = roundCount / StackCount;
	roundCount++;
	roundCount %= StackCount * StackCount;
	float theta = j * thetaStep;
	float phi = i * phiStep;

	switch (emittype)
	{
	case EmittType::Direct:
		csInput[particleCount].Direction = ComDir;
		break;
	case EmittType::Around:
	
		csInput[particleCount].Direction = Vector3
		(
			(sinf(phi) * cosf(theta)),
			(cosf(phi)),
			(sinf(phi) * sinf(theta))
		);
		D3DXVec3Normalize(&csInput[particleCount].Direction, &csInput[particleCount].Direction);
		break;
	case EmittType::Cone:
		break;
	default:
		break;
	}
}

//�� ������ ��ƼŬ �����
void ParticleEmitter::EmittSphere()
{

	float phiStep = Math::PI / StackCount;
	float thetaStep = 2.0f * Math::PI / StackCount;
	//���� ��ƼŬ ������ ���� ���� ��ƼŬ ���� ������ �޶���
	int j = roundCount % StackCount;
	int i = roundCount / StackCount;
	roundCount++;
	roundCount %= StackCount * StackCount;
	//���� �����ų ������
	float currR = ((i) * radiusStep);

	float theta = j * thetaStep;
	float phi = i * phiStep;

	
	csInput[particleCount].LifeTime = Math::Random(-randLifeSpan, randLifeSpan);
	csInput[particleCount].LifeTime += csInfoDesc.Time;
	csInput[particleCount].Time		= 0.0f;
	csInput[particleCount].Postion	= Math::RandomVec3(-randPosSpan, randPosSpan);
	csInput[particleCount].Rotation = Math::RandomVec3(-randRotSpan, randRotSpan);
	csInput[particleCount].Rotation += particleRot;
	csInput[particleCount].Postion	+= Vector3
	(
		(currR * sinf(phi) * cosf(theta)),
		(currR * cosf(phi)),
		(currR * sinf(phi) * sinf(theta))
	);
	csInput[particleCount].RotDir	= particleRotDir;
	switch (emittype)
	{
	case EmittType::Direct:
		csInput[particleCount].Direction = ComDir;
		break;
	case EmittType::Around:
		D3DXVec3Normalize(&csInput[particleCount].Direction, &csInput[particleCount].Postion);
		break;
	case EmittType::Cone:
		break;
	default:
		break;
	}

}

void ParticleEmitter::EmittCube()
{
	csInput[particleCount].Postion = Math::RandomVec3(-randPosSpan, randPosSpan);
	csInput[particleCount].Rotation = Math::RandomVec3(-randRotSpan, randRotSpan);
	csInput[particleCount].Rotation += particleRot;
	csInput[particleCount].RotDir = particleRotDir;
	csInput[particleCount].LifeTime = Math::Random(-randLifeSpan, randLifeSpan);
	csInput[particleCount].LifeTime += csInfoDesc.Time;
	csInput[particleCount].Time = 0.0f;
	
	
	int xCount = CubeSize[0];
	int yCount = CubeSize[1];
	int zCount = CubeSize[2];

	int i = cubeCount % xCount;
	int j = cubeCount / xCount;
	j %= yCount;
	int k = cubeCount / (xCount*yCount);

	cubeCount++;
	cubeCount %= xCount * yCount * zCount;

	Vector3 position = -Vector3(CubeSize[0], CubeSize[1], CubeSize[2]) * 0.5f*CubeStep;
	position.x += i * CubeStep;
	position.y += j * CubeStep;
	position.z += k * CubeStep;

	csInput[particleCount].Postion += position;


	switch (emittype)
	{
	case EmittType::Direct:
		csInput[particleCount].Direction = ComDir;
		break;
	case EmittType::Around:
		D3DXVec3Normalize(&csInput[particleCount].Direction, &csInput[particleCount].Postion);
		break;
	case EmittType::Cone:
		break;
	default:
		break;
	}
}

void ParticleEmitter::ChangeTex(wstring filePath)
{
	if (filePath.length() < 1)
	{
		Path::OpenFileDialog(L"", Path::ImageFilter, L"../../_Textures/", bind(&ParticleEmitter::ChangeTex, this, placeholders::_1));
	}
	else
	{
		SafeDelete(particleTex);
		wstring path	= Path::GetFileName(filePath);
		wstring dir		= Path::GetDirectoryName(filePath);

		particleTex		= new Texture(path,dir);
	}
}



/////////////////////////////////////////////////////
////	ComputeShader
/////////////////////////////////////////////////////

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
