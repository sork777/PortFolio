#pragma once

class Orbit : public Camera
{
public:
	Orbit(const float& rad,const float& minRad, const float& maxRad);
	virtual ~Orbit();
	

	//궤도를 돌 포지션
	void SetObjPos(const Vector3& objPos = Vector3(0,0,0)) { this->objPos = objPos; }

	void Update() override;
	void Speed(float rot);
	void SetRad(const float& rad, const float& minRad, const float& maxRad);

	void ViewCameraArea();
	void Property();

	void CameraMove(Vector3& val);
private:
	void OrbitUp();
	void View() override;

private:
	float rSpeed;
	float phi;
	float theta;
	
	float minRad;
	float maxRad;
	float rad;

	Vector3 objPos;
	Vector3 oUp;
};

