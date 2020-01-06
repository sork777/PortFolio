#include "Framework.h"
#include "Transform.h"

Transform::Transform()
	: shader(NULL), buffer(NULL)
	, position(0, 0, 0), scale(1, 1, 1), rotation(0, 0, 0)
	//, parent(NULL)
{
	D3DXMatrixIdentity(&bufferDesc.World);
	D3DXMatrixIdentity(&local);
	D3DXMatrixIdentity(&pMatrix);
}

Transform::Transform(Shader * shader)
	: position(0, 0, 0), scale(1, 1, 1), rotation(0, 0, 0)//, parent(NULL)
{
	SetShader(shader);

	D3DXMatrixIdentity(&bufferDesc.World);
	D3DXMatrixIdentity(&local);
	D3DXMatrixIdentity(&pMatrix);

}

Transform::~Transform()
{
	SafeDelete(buffer);
}

void Transform::Set(Transform * transform)
{
	position = transform->position;
	scale = transform->scale;
	rotation = transform->rotation;

	UpdateWorld();
}

void Transform::SetShader(Shader* shader)
{
	this->shader = shader;

	buffer = new ConstantBuffer(&bufferDesc, sizeof(BufferDesc));
	sBuffer = shader->AsConstantBuffer("CB_World");
}

void Transform::Position(float x, float y, float z)
{
	Position(Vector3(x, y, z));
}

void Transform::Position(Vector3 & vec)
{
	position = vec;

	UpdateWorld();
}

void Transform::Position(Vector3 * vec)
{
	*vec = position;
}

void Transform::Scale(float x, float y, float z)
{
	Scale(Vector3(x, y, z));
}

void Transform::Scale(Vector3 & vec)
{
	scale = vec;

	UpdateWorld();
}

void Transform::Scale(Vector3 * vec)
{
	*vec = scale;
}

void Transform::Rotation(float x, float y, float z)
{
	Rotation(Vector3(x, y, z));
}

void Transform::Rotation(Vector3 & vec)
{
	rotation = vec;

	UpdateWorld();
}

void Transform::Rotation(Vector3 * vec)
{
	*vec = rotation;
}

void Transform::RotationDegree(float x, float y, float z)
{
	RotationDegree(Vector3(x, y, z));
}

void Transform::RotationDegree(Vector3 & vec)
{
	Vector3 temp;

	temp.x = Math::ToRadian(vec.x);
	temp.y = Math::ToRadian(vec.y);
	temp.z = Math::ToRadian(vec.z);

	Rotation(temp);
}

void Transform::RotationDegree(Vector3 * vec)
{
	Vector3 temp;

	temp.x = Math::ToDegree(rotation.x);
	temp.y = Math::ToDegree(rotation.y);
	temp.z = Math::ToDegree(rotation.z);

	*vec = temp;
}

Vector3 Transform::Forward()
{
	Vector3 nF;
	D3DXVec3Normalize(&nF, &Vector3(bufferDesc.World._31, bufferDesc.World._32, bufferDesc.World._33));
	return nF;
}

Vector3 Transform::Up()
{
	Vector3 nU;
	D3DXVec3Normalize(&nU, &Vector3(bufferDesc.World._21, bufferDesc.World._22, bufferDesc.World._23));

	return nU;
}

Vector3 Transform::Right()
{
	Vector3 nR;
	D3DXVec3Normalize(&nR, &Vector3(bufferDesc.World._11, bufferDesc.World._12, bufferDesc.World._13));

	return nR;
}

void Transform::World(Matrix & matrix)
{
	Matrix InvParent;
	/*D3DXMatrixIdentity(&InvParent);

	if (parent)
		InvParent = parent->World();
	*/
	D3DXMatrixInverse(&InvParent, NULL, &pMatrix);
	local = matrix*InvParent;
	bufferDesc.World = matrix;
	Math::MatrixDecompose(local, scale, rotation, position);
}

void Transform::Local(Matrix & matrix)
{
	local = matrix;

	bufferDesc.World = local* pMatrix;
	/*if (parent)
		bufferDesc.World *= parent->World();*/

	Math::MatrixDecompose(matrix, scale, rotation, position);
}

void Transform::Parent(Matrix &  matrix)
{
	pMatrix = matrix;
	bufferDesc.World = local * pMatrix;
}

void Transform::UpdateWorld()
{
	D3DXMATRIX S, R, T;
	D3DXMatrixScaling(&S, scale.x, scale.y, scale.z);
	D3DXMatrixRotationYawPitchRoll(&R, rotation.y, rotation.x, rotation.z);
	D3DXMatrixTranslation(&T, position.x, position.y, position.z);

	local = S * R * T;

	bufferDesc.World = local* pMatrix;
	/*if(parent)
		bufferDesc.World *= parent->World();*/
}

void Transform::Update()
{
	UpdateWorld();
}

void Transform::Render()
{
	if (shader == NULL) return;

	buffer->Apply();
	sBuffer->SetConstantBuffer(buffer->Buffer());
}
