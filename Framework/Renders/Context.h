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

	
public:
	inline class Perspective* GetPerspective() { return perspective; }
	inline class Viewport* GetViewport() { return viewport; }
	//�̸� �����ϵ� ����Ǽ��� ��� ���Ϸ��� ���漱��
	inline class Camera* GetCamera() { return camera; }
	inline class Camera* GetSubCamera() { return subCamera; }

	//�ܺο��� �����ϴ� ��쵵 ���Ƽ�...
	Color& LightAmbient() { return lightAmbient; }
	Color& LightSpecular() { return lightSpecular; }
	Vector3& LightDirection() { return lightDirection; }
	Vector3& LightPosition() { return lightPosition; }

	/* NULL�̸� ����ī�޶� ����? */
	inline void SetMainCamera() { subCamera = NULL; }
	inline void SetSubCamera(Camera* camera) { subCamera = camera; }
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
	inline Orbit* GetOrbitCam() { return orbitCam; }
	inline Freedom* GetFreeCam() { return freeCam; }

	void SetFree();
	void SetObit();
};