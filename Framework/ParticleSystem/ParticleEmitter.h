#pragma once

#define MAX_INSTANCE 512

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
	Cone		
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

	bool		bPropertyOpen = true;
private:
	ParticleVertex		point;
	VertexBuffer*		vertexBuffer;

	UINT				particleCount	= 0;
	Texture*			particleTex		= NULL;
	ParticleDesc		instDesc[MAX_INSTANCE];
	ConstantBuffer*		instanceBuffer;	//인스턴싱용 버퍼

	ID3DX11EffectConstantBuffer*			sInstanceBuffer;
	ID3DX11EffectShaderResourceVariable*	sParticleTex;

/////////////////////////////////////////////////////
//// 
/////////////////////////////////////////////////////
	EmittType		emittype	= EmittType::Direct;
	EmitterType		emitter		= EmitterType::Point;
	ParticleType	particle	= ParticleType::Texture;
	string	current_emitter		= "Point";
	string	current_emit		= "Direct";
	string	current_particle	= "Texture";

private:
	Vector3	particleScale	= Vector3(1, 1, 1);		//얘만 따로 써서 공통으로 크기 조절
	Vector3	particleRot		= Vector3(0, 0, 0);		//파티클 회전
	Vector3	particleRotDir	= Vector3(0, 0, 0);		//파티클 회전방향
	Vector3	ComDir			= Vector3(0, 0, 0);

	int		EmittPerSec		= 100;		//초당 방출량
	float	radiusStep		= 0.5f;		//방출을 위한 간격
	int		roundCount		= 0;

	float	randPosSpan		= 0.0f;
	float	randRotSpan		= 0.0f;
	float	randLifeSpan	= 0.0f;

	int		StackCount		= 10;		//구의 각도 구분치
	int		cubeCount		= 0;		
	float	CubeStep		= 0.5f;		//방출을 위한 간격
	int		CubeSize[3]		= { 1,1,1 };
	float	ConeAngle		= 10.0f;	//Degree

	float	elapsed			= 0.0f;
	//float	timeSinceUpdate	= 0.0f;

/////////////////////////////////////////////////////
////	ComputeShader
/////////////////////////////////////////////////////

private:
	void CreateComputeShader();

private:
	struct ParticleInfoDesc
	{
		Color StartColor	= Color(1, 1, 0, 1);
		Color EndColor		= Color(1, 0, 0, 1);

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

