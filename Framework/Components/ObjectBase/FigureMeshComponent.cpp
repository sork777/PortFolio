#include "Framework.h"
#include "FigureMeshComponent.h"



FigureMeshComponent::FigureMeshComponent(Mesh* mesh)
{
	MeshCopy(*mesh);


	componentName = L"FigureMeshComp";
	type = ObjectBaseComponentType::FigureMesh;
	meshRender = new MeshRender(shader, mesh);
}

FigureMeshComponent::FigureMeshComponent(const FigureMeshComponent& meshComp)
	:ObjectBaseComponent(meshComp)
{
	componentName = L"FigureMeshComp";
	type = ObjectBaseComponentType::FigureMesh;
	
	MeshCopy(*meshComp.mesh);

	meshRender = new MeshRender(shader, mesh);
	Super::ClonningChildren(meshComp.children);
}


FigureMeshComponent::~FigureMeshComponent()
{
	SafeDelete(mesh);
	SafeDelete(meshRender);
}

void FigureMeshComponent::CompileComponent(const FigureMeshComponent & OBComp)
{
	Super::CompileComponent(OBComp);
	// 도형 메시 복사
	MeshCopy(*OBComp.mesh);
	// 도형 메시 교체
	meshRender->MeshChanger(mesh);
}

void FigureMeshComponent::MeshCopy(Mesh& oMesh)
{
	MeshType meshType = oMesh.GetMeshType();
	switch (meshType)
	{
	case MeshType::None:
		break;
	case MeshType::Cylinder:
		mesh = new MeshCylinder(*dynamic_cast<MeshCylinder*>(&oMesh));
		break;
	case MeshType::Cube:
		mesh = new MeshCube(*dynamic_cast<MeshCube*>(&oMesh));
		break;
	case MeshType::Grid:
		mesh = new MeshGrid(*dynamic_cast<MeshGrid*>(&oMesh));
		break;
	case MeshType::Quad:
		mesh = new MeshQuad(*dynamic_cast<MeshQuad*>(&oMesh));
		break;
	case MeshType::Sphere:
		mesh = new MeshSphere(*dynamic_cast<MeshSphere*>(&oMesh));
		break;
	default:
		break;
	}
}

void FigureMeshComponent::Update()
{
	meshRender->Update();
	Super::Update();
}

void FigureMeshComponent::Render()
{
	meshRender->Render();
	Super::Render();
}

bool FigureMeshComponent::Property(const int& instance)
{
	if (0 >= GetInstSize())		return false;
	if (0 >= GetInstSize() - instance)		return false;

	bool bChange = false;
	if (ImGui::CollapsingHeader("Materials", ImGuiTreeNodeFlags_DefaultOpen))
	{
		bChange |= meshRender->GetMaterial()->Property();
	}
	ImGui::Separator();
	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (instance < 0)
		{
			if (GetTransform()->Property())
			{
				bChange = true;
				baseTransform->Local(GetTransform()->Local());
			}
			if (ImGui::Button("Reset"))
			{
				baseTransform->Local(baseInitTransform);
				GetTransform()->Local(baseTransform);
			}
		}
		else
		{
			if (GetTransform(instance)->Property())
			{
				chageTrans[instance] = true;
				bChange = true;
			}
			if (ImGui::Button("Reset"))
			{
				GetTransform(instance)->Local(baseInitTransform);
				chageTrans[instance] = false;
			}
		}
	}

	bChange |= Super::Property(instance);
	return bChange;
}

void FigureMeshComponent::Tech(const UINT & mesh, const UINT & model, const UINT & anim)
{
	meshRender->Tech(mesh);
	Super::Tech(mesh, model, anim);	
}

void FigureMeshComponent::Pass(const UINT & mesh, const UINT & model, const UINT & anim)
{
	meshRender->Pass(mesh);
	Super::Pass(mesh, model, anim);
}

void FigureMeshComponent::SetShader(Shader * shader)
{
	Super::SetShader(shader);
	meshRender->GetMesh()->SetShader(shader);
}

void FigureMeshComponent::AddInstanceData()
{
	int index = meshRender->GetInstSize();

	meshRender->AddInstance();
	meshRender->GetTransform(index)->Local(baseInitTransform);
	Super::AddInstanceData();
	meshRender->UpdateTransforms();
}

void FigureMeshComponent::DelInstanceData(const UINT & instance)
{
	if (instance >= meshRender->GetInstSize())
		return;
	meshRender->DelInstance(instance);
	Super::DelInstanceData(instance);
}

const UINT & FigureMeshComponent::GetInstSize()
{
	return meshRender->GetInstSize();
}

Transform * FigureMeshComponent::GetTransform(const UINT & instance)
{
	if (instance >= meshRender->GetInstSize())
		return NULL;
	return meshRender->GetTransform(instance);
}
