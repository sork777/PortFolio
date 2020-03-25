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
	inline Material* GetMaterial() { return material; }
	void SetMaterial(Material* material);

	inline Transform* GetTransform(const UINT& index) { return transforms[index]; }
	void UpdateTransforms();

private:
	void AddTransform();

private:
	Mesh* mesh;
	Material* material = NULL;

	vector<Transform *> transforms;
	Matrix worlds[MAX_MESH_INSTANCE];

	VertexBuffer* instanceBuffer;
};