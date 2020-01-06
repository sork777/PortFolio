#pragma once

class Projection
{
public:
	Projection(float width, float height, float zn, float zf, float fov);
	virtual ~Projection();

	void GetMatrix(Matrix* matrix);
	float GetNearPlane() { return zn; }
	float GetFarPlane() { return zf; }
	float GetFOV() { return fov; }
protected:
	virtual void Set(float width, float height, float zn, float zf, float fov);

	Projection();

protected:
	float width, height;
	float zn, zf;
	float fov;

	Matrix matrix;
};