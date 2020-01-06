#pragma once

#define MAX_POINT_LIGHT 32
#define MAX_SPOT_LIGHT 32
#define MAX_CAPSULE_LIGHT 32
struct PointLight
{
	Color Ambient;
	Color Diffuse;
	Color Specular;

	Vector3 Position;
	float Range;

	float Intensity;
	float Padding[3];
};

struct SpotLight
{
	Color Ambient;
	Color Diffuse;
	Color Specular;

	Vector3 Position;
	float Range;

	Vector3 Direction;
	float Angle;

	float Intensity;
	float Padding[3];
};

struct CapsuleLight
{
	Color Ambient;
	Color Diffuse;
	Color Specular;

	Vector3 Position;
	float Range;

	Vector3 Direction;
	float Length;

	float Intensity;
	float Padding[3];
};

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

	class Perspective* GetPerspective() { return perspective; }
	class Viewport* GetViewport() { return viewport; }
	//미리 컴파일된 헤더의순서 상관 안하려고 전방선언
	class Camera* GetCamera() { return camera; }

	//외부에서 수정하는 경우도 많아서...
	Color& LightAmbient() { return lightAmbient; }
	Color& LightSpecular() { return lightSpecular; }
	Vector3& LightDirection() { return lightDirection; }
	Vector3& LightPosition() { return lightPosition; }

	UINT PointLights(OUT PointLight* lights);
	UINT PointLightsCount();
	void AddPointLight(PointLight& light);
	PointLight& GetPointLight(UINT index);

	UINT SpotLights(OUT SpotLight* lights);
	UINT SpotLightsCount();
	void AddSpotLight(SpotLight& light);
	SpotLight& GetSpotLight(UINT index);

	UINT CapsuleLights(OUT CapsuleLight* lights);
	UINT CapsuleLightsCount();
	void AddCapsuleLight(CapsuleLight& light);
	CapsuleLight& GetCapsuleLight(UINT index);
	
	/* NULL이면 메인카메라 존재? */
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


	UINT pointLightCount;
	PointLight pointLights[MAX_POINT_LIGHT];

	UINT spotLightCount;
	SpotLight spotLights[MAX_SPOT_LIGHT];

	UINT capsuleLightCount;
	CapsuleLight capsuleLights[MAX_CAPSULE_LIGHT];
private:
	class Camera* camera;
	class Freedom* freeCam;
	class Orbit* orbitCam;
	/* 임시? */
	class Camera* subCamera;
public:
	/* 카메라로 받아서 세팅하면 꼬여서 따로 호출 */
	Orbit* GetOrbitCam() { return orbitCam; }
	Freedom* GetFreeCam() { return freeCam; }
};