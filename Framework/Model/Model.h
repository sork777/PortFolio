#pragma once
#define MAX_MODEL_TRANSFORMS 250
#define MAX_MODEL_KEYFRAMES 500
#define MAX_MODEL_INSTANCE 500
#define MAX_ANIMATION_CLIPS 25

class ModelBone;
class ModelMesh;

//TODO: 어태치 관련 전부 액터로 빼기

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

	void Update();
	virtual void Render();
	
	void Pass(UINT pass);
	void Tech(UINT tech);

	void UpdateTransforms();
	
	virtual void AddInstance();
	virtual void DelInstance(UINT instance);


	UINT GetInstSize() { return transforms.size(); }
	Transform* GetTransform(UINT instance) { return transforms[instance]; }
	virtual Matrix GetboneTransform(UINT instance, UINT boneIndex) = 0;

protected:
	void AddTransform();
	virtual void CreateTexture() = 0;

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

public:
	void ReadMaterial(wstring file, wstring directoryPath = L"../../_Textures/");
	void ReadMesh(wstring file, wstring directoryPath = L"../../_Models/");

private:
	void BindBone();
	void BindMesh();

public:
	void Attach(Model* model, int parentBoneIndex, UINT instanceIndex, Transform* transform = NULL);

	void AddSocket(int parentBoneIndex, wstring bonename = L"");
private:
	vector<Material *> materials;
	
	ModelBone* rootBone;
	vector<ModelBone *> bones;
	vector<ModelMesh *> meshes;
		
	vector< AttachModelData * > attaches;

protected:
	Shader* shader;

	vector<Transform *> transforms;
	Matrix worlds[MAX_MODEL_INSTANCE];

	VertexBuffer* instanceBuffer;

	ID3D11Texture2D* texture = NULL;
	ID3D11ShaderResourceView* srv;

};