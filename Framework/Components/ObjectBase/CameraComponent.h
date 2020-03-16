#pragma once

/*
	ī�޶� ������ �ֱ�.
	���ٽ�Ʈ�� ����ī�޶� ����?
	������ �÷��̾�� �ڱ�͸� ��������Ŷ� 
	�ν��Ͻ� ������ �ʿ� ����.
*/

class CameraComponent :public ObjectBaseComponent
{
public:
	CameraComponent();
	~CameraComponent();

	// ObjectBaseComponent��(��) ���� ��ӵ�
	virtual void Update() override;
	virtual void Render() override;
	virtual bool Property(const UINT & instance) override;
	virtual Transform * GetTransform(const UINT & instance = 0) override;


	virtual void AddInstanceData() override {}
	virtual void DelInstanceData(const UINT& instance) override {}
private:
	Camera* camera;
};

