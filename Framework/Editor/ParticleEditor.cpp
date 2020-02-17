#include "Framework.h"
#include "ParticleEditor.h"
#include "Environment/Sky.h"



ParticleEditor::ParticleEditor()
{
	Initialize();
}


ParticleEditor::~ParticleEditor()
{
	Destroy();
}

void ParticleEditor::Initialize()
{
	shader = new Shader(L"027_Animation.fx");
	sky = new Sky(L"Environment/GrassCube1024.dds");
	//Create Material
	{
		floor = new Material(shader);
		floor->LoadDiffuseMap("Floor.png");
	}

	//Create Mesh
	{
		Transform* transform = NULL;

		grid = new MeshRender(shader, new MeshGrid(5, 5));
		transform = grid->AddTransform();
		transform->Position(0, 0, 0);
		transform->Scale(12, 1, 12);
	}
	grid->UpdateTransforms();

	emitter = new ParticleEmitter();
	//particle_emit = new ParticleEmitter();
}

void ParticleEditor::Destroy()
{
	SafeDelete(sky);
	SafeDelete(grid);
	SafeDelete(floor);

	SafeDelete(shader);
}

void ParticleEditor::Update()
{
	sky->Update();
	grid->Update();
	emitter->Update();
	if (ImGui::Button("SaveTest"))
		emitter->SaveParticle();
	if (ImGui::Button("LoadTest"))
		emitter->LoadParticle();
}

void ParticleEditor::Render()
{
	sky->Render();

	floor->Render();
	grid->Pass(0);
	grid->Render();
	emitter->Render();
	emitter->Property();
}
