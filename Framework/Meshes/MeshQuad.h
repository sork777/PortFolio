#pragma once
#include "Mesh.h"

class MeshQuad : public Mesh
{
public:
	MeshQuad();
	MeshQuad(const MeshQuad& mesh);
	~MeshQuad();

private:
	void Create() override;
};