#include "Framework.h"
#include "Objects/Actor/Actor.h"
#include "ActorEditor.h"
#include "Environment/Sky.h"


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
	//에디터 모드 활성화
	actor->SetEditMode(true);
	
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
}

void ActorEditor::Destroy()
{
	SafeDelete(sky);

	SafeDelete(shader);
	boneTrans.clear();
	boneTrans.shrink_to_fit();
}

void ActorEditor::Update()
{
	actor->Update();
}

void ActorEditor::PreRender()
{
	Vector3 campos, camrot;
	Context::Get()->GetCamera()->RotationDegree(&camrot);
	Context::Get()->GetCamera()->Position(&campos);

	// 버튼 이미지를 위한 카메라 고정
	Context::Get()->GetCamera()->RotationDegree(0, 0, 0);
	Context::Get()->GetCamera()->Position(0, 45, -100);

	renderTarget->Set(depthStencil->DSV());
	{
		sky->Render();

		Animate();
		actor->Tech(0, 0, 0);
		actor->Pass(0, 1, 2);
		actor->Render();
	}
	editSrv = renderTarget->SRV();

	Context::Get()->GetCamera()->RotationDegree(camrot);
	Context::Get()->GetCamera()->Position(campos);

}

void ActorEditor::Render()
{
	if (false == bEdit) return;

	ImguiWindow_Begin();
	{
		ImVec2 size = ImGui::GetWindowSize();

	}
	ImguiWindow_End();
}

void ActorEditor::PostRender()
{
}

void ActorEditor::ImguiWindow_Begin()
{
	actor->GetTransform(0)->Position(&originActorPosition);
	actor->GetTransform(0)->Position(Vector3(0, 0, 0));

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
	actor->GetTransform(0)->Position(originActorPosition);
}

void ActorEditor::imguiWinChild_CompHeirarchy(const ImVec2 & size)
{
}

void ActorEditor::imguiWinChild_CompProperty(const ImVec2 & size)
{
}

void ActorEditor::ImguiWinChild_BoneHeirarchy(const ImVec2 & size)
{
}

void ActorEditor::ImguiWinChild_AnimCon()
{	
	ImGui::BeginChild("Animation",ImVec2(0,0),true);
	{
		if (curAnimator->ClipCount() > 0)
		{
			bool bChange = false;
			bChange |= ViewAnims();
			bChange |= ImGui::SliderFloat("TakeTime", &takeTime, 0.1f,2.0f);

			if (bChange)
				curAnimator->PlayClip(0, selectedClip, takeTime);

			curAnimator->Update();

			int frameCount = curAnimator->GetFrameCount(0) - 1;
			ImGui::SliderInt("Frames", &selectedFrame, 0, frameCount);
			selectedFrame = Math::Clamp(selectedFrame, 0, frameCount);
		}
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
			curAnimator->SetFrame(0, 0);
			selectedFrame = 0;
			state = AnimationState::Stop;
		}
		ImGui::SameLine();
	}
	ImGui::EndChild();
}

bool ActorEditor::ViewAnims()
{
	bool bChange = false;
	if (ImGui::CollapsingHeader("ClipList"))
	{
		ImVec2 listBoxSize = ImVec2(ImGui::GetWindowContentRegionMax().x * 10.0f, 30.0f *clips.size());
		ImGui::ListBoxHeader("##ComponentList", listBoxSize);
		{
			int preSel = selectedClip;
			for (int i = 0; i < clips.size(); i++)
			{
				string name = String::ToString(clips[i]->Name());
				bool bSelected = (selectedClip == i);
				if (ImGui::Selectable(name.c_str(), bSelected))
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
