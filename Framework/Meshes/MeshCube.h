#pragma once
#include "Mesh.h"

class MeshCube : public Mesh
{
public:
	MeshCube();
	MeshCube(const MeshCube& mesh);
	~MeshCube();

protected:
	void Create() override;
};