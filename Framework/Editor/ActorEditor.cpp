#include "Framework.h"
#include "ActorEditor.h"
#include "Environment/Sky.h"
#include "Viewer/Orbit.h"
#include "./ImGui_New/imgui.h"
#include "Utilities/ImGuizmo.h"

ActorEditor::ActorEditor()	
{
	//생성자에서는 환경 설정만
	Env_Init();	
}


ActorEditor::~ActorEditor()
{
	Destroy();
}

// 환경설정?
void ActorEditor::Env_Init()
{
	// 에딧용에서는 그냥 포워드 랜더로
	shader = SETSHADER(L"027_Animation.fx");
	sky = new Sky(L"Environment/GrassCube1024.dds");
	shader->AsSRV("SkyCubeMap")->SetResource(sky->CubeSRV());
	Texture* brdfLut = new Texture(L"MaterialPBR/ibl_brdf_lut.png");
	shader->AsSRV("BRDFLUT")->SetResource(brdfLut->SRV());
	//Create Material
	{
		floor = new Material(shader);
		floor->LoadDiffuseMap("Floor.png");
		floor->LoadNormalMap("Floor_Normal.png");
		floor->LoadSpecularMap("Floor_Specular.png");
		floor->LoadHeightMap("Floor_Displacement.png");
	}

	//Create Mesh
	{
		Transform* transform = NULL;

		grid = new MeshRender(shader, new MeshGrid(5, 5));
		grid->SetMaterial(floor);
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

	orbitCam = new Orbit(50.0f, 30.0f, 80.0f);	
}

void ActorEditor::Initialize()
{
	// 에디터 모드 활성화
	bEdit = true;
	bPlay = false;

	mouseVal = Vector3(2.5f, 1.0f, 0);
	TargetPos = Vector3(0, 5.0f, 0);
	
	selectedFrame = 0;
	selectedClip = 0;
	selecedComp = NULL;
	takeTime = 1.0f;
	
	curModel = e_Actor->GetRootMeshData()->GetMesh();
	curAnimator = e_Actor->GetRootMeshData()->GetAnimation();
	bAnimate = curAnimator ? true : false;
	if (bAnimate)
	{
		clips = curAnimator->Clips();
		curAnimator->SetAnimState();
	}
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
		sky->Update();
		grid->Update();
		sky->Render();

		grid->Tech(1);
		floor->Render();
		grid->Render();

		e_Actor->Tech(1, 1, 1);
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
			imguiWinChild_CompHeirarchy(ImVec2(size.x*0.2f, size.y*0.35f));
			//ImGui::Separator();
			ImGui::BeginChild("Cam_Property", ImVec2(size.x*0.2f, size.y*0.3f), true);
			ImGui::Text("mouse <%.2f,%.2f,%.2f>", mouseVal.x, mouseVal.y, mouseVal.z);
			orbitCam->Property();
			ImGui::EndChild();

		}
		ImGui::EndGroup();
		ImGui::SameLine();

		ImGui::BeginChild("EditorRender", ImVec2(size.x*0.5f, size.y), true, ImGuiWindowFlags_AlwaysHorizontalScrollbar);
		{

			ImGui::Image
			(
				editSrv ? editSrv : nullptr, ImVec2(size.x*0.5f, size.y*0.8f)
			);
			ImgOffset = ImGui::GetItemRectMin();
			ImgSize = ImGui::GetItemRectSize();

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
				mouseVal.y -= mousXY.y * 0.1f*Time::Delta();
			}

			orbitCam->CameraMove(mouseVal);
		}
			if (NULL != selecedComp)
				RenderGizmo(selecedComp->GetTransform(0));

			ImguiWinChild_AnimCon(ImVec2(size.x*0.5f, size.y*0.2f));
			ImGui::EndChild();
		}


		//랜더될 RTV 위치및 크기

		ImGui::SameLine();
		ImGui::BeginGroup();
		{
			imguiWinChild_CompProperty(ImVec2(size.x*0.3f, size.y));
		}
		ImGui::EndGroup();
	}
	ImguiWindow_End();
}

void ActorEditor::PostRender()
{
}

void ActorEditor::SetActor(Actor * actor)
{
	actor->ActorSyncBaseTransform();
	this->actor = actor;
	e_Actor = new Actor(*actor);
	e_Actor->EditModeOn();
	e_Actor->SetShader(shader);
	e_Actor->GetTransform()->Position(Vector3(0, 0, 0));
	e_Actor->GetTransform()->Rotation(Vector3(0, 0, 0));
	Initialize();
}

void ActorEditor::ImguiWindow_Begin()
{
	ImGui::SetWindowSize(ImVec2(800, 640), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSizeConstraints
	(
		ImVec2(800, 640),
		ImVec2(1280, 768)
	);

	ImGuiWindowFlags windowFlags =
		//ImGuiWindowFlags_NoResize
		ImGuiWindowFlags_NoMove	|
		ImGuiWindowFlags_AlwaysVerticalScrollbar |
		ImGuiWindowFlags_AlwaysHorizontalScrollbar;
	ImGui::Begin("Editor", &bEdit, windowFlags);

}

void ActorEditor::ImguiWindow_End()
{
	ImGui::End();
}

void ActorEditor::imguiWinChild_CompHeirarchy(const ImVec2 & size)
{
	ImGui::BeginChild("Component_Heirarchy", size, true);
	{
		e_Actor->ShowCompHeirarchy(&selecedComp);
	}
	ImGui::EndChild();
}

void ActorEditor::imguiWinChild_CompProperty(const ImVec2 & size)
{
	ImGui::BeginChild("Component_Property", size, true);
	{
		if (selecedComp != NULL)
			selecedComp->Property();
	}
	ImGui::EndChild();
}

void ActorEditor::ImguiWinChild_AnimCon(const ImVec2 & size)
{	
	//TODO: 현재 선택한 comp가 animator를 가지고 있으면 클립선택하게. 0902
	ImGui::BeginChild("Animation", size,true);
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
	// 루트는 남으므로 굳이 삭제 필요 없음
	actor->ActorCompile(*e_Actor);
	actor->SetShader(actor->GetShader());
	actor->Update();
	actor->ToReMakeIcon(true);
	//actor->ActorSyncBaseTransform();
}

void ActorEditor::EditActorReset()
{
	//기존 에딧액터를 삭제후 다시 만들기
	e_Actor->ActorCompile(*actor);
	e_Actor->SetShader(shader);

	curModel = e_Actor->GetRootMeshData()->GetMesh();
	curAnimator = e_Actor->GetRootMeshData()->GetAnimation();
	bAnimate = curAnimator ? true : false;
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
