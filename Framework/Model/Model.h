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

	void SetShader(Shader* shader);

	void Update();
	void Render(const int& drawCount = -1);
	
	void Pass(const UINT& pass);
	void Tech(const UINT& tech);

	void UpdateTransforms();
	
	void AddInstance();
	void DelInstance(const UINT& instance);

	inline const UINT& GetInstSize()						{ return transforms.size(); }
	inline Transform* GetTransform(const UINT& instance)	{ return transforms[instance]; }
protected:
	void AddTransform();

public:
	inline Shader* GetShader()						{ return shader; }
	inline ID3D11ShaderResourceView* GetBoneSrv()	{ return boneSrv; }
	inline const bool& IsAnimationModel()			{ return bAnimated; }

public:
	inline const wstring& Name()			{ return name; }
	inline const wstring& MaterialPath()	{ return materialFilePath; }
	inline const wstring& MaterialDir()		{ return materialDirPath; }
	inline const wstring& MeshPath()		{ return meshFilePath; }
	inline const wstring& MeshDir()			{ return meshDirPath; }

public:
	inline const UINT& MaterialCount()					{ return materials.size(); }
	inline vector<Material *>& Materials()				{ return materials; }
	inline Material* MaterialByIndex(const UINT& index)	{ return materials[index]; }
	Material* MaterialByName(const wstring& name);

	inline const UINT& BoneCount()						{ return bones.size(); }
	inline vector<ModelBone *>& Bones()					{ return bones; }
	inline ModelBone* BoneByIndex(const UINT& index)	{ return bones[index]; }
	ModelBone* BoneByName(const wstring& name);
	const int& BoneIndexByName(const wstring& name);

	inline const UINT& MeshCount()						{ return meshes.size(); }
	inline vector<ModelMesh *>& Meshes()				{ return meshes; }
	inline ModelMesh* MeshByIndex(const UINT& index)	{ return meshes[index]; }
	ModelMesh* MeshByName(const wstring& name);

public:
	void ReadMaterial(const wstring& file, const wstring& directoryPath = L"../../_Textures/Model/");
	void ReadMesh(const wstring& file, const wstring& directoryPath = L"../../_Models/");

private:
	void BindBone();
	void BindMesh();

public:
	void AddSocket(const int& parentBoneIndex, const wstring& socketName = L"");

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


#pragma region 애니메이션의 변화 조정 영역
public:
	void UpdateBoneTransform(const UINT& part, const UINT& clipID, Transform* transform);
	void UpdateChildBones(const UINT& parentID, const UINT& childID, const UINT& clipID);
	void AddSocketEditData(const UINT& boneID, const UINT& clipCount);

	inline ID3D11ShaderResourceView* GetEditSrv() { return editSrv; }
private:
	void CreateAnimEditTexture();

private:
	// 애니메이션의 글로벌 변화
	// 최종적으로 클립 저장시 곱해서 변화된 애를 저장할 것.
	ID3D11Texture2D* editTexture = NULL;
	ID3D11ShaderResourceView* editSrv;
	ID3DX11EffectShaderResourceVariable* sAnimEditSRV;

	Matrix animEditTrans[MAX_ANIMATION_CLIPS][MAX_MODEL_TRANSFORMS];

#pragma endregion
public:
	const int& BoneHierarchy(int* click);
	void ChildBones(ModelBone * bone,int* click);

private:
	int selectedBoneNum = -1;

};