#pragma once

//프로그램에서 사용할 전체 값을 넣어놓은 곳
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
	//미리 컴파일된 헤더의순서 상관 안하려고 전방선언
	inline class Camera* GetCamera() { return camera; }
	inline class Camera* GetSubCamera() { return subCamera; }

	//외부에서 수정하는 경우도 많아서...
	Color& LightAmbient() { return lightAmbient; }
	Color& LightSpecular() { return lightSpecular; }
	Vector3& LightDirection() { return lightDirection; }
	Vector3& LightPosition() { return lightPosition; }

	/* NULL이면 메인카메라 존재? */
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
	/* 임시? */
	class Camera* subCamera;
public:
	/* 카메라로 받아서 세팅하면 꼬여서 따로 호출 */
	inline Orbit* GetOrbitCam() { return orbitCam; }
	inline Freedom* GetFreeCam() { return freeCam; }

	void SetFree();
	void SetObit();
};