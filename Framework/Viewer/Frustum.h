#pragma once
class Frustum
{
public:
	Frustum(Camera* camera=NULL,Perspective* perspective=NULL);
	~Frustum();
	
	void Update();
	void Planes(Plane* plane);

	bool ContainPoint(Vector3& position);
	bool ContainRect(float xCenter, float yCenter, float zCenter, float xSize, float ySize, float zSize);
	bool ContainRect(Vector3 center, Vector3 size);
	bool ContainCube(Vector3& center, float radius);
private:
	Plane planes[6];

	Camera* camera;
	Perspective* perspective;
};

