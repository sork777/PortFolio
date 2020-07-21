#include "Framework.h"
#include "ActorEditor.h"
#include "Environment/Sky.h"
#include "Viewer/Orbit.h"
#include "./ImGui_New/imgui.h"
#include "Utilities/ImGuizmo.h"

ActorEditor::ActorEditor(Actor* actor)	
	:actor(actor)
{
	// 받아온 액터를 통해 에딧용 액터 복사 생성
	shader = actor->GetRootMeshData()->GetMesh()->GetShader();
	e_Actor = new Actor(*actor);	
	e_Actor->GetTransform()->Position(Vector3(0, 0, 0));
	e_Actor->GetTransform()->Rotation(Vector3(0, 0, 0));
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
	
	curModel = e_Actor->GetRootMeshData()->GetMesh();
	curAnimator = e_Actor->GetRootMeshData()->GetAnimation();
	bAnimate = curAnimator ? true : false;
	if (bAnimate)
	{
		clips = curAnimator->Clips();
		curAnimator->SetAnimState();
	}
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
		grid->AddInstance();
		transform = grid->GetTransform(0);
		transform->Position(0, 0, 0);
		transform->Scale(12, 1, 12);
	}
	grid->UpdateTransforms();

	float width = D3D::Width();
	float height = D3D::Height();
	renderTarget = new RenderTarget((UINT)width, (UINT)height);
	depthStencil = new DepthStencil(width, height);

	orbitCam = new Orbit(80.0f, 30.0f, 150.0f);
	orbitCam->SetObjPos(TargetPos);
	orbitCam->CameraMove(mouseVal);
	orbitCam->Update();
}

void ActorEditor::Destroy()
{
	SafeDelete(e_Actor);
	SafeDelete(orbitCam);
	SafeDelete(depthStencil);
	SafeDelete(renderTarget);
	SafeDelete(sky);
	SafeDelete(grid);
	SafeDelete(floor);
	
}

void ActorEditor::Update()
{
	if (false == bEdit)		return;
	//액터의 에딧모드 활성화
	//sky->Update();
	grid->Update();

	e_Actor->Update();
	
	orbitCam->Update();
}

void ActorEditor::PreRender()
{
	if (false == bEdit)		return;

	Context::Get()->SetSubCamera(orbitCam);

	// 오빗카메라 업데이트

	renderTarget->Set(depthStencil->DSV());
	//renderTarget->Set(NULL);
	{
		//sky->Render();

		floor->Render();
		grid->Render();

		e_Actor->Tech(0, 0, 0);
		e_Actor->Pass(0, 1, 2);
		e_Actor->Render();

	}
	editSrv = renderTarget->SRV();

	// 컨텍스트의 서브카메라 삭제
	Context::Get()->SetMainCamera();

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
				ActorCompile();
			}
			ImGui::SameLine();
			if (ImGui::Button("ResetActor"))
			{
				EditActorReset();
			}
			imguiWinChild_CompHeirarchy(size);
			//ImGui::Separator();
			ImguiWinChild_AnimCon(size);
			ImGui::BeginChild("Cam_Property", ImVec2(size.x*0.3f, size.y*0.3f), true);
			ImGui::Text("mouse <%.2f,%.2f,%.2f>", mouseVal.x, mouseVal.y, mouseVal.z);
			orbitCam->Property();
			ImGui::EndChild();

		}
		ImGui::EndGroup();
		ImGui::SameLine();
		ImGui::Image
		(
			editSrv? editSrv : nullptr, ImVec2(size.x*0.4f, size.y)
		);
		if (ImGui::IsMouseHoveringWindow())
		{
			ImVec2 mousXY = ImGui::GetMouseDragDelta(1);
			if (ImGui::IsKeyPressed(VK_SHIFT))
			{
				if (ImGui::IsKeyDown('E'))
					mouseVal.z += 50.0f*Time::Delta();
				else if (ImGui::IsKeyDown('Q'))
					mouseVal.z -= 50.0f*Time::Delta();

				if (ImGui::IsKeyDown('W'))
					TargetPos += 50.0f*Time::Delta()*orbitCam->Up();
				else if (ImGui::IsKeyDown('S'))
					TargetPos -= 50.0f*Time::Delta()*orbitCam->Up();
				
				if (ImGui::IsKeyDown('A'))
					TargetPos -= 50.0f*Time::Delta()*orbitCam->Right();
				else if (ImGui::IsKeyDown('D'))
					TargetPos += 50.0f*Time::Delta()*orbitCam->Right();
				orbitCam->SetObjPos(TargetPos);
			}
			if (ImGui::IsMouseDown(1) == true)
			{
				mouseVal.x += mousXY.x * 0.1f*Time::Delta();
				mouseVal.y += mousXY.y * 0.1f*Time::Delta();
			}

			orbitCam->CameraMove(mouseVal);
		}


		//랜더될 RTV 위치및 크기
		ImgOffset = ImGui::GetItemRectMin();
		ImgSize = ImGui::GetItemRectSize();
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
		ImGuiWindowFlags_NoResize
		|ImGuiWindowFlags_NoMove;
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
		e_Actor->ShowCompHeirarchy(&selecedComp);
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

			selectedFrame = curAnimator->GetCurrFrame(0);
			int frameCount = curAnimator->GetFrameCount(0) - 1;
			ImGui::SliderInt("Frames", &selectedFrame, 0, frameCount);
			selectedFrame = Math::Clamp(selectedFrame, 0, frameCount);
		}
		PlayButton();
	}
	ImGui::EndChild();
}

void ActorEditor::ActorCompile()
{
	//원본 액터를 에딧용 액터를 통해 재컴파일
	actor->ActorCompile(*e_Actor);
}

void ActorEditor::EditActorReset()
{
	//기존 에딧액터를 삭제후 다시 만들기
	SafeDelete(e_Actor);
	e_Actor = new Actor(*actor);

	curModel = e_Actor->GetRootMeshData()->GetMesh();
	curAnimator = e_Actor->GetRootMeshData()->GetAnimation();
	if (bAnimate)
	{
		clips = curAnimator->Clips();
		curAnimator->SetAnimState();
	}
	selecedComp = NULL;
}

void ActorEditor::PlayButton()
{
	/* Play */
	bool playbutton = (state == AnimationState::Play);
	bool bButtonClicked = false;
	if (!playbutton)
	{
		//curAnimator->SetFrame(0, selectedFrame);
		if (ImGui::Button("Play"))
		{
			state = AnimationState::Play;
			bButtonClicked = true;
		}
	}
	else if (ImGui::Button("Pause"))
	{
		state = AnimationState::Pause;
			bButtonClicked = true;
	}

	ImGui::SameLine();
	if (ImGui::Button("Stop"))
	{
		state = AnimationState::Stop;
		bButtonClicked = true;
		selectedFrame = 0;

	}
	if (true == bButtonClicked)
		curAnimator->SetAnimState(state, 0);
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
				string name = String::ToString(clips[i]->FileName());
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
	

	//float width = D3D::Get()->Width();
	//float height = D3D::Get()->Height();
	Matrix view;
	orbitCam->GetMatrix(&view);
	Matrix proj = Context::Get()->Projection();


	ImGuizmo::SetOrthographic(true);
	ImGuizmo::SetDrawlist();
	//offset,windowsize 순
	ImGuizmo::SetRect(ImgOffset.x, ImgOffset.y, ImgSize.x, ImgSize.y);
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
