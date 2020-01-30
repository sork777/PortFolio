#pragma once

// �𵨿��� ���� Ŭ�����¸� �޾� ��������
// �ν��Ͻ����� ���� �����̴� ����.
#define MAX_ILLUSION_COUNT 100
class ModelIllusion
{
public:
	ModelIllusion(ModelAnimator* animator,UINT meshIdx=0,UINT illusionCount=50);
	~ModelIllusion();

	void Initialize();
	void Update(TweenDesc tween,float gap=0.0f);
	void Render();

	bool Property();
	Transform* GetTransform() { return transform; }

	void SetAnimEditSrv(ID3D11ShaderResourceView* srv) { animEditSrv = srv; }
private:
	void SetAndShiftBuffer(TweenDesc& insertFirstTween, float& gap);

private:
	UINT tech=0;
	UINT pass=0;
	Shader* shader;
	PerFrame* perframe;
	Transform* transform;
	Material* material;

	VertexBuffer* vertexBuffer = NULL;
	IndexBuffer* indexBuffer = NULL;

	UINT indexCount;
	TweenDesc tweenDesc[MAX_ILLUSION_COUNT];
	Color illusionColor = Color(1, 1, 1, 1);
	vector<TweenDesc> tweens;
	ConstantBuffer* frameBuffer;
	ID3DX11EffectConstantBuffer* sFrameBuffer;

	ID3D11ShaderResourceView* boneSrv;
	ID3D11ShaderResourceView* transformSrv;
	ID3D11ShaderResourceView* animEditSrv;
private:
	UINT illusionCount;
	bool bViewTrail;
		
	float deltaStoreTime;

};

