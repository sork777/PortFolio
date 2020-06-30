#include "Framework.h"
#include "ParticleEmitter.h"
#include "Utilities/Xml.h"



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
	
}

void ParticleEmitter::Initialize()
{
	pass	= 0;
	tech	= 0;
	
	shader			= SETSHADER(L"Effect/ParticleEmitter.fx");
	computeShader	= SETSHADER(L"Effect/ParticleSimulation.fx");

	perFrame		= new PerFrame(shader);
	transform		= new Transform(shader);
	transform->Position(0, 10, 0);

	//인스턴싱이라 얘 하나 있으면 됨
	point.Position	= Vector3(0, 0, 0);
	point.Scale		= Vector2(1, 1);
	vertexBuffer	= new VertexBuffer(&point, 1, sizeof(ParticleVertex));

	for (UINT i = 0; i < MAX_INSTANCE; i++)
	{
		D3DXMatrixIdentity(&instDesc[i].world);
	}

	instanceBuffer	= new VertexBuffer(&instDesc, MAX_INSTANCE, sizeof(ParticleDesc),1,true);
	sInstanceBuffer = shader->AsConstantBuffer("CB_Particle");
	sParticleTex	= shader->AsSRV("ParticleTex");
	particleTex		= new Texture(L"Particle/Smoke.png");
	CreateComputeShader();
}

void ParticleEmitter::Update()
{
	perFrame->Update();
	transform->Update();

	if (bPlay == true)
	{
		PlayParticles();
		UpdateParticles();
	}
}

void ParticleEmitter::Render()
{

	perFrame->Render();
	transform->Render();

	if (vertexBuffer != NULL)
		vertexBuffer->Render();
	instanceBuffer->Render();
	//sInstanceBuffer->SetConstantBuffer(instanceBuffer->Buffer());
	if(particleTex !=NULL)
		sParticleTex->SetResource(particleTex->SRV());
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	//인스턴싱 갯수 조절로 파티클 갯수 조절
	shader->DrawInstanced(tech, pass, 1, particleCount);
}

void ParticleEmitter::Property()
{
	ImGui::Begin("EmitterProperty", &bPropertyOpen);
	{
		ImGuiTreeNodeFlags collapsFlag = ImGuiTreeNodeFlags_DefaultOpen 
			| ImGuiTreeNodeFlags_CollapsingHeader;

		ImGui::Checkbox("ParticlePlay", &bPlay);
		ImGui::SliderInt("EmittPerSecond", &emittPerSec, 1, 256);
		ImGui::LabelText("##P_Count", "ParticleCount : %d", particleCount);
		ImGui::Separator();
		if (ImGui::CollapsingHeader("TypeSelection", collapsFlag))
		{
			//콤보박스 브러시 타입
			string emitterTypes[]	= { "Point", "Sphere" , "Cube" };
			string emittTypes[]		= { "Direct", "Round" };
			string particleTypes[]	= { "Texture","Quad", "Diamond" };

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
						roundCount = 0;
						cubeCount = 0;
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
		}
		ImGui::Separator();
		if (ImGui::CollapsingHeader("InitProperty", collapsFlag))
		{
			ImGui::SliderFloat3("InitScale", (float*)&InitScale, 0.1f, 10.0f);

			InitRot.x = Math::ToDegree(InitRot.x);
			InitRot.y = Math::ToDegree(InitRot.y);
			InitRot.z = Math::ToDegree(InitRot.z);
			ImGui::SliderFloat3("InitRot", (float*)&InitRot, 0.0f, 170.0f);
			InitRot.x = Math::ToRadian(InitRot.x);
			InitRot.y = Math::ToRadian(InitRot.y);
			InitRot.z = Math::ToRadian(InitRot.z);

			Torque.x = Math::ToDegree(Torque.x);
			Torque.y = Math::ToDegree(Torque.y);
			Torque.z = Math::ToDegree(Torque.z);
			ImGui::SliderFloat3("Torque", (float*)&Torque, 0.0f, 170.0f);
			Torque.x = Math::ToRadian(Torque.x);
			Torque.y = Math::ToRadian(Torque.y);
			Torque.z = Math::ToRadian(Torque.z);

			ImGui::SliderFloat3("Force", (float*)&Force, -1.0f, 1.0f);
		}
		ImGui::Separator();
		if (ImGui::CollapsingHeader("RandomVarience", collapsFlag))
		{
			ImGui::SliderFloat("RandLifeVar", &randLifeVar, 0.0f, 10.0f);
			ImGui::SliderFloat("RandPosVar", &randPosVar, 0.0f, 10.0f);

			randRotVar = Math::ToDegree(randRotVar);
			ImGui::SliderFloat("RandRotVar", &randRotVar, 0.0f, 15.0f);
			randRotVar = Math::ToRadian(randRotVar);

			ImGui::SliderFloat("RandForceVar", &randForceVar, 0.0f, 1.0f);
			randTorqueVar = Math::ToDegree(randTorqueVar);
			ImGui::SliderFloat("RandTorqueVar", &randTorqueVar, 0.0f, 1.0f);
			randTorqueVar = Math::ToRadian(randTorqueVar);
		}
		ImGui::Separator();
		if (ImGui::CollapsingHeader("ParticleProperty", collapsFlag))
		{
			ImGui::ColorEdit4("StartColor", csInfoDesc.StartColor);
			ImGui::ColorEdit4("EndColor", csInfoDesc.EndColor);
			ImGui::Separator();
			ImGui::SliderFloat("LifeTime", &csInfoDesc.Time, 0.1f, 30.0f);
			ImGui::SliderFloat("Velocity", &csInfoDesc.Velocity, 0.1f, 100.0f);
			ImGui::SliderFloat("Accelation", &csInfoDesc.Accelation, -10.0f, 10.0f);
			ImGui::SliderFloat("RotVelocity", &csInfoDesc.RotVel, 0.1f, 10.0f);
			ImGui::SliderFloat("RotAccelation", &csInfoDesc.RotAccel, -10.0f, 10.0f);

			ImGui::Separator();
			ID3D11ShaderResourceView* srv = NULL;
			if (particleTex != NULL)
				srv = particleTex->SRV();
			if (ImGui::ImageButton(srv, ImVec2(50, 50)))
			{
				ChangeTex();
			}
			ImGui::Separator();
			if (emitter == EmitterType::Sphere || emittype == EmittType::Around)
			{
				ImGui::SliderFloat("RadiusStep", &radiusStep, 0.01f, 1.0f);
				ImGui::SliderInt("SphereDivide", &sphereDivide, 10, 20);
			}
			else if (emitter == EmitterType::Cube)
			{
				ImGui::SliderFloat("CubeStep", &cubeStep, 0.1f, 10.0f);
				ImGui::SliderInt3("CubeSize", cubeSize, 1, 100);
			}
			
		}
		transform->Property();		
	}
	ImGui::End();

}

#pragma region Particle 업데이트

void ParticleEmitter::PlayParticles()
{
	csInfoDesc.DeltaTime = Time::Get()->Delta();
	
	//방출
	EmittParticles();

	//input데이터 업뎃
	computeBuffer->UpdateInput();
	//CS 계산
	csConstBuffer->Apply();
	computeShader->AsConstantBuffer("CB_CS")->SetConstantBuffer(csConstBuffer->Buffer());
	computeShader->AsSRV("InputDatas")->SetResource(computeBuffer->SRV());
	computeShader->AsUAV("OutputDatas")->SetUnorderedAccessView(computeBuffer->UAV());

	computeShader->Dispatch(csTech, csPass, ceil((float)particleCount /1024.0f), 1, 1);
	//계산 결과 카피
	computeBuffer->Copy(csOutput, sizeof(CS_OutputDesc) * MAX_INSTANCE);
}

void ParticleEmitter::UpdateParticles()
{
	
	//얘는 공용으로 쓸거임
	Matrix S;
	D3DXMatrixScaling(&S, InitScale.x, InitScale.y, InitScale.z);
	//빌보드와 달리 실행시 매 타임 바꿔줘야함

	int outidx = 0;
	//outidx는 계산한 데이터들의 인덱스
	//i는 파이프라인에 보내기 위한 데이터
	for (UINT i = 0; i < particleCount; )
	{
		// 생존 기간을 넘은 파티클 삭제
		//* 실제로는 파티클 카운터를 줄이고 다음 인덱스의 데이터로 교체.
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

	D3D11_MAPPED_SUBRESOURCE subResource;
	D3D::GetDC()->Map(instanceBuffer->Buffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
	{
		memcpy(subResource.pData, &instDesc, sizeof(ParticleDesc) * MAX_INSTANCE);
	}
	D3D::GetDC()->Unmap(instanceBuffer->Buffer(), 0);
}

void ParticleEmitter::EmittParticles()
{
	//경과 시간에 델타 적립
	elapsed += csInfoDesc.DeltaTime;

	if (particleCount >= MAX_INSTANCE)
		return;
	// 시간당 방출량을 곱해서 방출할 파티클양 결정
	int emitCount = elapsed * emittPerSec;

	// 방출량을 더하면 파티클최대치를 초과하는 경우
	if (particleCount + emitCount >= MAX_INSTANCE)
		emitCount = MAX_INSTANCE - particleCount-1;

	// 방출 반복
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

		csInput[particleCount].RotDir.x *= (1.0f + Math::Random(-randTorqueVar, randTorqueVar));
		csInput[particleCount].RotDir.y *= (1.0f + Math::Random(-randTorqueVar, randTorqueVar));
		csInput[particleCount].RotDir.z *= (1.0f + Math::Random(-randTorqueVar, randTorqueVar));
		csInput[particleCount].Direction.x *= (1.0f + Math::Random(-randForceVar, randForceVar));
		csInput[particleCount].Direction.y *= (1.0f + Math::Random(-randForceVar, randForceVar));
		csInput[particleCount].Direction.z *= (1.0f + Math::Random(-randForceVar, randForceVar));
		//파티클 증가
		particleCount++;

		//방출량 감소
		emitCount--;
		//경과값 감소
		elapsed -= 1.0f/ emittPerSec;
	}
	
}

//점 형식의 파티클 방출기
void ParticleEmitter::EmittPoint()
{
	csInput[particleCount].Postion	= Math::RandomVec3(-randPosVar, randPosVar);
	csInput[particleCount].Rotation = Math::RandomVec3(-randRotVar, randRotVar);
	csInput[particleCount].Rotation += InitRot;
	csInput[particleCount].RotDir	= Torque;
	csInput[particleCount].LifeTime = Math::Random(-randLifeVar, randLifeVar);
	csInput[particleCount].LifeTime += csInfoDesc.Time;
	csInput[particleCount].Time		= 0.0f;

	float phiStep = Math::PI / sphereDivide;
	float thetaStep = 2.0f * Math::PI / sphereDivide;
	int j = roundCount % sphereDivide;
	int i = roundCount / sphereDivide;
	roundCount++;
	roundCount %= sphereDivide * sphereDivide;
	float theta = j * thetaStep;
	float phi = i * phiStep;

	switch (emittype)
	{
	case EmittType::Direct:
		csInput[particleCount].Direction = Force;
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
	default:
		break;
	}
}

//구 형식
void ParticleEmitter::EmittSphere()
{

	float phiStep = Math::PI / sphereDivide;
	float thetaStep = 2.0f * Math::PI / sphereDivide;
	//이전 파티클 갯수에 따라 현재 파티클 방출 각도가 달라짐
	int j = roundCount % sphereDivide;
	int i = roundCount / sphereDivide;
	roundCount++;
	roundCount %= sphereDivide * sphereDivide;
	//현재 방출시킬 반지름
	float currR = ((i) * radiusStep);

	float theta = j * thetaStep;
	float phi = i * phiStep;

	
	csInput[particleCount].LifeTime = Math::Random(-randLifeVar, randLifeVar);
	csInput[particleCount].LifeTime += csInfoDesc.Time;
	csInput[particleCount].Time		= 0.0f;
	csInput[particleCount].Postion	= Math::RandomVec3(-randPosVar, randPosVar);
	csInput[particleCount].Rotation = Math::RandomVec3(-randRotVar, randRotVar);
	csInput[particleCount].Rotation += InitRot;
	csInput[particleCount].Postion	+= Vector3
	(
		(currR * sinf(phi) * cosf(theta)),
		(currR * cosf(phi)),
		(currR * sinf(phi) * sinf(theta))
	);
	csInput[particleCount].RotDir	= Torque;
	switch (emittype)
	{
	case EmittType::Direct:
		csInput[particleCount].Direction = Force;
		break;
	case EmittType::Around:
		D3DXVec3Normalize(&csInput[particleCount].Direction, &csInput[particleCount].Postion);
		break;
	default:
		break;
	}
}
//큐브 형식 : 날씨 효과 할때?
void ParticleEmitter::EmittCube()
{
	csInput[particleCount].Postion = Math::RandomVec3(-randPosVar, randPosVar);
	csInput[particleCount].Rotation = Math::RandomVec3(-randRotVar, randRotVar);
	csInput[particleCount].Rotation += InitRot;
	csInput[particleCount].RotDir = Torque;
	csInput[particleCount].LifeTime = Math::Random(-randLifeVar, randLifeVar);
	csInput[particleCount].LifeTime += csInfoDesc.Time;
	csInput[particleCount].Time = 0.0f;
	
	
	int xCount = cubeSize[0];
	int yCount = cubeSize[1];
	int zCount = cubeSize[2];

	int i = cubeCount % xCount;
	int j = cubeCount / xCount;
	j %= yCount;
	int k = cubeCount / (xCount*yCount);

	cubeCount++;
	cubeCount %= xCount * yCount * zCount;

	Vector3 position = -Vector3(cubeSize[0], cubeSize[1], cubeSize[2]) * 0.5f*cubeStep;
	position.x += i * cubeStep;
	position.y += j * cubeStep;
	position.z += k * cubeStep;

	csInput[particleCount].Postion += position;


	switch (emittype)
	{
	case EmittType::Direct:
		csInput[particleCount].Direction = Force;
		break;
	case EmittType::Around:
		D3DXVec3Normalize(&csInput[particleCount].Direction, &csInput[particleCount].Postion);
		break;
	default:
		break;
	}
}

#pragma endregion

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

void ParticleEmitter::LoadParticle(wstring path)
{
	if (path.length() < 1)
	{
		function<void(wstring)> f = bind(&ParticleEmitter::LoadParticle, this, placeholders::_1);
		Path::OpenFileDialog(L"", L"ParticleData\0*.particle", L"", f);
	}
	else
	{
		particleCount = 0;
		cubeCount = 0;
		roundCount = 0;

		elapsed = 0.0f;
		SafeDelete(particleTex);

		wstring particlePath = L"NoTex";
		wstring particleDir = L"NoTex";

		Xml::XMLDocument* document = new Xml::XMLDocument();
		Xml::XMLError error = document->LoadFile(String::ToString(path).c_str());
		assert(error == Xml::XML_SUCCESS);

		Xml::XMLElement* root = document->FirstChildElement();
		emittPerSec = root->IntAttribute("EPS");
		particlePath = String::ToWString( root->Attribute("TexturePath"));
		particleDir = String::ToWString( root->Attribute("TextureDir"));

		if (particlePath.find(L"NoTex") != string::npos)
		{
			particleTex = new Texture(particlePath, particleDir);
		}


		Xml::XMLElement* node = root->FirstChildElement();
		{
			emittype	= EmittType(node->IntAttribute("EmitType"));
			emitter		= EmitterType(node->IntAttribute("EmitterType"));
			particle	= ParticleType(node->IntAttribute("ParticleType"));
			switch (emittype)
			{
			case EmittType::Direct:
				current_emit = "Direct";
				break;
			case EmittType::Around:
				current_emit = "Around";
				break;
			}
			switch (emitter)
			{
			case EmitterType::Point:
				current_emitter = "Point";
				break;
			case EmitterType::Sphere:
				current_emitter = "Sphere";
				break;
			case EmitterType::Cube:
				current_emitter = "Cube";
				break;					
			}
			switch (particle)
			{
			case ParticleType::Texture:
				current_particle = "Texture";
				pass = 0;
				break;
			case ParticleType::Quad:
				current_particle = "Quad";
				pass = 1;
				break;
			case ParticleType::Diamond:
				current_particle = "Diamond";
				pass = 2;
				break;
			}
		}
		node = node->NextSiblingElement();
		{
			Xml::XMLElement* element = node->FirstChildElement();

			InitScale.x=element->FloatAttribute("X");
			InitScale.y=element->FloatAttribute("Y");
			InitScale.z=element->FloatAttribute("Z");
			element = element->NextSiblingElement();

			InitRot.x = element->FloatAttribute("X");
			InitRot.y = element->FloatAttribute("Y");
			InitRot.z = element->FloatAttribute("Z");
			element = element->NextSiblingElement();

			Torque.x = element->FloatAttribute("X");
			Torque.y = element->FloatAttribute("Y");
			Torque.z = element->FloatAttribute("Z");
			element = element->NextSiblingElement();

			Force.x = element->FloatAttribute("X");
			Force.y = element->FloatAttribute("Y");
			Force.z = element->FloatAttribute("Z");
			element = element->NextSiblingElement();
		}
		node = node->NextSiblingElement();
		{
			Xml::XMLElement* element = node->FirstChildElement();
			randPosVar	= element->FloatAttribute("PosVar");
			element		= element->NextSiblingElement();

			randRotVar	= element->FloatAttribute("RotVar");
			element		= element->NextSiblingElement();

			randLifeVar	= element->FloatAttribute("LifeVar");
			element		= element->NextSiblingElement();

			randForceVar	= element->FloatAttribute("ForceVar");
			element			= element->NextSiblingElement();

			randTorqueVar	= element->FloatAttribute("TorqueVar");
		}
		node = node->NextSiblingElement();

		{
			Xml::XMLElement* element = node->FirstChildElement();
			Xml::XMLElement* color = element->FirstChildElement();
			csInfoDesc.StartColor.r = color->FloatAttribute("R");
			csInfoDesc.StartColor.g = color->FloatAttribute("G");
			csInfoDesc.StartColor.b = color->FloatAttribute("B");
			csInfoDesc.StartColor.a = color->FloatAttribute("A");
			color = color->NextSiblingElement();

			csInfoDesc.EndColor.r = color->FloatAttribute("R");
			csInfoDesc.EndColor.g = color->FloatAttribute("G");
			csInfoDesc.EndColor.b = color->FloatAttribute("B");
			csInfoDesc.EndColor.a = color->FloatAttribute("A");

			element = element->NextSiblingElement();
			csInfoDesc.Time = element->FloatAttribute("Life");

			element = element->NextSiblingElement();
			csInfoDesc.Velocity = element->FloatAttribute("Velocity");

			element = element->NextSiblingElement();
			csInfoDesc.Accelation = element->FloatAttribute("Accelation");

			element = element->NextSiblingElement();
			csInfoDesc.RotVel = element->FloatAttribute("RotVel");

			element = element->NextSiblingElement();
			csInfoDesc.RotAccel = element->FloatAttribute("RotAccel");
		}

		node = node->NextSiblingElement();
		{
			Xml::XMLElement* element = node->FirstChildElement();
			radiusStep		= element->FloatAttribute("RadiusStep");
			element			= element->NextSiblingElement();
			
			cubeStep		= element->FloatAttribute("CubeStep");
			element			= element->NextSiblingElement();
			
			sphereDivide	= element->IntAttribute("SphereDivide");		
			element			= element->NextSiblingElement();

			cubeSize[0]		= element->IntAttribute("X");
			cubeSize[1]		= element->IntAttribute("Y");
			cubeSize[2]		= element->IntAttribute("Z");
		}
	}
}

void ParticleEmitter::SaveParticle(wstring path)
{
	if (path.length() < 1)
	{
		function<void(wstring)> f = bind(&ParticleEmitter::SaveParticle, this, placeholders::_1);
		Path::SaveFileDialog(L"", L"ParticleData\0*.particle", L"", f);
	}
	else
	{
		string folder = String::ToString(Path::GetDirectoryName(path));
		string file = String::ToString(Path::GetFileName(path));

		Path::CreateFolders(folder);

		wstring particlePath=L"NoTex";
		wstring particleDir	=L"NoTex";
		if (particleTex != NULL)
		{
			particlePath = particleTex->GetFile();
			particleDir = particleTex->GetDir();
		}
		Xml::XMLDocument* document = new Xml::XMLDocument();

		Xml::XMLDeclaration* decl = document->NewDeclaration();
		document->LinkEndChild(decl);

		Xml::XMLElement* root = document->NewElement("ParticleInfo");
		root->SetAttribute("EPS", emittPerSec);
		root->SetAttribute("TexturePath", String::ToString(particlePath).c_str());
		root->SetAttribute("TextureDir", String::ToString(particleDir).c_str());
		document->LinkEndChild(root);

		Xml::XMLElement* type = document->NewElement("TypeInfo");
		type->SetAttribute("EmitType", (int)emittype);
		type->SetAttribute("EmitterType", (int)emitter);
		type->SetAttribute("ParticleType", (int)particle);
		root->LinkEndChild(type);

		{
			Xml::XMLElement* init = document->NewElement("InitialData");
			Xml::XMLElement* element = document->NewElement("Scale");
			element->SetAttribute("X", InitScale.x);
			element->SetAttribute("Y", InitScale.y);
			element->SetAttribute("Z", InitScale.z);
			init->LinkEndChild(element);

			element = document->NewElement("Rotation");
			element->SetAttribute("X", InitRot.x);
			element->SetAttribute("Y", InitRot.y);
			element->SetAttribute("Z", InitRot.z);
			init->LinkEndChild(element);
			element = document->NewElement("Torque");
			element->SetAttribute("X", Torque.x);
			element->SetAttribute("Y", Torque.y);
			element->SetAttribute("Z", Torque.z);
			init->LinkEndChild(element);
			element = document->NewElement("Force");
			element->SetAttribute("X", Force.x);
			element->SetAttribute("Y", Force.y);
			element->SetAttribute("Z", Force.z);
			init->LinkEndChild(element);
			root->LinkEndChild(init);
		}
		{
			Xml::XMLElement* rand = document->NewElement("RandomVariants");
			Xml::XMLElement* element = document->NewElement("PosVar");
			element->SetAttribute("PosVar", randPosVar);
			rand->LinkEndChild(element);

			element = document->NewElement("RotVar");
			element->SetAttribute("RotVar", randRotVar);
			rand->LinkEndChild(element);

			element = document->NewElement("LifeVar");
			element->SetAttribute("LifeVar", randLifeVar);
			rand->LinkEndChild(element);

			element = document->NewElement("ForceVar");
			element->SetAttribute("ForceVar", randForceVar);
			rand->LinkEndChild(element);

			element = document->NewElement("TorqueVar");
			element->SetAttribute("TorqueVar", randTorqueVar);
			rand->LinkEndChild(element);
			root->LinkEndChild(rand);
		}

		{
			Xml::XMLElement* common = document->NewElement("Common");
			Xml::XMLElement* element = document->NewElement("Color");
			Xml::XMLElement* color = document->NewElement("StartColor");
			color->SetAttribute("R", csInfoDesc.StartColor.r);
			color->SetAttribute("G", csInfoDesc.StartColor.g);
			color->SetAttribute("B", csInfoDesc.StartColor.b);
			color->SetAttribute("A", csInfoDesc.StartColor.a);
			element->LinkEndChild(color);
			color = document->NewElement("EndColor");
			color->SetAttribute("R", csInfoDesc.EndColor.r);
			color->SetAttribute("G", csInfoDesc.EndColor.g);
			color->SetAttribute("B", csInfoDesc.EndColor.b);
			color->SetAttribute("A", csInfoDesc.EndColor.a);
			element->LinkEndChild(color);
			common->LinkEndChild(element);

			element = document->NewElement("Life");
			element->SetAttribute("Life", csInfoDesc.Time);
			common->LinkEndChild(element);

			element = document->NewElement("Velocity");
			element->SetAttribute("Velocity", csInfoDesc.Velocity);
			common->LinkEndChild(element);
			
			element = document->NewElement("Accelation");
			element->SetAttribute("Accelation", csInfoDesc.Accelation);
			common->LinkEndChild(element);
			
			element = document->NewElement("RotVel");
			element->SetAttribute("RotVel", csInfoDesc.RotVel);
			common->LinkEndChild(element);
			
			element = document->NewElement("RotAccel");
			element->SetAttribute("RotAccel", csInfoDesc.RotAccel);
			common->LinkEndChild(element);
			root->LinkEndChild(common);
		}
			   		 
		{
			Xml::XMLElement* etc = document->NewElement("ETC");
			Xml::XMLElement* element = document->NewElement("RadiusStep");
			element->SetAttribute("RadiusStep", radiusStep);
			etc->LinkEndChild(element);

			element = document->NewElement("CubeStep");
			element->SetAttribute("CubeStep", cubeStep);
			etc->LinkEndChild(element);

			element = document->NewElement("SphereDivide");
			element->SetAttribute("SphereDivide", sphereDivide);
			etc->LinkEndChild(element);
			element = document->NewElement("CubeSize");
			element->SetAttribute("X", cubeSize[0]);
			element->SetAttribute("Y", cubeSize[1]);
			element->SetAttribute("Z", cubeSize[2]);
			etc->LinkEndChild(element);
			root->LinkEndChild(etc);
		}

		if (Path::GetExtension(file).compare("particle") != 0)
			file = Path::GetFilePathWithoutExtension( file) + ".particle";

		document->SaveFile((folder + file).c_str());
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
