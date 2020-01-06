#pragma once
#define MAX_MODEL_TRANSFORMS 250
#define MAX_MODEL_KEYFRAMES 500
#define MAX_MODEL_INSTANCE 500
#define MAX_ANIMATION_CLIPS 25

class ModelBone;
class ModelMesh;
class ModelClip;

enum ModelType
{
	Model_None,
	Model_Render,
	Model_Animator
};

struct AttachModelData
{
	ModelType type = ModelType::Model_None;
	UINT AttachedBoneIndex = -1;
	vector<UINT> AttachInstances;
	class Model* model = NULL;
};


class Model
{
public:
	typedef VertexTextureNormalTangentBlend ModelVertex;

	Model(Shader* shader);
	~Model();

#pragma region Render와Animator에 있던 공용함수 

	void Update();
	virtual void Render();
	
	void Pass(UINT pass);
	void Tech(UINT tech);

	void UpdateTransforms();
	//void UpdateMaterial();

	UINT GetInstSize() { return transforms.size(); }
	Transform* AddTransform();
	Transform* GetTransform(UINT instance) { return transforms[instance]; }
	
	virtual Matrix GetboneTransform(UINT instance, UINT boneIndex) = 0;
protected:
	virtual void CreateTexture() = 0;

protected:
	Shader* shader;

	vector<Transform *> transforms;
	Matrix worlds[MAX_MODEL_INSTANCE];

	VertexBuffer* instanceBuffer;

	ID3D11Texture2D* texture = NULL;
	ID3D11ShaderResourceView* srv;

#pragma endregion

public:
	UINT MaterialCount() { return materials.size(); }
	vector<Material *>& Materials() { return materials; }
	Material* MaterialByIndex(UINT index) { return materials[index]; }
	Material* MaterialByName(wstring name);

	UINT BoneCount() { return bones.size(); }
	vector<ModelBone *>& Bones() { return bones; }
	ModelBone* BoneByIndex(UINT index) { return bones[index]; }
	ModelBone* BoneByName(wstring name);

	UINT MeshCount() { return meshes.size(); }
	vector<ModelMesh *>& Meshes() { return meshes; }
	ModelMesh* MeshByIndex(UINT index) { return meshes[index]; }
	ModelMesh* MeshByName(wstring name);

	UINT ClipCount() { return clips.size(); }
	vector<ModelClip *>& Clips() { return clips; }
	ModelClip* ClipByIndex(UINT index) { return clips[index]; }
	ModelClip* ClipByName(wstring name);

public:
	void ReadMaterial(wstring file, wstring directoryPath = L"../../_Textures/");
	void ReadMesh(wstring file, wstring directoryPath = L"../../_Models/");
	void ReadClip(wstring file, wstring directoryPath = L"../../_Models/");

private:
	void BindBone();
	void BindMesh();

public:
	//void Attach(Model* model, int parentBoneIndex, Transform* transform = NULL);
	void Attach(Model* model, int parentBoneIndex, UINT instanceIndex, Transform* transform = NULL);

	void AddSocket(int parentBoneIndex, wstring bonename = L"");
private:
	vector<Material *> materials;
	
	ModelBone* rootBone;
	vector<ModelBone *> bones;
	vector<ModelMesh *> meshes;
	vector<ModelClip *> clips;
	
	vector< AttachModelData * > attaches;
};