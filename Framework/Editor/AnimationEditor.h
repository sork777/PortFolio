#pragma once
#include "IEditor.h"

/*
������ �ܺο� �ִ� ���͵� ��� �߿���
����(��)�� �ϳ� ����
�¿� ���� 
�� ����,
����ġ ��,
���׸��� ����,
Ʈ������ ����,
�ִϸ��̼� ����,
��� ����

���� ����Ʈ�� ���Ͻý��� ������ X
TODO: ���� ������?
������ ������ ����Ǵ°Ŵϱ� ���͸� �����ϸ� �ɱ�?
*/

class AnimationEditor :public IEditor
{
public:
	AnimationEditor();
	~AnimationEditor();

	// IEditor��(��) ���� ��ӵ�
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

