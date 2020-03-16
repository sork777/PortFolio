#pragma once
#define MAX_MESH_INSTANCE 500

class MeshRender
{
public:
	MeshRender(Shader* shader, Mesh* mesh);
	~MeshRender();

	void Update();
	void Render();

	inline void Tech(const UINT& val) { mesh->Tech(val); }
	inline void Pass(const UINT& val) { mesh->Pass(val); }

	inline Mesh* GetMesh() { return mesh; }
	inline const UINT& GetInstSize() { return transforms.size(); }
public:
	void AddInstance();
	void DelInstance(const UINT& instance);

public:
	Material* GetMaterial();
	void SetMaterial(Material* material);

	MaterialPBR* GetMaterialPBR();
	void SetMaterialPBR(MaterialPBR* material);

	inline Transform* GetTransform(const UINT& index) { return transforms[index]; }
	void UpdateTransforms();

private:
	void AddTransform();

private:
	enum class MaterialType
	{
		None,
		Material,
		MaterialPBR
	}type = MaterialType::None;
private:
	Mesh* mesh;
	Material* material = NULL;
	MaterialPBR* materialpbr = NULL;

	vector<Transform *> transforms;
	Matrix worlds[MAX_MESH_INSTANCE];

	VertexBuffer* instanceBuffer;
};