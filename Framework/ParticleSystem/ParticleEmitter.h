#pragma once

#define MAX_INSTANCE 4096

class ParticleEmitter
{
public:
	ParticleEmitter();
	~ParticleEmitter();

	void Update();
	void Render();


	UINT& Pass() { return pass; }
	UINT& Tech() { return tech; }
	void Pass(UINT val) { pass = val; }
	void Tech(UINT val) { tech = val; }

private:
	void Initialize();

	void PlayParticle();
	void CreateComputeShader();

private:
	struct ParticleVertex
	{
		Vector3 Position;
		Vector2 Scale;
	};

	struct Particle
	{
		Vector3		Postion;
		Vector3		Rotation;

		Vector3		Direction;
		Vector3		RotDir;		
		float		LifeTime;
	};

private:
	//Instancing�� ������
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

private:
	vector<Particle> particles;

private:
	ParticleVertex		point;
	VertexBuffer*		vertexBuffer;

	UINT				particleCount;
	ParticleDesc		instDesc[MAX_INSTANCE];
	ConstantBuffer*		instanceBuffer;	//�ν��Ͻ̿� ����
	Texture*			particleTex;

	ID3DX11EffectConstantBuffer*			sInstanceBuffer;
	ID3DX11EffectShaderResourceVariable*	sParticleTex;


/////////////////////////////////////////////////////
////	ComputeShader
/////////////////////////////////////////////////////

private:
	struct ParticleInfoDesc
	{
		float Time			= 0.0f;		// �⺻ �����ð�
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

		float		LifeTime	= 0.0f;	// ���� �����ð�
		Color		StartColor;
		Color		EndColor;		
	};
	struct CS_OutputDesc
	{
		Vector3		Postion		= Vector3(0, 0, 0);
		Vector3		Rotation	= Vector3(0, 0, 0);
		Color		ResultColor = Color(0, 0, 0, 0);	//start�� end�� life/time�� lerp
		float		LifeTime	= 0.0f;	// ���� �����ð�
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

