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
	FigureMeshComponent(MeshRender* mesh);
	~FigureMeshComponent();
		
	// ObjectBaseComponent��(��) ���� ��ӵ�
	virtual void Update() override;
	virtual void Render() override;
	virtual bool Property(const UINT& instance = 0) override;

	virtual void Tech(const UINT& mesh, const UINT& model, const UINT& anim) override;
	virtual void Pass(const UINT& mesh, const UINT& model, const UINT& anim) override;
	
	virtual void SetShader(Shader* shader) override;

	virtual void AddInstanceData() override;
	virtual void DelInstanceData(const UINT& instance) override;
	virtual const UINT& GetInstSize() override;
	
	virtual Transform* GetTransform(const UINT& instance = 0) override;
public:
	inline MeshRender* GetMeshRender() { return meshRender; }

private:
	MeshRender* meshRender;
};

