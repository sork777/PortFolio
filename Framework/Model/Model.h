#pragma once
#define MAX_BONE_TRANSFORMS 250
#define MAX_ANIM_KEYFRAMES 500
#define MAX_MODEL_INSTANCE 500
#define MAX_ANIMATION_CLIPS 15

class ModelBone;
class ModelMesh;
/*
	TODO: 0904 Animator랑 Render의 텍스쳐맵에서 INSTANCE관련 부분 배제하기
	위에 define한 4종을 동적으로 메모리 재설정 시키기
	메모리 차지하는게 커서 여러 모델 못함...
*/
class Model
{
public:
	typedef VertexTextureNormalTangentBlend ModelVertex;

	Model(Shader* shader);
	Model(const Model& model);
	~Model();


	void ModelMeshChanger(const Model& model);

	void SetShader(Shader* shader);
	void Initialize();
	void Update();
	void Render();
	
	void Pass(const UINT& pass);
	void Tech(const UINT& tech);

	void UpdateTransforms();
	
	void AddInstance();
	void DelInstance(const UINT& instance);

	const UINT& GetInstSize()						{ return transforms.size(); }
	Transform* GetTransform(const UINT& instance=0)	{ return transforms[instance]; }
protected:
	void AddTransform();

public:
	Shader* GetShader()						{ return shader; }
	ID3D11ShaderResourceView* GetBoneSrv()	{ return boneSrv; }
	const bool& IsAnimationModel()			{ return bAnimated; }

public:
	const wstring& Name()			{ return name; }
	const wstring& MaterialPath()	{ return materialFilePath; }
	const wstring& MaterialDir()		{ return materialDirPath; }
	const wstring& MeshPath()		{ return meshFilePath; }
	const wstring& MeshDir()			{ return meshDirPath; }

public:
	const UINT& MaterialCount()					{ return materials.size(); }
	vector<Material *>& Materials()				{ return materials; }
	Material* MaterialByIndex(const UINT& index)	{ return materials[index]; }
	Material* MaterialByName(const wstring& name);

	const UINT& BoneCount()								{ return bones.size(); }
	inline vector<ModelBone *>& Bones()					{ return bones; }
	inline ModelBone* BoneByIndex(const UINT& index)	{ return bones[index]; }
	ModelBone* BoneByName(const wstring& name);
	const int& BoneIndexByName(const wstring& name);

	const UINT& MeshCount()						{ return meshes.size(); }
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

public:
	void SetAnimClip(const UINT& animClipCount = 0) { 
		this->animClipCount = animClipCount; 
		if (animClipCount < 1) this->animClipCount = 1;
		editTexture = NULL;
	}

	const UINT& GetTech() { return tech; }
	const UINT& GetPass() { return pass; }
private:
	Shader* shader;
	UINT tech = 0;
	UINT pass = 0;
	bool	bAnimated = false;
	UINT	prevBoneCount = 0;
	UINT	animClipCount = 1;

	//인스턴싱 정보
	Matrix				worlds[MAX_MODEL_INSTANCE];
	VertexBuffer*		instanceBuffer = NULL;
	vector<Transform *> transforms;
	
	// 모델이 가지고 있을 본
	Matrix									boneTrans[MAX_BONE_TRANSFORMS];
	ID3D11Texture1D*						bonebuffer = NULL;
	ID3D11ShaderResourceView*				boneSrv = NULL;
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
	ID3D11Texture2D*						editTexture = NULL;
	ID3D11ShaderResourceView*				editSrv	= NULL;
	ID3DX11EffectShaderResourceVariable*	sAnimEditSRV;

	Matrix animEditTrans[MAX_ANIMATION_CLIPS][MAX_BONE_TRANSFORMS];

#pragma endregion
public:
	const int& BoneHierarchy(int* click);
	void ChildBones(ModelBone * bone,int* click);
	//다른 자식comp에서도 같은 본이 자동선택되는걸 방지하기 위함
	void SetSelectedBoneNum(const int& bone = -1) { selectedBoneNum = bone; }
private:
	int selectedBoneNum = -1;

public:
	const Vector3& GetMin() { return minV; }
	const Vector3& GetMax() { return maxV; }

private:
	void CalcMeshVolume();
	
public:
	const bool& IsAdded() { return bAdded; }
private:
	bool bCalcVolume = false;
	bool bAdded = false;
	Vector3 minV, maxV;
};