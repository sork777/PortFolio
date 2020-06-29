#pragma once



/*
	MeshRender에서 가져올거임.
	일단 받는건 Mesh를 받아서 생성

	하는 일
	1. 메시 출력
	2. 모델 속성 확인(마테리얼, 트랜스폼, 애니메이션 클립)
	3. 있으면 물리도.	>> Collision은 컴포넌트 줄거임.
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
	// 메시컴포넌트를 받아서 
	void MeshCopy(Mesh& oMesh);

public:
	// ObjectBaseComponent을(를) 통해 상속됨
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
	// 메시는 공유
	Mesh* mesh;
	MeshRender* meshRender;

};