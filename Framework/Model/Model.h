#pragma once
#define MAX_MODEL_TRANSFORMS 250
#define MAX_MODEL_KEYFRAMES 500
#define MAX_MODEL_INSTANCE 500
#define MAX_ANIMATION_CLIPS 15

class ModelBone;
class ModelMesh;

class Model
{
public:
	typedef VertexTextureNormalTangentBlend ModelVertex;

	Model(Shader* shader);
	Model(Model* model);
	~Model();

	void Update();
	void Render();
	
	void Pass(UINT pass);
	void Tech(UINT tech);

	void UpdateTransforms();
	
	void AddInstance();
	void DelInstance(UINT instance);

	UINT GetInstSize()						{ return transforms.size(); }
	Transform* GetTransform(UINT instance)	{ return transforms[instance]; }
protected:
	void AddTransform();

public:
	Shader* GetShader()						{ return shader; }
	ID3D11ShaderResourceView* GetBoneSrv()	{ return boneSrv; }
	bool& IsAnimationModel()				{ return bAnimated; }

public:
	wstring& Name()			{ return name; }
	wstring& MaterialPath() { return materialFilePath; }
	wstring& MaterialDir()	{ return materialDirPath; }
	wstring& MeshPath()		{ return meshFilePath; }
	wstring& MeshDir()		{ return meshDirPath; }

public:
	UINT MaterialCount()					{ return materials.size(); }
	vector<Material *>& Materials()			{ return materials; }
	Material* MaterialByIndex(UINT index)	{ return materials[index]; }
	Material* MaterialByName(wstring name);

	UINT BoneCount()						{ return bones.size(); }
	vector<ModelBone *>& Bones()			{ return bones; }
	ModelBone* BoneByIndex(UINT index)		{ return bones[index]; }
	ModelBone* BoneByName(wstring name);
	int BoneIndexByName(wstring name);

	UINT MeshCount()						{ return meshes.size(); }
	vector<ModelMesh *>& Meshes()			{ return meshes; }
	ModelMesh* MeshByIndex(UINT index)		{ return meshes[index]; }
	ModelMesh* MeshByName(wstring name);

public:
	void ReadMaterial(wstring file, wstring directoryPath = L"../../_Textures/Model/");
	void ReadMesh(wstring file, wstring directoryPath = L"../../_Models/");

private:
	void BindBone();
	void BindMesh();

public:
	void AddSocket(int parentBoneIndex, wstring socketName = L"");

private:
	void CreateBoneBuffer();

private:
	wstring					name;
	wstring					materialFilePath, materialDirPath;
	wstring					meshFilePath, meshDirPath;

	ModelBone*				rootBone;
	vector<Material *>		materials;
	vector<ModelBone *>		bones;
	vector<ModelMesh *>		meshes;

protected:
	Shader* shader;
	bool	bAnimated = false;

	//인스턴싱 정보
	Matrix				worlds[MAX_MODEL_INSTANCE];
	VertexBuffer*		instanceBuffer;
	vector<Transform *> transforms;
	
	// 모델이 가지고 있을 본
	Matrix									boneTrans[MAX_MODEL_TRANSFORMS];
	ID3D11Texture1D*						bonebuffer = NULL;
	ID3D11ShaderResourceView*				boneSrv;
	ID3DX11EffectShaderResourceVariable*	sBoneTransformsSRV;

};