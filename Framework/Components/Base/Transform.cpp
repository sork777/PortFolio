#include "Framework.h"
#include "Transform.h"

Transform::Transform()
	: buffer(NULL)
	, position(0, 0, 0), scale(1, 1, 1), rotation(0, 0, 0)
	, parent(NULL)
{
	type = BaseCompType::Transform;
	shader = NULL;
	
	D3DXMatrixIdentity(&bufferDesc.World);
	D3DXMatrixIdentity(&local);
	D3DXMatrixIdentity(&pMatrix);
	preLocal = local;
	prePMatrix = pMatrix;
}

Transform::Transform(Shader * shader)
	: position(0, 0, 0), scale(1, 1, 1), rotation(0, 0, 0), parent(NULL)
{
	type = BaseCompType::Transform;
	SetShader(shader);

	D3DXMatrixIdentity(&bufferDesc.World);
	D3DXMatrixIdentity(&local);
	D3DXMatrixIdentity(&pMatrix);
	preLocal = local;
	prePMatrix = pMatrix;
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
	if (parent != NULL)
		pMatrix = parent->World();

	preLocal = local;
	Matrix InvParent;
	D3DXMatrixInverse(&InvParent, NULL, &pMatrix);

	local = matrix*InvParent;
	Math::MatrixDecompose(local, scale, rotation, position);

	bufferDesc.World = local * pMatrix;
	D3DXMatrixInverse(&bufferDesc.InvWorld, NULL, &bufferDesc.World);
}

void Transform::Local(Matrix & matrix)
{
	if (parent != NULL)
		pMatrix = parent->World();
	preLocal = local;

	local = matrix;
	Math::MatrixDecompose(matrix, scale, rotation, position);

	bufferDesc.World = local* pMatrix;
	D3DXMatrixInverse(&bufferDesc.InvWorld, NULL, &bufferDesc.World);
}

void Transform::Parent(Transform * parent)
{
	this->parent = parent;	
	prePMatrix = pMatrix;
	pMatrix = parent->World();

	bufferDesc.World = local * pMatrix;
	D3DXMatrixInverse(&bufferDesc.InvWorld, NULL, &bufferDesc.World);
}

void Transform::Parent(Matrix &  matrix)
{
	if (parent == NULL)
		parent = new Transform();

	prePMatrix = pMatrix;
	pMatrix = matrix;
	parent->World(matrix);
	
	bufferDesc.World = local * pMatrix;
	D3DXMatrixInverse(&bufferDesc.InvWorld, NULL, &bufferDesc.World);
}

void Transform::UpdateWorld()
{
	D3DXMATRIX S, R, T;
	D3DXMatrixScaling(&S, scale.x, scale.y, scale.z);
	D3DXMatrixRotationYawPitchRoll(&R, rotation.y, rotation.x, rotation.z);
	D3DXMatrixTranslation(&T, position.x, position.y, position.z);

	local = S * R * T;

	if (parent != NULL)
	{
		pMatrix = parent->World();
	}

	bufferDesc.World = local* pMatrix;
	D3DXMatrixInverse(&bufferDesc.InvWorld, NULL, &bufferDesc.World);
}

void Transform::Update()
{
	preLocal = local;
	prePMatrix = pMatrix;
	UpdateWorld();
}

void Transform::Render()
{
	if (shader == NULL) return;

	buffer->Apply();
	sBuffer->SetConstantBuffer(buffer->Buffer());
}

bool Transform::Property()
{
	bool bChange = false;
	
	bChange |= (preLocal != local);
	bChange |= (prePMatrix != pMatrix);
	{
		ImGui::PushID(this);
		ImGui::Text("Transform");
		bChange |= ImGui::SliderFloat3("Scale", (float*)&scale, 0.01f, 100.0f);
		scale.x = Math::Clamp(scale.x, 0.01f, FLT_MAX);
		scale.y = Math::Clamp(scale.y, 0.01f, FLT_MAX);
		scale.z = Math::Clamp(scale.z, 0.01f, FLT_MAX);
		Vector3 degree;
		degree.x = Math::ToDegree(rotation.x);
		degree.y = Math::ToDegree(rotation.y);
		degree.z = Math::ToDegree(rotation.z);
		bChange |= ImGui::SliderFloat3("Rotation", (float*)&degree, -170, 170);
		rotation.x = Math::ToRadian(degree.x);
		rotation.y = Math::ToRadian(degree.y);
		rotation.z = Math::ToRadian(degree.z);
		bChange |= ImGui::SliderFloat3("Position", (float*)&position, -1000.0f, 1000.0f);
		ImGui::PopID();
	}
	Update();
	
	return bChange;
}
