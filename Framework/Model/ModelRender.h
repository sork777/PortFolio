#pragma once

class ModelRender : public Model
{
public:
	ModelRender(Shader* shader);
	~ModelRender();
	
public:
	void UpdateTransform(UINT instanceId, UINT boneIndex, Transform& transform);

private:
	virtual Matrix GetboneTransform(UINT instance, UINT boneIndex) override;
	virtual void CreateTexture() override;

private:
	Matrix bones[MAX_MODEL_TRANSFORMS];
	Matrix boneTransforms[MAX_MODEL_INSTANCE][MAX_MODEL_TRANSFORMS];
};