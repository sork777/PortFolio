#pragma once

class ModelRender
{
public:
	ModelRender(Model* model);
	~ModelRender();

	void Update();
	void Render();

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
	ID3D11ShaderResourceView* srv = NULL;
	ID3DX11EffectShaderResourceVariable* sTransformsSRV;

private:
	Matrix bones[MAX_BONE_TRANSFORMS];
	Matrix boneTransforms[MAX_MODEL_INSTANCE][MAX_BONE_TRANSFORMS];
};