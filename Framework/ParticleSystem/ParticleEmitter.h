#pragma once

#define MAX_PARTICLE_INSTANCE 4096

// 방출기 형태
enum class EmitterType
{
	Point = 0,
	Sphere,
	Cube,
};
// 방출 방식
enum class EmittType
{
	Direct = 0,	// 주어진 방향으로 가는 형식
	Around,		// 중심에서의 방향에 따라 주변으로 퍼지는 방식
};
// 방출 방식
enum class ParticleType
{
	Texture = 0,	
	Quad,
	Diamond,		
};

class ParticleEmitter
{
public:
	ParticleEmitter();
	~ParticleEmitter();

	void Update();
	void Render();

	void Property();

private:
	void Initialize();

	void PlayParticles();
	void UpdateParticles();
	void EmittParticles();

private:
	void EmittPoint();
	void EmittSphere();
	void EmittCube();

private:
	void ChangeTex(wstring filePath =L"");

public:
	void LoadParticle(wstring path = L"");
	void SaveParticle(wstring path = L"");

private:
	struct ParticleVertex
	{
		Vector3 Position;
		Vector2 Scale;
	};
	//Instancing용 데이터
	struct ParticleDesc
	{
		Matrix world;
		Color color;
	};

private:
	Shader*		shader;
	UINT		tech=0;
	UINT		pass=0;
	Transform*	transform;
	PerFrame*	perFrame;

	bool		bPlay = true;
	bool		bPropertyOpen = true;
private:
	ParticleVertex		point;
	VertexBuffer*		vertexBuffer;

	UINT				particleCount	= 0;
	Texture*			particleTex		= NULL;
	ParticleDesc		instDesc[MAX_PARTICLE_INSTANCE];
	VertexBuffer*		instanceBuffer;	//인스턴싱용 버퍼

	ID3DX11EffectConstantBuffer*			sInstanceBuffer;
	ID3DX11EffectShaderResourceVariable*	sParticleTex;

/////////////////////////////////////////////////////
//// 
/////////////////////////////////////////////////////
private:
	EmittType		emittype	= EmittType::Direct;
	EmitterType		emitter		= EmitterType::Point;
	ParticleType	particle	= ParticleType::Texture;

	string	current_emit		= "Direct";
	string	current_emitter		= "Point";
	string	current_particle	= "Texture";

private:
	// 초기 값
	Vector3	InitScale	= Vector3(3, 3, 1);		//얘만 따로 써서 공통으로 크기 조절
	Vector3	InitRot		= Vector3(0, 0, 0);		//파티클 초기 회전
	Vector3	Torque		= Vector3(0, 0, 0);		//파티클 회전 값
	Vector3	Force		= Vector3(1, 1, 0);

	//랜덤 변동치 
	float	randPosVar		= 1.0f;
	float	randRotVar		= 0.0f;
	float	randLifeVar		= 4.0f;
	float	randForceVar	= 0.0f;
	float	randTorqueVar	= 0.0f;

	int		emittPerSec		= 100;		//초당 방출량
	float	radiusStep		= 0.1f;		//방출을 위한 간격
	float	cubeStep		= 0.5f;		//방출을 위한 간격

	int		cubeSize[3]		= { 1,1,1 };
	int		sphereDivide	= 10;		//구의 각도 구분치
private:
	int		cubeCount		= 0;		//방출 횟수 
	int		roundCount		= 0;		//방출 횟수 

	float	elapsed			= 0.0f;		//경과 시간
/////////////////////////////////////////////////////
////	ComputeShader
/////////////////////////////////////////////////////

private:
	void CreateComputeShader();

private:
	struct ParticleInfoDesc
	{
		Color StartColor	= Color(1, 1, 1, 1);
		Color EndColor		= Color(1, 1, 1, 1);

		float Time			= 10.0f;	// 기본 생존시간
		float DeltaTime		= 0.0f;		// Delta
		float Velocity		= 1.0f;
		float Accelation	= 0.0f;	
		
		float RotVel		= 1.0f;
		float RotAccel		= 0.0f;	

		Vector2 Padding;
	}csInfoDesc;

	struct CS_InputDesc
	{
		Vector3		Postion;		
		Vector3		Rotation;

		Vector3		Direction;
		Vector3		RotDir;

		float		LifeTime	= 0.0f;	// 생존기간
		float		Time		= 0.0f;	// 생존시간
	};
	struct CS_OutputDesc
	{
		Vector3		Postion		= Vector3(0, 0, 0);
		Vector3		Rotation	= Vector3(0, 0, 0);

		float		LifeTime	= 0.0f;	// 남은 생존시간
		float		Time		= 0.0f;	// 생존시간
		Color		ResultColor = Color(0, 0, 0, 0);	//start와 end가 life/time에 lerp
	};

	CS_InputDesc*	csInput		= NULL;
	CS_OutputDesc*	csOutput	= NULL;

private:
	UINT		csTech = 0;
	UINT		csPass = 0;
	Shader*				computeShader;
	StructuredBuffer*	computeBuffer;
	ConstantBuffer*		csConstBuffer;	

	ID3DX11EffectUnorderedAccessViewVariable*	sUav;

};

