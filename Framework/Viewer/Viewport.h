#pragma once

class Viewport
{
public:
	Viewport(float width, float height, float x = 0, float y = 0, float minDepth = 0, float maxDepth = 1);
	~Viewport();

	void RSSetViewport();
	void Set(float width, float height, float x = 0, float y = 0, float minDepth = 0, float maxDepth = 1);

	float GetWidth() { return width; }
	float GetHeight() { return height; }

	//OUT 리턴값인지 아닌지 명시해주려고 그외 기능 없음
	void GetRay(OUT Vector3* position, OUT Vector3 *direction, Matrix& W, Matrix& V, Matrix& P);
	
	void Project(OUT Vector3* position, Vector3& source, Matrix& W, Matrix& V, Matrix& P);

	//리턴 받을 값, 변환 할 값, WVP
	void Unproject(OUT Vector3* position, Vector3& source, Matrix& W, Matrix& V, Matrix& P);
private:
	float x, y;
	float width, height;
	float minDepth, maxDepth;

	D3D11_VIEWPORT viewport;
};