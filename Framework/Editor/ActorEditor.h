#pragma once
#include "IEditor.h"

/*
	액터의 컴포넌트 구성
	에디터용 랜더
	애니메이션 플레이 확인
	본의 수정 같은 애니메이션 에딧 관련은 따로둠
*/
class Actor;


class ActorEditor :public IEditor
{
public:
	ActorEditor(Actor* actor);
	~ActorEditor();

	// IEditor을(를) 통해 상속됨
	virtual void Initialize() override;
	virtual void Destroy() override;

	virtual void Update() override;
	virtual void PreRender() override;
	virtual void Render() override;
	virtual void PostRender() override;

	void EditModeOn() { bEdit = true; }
	const bool& IsEditMode() { return bEdit; }
private:
	void ImguiWindow_Begin();
	void ImguiWindow_End();

private:
	void imguiWinChild_CompHeirarchy(const ImVec2& size = ImVec2(0, 0));
	void imguiWinChild_CompProperty(const ImVec2& size = ImVec2(0, 0));
	void ImguiWinChild_AnimCon(const ImVec2& size = ImVec2(0, 0));

private:
	//void CompileButton();
	void PlayButton();
	bool ViewAnims();
	void Animate();	

	void RenderGizmo(Transform* sTransform);

private:
	enum class AnimationState : UINT
	{
		Stop = 0,
		Play,
		Pause
	} state = AnimationState::Stop;

private:	
	enum class GizmoType
	{
		None,
		Model,
		Component
	};

	GizmoType gizmoType = GizmoType::None;
private:
	int selectedFrame	= 0;
	//int selectedBone = 0;
	int selectedClip = 0;

	float takeTime	= 1.0f;
	bool bPlay		= false;
	bool bAnimate	= false;
	bool bEdit		= false;

private:
	Shader*	shader;	
	
	ModelAnimator*	curAnimator = NULL;
	Model* curModel	= NULL;
	Actor* actor	= NULL;

	ObjectBaseComponent* selecedComp = NULL;
	vector<ModelClip*>	clips;
	Vector3 originActorPosition;
private:
	// 환경
	class Sky*	sky;
	Material*	floor;
	MeshRender* grid;

	class Orbit* orbitCam;
	RenderTarget* renderTarget;
	DepthStencil* depthStencil;
	ID3D11ShaderResourceView* editSrv;
};
