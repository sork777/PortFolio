#pragma once

/*
	카메라 가지고 있기.
	콘텐스트의 서브카메라 조절?
	어차피 플레이어면 자기것만 들고있을거라 
	인스턴싱 데이터 필요 없음.
*/

class CameraComponent :public ObjectBaseComponent
{
public:
	CameraComponent();
	~CameraComponent();

	// ObjectBaseComponent을(를) 통해 상속됨
	virtual void Update() override;
	virtual void Render() override;
	virtual bool Property() override;
	virtual Transform * GetTransform(const UINT & instance = 0) override;

private:
	Camera* camera;
};

