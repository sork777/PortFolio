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
	FigureMeshComponent(MeshRender* mesh);
	~FigureMeshComponent();
		
	// ObjectBaseComponent을(를) 통해 상속됨
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

