#include "Framework.h"
#include "AnimationEditor.h"
#include "Viewer/Freedom.h"
#include "Environment/Sky.h"


AnimationEditor::AnimationEditor()
{
	Initialize();
}


AnimationEditor::~AnimationEditor()
{
	Destroy();
}

void AnimationEditor::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(23, 0, 0);
	Context::Get()->GetCamera()->Position(0, 32, -67);
	((Freedom *)Context::Get()->GetCamera())->Speed(20, 2);


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
}

void AnimationEditor::Destroy()
{
	SafeDelete(sky);
	SafeDelete(grid);
	SafeDelete(floor);
	SafeDelete(shader);
}

void AnimationEditor::Update()
{
	sky->Update();
	grid->Update();

	if (mainModel != NULL)
	{
		mainModel->Update();
		for (Attach att : attaches)
		{
			if (att.collider != NULL)
			{
				UINT bone = att.bone;
				Matrix attach = mainModel->GetboneWorld(0, bone);
				att.collider->GetTransform()->Parent(attach);
				att.collider->Update();
			}
		}
	}
}

void AnimationEditor::PreRender()
{
}

void AnimationEditor::Render()
{
	sky->Render();

	Pass(0, 1, 2);

	floor->Render();
	grid->Render();
	for (MeshRender* temp : meshes)
		temp->Render();

	for (Model* temp : models)
		temp->Render();
	if (mainModel != NULL)
	{
		mainModel->Render();
	}
}

void AnimationEditor::PostRender()
{
}

void AnimationEditor::Pass(UINT mesh, UINT model, UINT anim)
{
	for (MeshRender* temp : meshes)
		temp->Pass(mesh);

	for (Model* temp : models)
	{
		if(dynamic_cast<ModelAnimator*>(temp))
			temp->Pass(anim);
		else if (dynamic_cast<ModelRender*>(temp))
			temp->Pass(model);
	}
	if (mainModel != NULL)
	{
		mainModel->Pass(anim);
	}
}

void AnimationEditor::ImGUIController()
{
}

void AnimationEditor::AnimationController()
{
}

void AnimationEditor::SelectedPartsViewer()
{
}

void AnimationEditor::PartsViewer()
{
}

void AnimationEditor::ChildViewer(ModelBone * bone)
{
}

void AnimationEditor::ModelsViewer()
{
}

void AnimationEditor::Popup()
{
}

void AnimationEditor::AddSocket()
{
}

void AnimationEditor::AddAnimation()
{
}

void AnimationEditor::ModelAttach()
{
}

void AnimationEditor::LoadModel(wstring path)
{
	if(path.length()<1)
	{ 
		Path::OpenFileDialog(L"", Path::FbxModelFilter, L"", bind(&AnimationEditor::LoadModel, this, placeholders::_1));
	}
	else
	{

	}
}
