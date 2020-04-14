#pragma once

class ModelRender
{
public:
	ModelRender(Model* model);
	~ModelRender();

	void Update();
	void Render(const int& drawCount =-1);

	void Pass(UINT pass) { model->Pass(pass); }
	void Tech(UINT tech) { model->Tech(tech); }
	
	void SetShader(Shader* shader);

public:
	void UpdateTransform(UINT instanceId, UINT boneIndex, Transform& transform);
	Model* GetModel() { return model; }

private:
	Matrix GetboneWorld(UINT instance, UINT boneIndex) ;
	void CreateTexture() ;

private:
	Model* model;
	Shader* shader;

	ID3D11Texture2D* texture = NULL;
	ID3D11ShaderResourceView* srv;
	ID3DX11EffectShaderResourceVariable* sTransformsSRV;

private:
	Matrix bones[MAX_MODEL_TRANSFORMS];
	Matrix boneTransforms[MAX_MODEL_INSTANCE][MAX_MODEL_TRANSFORMS];
};