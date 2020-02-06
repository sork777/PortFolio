#pragma once
#define MAX_MESH_INSTANCE 500

class MeshRender
{
public:
	MeshRender(Shader* shader, Mesh* mesh);
	~MeshRender();

	Mesh* GetMesh() { return mesh; }

	void Update();
	void Render();

	void Tech(UINT val) { mesh->Tech(val); }
	void Pass(UINT val) { mesh->Pass(val); }

	Material* GetMaterial();
	void SetMaterial(Material* material);

	MaterialPBR* GetMaterialPBR();
	void SetMaterialPBR(MaterialPBR* material);

	Transform* AddTransform();
	Transform* GetTransform(UINT index) { return transforms[index]; }
	void UpdateTransforms();

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