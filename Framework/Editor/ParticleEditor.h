#pragma once
#include "IEditor.h"
#include "ParticleSystem/ParticleEmitter.h"

class ParticleEditor :public IEditor
{
public:
	ParticleEditor();
	~ParticleEditor();

	// IEditor을(를) 통해 상속됨
	virtual void Initialize() override;
	virtual void Destroy() override;

	virtual void Update() override;
	virtual void PreRender() override {};
	virtual void Render() override;
	virtual void PostRender() override {};

private:
	Shader*		shader;

	class Sky*	sky;
	Material*	floor;
	MeshRender* grid;

	ParticleEmitter* emitter;
//	ParticleEmitter* particle_emit;
};

