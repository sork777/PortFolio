#include "Framework.h"
#include "Camera.h"



Camera::Camera()
	:position(0, 0, 0)
	, up(0, 1, 0), right(1, 0, 0), forward(0, 0, 1)
	, rotation(0, 0, 0)
{
	//항상 로테이션이 우선?
	D3DXMatrixIdentity(&matRotation);
	D3DXMatrixIdentity(&matView);
	
	Rotation();
	Move();
}


Camera::~Camera()
{
}

void Camera::Position(float x, float y, float z)
{
	Position(Vector3(x, y, z));
}

void Camera::Position(Vector3 & vec)
{
	position = vec;

	//좌표가 변함->이동 했음
	Move();
}

void Camera::Position(Vector3 * vec)
{
	*vec = position;
}

void Camera::Rotation(float x, float y, float z)
{
	Rotation(Vector3(x, y, z));
}
void Camera::Rotation(Vector3 & vec)
{
	rotation = vec;
	Rotation();
}
void Camera::Rotation(Vector3 * vec)
{
	*vec = rotation;
}

void Camera::RotationDegree(float x, float y, float z)
{
	RotationDegree(Vector3(x, y, z));
}

void Camera::RotationDegree(Vector3 & vec)
{
	//rotation = vec * Math::PI / 180;
	//1radian 값을 직접 때려줌, float이 소숫점 8자리니 거기까지 써줌
	rotation = vec * 0.01745328f;		
	Rotation();
}

void Camera::RotationDegree(Vector3 * vec)
{
	//*vec = rotation * 180 / Math::PI;
	*vec = rotation * 57.29577951f;
}

void Camera::GetMatrix(Matrix * matrix)
{
	//대입보다 빠름
	memcpy(matrix, &matView, sizeof(Matrix));
}

void Camera::Move()
{
	//값이 바뀌었으니 행렬 다시
	View();
}

void Camera::Rotation()
{
	Matrix X, Y, Z;
	D3DXMatrixRotationX(&X, rotation.x);	//X회전
	D3DXMatrixRotationY(&Y, rotation.y);	//Y회전
	D3DXMatrixRotationZ(&Z, rotation.z);	//Z회전

	matRotation = X * Y * Z;		//행렬곱

	//누적으로 가기때문에 2칸에 forward가 들어가지 않음
	//forward의 절대적인 값인 0,0,1을 사용.
	D3DXVec3TransformNormal(&forward, &Vector3(0, 0, 1), &matRotation);
	D3DXVec3TransformNormal(&right, &Vector3(1, 0, 0), &matRotation);
	D3DXVec3TransformNormal(&up, &Vector3(0, 1, 0), &matRotation);
	
}

void Camera::View()
{
	D3DXMatrixLookAtLH(&matView, &position, &(position + forward), &up);
}

void Camera::SetMatrix(Matrix & matrix)
{
	memcpy(matView, &matrix, sizeof(Matrix));
}
