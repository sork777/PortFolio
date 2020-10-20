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
	void Env_Init();
public:
	ActorEditor();
	~ActorEditor();

	// IEditor을(를) 통해 상속됨
	virtual void Initialize() override;
	virtual void Destroy() override;

	virtual void Update() override;
	virtual void PreRender() override;
	virtual void Render() override;
	virtual void PostRender() override;

public:
	const bool& IsOpenedEditor() { return bEdit; }
	void SetActor(Actor* actor);

private:
	void ImguiWindow_Begin();
	void ImguiWindow_End();

private:
	void imguiWinChild_CompHeirarchy(const ImVec2& size = ImVec2(0, 0));
	void imguiWinChild_CompProperty(const ImVec2& size = ImVec2(0, 0));
	void ImguiWinChild_AnimCon(const ImVec2& size = ImVec2(0, 0));

private:
	void ActorCompile();
	void EditActorReset();
	void PlayButton();
	bool ViewAnims();

	void RenderGizmo(Transform* sTransform);

private:
	AnimationState state;

private:	
	enum class GizmoType
	{
		None,
		Model,
		Component
	};

	GizmoType gizmoType = GizmoType::None;
private:
	Vector3 mouseVal = Vector3(2.5f, 1.0f, 0);
	Vector3 TargetPos = Vector3(0, 5.0f, 0);
	ImVec2 ImgOffset = ImVec2(0, 0);
	ImVec2 ImgSize = ImVec2(0, 0);
	
private:
	int selectedFrame	= 0;
	int selectedClip = 0;

	float takeTime	= 1.0f;
	bool bPlay		= false;
	bool bAnimate	= false;
	bool bEdit		= false;

private:
	Shader*	shader;	
	Shader*	oShader;	
	
	ModelAnimator*	curAnimator = NULL;
	Model* curModel	= NULL;
	Actor* e_Actor	= NULL;
	Actor* actor	= NULL;

	ObjectBaseComponent* selecedComp = NULL;
	vector<ModelClip*>	clips;

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
