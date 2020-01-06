#pragma once


class Camera
{
public:
	Camera();
	virtual ~Camera();

	virtual void Update() = 0;

public:
	void Position(float x, float y, float z);
	void Position(Vector3& vec);		//Set?
	void Position(Vector3* vec);		//Get?

	void Rotation(float x, float y, float z);
	void Rotation(Vector3& vec);
	void Rotation(Vector3* vec);

	void RotationDegree(float x, float y, float z);
	void RotationDegree(Vector3& vec);
	void RotationDegree(Vector3* vec);


	void GetMatrix(Matrix* matrix);		//view��� ����

	//const Vector3 const& Forward(const int const* a) const { return forward; }		//�ǵ� �� �Լ������� ������� �Ұ���
	Vector3& Forward() { return forward; }
	Vector3& Right() { return right; }
	Vector3& Up() { return up; }

protected:
	virtual void Move();
	virtual void Rotation();
protected:
	virtual void View();
	void SetMatrix(Matrix& matrix);

private:
	Vector3 position;
	Vector3 rotation;

	Vector3 forward;
	Vector3 up;
	Vector3 right;

	Matrix matRotation;
protected:
	Matrix matView;
};
