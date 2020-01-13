#pragma once
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

class AnimationEditor
{
public:
	AnimationEditor();
	~AnimationEditor();

	void Destroy();
	void Update();
	void PreRender();
	void Render();
	void PostRender();

private:
	void ImGUIController();
	void AnimationController();

private:
	void SelectedPartsViewer();
	void ModelsViewer();

	void PartsViewer();
	void ChildViewer(ModelBone * bone);
private:
	void Popup();
	void AddSocket();

	void AddAnimation();
	void ModelAttach();
private:
	int selectedFrame = 0;
	int selectedModel = 0;
	int selected = 0;
	bool bPlay = false;

	Transform* selectedTransform = NULL;
	UINT clip = 0;
	UINT instance = 0;
	float takeTime = 1.0f;
};

