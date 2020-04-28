#include "Framework.h"
#include "ActorEditor.h"
#include "Environment/Sky.h"
#include "Viewer/Orbit.h"
#include "./ImGui_New/imgui.h"
#include "Utilities/ImGuizmo.h"

ActorEditor::ActorEditor(Actor* actor)
	:actor(actor)
{
	Initialize();
}


ActorEditor::~ActorEditor()
{
	Destroy();
}

void ActorEditor::Initialize()
{
	// 에디터 모드 활성화
	bEdit = true;
	// 0번 인스턴스만 랜더될거임.
	actor->SetEditMode(true);
	
	curModel = actor->GetRootMeshData()->GetMesh();
	curAnimator = actor->GetRootMeshData()->GetAnimation();
	bAnimate = curAnimator ? true : false;
	if (bAnimate)
	{
		clips = curAnimator->Clips();
	}
	sky = new Sky(L"Environment/GrassCube1024.dds");	

	float width = D3D::Width();
	float height = D3D::Height();
	renderTarget = new RenderTarget((UINT)width, (UINT)height);
	depthStencil = new DepthStencil(width, height);

	orbitCam = new Orbit(80.0f, 50.0f, 150.0f);
}

void ActorEditor::Destroy()
{
	SafeDelete(sky);

	SafeDelete(shader);
}

void ActorEditor::Update()
{
	if (false == bEdit)		return;
	//액터의 에딧모드 활성화
	actor->SetEditMode(true);
	actor->Update();
}

void ActorEditor::PreRender()
{
	if (false == bEdit)		return;

	Context::Get()->SetSubCamera(orbitCam);
	//Vector3 campos, camrot;
	//Context::Get()->GetCamera()->RotationDegree(&camrot);
	//Context::Get()->GetCamera()->Position(&campos);

	actor->GetTransform(0)->Position(&originActorPosition);
	orbitCam->SetObjPos(originActorPosition);
	orbitCam->Update();
	// 버튼 이미지를 위한 카메라 고정
	//Context::Get()->GetCamera()->RotationDegree(11, 0, 0);
	// 액터 인스턴스를 기준으로 화면 돌리기.
	//Context::Get()->GetCamera()->Position(originActorPosition.x, originActorPosition.y + 10.0f , originActorPosition.z - 50.0f);
	sky->Update();

	renderTarget->Set(depthStencil->DSV());
	//renderTarget->Set(NULL);
	{
		sky->Render();

		Animate();
		actor->Tech(0, 0, 0);
		actor->Pass(0, 1, 2);
		actor->Render();

	}
	editSrv = renderTarget->SRV();

	Context::Get()->SetMainCamera();
	//Context::Get()->GetCamera()->RotationDegree(camrot);
	//Context::Get()->GetCamera()->Position(campos);

	//프리 랜더가 끝나면 액터의 에딧모드 종료.
	actor->SetEditMode(false);
}

void ActorEditor::Render()
{
	if (false == bEdit)		return;
	

	ImguiWindow_Begin();
	{
		ImVec2 size = ImGui::GetWindowSize();
		ImGui::BeginGroup();
		{
			if (ImGui::Button("Compile"))
			{
				actor->ActorCompile();
			}
			imguiWinChild_CompHeirarchy(size);
			//ImGui::Separator();
			ImguiWinChild_AnimCon(size);
		}
		ImGui::EndGroup();
		ImGui::SameLine();
		ImGui::Image
		(
			editSrv? editSrv : nullptr, ImVec2(size.x*0.4f, size.y)
		);
		ImGui::SameLine();

		if (NULL != selecedComp)
			RenderGizmo(selecedComp->GetTransform(0));

		ImGui::BeginGroup();
		{
			imguiWinChild_CompProperty(size);
		}
		ImGui::EndGroup();
	}
	ImguiWindow_End();
}

void ActorEditor::PostRender()
{
}

void ActorEditor::ImguiWindow_Begin()
{
	ImGui::SetWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSizeConstraints
	(
		ImVec2(800, 600),
		ImVec2(1280, 720)
	);

	ImGuiWindowFlags windowFlags =
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove;
	ImGui::Begin("Editor", &bEdit, windowFlags);

}

void ActorEditor::ImguiWindow_End()
{
	ImGui::End();
}

void ActorEditor::imguiWinChild_CompHeirarchy(const ImVec2 & size)
{
	ImGui::BeginChild("Component_Heirarchy", ImVec2(size.x*0.3f, size.y*0.35f), true);
	{
		actor->ShowCompHeirarchy(&selecedComp);
	}
	ImGui::EndChild();
}

void ActorEditor::imguiWinChild_CompProperty(const ImVec2 & size)
{
	ImGui::BeginChild("Component_Property", ImVec2(size.x*0.3f, size.y), true);
	{
		if (selecedComp != NULL)
			selecedComp->Property();
	}
	ImGui::EndChild();
}

void ActorEditor::ImguiWinChild_AnimCon(const ImVec2 & size)
{	
	ImGui::BeginChild("Animation",ImVec2(size.x*0.3f, size.y*0.35f),true);
	{
		if (curAnimator->ClipCount() > 0)
		{
			if (true == ViewAnims())
			{
				curAnimator->SetFrame(0, 0);
				curAnimator->PlayClip(0, selectedClip, takeTime);
			}

			if (ImGui::SliderFloat("TakeTime", &takeTime, 0.1f, 2.0f))
				curAnimator->PlayClip(0, selectedClip, takeTime);

			curAnimator->Update();

			int frameCount = curAnimator->GetFrameCount(0) - 1;
			ImGui::SliderInt("Frames", &selectedFrame, 0, frameCount);
			selectedFrame = Math::Clamp(selectedFrame, 0, frameCount);
		}
		PlayButton();
	}
	ImGui::EndChild();
}

void ActorEditor::PlayButton()
{
	/* Play */
	bool playbutton = (state == AnimationState::Play);

	if (!playbutton)
	{
		curAnimator->SetFrame(0, selectedFrame);
		if (ImGui::Button("Play"))
		{
			state = AnimationState::Play;
		}
	}
	else if (ImGui::Button("Pause"))
	{
		state = AnimationState::Pause;
	}

	ImGui::SameLine();
	if (ImGui::Button("Stop"))
	{
		state = AnimationState::Stop;
	}
}

bool ActorEditor::ViewAnims()
{
	bool bChange = false;
	if (ImGui::CollapsingHeader("ClipList"))
	{
		ImVec2 listBoxSize = ImVec2(ImGui::GetWindowContentRegionMax().x * 10.0f, 30.0f *clips.size());
		ImGui::ListBoxHeader("##Clip_List", listBoxSize);
		{
			int preSel = selectedClip;
			for (int i = 0; i < clips.size(); i++)
			{
				string name = String::ToString(clips[i]->Name());
				bool bSelected = (selectedClip == i);
				if (ImGui::Selectable(name.c_str(), bSelected,ImGuiSelectableFlags_AllowDoubleClick))
				{
					selectedClip = i;
				}
			}
			//일치하면 false
			bChange = (preSel != selectedClip);
			ImGui::ListBoxFooter();
		}
	}
	return bChange;
}

void ActorEditor::Animate()
{
	if (false == bAnimate)
		return;

	switch (state)
	{
	case AnimationState::Stop:
		curAnimator->SetFrame(0, 0);
		selectedFrame = 0;
		bPlay = false;
		break;
	case AnimationState::Play:
		selectedFrame = curAnimator->GetCurrFrame(0);
		bPlay = true;
		break;
	case AnimationState::Pause:
		bPlay = false;
		break;
	default:
		break;
	}
	if (bPlay)
		curAnimator->PlayAnim();
}

void ActorEditor::RenderGizmo(Transform * sTransform)
{
	static ImGuizmo::OPERATION operation = ImGuizmo::TRANSLATE;
	static ImGuizmo::MODE mode = ImGuizmo::WORLD;

	if (ImGui::IsKeyPressed(87)) // w
		operation = ImGuizmo::TRANSLATE;
	if (ImGui::IsKeyPressed(69)) // e
		operation = ImGuizmo::ROTATE;
	if (ImGui::IsKeyPressed(82)) // r
		operation = ImGuizmo::SCALE;

	Matrix matrix = sTransform->World();
	

	float width = D3D::Get()->Width();
	float height = D3D::Get()->Height();
	//Matrix view = Context::Get()->View();
	Matrix view;
	orbitCam->GetMatrix(&view);
	Matrix proj = Context::Get()->Projection();


	ImGuizmo::SetOrthographic(true);
	ImGuizmo::SetDrawlist();
	//offset,windowsize 순
	ImGuizmo::SetRect(0, 0, width, height);
	ImGuizmo::Manipulate
	(
		view,
		proj,
		operation,
		mode,
		matrix
	);

	sTransform->World(matrix);
}
