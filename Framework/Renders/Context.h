#pragma once

//���α׷����� ����� ��ü ���� �־���� ��
class Context
{
public:
	static Context* Get();
	static void Create();
	static void Delete();

private:
	Context();
	~Context();

public:
	void Update();
	void Render();
	void ResizeScreen();

	Matrix View();
	Matrix Projection();

	class Perspective* GetPerspective() { return perspective; }
	class Viewport* GetViewport() { return viewport; }
	//�̸� �����ϵ� ����Ǽ��� ��� ���Ϸ��� ���漱��
	class Camera* GetCamera() { return camera; }

	//�ܺο��� �����ϴ� ��쵵 ���Ƽ�...
	Color& LightAmbient() { return lightAmbient; }
	Color& LightSpecular() { return lightSpecular; }
	Vector3& LightDirection() { return lightDirection; }
	Vector3& LightPosition() { return lightPosition; }

	/* NULL�̸� ����ī�޶� ����? */
	void SetMainCamera() { subCamera = NULL; }
	void SetSubCamera(Camera* camera) { subCamera = camera; }
private:
	static Context* instance;

private:
	class Perspective* perspective;
	class Viewport* viewport;

	Color lightAmbient;
	Color lightSpecular;
	Vector3 lightDirection;
	Vector3 lightPosition;

private:
	class Camera* camera;
	class Freedom* freeCam;
	class Orbit* orbitCam;
	/* �ӽ�? */
	class Camera* subCamera;
public:
	/* ī�޶�� �޾Ƽ� �����ϸ� ������ ���� ȣ�� */
	Orbit* GetOrbitCam() { return orbitCam; }
	Freedom* GetFreeCam() { return freeCam; }
};