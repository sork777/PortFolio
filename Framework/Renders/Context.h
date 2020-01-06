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
	/* �ӽ�? */
	class Camera* subCamera;
public:
	/* ī�޶�� �޾Ƽ� �����ϸ� ������ ���� ȣ�� */
	Orbit* GetOrbitCam() { return orbitCam; }
	Freedom* GetFreeCam() { return freeCam; }
};