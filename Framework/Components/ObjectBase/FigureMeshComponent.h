#pragma once



/*
	MeshRender���� �����ð���.
	�ϴ� �޴°� Mesh�� �޾Ƽ� ����

	�ϴ� ��
	1. �޽� ���
	2. �� �Ӽ� Ȯ��(���׸���, Ʈ������, �ִϸ��̼� Ŭ��)
	3. ������ ������.	>> Collision�� ������Ʈ �ٰ���.
*/
class Mesh;
class MeshRender;

class FigureMeshComponent :public ObjectBaseComponent
{
public:
	FigureMeshComponent(Mesh* mesh);
	FigureMeshComponent(const FigureMeshComponent& meshComp);
	~FigureMeshComponent();
		
	void CompileComponent(const FigureMeshComponent& OBComp);

private:
	// �޽�������Ʈ�� �޾Ƽ� 
	void MeshCopy(Mesh& oMesh);

public:
	// ObjectBaseComponent��(��) ���� ��ӵ�
	virtual void Update() override;
	virtual void Render() override;
	bool Property(const UINT& instance = 0) ;

	void Tech(const UINT& mesh, const UINT& model, const UINT& anim) ;
	void Pass(const UINT& mesh, const UINT& model, const UINT& anim) ;
	
	void SetShader(Shader* shader) ;
	MeshRender* GetMeshRender() { return meshRender; }

public:
	void AddInstanceData() ;
	void DelInstanceData(const UINT& instance) ;

	virtual const UINT& GetInstSize() override;	
	virtual Transform* GetTransform(const UINT& instance = 0) override;

private:
	// �޽ô� ����
	Mesh* mesh;
	MeshRender* meshRender;

};