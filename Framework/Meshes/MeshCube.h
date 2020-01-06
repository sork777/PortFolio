#pragma once
#include "Mesh.h"

class MeshCube : public Mesh
{
public:
	MeshCube();
	~MeshCube();

protected:
	void Create() override;
};