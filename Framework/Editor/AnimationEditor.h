#pragma once
#include "IEditor.h"

/*
에디터 외부에 있는 액터들 목록 중에서
액터(모델)을 하나 받음
걔에 대한 
본 정보,
어태치 모델,
마테리얼 정보,
트랜스폼 정보,
애니메이션 정보,
등만을 가짐

따로 임포트나 파일시스템 관련은 X
TODO: 저장 정보는?
액터의 정보가 변경되는거니까 액터를 저장하면 될까?
*/

class AnimationEditor :public IEditor
{
public:
	AnimationEditor();
	~AnimationEditor();

	// IEditor을(를) 통해 상속됨
	virtual void Initialize() override;
	virtual void Destroy() override;

	virtual void Update() override;
	virtual void PreRender() override;
	virtual void Render() override;
	virtual void PostRender() override;

private:
	void ImGUIController();
	void AnimationController();

private:
	void SelectedPartsViewer();

	void PartsViewer();
	void ChildViewer(ModelBone * bone);
private:
	void Popup();
	void AddSocket();

	void AddAnimation();
	void ModelAttach();

private:
	int selectedFrame = 0;
	int selected = 0;
	bool bPlay = false;

	Transform* selectedTransform = NULL;
	UINT clip = 0;
	UINT instance = 0;
	float takeTime = 1.0f;

private:
	ModelAnimator* mainModel;

};

