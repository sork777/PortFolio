#pragma once

class Transform
{
public:
	Transform();
	Transform(Shader* shader);
	~Transform();

	void Set(Transform* transform);

	void SetShader(Shader* shader);

	void Position(float x, float y, float z);
	void Position(Vector3& vec);
	void Position(Vector3* vec);

	void Scale(float x, float y, float z);
	void Scale(Vector3& vec);
	void Scale(Vector3* vec);

	void Rotation(float x, float y, float z);
	void Rotation(Vector3& vec);
	void Rotation(Vector3* vec);

	void RotationDegree(float x, float y, float z);
	void RotationDegree(Vector3& vec);
	void RotationDegree(Vector3* vec);

	Vector3 Forward();
	Vector3 Up();
	Vector3 Right();

	void World(Matrix& matrix);
	Matrix& World() { return bufferDesc.World; }

	void Local(Matrix& matrix);
	Matrix& Local() { return local; }


	void Parent(Matrix& matrix);
	Matrix& Parent() { return pMatrix; }

	//void Parent(Transform* parent);
	//Transform* Parent() {return parent; }

private:
	void UpdateWorld();

public:
	void Update();
	void Render();
	void Property();

private:
	struct BufferDesc
	{
		Matrix World;
	} bufferDesc;

private:
	Shader * shader;

	ConstantBuffer* buffer;
	ID3DX11EffectConstantBuffer* sBuffer;

	Matrix local;
	Matrix pMatrix;
	//Transform* parent;
	Vector3 position;
	Vector3 scale;
	Vector3 rotation;
};