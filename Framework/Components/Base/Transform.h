#pragma once

class Transform :public BaseComponent
{
public:
	Transform();
	Transform(Shader* shader);
	~Transform();
	
	virtual void Update() override;
	virtual void Render() override;
	virtual bool Property() override;
	//virtual void Redo() override{};
	//virtual void Undo() override{};

	virtual void SetShader(Shader* shader) override;

public:
	void Set(Transform* transform);

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

	void Parent(Transform* parent);
	Transform* ParentTransform() { return parent; }

	void Parent(Matrix& matrix);
	Matrix& Parent() { return pMatrix; }

private:
	void UpdateWorld();

private:
	struct BufferDesc
	{
		Matrix World;
		Matrix InvWorld;
	} bufferDesc;

private:
	ConstantBuffer* buffer;
	ID3DX11EffectConstantBuffer* sBuffer;

	Matrix local;
	Matrix pMatrix;
	Vector3 position;
	Vector3 scale;
	Vector3 rotation;

	Transform* parent;
private:
	Matrix preLocal;
	Matrix prePMatrix;
};